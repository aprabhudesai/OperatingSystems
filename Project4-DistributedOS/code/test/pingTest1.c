#include "syscall.h"

int main()
{
	int lock1,locktemp,c1;
	
	WriteToConsole("\nUser Program 1\n",-1,-1,-1);
	WriteToConsole("\nCreating Lock - lock1\n",-1,-1,-1);
	if((locktemp = CreateLock("lock1",5)) == -1){
		WriteToConsole("Error Creating locktemp in User Program 1\n\0",-1,-1,-1);
		Exit(0);
	}
	
	WriteToConsole("\nAcquiring lock1\n", -1, -1, -1);
	if((Acquire("lock1",5)) == -1){
		WriteToConsole("Error acquiring lock1 in User Program 1\n\0",-1,-1,-1);
		Exit(0);
	}
	
	WriteToConsole("\nCreating Condition Variable cv1\n",-1,-1,-1);
	c1 = CreateCV("cv1",3);
	
	WriteToConsole("\nCalling Wait on cv1\n",-1,-1,-1);
	WriteToConsole("\n---- Please Start the program pingTest2 and pingTest3 in two different windows ----\n",-1,-1,-1);
	Wait("cv1",3,"lock1",5);
	WriteToConsole("\nGot Signal from User Program 2 & User Program 3\n",-1,-1,-1);
	WriteToConsole("\nUser Program 1 Exiting without releasing lock1\n",-1,-1,-1);
	Yield();
	Yield();
	Exit(0);
}
