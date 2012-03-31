#include "syscall.h"

int main(){
	int lock1,mv1,cv1;
	
	lock1 = CreateLock("lock1",5);
	cv1 = CreateCV("cv1",3);
	mv1 = CreateMV("mv1",3,5);
	
	WriteToConsole("Lock1 created in rpcClient2 with lockid %d\n\0",lock1,-1,-1);
	WriteToConsole("cv1 created in rpcClient2 with cvid %d\n\0",cv1,-1,-1);
	WriteToConsole("mv1 created in rpcClient2 with mvid %d\n\0",mv1,-1,-1);

	WriteToConsole("\nAcquire lock1: %d in rpcClient2\n\0", lock1, -1, -1);
	if((Acquire(lock1)) == -1){
		WriteToConsole("Error acquiring lock1 with id:%d in rpcClient2\n\0",lock1,-1,-1);
		Exit(0);
	}
	
	WriteToConsole("\nWaiting on CV1 Lock1 to get broadcast signal from rpcClient1\n\0", -1, -1, -1);
	WriteToConsole("\nNote: Now run rpcClient3\n\0", -1, -1, -1);
	if((Wait(cv1, lock1)) == -1){
		WriteToConsole("Error waiting on cv1 and lock1 in rpcClient1\n\0",-1,-1,-1);
		Exit(0);
	}
	WriteToConsole("\nGot Broadcast on CV1 Lock1 from rpcClient1\n\0", -1, -1, -1);
	WriteToConsole("\nValue of MV1 index 3 set by rpcClient1 is %d\n\0", GetMV(mv1,3), -1, -1);
	Release(lock1);
	
	DeleteLock(lock1);
	WriteToConsole("Sent DeletLock Request id: %d\n\0",lock1,-1,-1);
	DeleteMV(mv1);
	WriteToConsole("Sent DeletMV Request id: %d\n\0",mv1,-1,-1);
	DeleteCV(cv1);
	WriteToConsole("Sent DeletCV Request id: %d\n\0",cv1,-1,-1);
	
	Exit(0);
}
