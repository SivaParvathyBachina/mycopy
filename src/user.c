#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <errno.h>
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include <signal.h>
#include "shared_mem.h"
#include <fcntl.h>
#define MILLION 1000000
#define NANOSECOND 1000000000

shared_mem *clock;
shmMsg *userClock;
int shmId,shmMsgId, x, processNumber,n;
sem_t *mySemaphore;
long childSeconds, childNanoseconds, totalChildTime, childMaxRunTime;
void signalHandler(int);
pid_t childpid;
int childCount = 0;
char *semName;


int randomNumberGenerator(int min, int max)
{
        return ((rand() % (max-min +1)) + min);
}

int main (int argc, char *argv[]) {

//int i;
//for ( i = 0; i < argc; i++) 
//        fprintf(stderr, "%d \n", argv[i] ); 

childpid = getpid();
//fprintf(stderr, "Child Created \n Child started execution in  worker::: %ld \n", childpid);
//fprintf(stderr, "*******************************************\n");

//fprintf(stderr, "Child with pid ######## %d \n", childpid);

while((x = getopt(argc,argv, "n:s:j:k:")) != -1)
switch(x)
{
case 'n': 
	n = atoi(optarg);
	break;

case 's':
        shmId = atoi(optarg);
        break;

case 'j': 
	shmMsgId = atoi(optarg);
	break;
case 'k':
	semName = optarg;
	break;
case '?':
        fprintf(stderr, "Invalid Arguments \n");
        return 1;
}


clock = (shared_mem*) shmat(shmId, NULL, 0);

if(clock == (void *) -1)
{
        perror("Error in attaching shared memory \n");
        exit(1);
}

userClock = (shmMsg*) shmat(shmMsgId, NULL, 0);

if(userClock == (void *) -1)
{
	perror("Error in attaching shared Memory for userClock \n");
	exit(1);
}

mySemaphore = sem_open (semName , 0); 
//sem_unlink(semName);

srand(time(NULL));
int randomnumber = randomNumberGenerator(1,1000000);

//childSeconds = clock -> seconds * NANOSECOND;
//childNanoseconds = clock -> nanoseconds;

totalChildTime = (clock -> seconds * NANOSECOND) + clock -> nanoseconds; 
childMaxRunTime = totalChildTime + randomnumber;

fprintf(stderr, "---------Child %d  Max Run Time %d-------------- \n", getpid(),childMaxRunTime);

while(1)
{
	sem_wait(mySemaphore);
	//sem_getvalue(mySemaphore,&sem_value);
	//fprintf(stderr, "After Sem Wait %d \n", sem_value);
	fprintf(stderr, "Passed Wait %d \n", getpid());
	 if(((clock -> seconds * NANOSECOND) + clock -> nanoseconds) >= childMaxRunTime)
	{
		fprintf(stderr, "########################################################### %d Inside Critical Section \n", getpid());
		if(userClock -> childpid == -1)
		{
			userClock -> childpid = getpid();
			userClock -> seconds = childMaxRunTime/ NANOSECOND;
			userClock -> nanoseconds = childMaxRunTime % NANOSECOND;
		        fprintf(stderr, "******************************************************  %d Exiting Critical Section \n", getpid());
			sem_post(mySemaphore);
			break;
		}
	} 
	// fprintf(stderr,"%d Inside Critical Section \n", getpid());
	sem_post(mySemaphore);
	//sem_getvalue(mySemaphore,&sem_value);
        //fprintf(stderr, "After Sem Post %d \n", sem_value);
	//fprintf(stderr, "Error in Unlocking Semaphore \n");
}


return 0;
}
