#include "syscall.h"

int
main()
{
	int lockId, lockStatus, cvId, cvStatus, mvId, mvStatus, mvValue, signalStatus;

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


	WriteToConsole("\nCreating a MV\n\0", -1, -1, -1);

	mvId = CreateMV("mv1",3, 5);
	if(mvId == -1)
	{
		WriteToConsole("\nError Occurred while creating MV. Exiting program\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nMV Created, MV Id returned by the server is : %d\n\0", mvId, -1, -1);
	}


	WriteToConsole("\nAcquiring Lock\n",-1,-1,-1);
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

	WriteToConsole("\nGetting MV value at index 1\n", -1, -1, -1);
	mvValue = GetMV(mvId, 1);
	if(mvValue == -1)
	{
		WriteToConsole("\nFailed to Get value from MV. Exiting Program\0n", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nGot value from MV at index 1: %d.\n\0", mvValue, -1, -1);
	}

	if(mvValue == 100)
	{
		signalStatus = Signal(cvId, lockId);
		if(signalStatus == -1)
		{
			WriteToConsole("\nFailed to Signal on the given Condition\0", -1, -1, -1);
			Exit(0);
		}
		else
		{
			WriteToConsole("\nSignalling on CV1 with Lock1\n\0", -1, -1, -1);
		}
	}

	WriteToConsole("\nSetting MV index 2 to 222\n", -1, -1, -1);
	mvStatus = SetMV(mvId, 2, 222);
	if(mvStatus == -1)
	{
		WriteToConsole("\nFailed to Set MV. Exiting Program\0n", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nMV set successfully.\n\0", -1, -1, -1);
	}


	WriteToConsole("\nReleasing Lock\n\0", -1, -1, -1);
	Release(lockId);
	if(lockStatus == -1)
	{
		WriteToConsole("\nFailed to Release Lock. Exiting Program\0n", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nReleased Lock\n\0", -1, -1, -1);
	}
   Exit(0);
}
