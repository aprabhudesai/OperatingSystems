#include "syscall.h"

int
main()
{
	int lockStatus, cvStatus, mvStatus, mvId, mvValue;

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


	WriteToConsole("\nCreating a MV\n\0", -1, -1, -1);

	mvStatus = CreateMV("mv1",3, 5);
	if(mvStatus == -1)
	{
		WriteToConsole("\nError Occurred while creating MV. Exiting program\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nMV Created by the server.\n\0", -1, -1, -1);
	}


	WriteToConsole("\nAcquiring lock to access Monitor Variable\n\0", -1, -1, -1);

	lockStatus = Acquire("lock1",5);
	if(lockStatus == -1)
	{
		WriteToConsole("\nError Occurred while acquiring Lock. Exiting program\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nLock acquired by the server.\n\0", -1, -1, -1);
	}

	WriteToConsole("\nSetting MV to 100\n", -1, -1, -1);
	mvStatus = SetMV("mv1",3, 1, 100);
	if(mvStatus == -1)
	{
		WriteToConsole("\nFailed to Set MV. Exiting Program\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nMV set successfully.\n\0", -1, -1, -1);
	}


	WriteToConsole("\nTrying to fetch value at invalid mv Index : %d\n\0", 21, -1,-1);
	mvValue = GetMV("mv1",3, 20);
	if(mvValue == -1)
	{
		WriteToConsole("\nInvalid mvIndex, Failed to fetch value at given index. Exiting Program\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nAbnormal Execution\n\0", -1, -1, -1);
	}

   Exit(0);
}
