#include "syscall.h"

int main()
{
	int lockId, lockStatus,locktemp,cv2,mv;
	
	WriteToConsole("\nUser Program 3\n",-1,-1,-1);
	
	WriteToConsole("\nCreating Monitor Variable - mv1\n",-1,-1,-1);
	mv = CreateMV("mv1",3,0);
	
	WriteToConsole("\nCreating Lock - lock2\n",-1,-1,-1);
	if((locktemp = CreateLock("lock2",5)) == -1){
		WriteToConsole("Error Creating lock2 in User Program 3\n\0",-1,-1,-1);
		Exit(0);
	}
	
	WriteToConsole("\nAcquiring lock2\n", -1, -1, -1);
	if((Acquire("lock2",5)) == -1){
		WriteToConsole("Error acquiring lock2 in User Program 3\n\0",-1,-1,-1);
		Exit(0);
	}
	WriteToConsole("\nCreating Condition Variable - cv2\n",-1,-1,-1);
	cv2 = CreateCV("cv2",3);
	
	WriteToConsole("\nCalling Wait on cv2\n",-1,-1,-1);
	WriteToConsole("\n---- Please Start the program pingTest4 and pingTest5 in two different windows ----\n",-1,-1,-1);
	Wait("cv2",3,"lock2",5);
	
	WriteToConsole("\nUser Program 3 Exiting without releasing lock2\n",-1,-1,-1);
   Exit(0);
}