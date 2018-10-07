#ifndef _SHARED_MEM_H_
#define _SHARED_MEM_H_
typedef struct
{
	long seconds;
	long nanoseconds;
}shared_mem;

typedef struct
{
	long seconds;
	long nanoseconds;
	pid_t childpid;
	int inUse;
}shmMsg;
#endif
