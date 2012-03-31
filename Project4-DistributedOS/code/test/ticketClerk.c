#include "syscall.h"
#define MAX_TC 5 /*Maximum number of TicketClerks*/

enum TCSTATE {TCBUSY,TCNOT_BUSY};
enum BREAKSTATE {ON_BREAK,OFF_BREAK};

int ticketClerkIndexLockId;	/*lock between ticket clerks to take their index after they are forked*/
int nextTCIndexMVId;

int ticketClerkManagerBreakLockId;
int ticketClerkManagerBreakCVId[MAX_TC];	/*lock between manager and ticket clerk to send him on breaks*/
int ticketClerkBreakCount = 0;

int numberOfTicketsForTCMVId;	/*A monitor used by the HeadCustomer to ask for required number of tickets from TicketClerk*/
int ticketClerkLockId[MAX_TC];	/*A lock between TicketClerk and HeadCustomer for mutual exclusion during the interaction*/
int ticketClerkCVId[MAX_TC];	/*CV to wait on a TicketClerk lock to begin interaction*/

int amt2TCMVId, totalAmtByTCMVId;	/*Used to keep the amount collected by TicketClerk during a transaction and total amount collected by each clerk*/
int ticketClerkLineLockId;	/* For mutual exclusion to enter Ticket Taker Line*/
int ticketClerkLineCVId[MAX_TC];	/*To wait on the line lock to start interaction with customer*/
int ticketClerkLineCountMVId;	/*To maintain the count of customer in each clerk's line*/

/*enum TCSTATE {TCBUSY,TCNOT_BUSY} ticketClerkState[MAX_TC] = {TCBUSY};*/		/*To check the state of TicketClerk if is busy or available*/
int ticketClerkStateMVId;

/*Initialize the state to busy so that if customers comes up before TicketClerk they will have to wait*/
/*enum BREAKSTATE {ON_BREAK,OFF_BREAK} TCBreakState[MAX_TC] = {OFF_BREAK,OFF_BREAK,OFF_BREAK,OFF_BREAK,OFF_BREAK};*/
int breakStateMVId;

int managerCustomerTCFlagMVId;	/*Used to inform customers in line to join a new line*/
int ticketClerkManagerCVId;	/*lock and CV between manager and ticket clerk going and coming back from breaks*/
int ticketClerkManagerLockId;

int mgrMovieTechExitMVId;
int myIndex;
char buff[1];
char *name = "";
char *name1 = "";

int myexp ( int count ) {
  int i, val=1;
  for (i=0; i<count; i++ ) {
    val = val * 10;
  }
  return val;
}

void itoa( char arr[], int size, int val ) {
  int i, max, dig, subval, loc;
  for (i=0; i<size; i++ ) {
    arr[i] = '\0';
  }

  for ( i=1; i<=2; i++ ) {
    if (( val / myexp(i) ) == 0 ) {
      max = i-1;
      break;
    }
  }

  subval = 0;
  loc = 0;
  for ( i=max; i>=0; i-- ) {
    dig = 48 + ((val-subval) / myexp(i));
    subval = (dig-48) * myexp(max);
    arr[loc] = dig;
    loc++;
  }
  return;
}
	
void createTCLocksCV()
{

	ticketClerkLineLockId = CreateLock("tcLineLock", 10);

	itoa(buff,1,myIndex);
	name = "ticketClerkLineCV";
	*(name + 17) =  buff[0];
	ticketClerkLineCVId[myIndex] = CreateCV(name, 18);

	name = "ticketClerkLock";
	*(name + 15) =  buff[0];
	ticketClerkLockId[myIndex] = CreateLock(name, 16);

	name = "ticketClerkCV";
	*(name + 13) =  buff[0];
	ticketClerkCVId[myIndex] = CreateCV(name, 14);

}

void CreateMVs()
{
/*	ticketClerkStateMVId = CreateMV("ticketClerkState",16,MAX_TC);
	managerCustomerTCFlagMVId = CreateMV("managerCustomerTCFlag", 21, MAX_TC);
	numberOfTicketsForTCMVId = CreateMV("numberOfTicketsForTC", 20, MAX_TC);
	ticketClerkLineCountMVId = CreateMV("ticketClerkLineCount", 20, MAX_TC);
	breakStateMVId = CreateMV("TCBreakState",12,MAX_TC);
	amt2TCMVId = CreateMV("amt2TC", 6, MAX_TC);
	totalAmtByTCMVId = CreateMV("totalAmtByTC", 12, MAX_TC);

	mgrMovieTechExitMVId = CreateMV("mgrMovieTechExit", 16, 1);*/
}

int main()
{
	int numOfTickets=0;
	ticketClerkIndexLockId = CreateLock("tcIndexLock",11);
	nextTCIndexMVId = CreateMV("nextTCIndex",11,1);

	ticketClerkManagerBreakLockId = CreateLock("tcManagerBreakLock", 18);
	ticketClerkManagerLockId = CreateLock("tcManagerLock", 13);	/* lock for updating the individual amount*/
	/*ticketClerkManagerCVId = CreateCV("ticketClerkManagerCV", 20);*/

	Acquire("tcIndexLock",11);	/*ticket clerk acquire index lock to find their index*/
	myIndex = GetMV("nextTCIndex",11,0);
	SetMV("nextTCIndex",11,0,myIndex + 1);
	Release("tcIndexLock",11);
	
	CreateMVs();
	createTCLocksCV();

	itoa(buff,1,myIndex);
	name = "tcManagerBreakCV";
	*(name + 16) =  buff[0];
	ticketClerkManagerBreakCVId[myIndex] = CreateCV(name, 17);

	while(1)
	{
		/*Acquire("tcManagerBreakLock", 18);	/*acquire manager tiecketclerk break lock*/
		Acquire("tcLineLock", 10);	/* acquire line lock to see if managers has told to go on break*/


		/*	make state as available*/
		SetMV("ticketClerkState",16, myIndex, TCNOT_BUSY);
		/*Release("tcManagerBreakLock", 18);*/

		if (GetMV("ticketClerkLineCount",20, myIndex) > 0)/* Yes I have customers*/
		{
			/*make myself busy*/
			/* make yourself busy beofre begining interaction*/
			SetMV("ticketClerkState",16, myIndex, TCBUSY);
			WriteToConsole("\nTicketClerk %d has a line length %d and is signaling a customer.\n\0",myIndex, GetMV("ticketClerkLineCount",20, myIndex), -1);

			/* as I am going to service this customer, decrement line count by 1*/

			SetMV("ticketClerkLineCount",20, myIndex, (GetMV("ticketClerkLineCount",20, myIndex) - 1) );

			/*wake up the customer to be serviced*/
			name = "ticketClerkLineCV";
			*(name + 17) =  buff[0];
			Signal(name,18,"tcLineLock", 10);
		}
		else
		{
			WriteToConsole("\nTicketClerk %d has no one in line. I am available for a customer.\n\0",myIndex, -1, -1);
			/*no one in line so i am not busy, so make myself not busy*/

			SetMV("ticketClerkState",16, myIndex, TCNOT_BUSY);
		}

		/*acquire a lock to start interaction with customer TT game*/
		name = "ticketClerkLock";
		*(name + 15) =  buff[0];
		Acquire(name,16);
		/* this lock is acquired before releasing ticketClerkLineLock to avoid context switching problems*/

		/*as a customer to interact is found then we dont need this lock currently*/
		Release("tcLineLock", 10);
		
		name1 = "ticketClerkCV";
		*(name1 + 13) =  buff[0];
		/*wait for customer to come to my counter*/
		Wait(name1,14, name,16);

		if(GetMV("mgrMovieTechExit", 16, 0) == 1)
		{
			Exit(0);
		}

		/*calculate total amount to be paid by the customer, store it in monitor variable and share it with customer*/

		numOfTickets = GetMV("numberOfTicketsForTC",20, myIndex);
		SetMV("amt2TC", 6,myIndex, numOfTickets * 12);
		/*tell the customer the amount to be paid*/

		WriteToConsole("\nTicketClerk %d has an order for %d tickets and the cost is %d.\n\0",myIndex, numOfTickets, (numOfTickets * 12) );
		Signal(name1,14, name,16);

		/*wait for the customer to make payment*/
		Wait(name1,14, name,16);

		Acquire("tcManagerLock", 13);
		/*Customer made the payment, add it to the total ticket collection*/
		SetMV("totalAmtByTC",12, myIndex, (GetMV("totalAmtByTC",12, myIndex) + (numOfTickets*12) ));

		Release("tcManagerLock", 13);

		/*provide customer the tickets*/
		Signal(name1,14, name,16);

		/*as interaction is over, release the lock*/
		Release(name,16);
	}
}
