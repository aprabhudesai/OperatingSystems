#include "syscall.h"
#define MAX_GROUPS 8


int main()
{

	int i,j=0,k,temp=3200,numOfMembers;
	/*Random(-12);*/
	for(i = 0 ; i < temp ; i++){

	}

	for(k = 0; k < MAX_GROUPS ; k++)
	{
		Exec("../test/headCustomer");
	}

	numOfMembers = MAX_GROUPS * 4;
	for(k = 0 ; k < numOfMembers; k ++){
		Exec("../test/groupMember");
	}

	Exit(0);
}
