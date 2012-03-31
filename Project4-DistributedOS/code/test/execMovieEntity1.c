#include "syscall.h"
#define MAX_TC 5
#define MAX_CC 5
#define MAX_TT 3

void delay(int time)
{
	int l = 0;
	for (l = 0 ; l < time; l++)
	{
		
	}
}

int main()
{
	int i;

	delay(551);
	for(i = 0 ; i < MAX_TC ; i++)
	{
		WriteToConsole("\nExec Ticket Clerk : %d\n\0", i, -1, -1);
		Exec("../test/ticketClerk");
	}

	for(i = 0 ; i < MAX_CC ; i++)
	{
		Exec("../test/concessionClerk");

	}

	for(i = 0 ; i < MAX_TT ; i++)
	{
		Exec("../test/ticketTaker");
	}

	Exit(0);
}
