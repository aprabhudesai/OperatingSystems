#include "syscall.h"

int
main()
{
	int lock1,mv1,cv1;
	
	lock1 = CreateLock("lock1",5);
	cv1 = CreateCV("cv1",3);
	mv1 = CreateMV("mv1",3,5);
	
	WriteToConsole("\nAcquiring lock1 in rpcClient3\n\0", -1, -1, -1);
	if((Acquire("lock1",5)) == -1){
		WriteToConsole("Error acquiring lock1 in rpcClient3\n\0",-1,-1,-1);
		Exit(0);
	}
	
	if((Signal("cv1",3, "lock1",5)) == -1){
			WriteToConsole("Error signaling on cv1 and lock1 in rpcClient1\n\0",-1,-1,-1);
			Exit(0);
	}


	WriteToConsole("\nWaiting on CV1 Lock1 to get broadcast signal from rpcClient1\n\0", -1, -1, -1);
	WriteToConsole("\nNote: Now run rpcClient3\n\0", -1, -1, -1);
	if((Wait("cv1",3, "lock1",5)) == -1){
		WriteToConsole("Error waiting on cv1 and lock1 in rpcClient1\n\0",-1,-1,-1);
		Exit(0);
	}
	WriteToConsole("\nGot Broadcast on CV1 Lock1 from rpcClient1\n\0", -1, -1, -1);
	WriteToConsole("\nValue of MV1 index 3 set by rpcClient1 is %d\n\0", GetMV("mv1",3,3), -1, -1);
	Release("lock1",5);
	
	DeleteLock("lock1",5);
	WriteToConsole(" Deleted Lock1\n\0",-1,-1,-1);
	DeleteMV("mv1",3);
	WriteToConsole(" Deleted MV1\n\0",-1,-1,-1);
	DeleteCV("cv1",3);
	WriteToConsole(" Deleted CV1\n\0",-1,-1,-1);
	
	Exit(0);
}
