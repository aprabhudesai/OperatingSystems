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


	WriteToConsole("\nTrying to call Wait with valid cvId : and invalid lockId : %d different than what was returned by server\n\0",cvId, lockId+10,-1);
	waitStatus = Wait(cvId, lockId);
	if(waitStatus == -1)
	{
		WriteToConsole("\nInvalid Lock Id. Failed to Wait on given condition. Exiting Program\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nAcquired Lock\n\0", -1, -1, -1);
	}

   Exit(0);
}
