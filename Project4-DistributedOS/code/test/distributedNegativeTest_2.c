#include "syscall.h"

int main()
{
	int lockStatus;

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

	WriteToConsole("\nTrying to release lock before acquiring it.\n\0",-1,-1,-1);
	lockStatus = Release("lock1",5);
	if(lockStatus == -1)
	{
		WriteToConsole("\nError. Not the lock owner. Failed to Release Lock. Exiting Program\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nRelease Lock\n\0", -1, -1, -1);
	}
   Exit(0);
}
