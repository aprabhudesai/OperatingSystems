#include "syscall.h"

int
main()
{
	int lockId, lockStatus, cvId, waitStatus;

	WriteToConsole("\nCreating a Lock\n\0", -1, -1, -1);

	lockId = CreateLock("lock1",5);
	if(lockId == -1)
	{
		WriteToConsole("\nError Occurred while creating Lock. Exiting program\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nLock Created, Lock Id returned by the server is : %d\n\0", lockId, -1, -1);
	}


	lockStatus = Acquire(lockId);
	if(lockStatus == -1)
	{
		WriteToConsole("\nFailed to Acquire Lock. Exiting Program\0n", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nAcquired Lock\n\0", -1, -1, -1);
	}

	WriteToConsole("\nCreating a CV\n\0", -1, -1, -1);

	cvId = CreateCV("cv1",3);
	if(cvId == -1)
	{
		WriteToConsole("\nError Occurred while creating CV. Exiting program\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nCV Created, CV Id returned by the server is : %d\n\0", cvId, -1, -1);
	}

	WriteToConsole("\nWaitning on condition : %d with lock : %d\n\0", cvId, lockId,-1);
	waitStatus = Wait(cvId, lockId);
	if(waitStatus == -1)
	{
		WriteToConsole("\nFailed to Wait on the given condition. Exiting Program\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nAbnormal Execution\n\0", -1, -1, -1);
	}

   Exit(0);
}
