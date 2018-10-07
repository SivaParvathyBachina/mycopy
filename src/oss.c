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
#include "shared_mem.h"
#define SHMSIZE 100
#define NANOSECOND 1000000000

pid_t childpid = 0;
int i,k, n,x,k,s,j,status,p;
pid_t *child_pids;
key_t myshmKey, shmMsgKey;
int shmId, shmMsgId;
shared_mem *clock; 
shmMsg *userClock;
FILE *logfile;
char *file_name;


void clearSharedMemory() {
fprintf(stderr, "Master Clock Value Seconds %d, Nanoseconds %d \n", clock -> seconds, clock -> nanoseconds);
shmdt((void *)clock);
shmdt((void *)userClock);
fprintf(stderr, "Master started detaching its shared memory \n");
shmctl(shmId, IPC_RMID, NULL);
shmctl(shmMsgId, IPC_RMID, NULL);
fprintf(stderr, "Master removed shared memory \n");
}

int randomNumberGenerator(int min, int max)
{
	return ((rand() % (max-min +1)) + min);
}


int main (int argc, char *argv[]) {

if (argc < 2){ // check for valid number of command line arguments
fprintf(stderr, "Invalid number of arguments. Please give it in the following format");
fprintf(stderr, "Usage: %s  -n processess -h [help] -p [error message]", argv[0]);
return 1;
}
while((x = getopt(argc,argv, "hn:s:")) != -1)
switch(x)
{
case 'h':
        fprintf(stderr, "Usage: %s -n processCount -h [help] -s childrenCount\n", argv[0]);
        return 1;
case 'n':
        n = atoi(optarg);
        break;
case 's':
	s = atoi(optarg);
	break;
/*case 'l':
	file_name = optarg;
case 't': 
	maxRunTime = atoi(optarg); */
case '?':
        fprintf(stderr, "Please give '-h' for help to see valid arguments \n");
        return 1;
}

if(s>n)
{
fprintf(stderr,  "Illegal COmmand Line Arguments..s cannot be greater  than n. \n", s ,n );
return 1;
}
if(n> 20)
{
fprintf(stderr, "There can only be a maximum of 20 processesin the system\n");
fprintf(stderr, "Setting back n to 20 \n");
n = 20;
}

//fprintf(stderr, "Allocating Shared Memory Starting \n");

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

clock -> seconds = 0;
clock -> nanoseconds = 0;
userClock -> seconds = 10;
userClock -> nanoseconds = 15;
userClock -> childpid = -1;
userClock -> inUse = 0;

//fprintf(stderr, "Allocated Shared Memory Done \n");

int childCount = n;
child_pids = (pid_t *)malloc(n * sizeof(int));
	for(i=0; i<s ;i++)
	{
		child_pids[i] = fork();
		if(child_pids[i] == 0)
		{
		char argument1[50], argument2[50], argument3[50], argument4[4];
		char *s_val = "-s";
		char *k_val = "-n";
		 char *shmMsgVal2 = "-j";
		char *arguments[] = {NULL,k_val,argument2 ,s_val, argument3,shmMsgVal2, argument4, NULL};
		arguments[0]="./user";
		sprintf(arguments[2], "%d", n);
		sprintf(arguments[4], "%d", shmId);
		sprintf(arguments[6], "%d", shmMsgId);
		execv("./user", arguments);
		fprintf(stderr, "Error in exec");
		}	
	j++;
	}
	
	/*waitpid(-1, &status, 0);
	while(j < childCount) {
	waitpid(-1, &status, 0);
	 child_pids[j] = fork();
         if(child_pids[j] == 0)
         {
	char ival[10], nval[50],sval[50], shmMsgVal[50];
         char *s_val2 = "-s";
         char *k_val2 = "-n";
	 char *shmMsgValNew = "-j";
	char *arguments2[] = {NULL,k_val2,nval ,s_val2, sval,shmMsgValNew,shmMsgVal, NULL};
	 arguments2[0]="./user";
         sprintf(arguments2[2], "%d", n);
         sprintf(arguments2[4], "%d", shmId);
	 sprintf(arguments2[6], "%d", shmMsgId);
         execv("./user", arguments2);
         fprintf(stderr, "Error in exec");
	}
       	j++;
	} */

int value = randomNumberGenerator(1000000, 2000000);
//fprintf(stderr, " Random Number %d \n", value);

while(/*clock -> seconds < 2*/ j < 3)
{
	if(clock -> nanoseconds >= NANOSECOND) 
	{
	clock -> seconds += 1;
	clock -> nanoseconds = 0;
	}
	clock -> nanoseconds += value;	
	fprintf(stderr, "Master Clock Value In While Loop Seconds %d, Nanoseconds %d \n", clock -> seconds, clock -> nanoseconds);
       // while(j < childCount) {
        waitpid(-1, &status, 0);
         child_pids[j] = fork();
         if(child_pids[j] == 0)
         {
        char ival[10], nval[50],sval[50], shmMsgVal[50];
         char *s_val2 = "-s";
         char *k_val2 = "-n";
         char *shmMsgValNew = "-j";
        char *arguments2[] = {NULL,k_val2,nval ,s_val2, sval,shmMsgValNew,shmMsgVal, NULL};
         arguments2[0]="./user";
         sprintf(arguments2[2], "%d", n);
         sprintf(arguments2[4], "%d", shmId);
         sprintf(arguments2[6], "%d", shmMsgId);
         execv("./user", arguments2);
         fprintf(stderr, "Error in exec");
        }
        j++;
}




while((waitpid(-1, &status, 0) > 0 )){};
clearSharedMemory();
return 0;
}


