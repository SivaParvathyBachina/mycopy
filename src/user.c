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

int randomNumberGenerator(int min, int max)
{
        return ((rand() % (max-min +1)) + min);
}

int main (int argc, char *argv[]) {

int i;
//for ( i = 0; i < argc; i++) 
//        fprintf(stderr, "%d \n", argv[i] ); 

childpid = getpid();
//fprintf(stderr, "Child Created \n Child started execution in  worker::: %ld \n", childpid);
//fprintf(stderr, "*******************************************\n");

while((x = getopt(argc,argv, "n:s:j:")) != -1)
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

 fprintf(stderr, "User Running \n");
//fprintf(stderr, "%d \n", sem_init(mySemaphore, 0,1));
mySemaphore = sem_open ("pSem", O_CREAT | O_EXCL, 0666, 1); 
fprintf(stderr, " Semaphore Initialization Success! \n");
sem_unlink ("pSem");

srand(time(NULL));
int randomnumber = randomNumberGenerator(1,1000000);
childSeconds = clock -> seconds * NANOSECOND;
childNanoseconds = clock -> nanoseconds;

totalChildTime = childSeconds + childNanoseconds; 
childMaxRunTime = totalChildTime + randomnumber;

fprintf(stderr, "---------Child Max Run Time %d-------------- \n", childMaxRunTime);

if(sem_wait(mySemaphore) == -1)
{
	perror("Error in Wait function of Semaphore \n");
	return 1;
}

//while(1)
//{
	fprintf(stderr, "Child Entered Critical Section %d  \n", childpid);
	if(((clock -> seconds * NANOSECOND) + clock -> nanoseconds) >= childMaxRunTime)
	{
		fprintf(stderr, "#####################################################Inside while Loop \n");
		if(userClock -> inUse == 0)
		{
			userClock -> inUse = 1;
			userClock -> childpid = getpid();
			userClock -> seconds = clock -> seconds;
			userClock -> nanoseconds = clock -> nanoseconds;
		}
	//	else 
	//	break;						
	}
//}


if(sem_post(mySemaphore) == -1)
{
	perror("Error in Signal Of Semaphore \n");
	return 1;
}

//sem_destroy(mySemaphore);
//fprintf(stderr, "Worker Clock Update ");
fprintf(stderr, "Master Clock Seconds %d, Nano Seconds %d \n", clock -> seconds, clock -> nanoseconds);
fprintf(stderr, "User  Seconds %d, User Nano seconds %d \n" ,userClock -> seconds, userClock -> nanoseconds);
fprintf(stderr, "Child Done Execution from worker with %ld \n", childpid);
fprintf(stderr, "------------------------------------------------ \n");
return 0;
}
