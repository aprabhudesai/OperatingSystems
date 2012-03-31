#include "syscall.h"

int lockid3;
int cvid3;
int ccFlag = 0;
int custFlag = 0;
int count = 0;
int workLock;
int workCV;

void concessionclerk()
{
	ccFlag = 1;
	Acquire(lockid3);
	count++;
	if(custFlag = 1)
	{
		WriteToConsole("Signaling Customer \n\0",-1,-1,-1);
		Signal(cvid3,lockid3);
	}
	else
	{
		WriteToConsole("Waiting for Customer\n\0",-1,-1,-1);
		Wait(cvid3,lockid3);
		WriteToConsole("Signaled for Customer\n\0",-1,-1,-1);
	}
	Release(lockid3);
	Acquire(workLock);
	if(count ==2)
	{
		WriteToConsole("Concession Clerk Signaling Main Thread\n\0",-1,-1,-1);
		Signal(workCV,workLock);
	}
	Release(workLock);
	Exit(0);
}

void customer()
{
	Acquire(lockid3);
	custFlag = 1;
	count++;
	if(ccFlag = 0)
	{
		WriteToConsole("Waiting for Concession Clerk \n\0",-1,-1,-1);
		Wait(cvid3,lockid3);
		WriteToConsole("Signaled by Concession Clerk \n\0",-1,-1,-1);
		Release(lockid3);
	}
	else
	{
		WriteToConsole("Signaling Concession Clerk \n\0",-1,-1,-1);
		Signal(cvid3,lockid3);
		Release(lockid3);
	}
	WriteToConsole("Customer Exiting\n\0",-1,-1,-1);
	Acquire(workLock);
	if(count ==2)
	{
		WriteToConsole("Customer Signaling Main Thread\n\0",-1,-1,-1);
		Signal(workCV,workLock);
	}
	Release(workLock);
	Exit(0);
}

int main()
{
	WriteToConsole("After Exec in : USERPROG 3: \n\0",-1,-1,-1);
	lockid3 = CreateLock("lock3",5);
	cvid3 = CreateCV("cv3",3);
	workLock = CreateLock("workLock",8);
	workCV = CreateCV("workCV",6);
	WriteToConsole("USERPROG 3: Created customer \n\0",-1,-1,-1);
	Fork(customer);
	WriteToConsole("USERPROG 3: Created concession clerk \n\0",-1,-1,-1);
	Fork(concessionclerk);
	Acquire(workLock);
	if(count < 2)
	{
		Wait(workCV,workLock);
	}
	Release(workLock);
	Exit(0);
}