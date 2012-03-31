#include "syscall.h"


int main()
{
	int lockStatus, cvStatus, mvStatus, waitStatus, signalStatus;

	WriteToConsole("\nCreating a Lock\n\0", -1, -1, -1);

	lockStatus = CreateLock("lock1",5);
	if(lockStatus == -1)
	{
		WriteToConsole("\nError Occurred while creating Lock. Exiting program\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nLock Created by the server\n\0", -1 , -1, -1);
	}


	cvStatus = CreateCV("cv1",3);
	if(cvStatus == -1)
	{
		WriteToConsole("\nError Occurred while creating CV. Exiting program\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nCV Created by the server\n\0", -1 , -1, -1);
	}

	mvStatus = CreateMV("mv1", 3, 1);
	if(mvStatus == -1)
	{
		WriteToConsole("\nError Occurred while creating MV. Exiting program\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nMV Created by the server\n\0", -1 , -1, -1);
	}



	lockStatus = CreateLock("lock2",5);
	if(lockStatus == -1)
	{
		WriteToConsole("\nError Occurred while creating Lock. Exiting program\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nLock Created by the server\n\0", -1 , -1, -1);
	}


	cvStatus = CreateCV("cv2",3);
	if(cvStatus == -1)
	{
		WriteToConsole("\nError Occurred while creating CV. Exiting program\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nCV Created by the server\n\0", -1 , -1, -1);
	}

	WriteToConsole("\nAcquiring Lock\n",-1,-1,-1);
	lockStatus = Acquire("lock2",5);
	if(lockStatus == -1)
	{
		WriteToConsole("\nFailed to Acquire Lock. Exiting Program\0n", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nAcquired Lock\n\0", -1, -1, -1);
	}

	WriteToConsole("\nGetting value from MV1.\n",-1,-1,-1);
	if(GetMV("mv1", 3, 0) == 100)
	{
		WriteToConsole("\nWaiting on the condition cv2 with lock2.\n",-1,-1,-1);
		waitStatus = Wait("cv2",3,"lock2",5);
		if(waitStatus == -1)
		{
			WriteToConsole("\nServer failed to wait. Exiting program.\n\0", -1, -1, -1);
			Exit(0);
		}
		else
		{
			WriteToConsole("\nClient 2 is waiting. Start client 3 to signal.\0n", -1, -1, -1);
		}
	}
	else
	{
		WriteToConsole("\nServer failed to get value from MV. Exiting program.\n\0", -1, -1, -1);
		Exit(0);
	}

	WriteToConsole("\nAcquiring Lock\n",-1,-1,-1);
	lockStatus = Acquire("lock1",5);
	if(lockStatus == -1)
	{
		WriteToConsole("\nFailed to Acquire Lock. Exiting Program\0n", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nAcquired Lock\n\0", -1, -1, -1);
	}


	WriteToConsole("\nGot signal from Client 3.\n",-1,-1,-1);

	if(GetMV("mv1", 3, 0) == 300)
	{
		WriteToConsole("\nSetting MV to 200 and before signaling client 1.\n",-1,-1,-1);
		SetMV("mv1", 3, 0, 200);
	}
	else
	{
		WriteToConsole("\nServer failed to get value from MV. Exiting program.\n\0", -1, -1, -1);
		Exit(0);
	}

	WriteToConsole("\nSignaling on the condition cv1 with lock1.\n",-1,-1,-1);
	signalStatus = Signal("cv1",3,"lock1",5);
	if(signalStatus == -1)
	{
		WriteToConsole("\nServer failed to signal. Exiting program.\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nClient 2 has to signaled Client 1.\0n", -1, -1, -1);
	}

	WriteToConsole("\nReleasing Lock\n\0", -1, -1, -1);
	lockStatus = Release("lock1",5);
	lockStatus = Release("lock2",5);
	if(lockStatus == -1)
	{
		WriteToConsole("\nFailed to Release Lock. Exiting Program\0n", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nReleased Lock\n\0", -1, -1, -1);
	}

	WriteToConsole("\nDeleting Lock\n\0", -1, -1, -1);
	lockStatus = DeleteLock("lock1",5);
	lockStatus = DeleteLock("lock2",5);
	if(lockStatus == -1)
	{
		WriteToConsole("\nFailed to Delete Lock. Exiting Program\0n", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nDeleted Lock\n\0", -1, -1, -1);
	}

	Exit(0);
}
