#include "syscall.h"

int main()
{
	int lockStatus, cvStatus, waitStatus;

	WriteToConsole("\nCreating a Lock\n\0", -1, -1, -1);

	lockStatus = CreateLock("lock1",5);
	if(lockStatus == -1)
	{
		WriteToConsole("\nError Occurred while creating Lock. Exiting program\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nLock Created by the server\n\0", -1 , -1, -1);
	}


	cvStatus = CreateCV("cv1",3);
	if(cvStatus == -1)
	{
		WriteToConsole("\nError Occurred while creating CV. Exiting program\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nCV Created by the server\n\0", -1 , -1, -1);
	}


	WriteToConsole("\nAcquiring Lock\n",-1,-1,-1);
	lockStatus = Acquire("lock1",5);
	if(lockStatus == -1)
	{
		WriteToConsole("\nFailed to Acquire Lock. Exiting Program\0n", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nAcquired Lock\n\0", -1, -1, -1);
	}


	WriteToConsole("\nWaiting on the condition cv1 with lock1. Start client 2 to signal.\n",-1,-1,-1);
	waitStatus = Wait("cv1",3,"lock1",5);
	if(lockStatus == -1)
	{
		WriteToConsole("\nServer failed to wait. Exiting program.\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nWaiting request processed by the server. \0n", -1, -1, -1);
	}

	WriteToConsole("\nGot signal from client 2.\n\0", -1, -1, -1);

	WriteToConsole("\nReleasing Lock\n\0", -1, -1, -1);
	lockStatus = Release("lock1",5);
	if(lockStatus == -1)
	{
		WriteToConsole("\nFailed to Release Lock. Exiting Program\0n", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nReleased Lock\n\0", -1, -1, -1);
	}

	WriteToConsole("\nDeleting Lock\n\0", -1, -1, -1);
	lockStatus = DeleteLock("lock1",5);
	if(lockStatus == -1)
	{
		WriteToConsole("\nFailed to Delete Lock. Exiting Program\0n", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nDeleted Lock\n\0", -1, -1, -1);
	}

	Exit(0);
}
