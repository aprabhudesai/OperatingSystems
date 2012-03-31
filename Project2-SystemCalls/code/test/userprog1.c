#include "syscall.h"

int x=0;
int lockid;
void fun1()
{
	WriteToConsole("In Function 1 - user prog 1\n\0",-1,-1,-1);
	Acquire(lockid);
	WriteToConsole("\nIn user prog1 fun1 - x: %d\n\0", x, -1, -1);
	Release(lockid);
	DeleteLock(lockid);
	WriteToConsole("\nDeleted Lock Id: %d\n\0", lockid, -1, -1);
	Exit(0);
}

int
main()
{
   /*int lockId = 1;*/
   /* Halt(); */
   /*void (*ptrfunc)();*/
   /*ptrfunc = &calc;*/
   int i;
   
   
   lockid = CreateLock("userProg1Lock",13);
   Acquire(lockid);
   WriteToConsole("\nAfter Exec - userprog 1\n\0", 26, -1, -1);
   
   Fork(fun1);
   /*WriteToConsole("After Fork To Multi\n\0", 21, -1, -1);*/
   i=0;
   while(i<10){
	WriteToConsole("\nIn user prog1 loop %d - x: %d\n\0", i, x, -1);
	i++;x++;
   }
   Release(lockid);
   
   Exit(0);
}

