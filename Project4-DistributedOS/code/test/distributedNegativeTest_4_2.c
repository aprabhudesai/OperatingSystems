#include "syscall.h"

int
main()
{
	int lockStatus, cvId, signalStatus;

	WriteToConsole("\nCreating a Lock\n\0", -1, -1, -1);

	lockStatus = CreateLock("lock2",5);
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

	cvId = CreateCV("cv2",3);
	if(cvId == -1)
	{
		WriteToConsole("\nError Occurred while creating CV. Exiting program\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nCV Created by the server.", -1, -1, -1);
	}


	WriteToConsole("\nAcquiring Lock.\n\0", -1, -1, -1);
	lockStatus = Acquire("lock2",5);
	if(lockStatus == -1)
	{
		WriteToConsole("\nError Occurred while Acquiring Lock. Exiting program\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nLock acquired.\n\0", -1, -1, -1);
	}

	WriteToConsole("\nTrying to Signal on which no body is waiting.\n\0",-1, -1,-1);
	signalStatus = Signal("cv2",3, "lock2",5);
	if(signalStatus == -1)
	{
		WriteToConsole("\nError, failed to Signal. Exiting Program.\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nServer signaled on the given condition.\n\0", -1, -1, -1);
	}


	WriteToConsole("\nReleasing Lock.\n\0", -1, -1, -1);
	lockStatus = Release("lock2",5);
	if(lockStatus == -1)
	{
		WriteToConsole("\nError Occurred while Releasing Lock. Exiting program\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nLock released.\n\0", -1, -1, -1);
	}
   Exit(0);
}
