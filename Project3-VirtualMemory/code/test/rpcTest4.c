#include "syscall.h"

int
main()
{
	int lock2, cv2, mv2;

	if((lock2 = CreateLock("lock2",5)) == -1){
		WriteToConsole("Error Creating lock2 in rpcClient4\n\0",-1,-1,-1);
		Exit(0);
	}
	WriteToConsole("lock2 created in rpcClient4 with lockid: %d\n\0",lock2,-1,-1);
	
	WriteToConsole("cv1 created in rpcClient4 with CVid: %d\n\0",cv2,-1,-1);
	if((cv2 = CreateCV("cv2", 3)) == -1){
		WriteToConsole("Error Creating CV2 in rpcClient4\n\0",-1,-1,-1);
		Exit(0);
	}
	WriteToConsole("cv2 created in rpcClient4 with CVid: %d\n\0",cv2,-1,-1);
	
	if((mv2 = CreateMV("mv2", 3, 5)) == -1){
		WriteToConsole("Error Creating MV2 in rpcClient2\n\0",-1,-1,-1);
		Exit(0);
	}
	WriteToConsole("mv2 created in rpcClient2 with MVid: %d\n\0",mv2,-1,-1);
	
	if((Acquire(lock2)) == -1){
		WriteToConsole("Error acquiring lock2 with id:%d in rpcClient4\n\0",lock2,-1,-1);
		Exit(0);
	}
	WriteToConsole("\nAcquired lock2: %d\n\0", lock2, -1, -1);
	WriteToConsole("\nValue of MV2 index 0 set by rpcClient4 is %d\n\0", GetMV(mv2,0), -1, -1);
	
	if((SetMV(mv2, 1, 5)) == -1){
		WriteToConsole("Error setting mv2 in rpcClient4\n\0",-1,-1,-1);
		Exit(0);
	}
	WriteToConsole("\nMV2 index: 1 set to 5 by rpcClient4\n\0", -1, -1, -1);
	
	WriteToConsole("\nSignalling on CV2 Lock2 to rpcClient1\n\0", -1, -1, -1);
	if((Signal(cv2, lock2)) == -1){
		WriteToConsole("Error signalling on cv2 and lock2 in rpcClient4\n\0",-1,-1,-1);
		Exit(0);
	}
	if((Release(lock2)) == -1){
		WriteToConsole("Error releasing lock2 with id:%d in rpcClient1\n\0",lock2,-1,-1);
		Exit(0);
	}
	WriteToConsole("\nReleased lock2: %d\n\0", lock2, -1, -1);

	DeleteLock(lock2);
	WriteToConsole("Sent DeletLock Request id: %d\n\0",lock2,-1,-1);
	DeleteCV(cv2);
	WriteToConsole("Sent DeletCV Request id: %d\n\0",cv2,-1,-1);
	DeleteMV(mv2);
	WriteToConsole("Sent DeleteMV Request id: %d\n\0",mv2,-1,-1);
	Exit(0);
}
