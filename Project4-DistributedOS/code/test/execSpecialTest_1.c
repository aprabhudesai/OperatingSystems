#include "syscall.h"
#define MAX_EXECS 60

int main()
{
	int i;
	for(i = 0 ; i < MAX_EXECS; i++)
	{

		Exec("../test/specialTest_1");

	}
	Exit(0);
}
