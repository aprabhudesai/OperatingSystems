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
   Fork(calc);
   Fork(multi);
   WriteToConsole("After Fork To Multi\n\0", 21, -1, -1);
   WriteToConsole("Calling user program : USERPROG1\n\0",-1,-1,-1);
   Exec("../test/userprog1\0");
   WriteToConsole("Calling user program : USERPROG2\n\0",-1,-1,-1);
   Exec("../test/userprog2\0");
   WriteToConsole("Calling user program : USERPROGINTERACT1\n\0",-1,-1,-1);
   Exec("../test/userprogInteract1\0");
   WriteToConsole("Calling TestSuite\n\0",-1,-1,-1);
   Exec("../test/testSuite\0");
   Exit(0);
}

