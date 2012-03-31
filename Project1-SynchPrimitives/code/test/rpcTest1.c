#include "syscall.h"

int main()
{
	int lock1,lock2,cv1,cv2,mv1,mv2,lock3,cv3,mv3;
	
	if((lock1 = CreateLock("lock1",5)) == -1){
		WriteToConsole("Error Creating lock1 in rpcClient1\n\0",-1,-1,-1);
		Exit(0);
	}
	WriteToConsole("lock1 created in rpcClient1 with lockid: %d\n\0",lock1,-1,-1);
	if((lock2 = CreateLock("lock2",5)) == -1){
		WriteToConsole("Error Creating lock2 in rpcClient1\n\0",-1,-1,-1);
		Exit(0);
	}
	WriteToConsole("lock2 created in rpcClient1 with lockid: %d\n\0",lock2,-1,-1);
	if((cv1 = CreateCV("cv1", 3)) == -1){
		WriteToConsole("Error Creating CV1 in rpcClient1\n\0",-1,-1,-1);
		Exit(0);
	}
	WriteToConsole("cv1 created in rpcClient1 with CVid: %d\n\0",cv1,-1,-1);
	if((cv2 = CreateCV("cv2", 3)) == -1){
		WriteToConsole("Error Creating CV2 in rpcClient1\n\0",-1,-1,-1);
		Exit(0);
	}
	WriteToConsole("cv2 created in rpcClient1 with CVid: %d\n\0",cv2,-1,-1);
	if((mv1 = CreateMV("mv1", 3, 5)) == -1){
		WriteToConsole("Error Creating MV1 in rpcClient1\n\0",-1,-1,-1);
		Exit(0);
	}
	WriteToConsole("mv1 created in rpcClient1 with MVid: %d\n\0",mv1,-1,-1);
	if((mv2 = CreateMV("mv2", 3, 5)) == -1){
		WriteToConsole("Error Creating MV2 in rpcClient1\n\0",-1,-1,-1);
		Exit(0);
	}
	WriteToConsole("mv2 created in rpcClient1 with MVid: %d\n\0",mv2,-1,-1);
	
	WriteToConsole("\nAcquire lock1: %d\n\0", lock1, -1, -1);
	if((Acquire(lock1)) == -1){
		WriteToConsole("Error acquiring lock1 with id:%d in rpcClient1\n\0",lock1,-1,-1);
		Exit(0);
	}
	
	WriteToConsole("\nAcquire lock2: %d\n\0", lock2, -1, -1);
	if((Acquire(lock2)) == -1){
		WriteToConsole("Error acquiring lock2 with id:%d in rpcClient1\n\0",lock2,-1,-1);
		Exit(0);
	}
	
	
	if((SetMV(mv2, 0, 1)) == -1){
		WriteToConsole("Error setting mv1 in rpcClient1\n\0",-1,-1,-1);
		Exit(0);
	}
	WriteToConsole("\nMV2 index: 0 set to 1 by rpcClient1\n\0", -1, -1, -1);
	
	WriteToConsole("\nWaiting on CV2 Lock2 for signal from rpcClient4\n\0", -1, -1, -1);
	WriteToConsole("\nNote: Now, please run rpcClient2\n\0", -1, -1, -1);
	
	if((Wait(cv2, lock2)) == -1){
		WriteToConsole("Error waiting on cv2 and lock2 in rpcClient1\n\0",-1,-1,-1);
		Exit(0);
	}
	WriteToConsole("\nGot Signal on CV2 Lock2 from rpcClient4 in rpcClient1\n\0", -1, -1, -1);
	WriteToConsole("\nValue of MV2 index 1 is %d in rpcClient1\n\0", GetMV(mv2,1), -1, -1);
	
	WriteToConsole("\nWaiting on CV1 Lock1 to get signal from rpcClient3\n\0", -1, -1, -1);
	if((Wait(cv1, lock1)) == -1){
		WriteToConsole("Error waiting on cv1 and lock1 in rpcClient1\n\0",-1,-1,-1);
		Exit(0);
	}
	WriteToConsole("\nGot Signal on CV1 Lock1 from rpcClient3\n\0", -1, -1, -1);
	
	if((SetMV(mv1, 3, 402)) == -1){
		WriteToConsole("Error setting mv1 in rpcClient1\n\0",-1,-1,-1);
		Exit(0);
	}
	WriteToConsole("\nMV1 index: 3 set to 402 in rpcClient1\n\0", -1, -1, -1);
	
	if((Broadcast(cv1,lock1)) == -1){
		WriteToConsole("Error Broadcasting on cv1 and lock1 in rpcClient1\n\0",-1,-1,-1);
		Exit(0);
	}
	WriteToConsole("Broadcasted on cv1 and lock1 to rpcClient2 and rpcClient3\n\0",-1,-1,-1);
	
	if((Release(lock1)) == -1){
		WriteToConsole("Error releasing lock1 with id:%d in rpcClient1\n\0",lock1,-1,-1);
		Exit(0);
	}
	WriteToConsole("\nReleased lock1: %d\n\0", lock1, -1, -1);
	
	if((Release(lock2)) == -1){
		WriteToConsole("Error releasing lock2 with id:%d in rpcClient1\n\0",lock2,-1,-1);
		Exit(0);
	}
	WriteToConsole("\nReleased lock2: %d\n\0", lock2, -1, -1);
	
	DeleteLock(lock1);
	WriteToConsole("Sent DeleteLock Request id: %d\n\0",lock1,-1,-1);
	DeleteLock(lock2);
	WriteToConsole("Sent DeleteLock Request id: %d\n\0",lock2,-1,-1);
	DeleteCV(cv1);
	WriteToConsole("Sent DeleteCV Request id: %d\n\0",cv1,-1,-1);
	DeleteCV(cv2);
	WriteToConsole("Sent DeleteCV Request id: %d\n\0",cv2,-1,-1);
	DeleteMV(mv1);
	WriteToConsole("Sent DeletMV Request id: %d\n\0",mv1,-1,-1);
	DeleteMV(mv2);
	WriteToConsole("Sent DeletMV Request id: %d\n\0",mv2,-1,-1);
	WriteToConsole("All resources requested to be deleted\n\0",-1,-1,-1);
	
	if((lock3 = CreateLock("lock3",5)) == -1){
			WriteToConsole("Error Creating lock3 in rpcClient1\n\0",-1,-1,-1);
			Exit(0);
		}
		WriteToConsole("lock3 created in rpcClient1 with lockid: %d\n\0",lock3,-1,-1);

		if((cv3 = CreateCV("cv3", 3)) == -1){
			WriteToConsole("Error Creating CV3 in rpcClient1\n\0",-1,-1,-1);
			Exit(0);
		}
		WriteToConsole("cv3 created in rpcClient1 with CVid: %d\n\0",cv3,-1,-1);

		if((mv3 = CreateMV("mv3", 3, 5)) == -1){
			WriteToConsole("Error Creating MV3 in rpcClient1\n\0",-1,-1,-1);
			Exit(0);
		}
		WriteToConsole("mv3 created in rpcClient1 with MVid: %d\n\0",mv3,-1,-1);

		WriteToConsole("\nAcquire lock3: %d\n\0", lock3, -1, -1);
		if((Acquire(lock3)) == -1){
			WriteToConsole("Error acquiring lock3 with id:%d in rpcClient1\n\0",lock3,-1,-1);
			Exit(0);
		}

		if((SetMV(mv3, 3, 1)) == -1){
			WriteToConsole("Error setting mv3 in rpcClient1\n\0",-1,-1,-1);
			Exit(0);
		}
		WriteToConsole("\nMV3 index: 3 set to 1 by rpcClient1\n\0", -1, -1, -1);

		if((Signal(cv3,lock3)) == -1){
				WriteToConsole("Error Signalling on cv3 and lock3 in rpcClient1\n\0",-1,-1,-1);
				Exit(0);
		}
		WriteToConsole("Signalled on cv3 and lock3 to rpcClient5\n\0",-1,-1,-1);

		WriteToConsole("\nRelease lock3: %d\n\0", lock3, -1, -1);
			if((Release(lock3)) == -1){
				WriteToConsole("Error Releasing lock3 with id:%d in rpcClient1\n\0",lock3,-1,-1);
				Exit(0);
		}
		DeleteLock(lock3);
		DeleteCV(mv3);
		DeleteMV(cv3);

		Exit(0);
}
