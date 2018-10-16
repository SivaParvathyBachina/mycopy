#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include <sys/time.h>
#include <signal.h>
#include <semaphore.h>
#include <fcntl.h>
#include "shared_mem.h"
#define SHMSIZE 100
#define SEMNAME "sembach19181"
#define NANOSECOND 1000000000

pid_t childpid = 0;
int i,m,k,x,k,s = 5,j,status,p;
pid_t *child_pids;
key_t myshmKey, shmMsgKey;
int shmId, shmMsgId, programRunTime = 2, position;
shared_mem *clock; 
shmMsg *userClock;
FILE *logfile;
char *file_name;
sem_t *mySemaphore;


void clearSharedMemory() {
fprintf(stderr, "------------------------------- CLEAN UP ----------------------- \n");
shmdt((void *)clock);
shmdt((void *)userClock);
fprintf(stderr,"Cloising File \n");
fclose(logfile);
fprintf(stderr, "OSS started detaching OSS Clock Memory \n");
fprintf(stderr, "OSS started detaching shmMsg Memory \n");
shmctl(shmId, IPC_RMID, NULL);
shmctl(shmMsgId, IPC_RMID, NULL);
sem_unlink(SEMNAME);
fprintf(stderr, "Unlinked Semaphore \n");
fprintf(stderr, "OSS Cleared the Shared Memory \n");
}

void killExistingChildren(){
for(k=0; k<s; k++)
{
if(child_pids[k] != 0)
{
fprintf(stderr, "Killing child with Id %d \n", child_pids[k]);
kill(child_pids[k], SIGTERM);
}
}
}

int randomNumberGenerator(int min, int max)
{
	return ((rand() % (max-min +1)) + min);
}

void myhandler(int s) {
if(s == SIGALRM)
{
fprintf(stderr, "Master Time Done\n");
killExistingChildren();
clearSharedMemory();
}

if(s == SIGINT)
{
fprintf(stderr, "Caught Ctrl + C Signal \n");
fprintf(stderr, "Killing the Existing Children in the system \n");
killExistingChildren();
clearSharedMemory();
}
exit(1);
}

int findElement(int childpid_val)
{
	for(m = 0; m < s; m++)
	{
		if(child_pids[m] == childpid_val)
		return m;
	}
}


int main (int argc, char *argv[]) {

if (argc < 2){ // check for valid number of command line arguments
fprintf(stderr, "Invalid number of arguments. Please give it in the following format");
fprintf(stderr, "Usage: %s  -n processess -h [help] -p [error message]", argv[0]);
return 1;
}
while((x = getopt(argc,argv, "hs:l:t:")) != -1)
switch(x)
{
case 'h':
        fprintf(stderr, "Usage: %s -n processCount -l logfile_name -t z [program_LifeTime] -h [help]\n", argv[0]);
        return 1;
case 's':
	s = atoi(optarg);
	break;
case 'l':
	file_name = optarg;
	break;
case 't': 
	programRunTime = atoi(optarg); 
	break;
case '?':
        fprintf(stderr, "Please give '-h' for help to see valid arguments \n");
        return 1;
}

signal(SIGALRM, myhandler);
alarm(programRunTime);
signal(SIGINT, myhandler);

myshmKey = ftok(".", 'c');
shmId = shmget(myshmKey, sizeof(shared_mem), IPC_CREAT | 0666);

if(shmId <0 )
{
	fprintf(stderr, "Error in shmget \n");
	exit(1);
}

clock = (shared_mem*) shmat(shmId, NULL, 0);

if(clock == (void *) -1)
{
	perror("Error in attaching shared memory --- Master \n");
	exit(1);
}

fprintf(stderr, "Allocated Shared Memory For OSS Clock \n");

shmMsgKey = ftok(".", 'x');
shmMsgId = shmget(shmMsgKey, sizeof(shmMsg), IPC_CREAT | 0666);

if(shmMsgId < 0)
{
	fprintf(stderr, "Error in Shmget for ShmMsg \n");
	exit(1);
}

userClock = (shmMsg*) shmat(shmMsgId, NULL, 0);
if(userClock == (void *) -1)
{
	perror("Error in attaching USer CLock SHMMSG \n");
	exit(1);
}

fprintf(stderr, "Allocated Shared Memory For shmMsg \n");

mySemaphore = sem_open(SEMNAME, O_CREAT, 0666,1 );

fprintf(stderr, "Created Semaphore with Name %s \n", SEMNAME);

clock -> seconds = 0;
clock -> nanoseconds = 0;
userClock -> seconds = 0;
userClock -> nanoseconds = 0;
userClock -> childpid = -1;
userClock -> inUse = 0;

fprintf(stderr, "Initialised the OSS and shmMsg Values \n");

if(file_name == NULL)
file_name = "default";
logfile = fopen(file_name, "w");

fprintf(stderr, "Opened Log File for writing Output::: %s \n", file_name);

if(logfile == NULL){
	perror("Error in opening file \n");
	exit(-1);
}

child_pids = (pid_t *)malloc(s * sizeof(int));
	for(i=0; i<s ;i++)
	{
		child_pids[i] = fork();
		if(child_pids[i] == 0)
		{
		char argument3[50], argument4[4], argument5[10];
		char *s_val = "-s";
		 char *shmMsgVal2 = "-j";
		char *semVal = "-k";
		char *arguments[] = {NULL,s_val, argument3,shmMsgVal2, argument4, semVal, argument5, NULL};
		arguments[0]="./user";
		sprintf(arguments[2], "%d", shmId);
		sprintf(arguments[4], "%d", shmMsgId);
		sprintf(arguments[6], "%s", SEMNAME);
		execv("./user", arguments);
		fprintf(stderr, "Error in exec");
		}	
	}
j = s;

int value = randomNumberGenerator(10, 20);
while(1)
{
	if(clock -> nanoseconds >= NANOSECOND) 
	{
	clock -> seconds += value;
	clock -> nanoseconds = 0;
	}
	clock -> nanoseconds += 1;	

	if( userClock -> childpid > 0)
        {
                fprintf(logfile, "OSS: Child PID %d is terminating at my time %ld.%ld, because it reached %ld.%ld in user \n", userClock -> childpid, clock -> seconds, clock -> nanoseconds, userClock -> seconds, userClock -> nanoseconds);
               
		waitpid(userClock -> childpid, &status, 0);

		// Created an array of size s and reutilized it for newly created processes.
		// While a child is being waited for, finding the position of that child in array and replacing that with the new one.
		position = findElement(userClock -> childpid);
		userClock -> childpid = -1;
        	
	 child_pids[position] = fork();
         if(child_pids[position]  == 0)
         {
	 char ival[10], nval[50],sval[50], shmMsgVal[50];
         char *s_val2 = "-s";
         char *shmMsgValNew = "-j";
         char *kval = "-k";
        char *arguments2[] = {NULL,s_val2, sval,shmMsgValNew,shmMsgVal,kval, nval, NULL};
         arguments2[0]="./user";
         sprintf(arguments2[2], "%d", shmId);
         sprintf(arguments2[4], "%d", shmMsgId);
         sprintf(arguments2[6], "%s", SEMNAME);
         execv("./user", arguments2);
         fprintf(stderr, "Error in exec");
        }
        }

}

while((waitpid(-1, &status, 0) > 0 )){};
clearSharedMemory();
return 0;
}


