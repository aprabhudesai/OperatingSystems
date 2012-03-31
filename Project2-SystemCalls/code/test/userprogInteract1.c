#include "syscall.h"

int lineLock;
int lineCV;
int lineFlag = 1;
int custFlag = 0;
int custNum = 1;
int tcNum = 1;
int workDoneLock;
int workDoneCV;
int workDoneState = 0;

void ticketClerk(){
    Acquire(lineLock);
	lineFlag = 0;
	WriteToConsole("LineLock Acquired by ticketClerk\n\0",-1,-1,-1);
	if(custFlag != 0){
		Signal(lineCV,lineLock);
		WriteToConsole("Customer waiting so signal customer\n\0",-1,-1,-1);
	}
	else{
		WriteToConsole("No Customer so waiting\n\0",-1,-1,-1);
		Wait(lineCV,lineLock);
		WriteToConsole("Received signal from customer\n\0",-1,-1,-1);
	}
	Release(lineLock);
	Acquire(workDoneLock);
	workDoneState++;
	if(workDoneState == 2)
	{
		Signal(workDoneCV,workDoneLock);
		WriteToConsole("Work done so TicketClerk signaling Main Thread\n\0",lineCV,-1,-1);
	}
	Release(workDoneLock);
	Exit(0);

}

void customer()
{
	
	Acquire(lineLock);
	WriteToConsole("Customer Acquired Lock : %d\n\0",lineLock,-1,-1);
	custFlag = 1;
	if(lineFlag == 1)
	{
		WriteToConsole("No Ticket Clerk so waiting\n\0",-1,-1,-1);
		Wait(lineCV,lineLock);
		WriteToConsole("Received signal from Ticket Clerk\n\0",-1,-1,-1);
	}
	else
	{
		Signal(lineCV,lineLock);
		WriteToConsole("Found Ticket Clerk so signaling\n\0",-1,-1,-1);
	}
	Release(lineLock);
	Acquire(workDoneLock);
	workDoneState++;
	if(workDoneState == 2)
	{
		Signal(workDoneCV,workDoneLock);
		WriteToConsole("Work done so Customer signaling Main Thread\n\0",lineCV,-1,-1);
	}
	Release(workDoneLock);
	Exit(0);
}


int main()
{
	lineLock = CreateLock("linelock",8);
	lineCV = CreateCV("linecv",6);
	workDoneLock = CreateLock("workDoneLock",12);
	workDoneCV = CreateCV("workDoneCV",10);
	Fork(customer);
	WriteToConsole("Customer %d Created\n\0",custNum,-1,-1);
	Fork(ticketClerk);
	WriteToConsole("Ticket Clerk %d Created\n\0",tcNum,-1,-1);
	Yield();
	Acquire(workDoneLock);
	if(workDoneState != 2)
	{
		WriteToConsole("Waiting for work to be completed\n\0",-1,-1,-1);
		Wait(workDoneCV,workDoneLock);
		WriteToConsole("Work done so going for delete CV and Lock\n\0",-1,-1,-1);
	}
	Release(workDoneLock);
	DeleteLock(lineLock);
	WriteToConsole("Deleted Lock ID: %d\n\0",lineLock,-1,-1);
	DeleteCV(lineCV);
	WriteToConsole("Deleted CV ID: %d\n\0",lineCV,-1,-1);
	Exit(0);
}