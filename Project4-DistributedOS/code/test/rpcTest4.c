#include "syscall.h"

int
main()
{
	int lock2, cv2, mv2;

	if((lock2 = CreateLock("lock2",5)) == -1){
		WriteToConsole("Error Creating lock2 in rpcClient4\n\0",-1,-1,-1);
		Exit(0);
	}
	WriteToConsole("lock2 created in rpcClient4.\n\0",-1,-1,-1);
	
	WriteToConsole("cv1 created in rpcClient4.\n\0",-1,-1,-1);
	if((cv2 = CreateCV("cv2", 3)) == -1){
		WriteToConsole("Error Creating CV2 in rpcClient4\n\0",-1,-1,-1);
		Exit(0);
	}
	WriteToConsole("cv2 created in rpcClient4.\n\0",-1,-1,-1);
	
	if((mv2 = CreateMV("mv2", 3, 5)) == -1){
		WriteToConsole("Error Creating MV2 in rpcClient2\n\0",-1,-1,-1);
		Exit(0);
	}
	WriteToConsole("mv2 created in rpcClient2.\n\0",-1,-1,-1);
	
	if((Acquire("lock2",5)) == -1){
		WriteToConsole("Error acquiring lock2 in rpcClient4\n\0",-1,-1,-1);
		Exit(0);
	}
	WriteToConsole("\nAcquired lock2.\n\0", -1, -1, -1);
	WriteToConsole("\nValue of MV2 index 0 set by rpcClient4 is %d\n\0", GetMV("mv2", 3,0), -1, -1);
	
	if((SetMV("mv2", 3, 1, 5)) == -1){
		WriteToConsole("Error setting mv2 in rpcClient4\n\0",-1,-1,-1);
		Exit(0);
	}
	WriteToConsole("\nMV2 index: 1 set to 5 by rpcClient4\n\0", -1, -1, -1);
	
	WriteToConsole("\nSignalling on CV2 Lock2 to rpcClient1\n\0", -1, -1, -1);
	if((Signal("cv2", 3, "lock2",5)) == -1){
		WriteToConsole("Error signalling on cv2 and lock2 in rpcClient4\n\0",-1,-1,-1);
		Exit(0);
	}
	if((Release("lock2",5)) == -1){
		WriteToConsole("Error releasing lock2 in rpcClient1\n\0",-1,-1,-1);
		Exit(0);
	}
	WriteToConsole("\nReleased lock2\n\0", -1, -1, -1);

	DeleteLock("lock2",5);
	WriteToConsole(" Deleted Lock2\n\0",-1,-1,-1);
	DeleteMV("mv2",3);
	WriteToConsole(" Deleted MV2\n\0",-1,-1,-1);
	DeleteCV("cv2",3);
	WriteToConsole(" Deleted CV2\n\0",-1,-1,-1);
	Exit(0);
}
