#include "syscall.h"

int main()
{
	int lock1,lock2,cv1,cv2,mv1,mv2,lock3,cv3,mv3;
	
	if((lock1 = CreateLock("lock1",5)) == -1){
		WriteToConsole("Error Creating lock1 in rpcClient1\n\0",-1,-1,-1);
		Exit(0);
	}
	WriteToConsole("lock1 created in rpcClient1.\n\0",-1,-1,-1);
	if((lock2 = CreateLock("lock2",5)) == -1){
		WriteToConsole("Error Creating lock2 in rpcClient1\n\0",-1,-1,-1);
		Exit(0);
	}
	WriteToConsole("lock2 created in rpcClient1.\n\0",-1,-1,-1);
	if((cv1 = CreateCV("cv1", 3)) == -1){
		WriteToConsole("Error Creating CV1 in rpcClient1\n\0",-1,-1,-1);
		Exit(0);
	}
	WriteToConsole("cv1 created in rpcClient1.\n\0",-1,-1,-1);
	if((cv2 = CreateCV("cv2", 3)) == -1){
		WriteToConsole("Error Creating CV2 in rpcClient1\n\0",-1,-1,-1);
		Exit(0);
	}
	WriteToConsole("cv2 created in rpcClient1.\n\0",-1,-1,-1);
	if((mv1 = CreateMV("mv1", 3, 5)) == -1){
		WriteToConsole("Error Creating MV1 in rpcClient1\n\0",-1,-1,-1);
		Exit(0);
	}
	WriteToConsole("mv1 created in rpcClient1.\n\0",-1,-1,-1);
	if((mv2 = CreateMV("mv2", 3, 5)) == -1){
		WriteToConsole("Error Creating MV2 in rpcClient1\n\0",-1,-1,-1);
		Exit(0);
	}
	WriteToConsole("mv2 created in rpcClient1.\n\0",-1,-1,-1);
	
	WriteToConsole("\nAcquiring lock1\n\0", -1, -1, -1);
	if((Acquire("lock1",5)) == -1){
		WriteToConsole("Error acquiring lock1 in rpcClient1\n\0",-1,-1,-1);
		Exit(0);
	}
	
	WriteToConsole("\nAcquiring lock2\n\0", -1, -1, -1);
	if((Acquire("lock2",5)) == -1){
		WriteToConsole("Error acquiring lock2 in rpcClient1\n\0",-1,-1,-1);
		Exit(0);
	}
	
	
	if((SetMV("mv2", 3, 0, 1)) == -1){
		WriteToConsole("Error setting mv1 in rpcClient1\n\0",-1,-1,-1);
		Exit(0);
	}
	WriteToConsole("\nMV2 index: 0 set to 1 by rpcClient1\n\0", -1, -1, -1);
	
	WriteToConsole("\nWaiting on CV2 Lock2 for signal from rpcClient4\n\0", -1, -1, -1);
	WriteToConsole("\nNote: Now, please run rpcClient2\n\0", -1, -1, -1);
	
	if((Wait("cv2", 3, "lock2", 5)) == -1){
		WriteToConsole("Error waiting on cv2 and lock2 in rpcClient1\n\0",-1,-1,-1);
		Exit(0);
	}
	WriteToConsole("\nGot Signal on CV2 Lock2 from rpcClient4 in rpcClient1\n\0", -1, -1, -1);
	WriteToConsole("\nValue of MV2 index 1 is %d in rpcClient1\n\0", GetMV("mv2", 3, 1), -1, -1);
	
	WriteToConsole("\nWaiting on CV1 Lock1 to get signal from rpcClient3\n\0", -1, -1, -1);
	if((Wait("cv1", 3, "lock1", 5)) == -1){
		WriteToConsole("Error waiting on cv1 and lock1 in rpcClient1\n\0",-1,-1,-1);
		Exit(0);
	}
	WriteToConsole("\nGot Signal on CV1 Lock1 from rpcClient3\n\0", -1, -1, -1);
	
	if((SetMV("mv1", 3, 3, 402)) == -1){
		WriteToConsole("Error setting mv1 in rpcClient1\n\0",-1,-1,-1);
		Exit(0);
	}
	WriteToConsole("\nMV1 index: 3 set to 402 in rpcClient1\n\0", -1, -1, -1);
	
	if((Broadcast("cv1", 3 ,"lock1", 5)) == -1){
		WriteToConsole("Error Broadcasting on cv1 and lock1 in rpcClient1\n\0",-1,-1,-1);
		Exit(0);
	}
	WriteToConsole("Broadcasted on cv1 and lock1 to rpcClient2 and rpcClient3\n\0",-1,-1,-1);
	
	if((Release("lock1", 5)) == -1){
		WriteToConsole("Error releasing lock1 in rpcClient1\n\0",-1,-1,-1);
		Exit(0);
	}
	WriteToConsole("\nReleased lock1\n\0", -1, -1, -1);
	
	if((Release("lock2", 5)) == -1){
		WriteToConsole("Error releasing lock2 in rpcClient1\n\0",-1,-1,-1);
		Exit(0);
	}
	WriteToConsole("\nReleased lock2\n\0", -1, -1, -1);
	
	DeleteLock("lock1", 5);
	WriteToConsole("Deleted Lock1\n\0",-1,-1,-1);
	DeleteLock("lock2", 5);
	WriteToConsole("Deleted Lock2\n\0",-1,-1,-1);
	DeleteCV("cv1", 3);
	WriteToConsole("Deleted CV1\n\0",-1,-1,-1);
	DeleteCV("cv2", 3);
	WriteToConsole("Deleted CV2\n\0",-1,-1,-1);
	DeleteMV("mv1", 3);
	WriteToConsole("Deleted MV1\n\0",-1,-1,-1);
	DeleteMV("mv2", 3);
	WriteToConsole("Deleted MV2\n\0",-1,-1,-1);
	WriteToConsole("All resources requested to be deleted\n\0",-1,-1,-1);
	
	if((lock3 = CreateLock("lock3",5)) == -1){
			WriteToConsole("Error Creating lock3 in rpcClient1\n\0",-1,-1,-1);
			Exit(0);
		}
		WriteToConsole("lock3 created in rpcClient1.\n\0",-1,-1,-1);

		if((cv3 = CreateCV("cv3", 3)) == -1){
			WriteToConsole("Error Creating CV3 in rpcClient1\n\0",-1,-1,-1);
			Exit(0);
		}
		WriteToConsole("cv3 created in rpcClient1. \n\0",-1,-1,-1);

		if((mv3 = CreateMV("mv3", 3, 5)) == -1){
			WriteToConsole("Error Creating MV3 in rpcClient1\n\0",-1,-1,-1);
			Exit(0);
		}
		WriteToConsole("mv3 created in rpcClient1\n\0",-1,-1,-1);

		WriteToConsole("\nAcquiring lock3\n\0", -1, -1, -1);
		if((Acquire("lock3", 5)) == -1){
			WriteToConsole("Error acquiring lock3 in rpcClient1\n\0",-1,-1,-1);
			Exit(0);
		}

		if((SetMV("mv3", 3, 3, 1)) == -1){
			WriteToConsole("Error setting mv3 in rpcClient1\n\0",-1,-1,-1);
			Exit(0);
		}
		WriteToConsole("\nMV3 index: 3 set to 1 by rpcClient1\n\0", -1, -1, -1);

		if((Signal("cv3", 3 ,"lock3", 5)) == -1){
				WriteToConsole("Error Signaling on cv3 and lock3 in rpcClient1\n\0",-1,-1,-1);
				Exit(0);
		}
		WriteToConsole("Signaled on cv3 and lock3 to rpcClient5\n\0",-1,-1,-1);

		WriteToConsole("\nReleasing lock3\n\0", lock3, -1, -1);
			if((Release("lock3", 5)) == -1){
				WriteToConsole("Error Releasing lock3 in rpcClient1\n\0",-1,-1,-1);
				Exit(0);
		}
		DeleteLock("lock3", 5);
		DeleteMV("mv3", 3);
		DeleteCV("cv3", 3);

		Exit(0);
}
