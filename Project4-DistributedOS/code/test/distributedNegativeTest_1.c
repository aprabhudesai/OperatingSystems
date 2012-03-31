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


	WriteToConsole("\nTrying to acquire lock which does not exist.\n\0",-1,-1,-1);

	lockStatus = Acquire("lock2",5);
	if(lockStatus == -1)
	{
		WriteToConsole("\nLock does not exist. Failed to Acquire Lock. Exiting Program\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nAcquired Lock\n\0", -1, -1, -1);
	}

   Exit(0);
}
