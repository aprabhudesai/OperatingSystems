#include "syscall.h"

int
main()
{
	int lockStatus, cvId,cvStatus, mvStatus, mvId, mvValue, lockId1, lockId2;

	WriteToConsole("\nCreating a Lock\n\0", -1, -1, -1);

	lockStatus = CreateLock("lock1", 5);
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


	WriteToConsole("\nCreating a MV\n\0", -1, -1, -1);

	mvStatus = CreateMV("mv1",3, 3);
	if(mvStatus == -1)
	{
		WriteToConsole("\nError Occurred while creating MV. Exiting program\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nMV Created by the server.\n\0", -1, -1, -1);
	}


	WriteToConsole("\nAcquiring Lock.\n\0", -1, -1, -1);
	lockStatus = Acquire("lock1",5);
	if(mvStatus == -1)
	{
		WriteToConsole("\nError Occurred while Acquiring Lock. Exiting program\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nLock acquired.\n\0", -1, -1, -1);
	}


	WriteToConsole("\nSetting MV to 100\n\0", -1, -1, -1);
	mvStatus = SetMV("mv1",3, 5, 100);
	if(mvStatus == -1)
	{
		WriteToConsole("\nFailed to Set MV. Exiting Program\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nMV set successfully.\n\0", -1, -1, -1);
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

}
