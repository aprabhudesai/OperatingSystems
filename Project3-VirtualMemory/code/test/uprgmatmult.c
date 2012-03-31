#include "syscall.h"

int main(){
	WriteToConsole("\nExec matmult 1 time\n\0",-1,-1,-1);
	Exec("../test/matmult\0");
	WriteToConsole("\nExec matmult 2 time\n\0",-1,-1,-1);
	Exec("../test/matmult1\0");
	Exit(0);
}