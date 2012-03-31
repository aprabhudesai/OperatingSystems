#include "syscall.h"

int main()
{
	int lockStatus, cvStatus, mvStatus;

	WriteToConsole("\nSending Create Lock request\n\0", -1, -1, -1);

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

	WriteToConsole("\nSending Create Monitor Variable request\n\0", -1, -1, -1);
	mvStatus = CreateMV("mv1", 3, 5);
	if(mvStatus == -1)
	{
		WriteToConsole("\nError Occurred while creating Monitor Variable. Exiting program\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nMonitor Variable Created or found created already by the server.\n\0", -1, -1, -1);
	}

	WriteToConsole("\nSending Create Condition Variable request\n\0", -1, -1, -1);
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

   Exit(0);
}
