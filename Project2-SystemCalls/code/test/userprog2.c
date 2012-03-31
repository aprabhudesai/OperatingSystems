#include "syscall.h"

void fun1()
{
	WriteToConsole("\nIn :USERPROG 2: fun1\n\0", -1, -1, -1);
	Exit(0);
}

int
main()
{
   /*int lockId = 1;*/
   /* Halt(); */
   /*void (*ptrfunc)();*/
   /*ptrfunc = &calc;*/
   WriteToConsole("\nAfter Exec :USERPROG 2:\n\0", -1, -1, -1);
   
   Fork(fun1);
   /*WriteToConsole("After Fork To Multi\n\0", 21, -1, -1);*/
   WriteToConsole("\nCalling Exec for : USERPROG 3: \n\0", -1, -1, -1);
   Exec("../test/userprog3\0");
	WriteToConsole("\nCalling Exec for : USERPROG 1: \n\0", -1, -1, -1);
	Exec("../test/userprog1\0");
   Exit(0);
}