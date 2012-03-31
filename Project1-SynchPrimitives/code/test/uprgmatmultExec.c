#include "syscall.h"

int main(){
	WriteToConsole("\nExec 2 matmults\n\0",-1,-1,-1);
	Exec("../test/matmult\0");
	Exec("../test/matmult\0");
	Exit(0);
}