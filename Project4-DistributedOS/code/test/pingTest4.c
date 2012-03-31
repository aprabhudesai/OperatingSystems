#include "syscall.h"

int main()
{
	int lockId, lockStatus,locktemp,mv;
	
	WriteToConsole("\nUser Program 4\n",-1,-1,-1);
	
	WriteToConsole("\nCreating Monitor Variable - mv1\n",-1,-1,-1);
	mv = CreateMV("mv1",3,0);
	
	WriteToConsole("\nCreating Lock - lock2\n",-1,-1,-1);
	if((locktemp = CreateLock("lock2",5)) == -1){
		WriteToConsole("Error Creating lock2 in User Program 4\n\0",-1,-1,-1);
		Exit(0);
	}
	
	WriteToConsole("\nAcquiring lock2\n", -1, -1, -1);
	if((Acquire("lock2",5)) == -1){
		WriteToConsole("Error acquiring lock2 in User Program 4\n\0",-1,-1,-1);
		Exit(0);
	}
	
	WriteToConsole("\nReleasing Lock lock2\n",-1,-1,-1);
	Release("lock2",5);
	
	WriteToConsole("\nUser Program 4 Exiting\n",-1,-1,-1);
   Exit(0);
}