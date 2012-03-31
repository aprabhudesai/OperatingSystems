#include "syscall.h"

int
main()
{
	int lockStatus, cvId, waitStatus;

	WriteToConsole("\nCreating a Lock\n\0", -1, -1, -1);

	lockStatus = CreateLock("lock1",5);
	if(lockStatus == -1)
	{
		WriteToConsole("\nError Occurred while creating Lock. Exiting program\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nLock Created by the server.\n\0", -1, -1, -1);
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
		WriteToConsole("\nCV Created by the server.", -1, -1, -1);
	}


	WriteToConsole("\nAcquiring Lock.\n\0", -1, -1, -1);
	lockStatus = Acquire("lock1",5);
	if(lockStatus == -1)
	{
		WriteToConsole("\nError Occurred while Acquiring Lock. Exiting program\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nLock acquired.\n\0", -1, -1, -1);
	}


	WriteToConsole("\nCalling Wait on lock1 and cv1.\n\0",-1, -1,-1);
	waitStatus = Wait("cv1",3, "lock1",5);
	if(waitStatus == -1)
	{
		WriteToConsole("\nError, Not the Lock owner. Exiting Program.\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nServer waiting on the given condition. Now start client 2\n\0", -1, -1, -1);
	}

	WriteToConsole("\nReleasing Lock.\n\0", -1, -1, -1);
	lockStatus = Release("lock1",5);
	if(lockStatus == -1)
	{
		WriteToConsole("\nError Occurred while Releasing Lock. Exiting program\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nLock released.\n\0", -1, -1, -1);
	}

   Exit(0);
}
