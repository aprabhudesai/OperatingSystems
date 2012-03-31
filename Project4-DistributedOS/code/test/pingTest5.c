#include "syscall.h"

int main()
{
	int lockId, lockStatus,locktemp,mv,cv2,signalStatus;
	
	WriteToConsole("\nUser Program 5\n",-1,-1,-1);
	WriteToConsole("\nCreating Lock - lock 1\n",-1,-1,-1);
	if((locktemp = CreateLock("lock1",5)) == -1){
		WriteToConsole("Error Creating locktemp in User Program 5\n\0",-1,-1,-1);
		Exit(0);
	}
	
	WriteToConsole("\nAcquiring lock1\n", -1, -1, -1);
	if((Acquire("lock1",5)) == -1){
		WriteToConsole("Error acquiring lock1 in User Program 5\n\0",-1,-1,-1);
		Exit(0);
	}
	
	WriteToConsole("\nCreating Monitor Variable - mv1 \n",-1,-1,-1);
	mv = CreateMV("mv1",3,0);
	
	WriteToConsole("\nCreating Lock - lock2\n",-1,-1,-1);
	if((locktemp = CreateLock("lock2",5)) == -1){
		WriteToConsole("Error Creating lock2 in User Program 5\n\0",-1,-1,-1);
		Exit(0);
	}
	
	WriteToConsole("\nAcquiring lock2\n", -1, -1, -1);
	if((Acquire("lock2",5)) == -1){
		WriteToConsole("Error acquiring lock2 in User Program 5\n\0",-1,-1,-1);
		Exit(0);
	}
	
	WriteToConsole("\nCreating Condition Variable - cv2\n",-1,-1,-1);
	cv2 = CreateCV("cv2",3);
	
	WriteToConsole("Signaling User Program 3\n\0",-1,-1,-1);
	signalStatus = Signal("cv2", 3,"lock2",5);
	if(signalStatus == -1)
	{
		WriteToConsole("\nFailed to Signal on the given Condition\0", -1, -1, -1);
		Exit(0);
	}
	else
	{
		WriteToConsole("\nSignalling on cv2 with lock2\n\0", -1, -1, -1);
	}
	
	WriteToConsole("\nUser Program 5 Exiting without releasing lock1 & lock2\n",-1,-1,-1);
   Exit(0);
}