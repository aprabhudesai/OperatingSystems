#include "syscall.h"

int main(){
	WriteToConsole("\nExec 2 sorts\n\0",-1,-1,-1);
	Exec("../test/sort\0");
	Exec("../test/sort\0");
	Exit(0);
}