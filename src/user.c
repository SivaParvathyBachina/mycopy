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
long int childSeconds, childNanoseconds, totalChildTime, childMaxRunTime;
void signalHandler(int);
pid_t childpid;
int childCount = 0;
char *semName;


int randomNumberGenerator(int min, int max)
{
        return (rand()%(max-min +1)) + min;
}

int main (int argc, char *argv[]) {

childpid = getpid();

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

totalChildTime = 0;
srand(time(NULL));
int randomnumber = randomNumberGenerator(1,1000000);

totalChildTime = (clock -> seconds * NANOSECOND) + (clock -> nanoseconds + randomnumber); 

int value;
while(1)
{
	 sem_wait(mySemaphore);
	 if(((clock -> seconds * NANOSECOND) + clock -> nanoseconds) >= totalChildTime)
	{
		if(userClock -> childpid == -1)
		{
			userClock -> seconds = clock -> seconds;
			userClock -> nanoseconds = clock -> nanoseconds;
			userClock -> childpid = getpid();
 			sem_post(mySemaphore);
			break;
		}
	} 
	sem_post(mySemaphore);
}

return 0;

}
