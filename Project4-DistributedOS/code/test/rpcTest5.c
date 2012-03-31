#include "syscall.h"

int
main()
{
	int lock3,mv3,cv3;

	lock3 = CreateLock("lock3",5);
	cv3 = CreateCV("cv3",3);
	mv3 = CreateMV("mv3",3,5);

	WriteToConsole("\nAcquiring lock3 in rpcclient5\n\0", -1, -1, -1);
	if((Acquire("lock3",5)) == -1){
		WriteToConsole("Error acquiring lock3 in rpcclient5\n\0",-1,-1,-1);
		Exit(0);
	}

	if((Wait("cv3",3, "lock3",5)) == -1){
			WriteToConsole("Error waiting on cv3 and lock3 in rpcClient1\n\0",-1,-1,-1);
			Exit(0);
	}

	WriteToConsole("\nGot Signal on cv3 lock3 from rpcClient1\n\0", -1, -1, -1);
	WriteToConsole("\nValue of mv3 index 3 set by rpcClient1 is %d\n\0", GetMV("mv3",3,3), -1, -1);
	Release("lock3",5);

	DeleteLock("lock3",5);
	WriteToConsole(" Deleted Lock3\n\0",-1,-1,-1);
	DeleteMV("mv3",3);
	WriteToConsole(" Deleted MV3\n\0",-1,-1,-1);
	DeleteCV("cv3",3);
	WriteToConsole(" Deleted CV3\n\0",-1,-1,-1);

	Exit(0);
}
