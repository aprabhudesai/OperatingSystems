#include "syscall.h"

int
main()
{
	int lockId, lockStatus,signalStatus,cv;
	WriteToConsole("\nUser Program 2\n",-1,-1,-1);
	WriteToConsole("\nCreating Lock - lock 1\n",-1,-1,-1);
	
	lockId = CreateLock("lock1",5);
	
	WriteToConsole("Acquiring lock1\n\0",-1,-1,-1);
	lockStatus = Acquire("lock1",5);
	if(lockStatus == -1)
	{
		WriteToConsole("\nFailed to Acquire Lock. Exiting Program\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nAcquired lock1\n\0", -1, -1, -1);
	}
	WriteToConsole("\nCreating Condition Variable cv1\n",-1,-1,-1);
	cv = CreateCV("cv1",3);
	WriteToConsole("Signaling User Program 1\n\0",-1,-1,-1);
	signalStatus = Signal("cv1", 3,"lock1",5);
	if(signalStatus == -1)
	{
		WriteToConsole("\nFailed to Signal on the given Condition\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nSignalling on cv1 with lock1\n\0", -1, -1, -1);
	}
		
	WriteToConsole("Release lock 1\n\0",-1,-1,-1);
	lockStatus = Release("lock1",5);
	if(lockStatus == -1)
	{
		WriteToConsole("\nFailed to Release Lock. Exiting Program\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nReleased Lock\n\0", -1, -1, -1);
	}
	WriteToConsole("Acquiring Lock1\n\0",-1,-1,-1);
	lockStatus = Acquire("lock1",5);
	if(lockStatus == -1)
	{
		WriteToConsole("\nFailed to Acquire Lock. Exiting Program\n\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nAcquired Lock\n\0", -1, -1, -1);
	}
	
	
	WriteToConsole("\nUser Program 2 Exiting without releasing lock 1\n",-1,-1,-1);
	Exit(0);
}
