/* halt.c
 *	Simple program to test whether running a user program works.
 *	
 *	Just do a "syscall" that shuts down the OS.
 *
 * 	NOTE: for some reason, user programs with global data structures 
 *	sometimes haven't worked in the Nachos environment.  So be careful
 *	out there!  One option is to allocate data structures as 
 * 	automatics within a procedure, but if you do this, you have to
 *	be careful to allocate a big enough stack to hold the automatics!
 */

#include "syscall.h"

void calc()
{
	WriteToConsole("In Calc\n\0",9,-1,-1);
	Exit(0);
}

void multi()
{
	WriteToConsole("In Multi\n\0",10,-1,-1);
	Exit(0);
}

int
main()
{
   /*int lockId = 1;*/
   /* Halt(); */
   /*void (*ptrfunc)();*/
   /*ptrfunc = &calc;*/
   
   Fork(calc);
   WriteToConsole("After Fork To Calc\n\0", 20, -1, -1);
   Fork(multi);
   WriteToConsole("After Fork To Multi\n\0", 21, -1, -1);
   WriteToConsole("Calling user program : USERPROG1\n\0",-1,-1,-1);
   Exec("../test/userprog1\0");
   WriteToConsole("Calling user program : USERPROG2\n\0",-1,-1,-1);
   Exec("../test/userprog2\0");
   WriteToConsole("Calling user program : USERPROGINTERACT1\n\0",-1,-1,-1);
   Exec("../test/userprogInteract1\0");
   WriteToConsole("After user program : USERPROGINTERACT1 : was called \n\0",-1,-1,-1);
   Exit(0);
}

