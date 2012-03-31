#include "syscall.h"
#define MAX_EXECS 60


int main()
{
	int i;
	CreateMV("mv1", 3, 1);
	SetMV("mv1", 3, 0, 0);

	for(i = 0 ; i < MAX_EXECS; i++)
	{
		Exec("../test/specialTest2_1");
	}
	Exit(0);
}
