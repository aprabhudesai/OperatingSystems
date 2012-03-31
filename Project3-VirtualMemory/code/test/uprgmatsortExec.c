#include "syscall.h"

int main()
{
	WriteToConsole("\nExec matmult\n\0",-1,-1,-1);
	Exec("../test/matmult");
	WriteToConsole("\nExec sort\n\0",-1,-1,-1);
	Exec("../test/sort");
	Exit(0);
}