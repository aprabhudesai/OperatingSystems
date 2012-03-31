#include "syscall.h"

int main()
{
	int lockStatus, cvStatus, mvStatus;

	WriteToConsole("\nCreating a Lock\n\0", -1, -1, -1);

	lockStatus = CreateLock("lock1",5);
	if(lockStatus == -1)
	{
		WriteToConsole("\nError Occurred while creating Lock. Exiting program\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nLock Created or found created already by the server.\n\0", -1, -1, -1);
	}


	mvStatus = CreateMV("mv1", 3, 1);
	if(mvStatus == -1)
	{
		WriteToConsole("\nError Occurred while creating Monitor Variable. Exiting program\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nMonitor Variable Created or found created already by the server.\n\0", -1, -1, -1);
	}

	cvStatus = CreateCV("cv1", 3);
	if(cvStatus == -1)
	{
		WriteToConsole("\nError Occurred while creating Condition Variable. Exiting program\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nCondition Variable Created or found created already by the server.\n\0", -1, -1, -1);
	}

	lockStatus = Acquire("lock1",5);
	if(lockStatus == -1)
	{
		WriteToConsole("\nError occurred while Acquiring Lock. Exiting program\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nLock acquired by the server.\n\0", -1, -1, -1);
	}

	WriteToConsole("\nSignaling on condition to specialTest2_1\n", -1, -1, -1);


	WriteToConsole("\n#################################\n\0", -1, -1, -1);
	WriteToConsole("\n\nFound Current MV Value to be: %d. Sending signal to client 1.\n\0", GetMV("mv1", 3, 0), -1, -1);
	WriteToConsole("\n#################################\n\0", -1, -1, -1);

	Signal("cv1", 3, "lock1", 5);

	lockStatus = Release("lock1",5);
	if(lockStatus == -1)
	{
		WriteToConsole("\nError occurred while Releasing Lock. Exiting program\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nLock released by the server.\n\0", -1, -1, -1);
	}
   Exit(0);
}
