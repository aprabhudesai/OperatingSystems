#include "syscall.h"

int
main()
{
	int lockId, lockStatus;

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

	WriteToConsole("\nTrying to release lock with id: %d different than what was returned by server\n\0",lockId,-1,-1);
	lockStatus = Release(lockId);
	if(lockStatus == -1)
	{
		WriteToConsole("\nInvalid Lock Id. Failed to Release Lock. Exiting Program\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nRelease Lock\n\0", -1, -1, -1);
	}
   Exit(0);
}
