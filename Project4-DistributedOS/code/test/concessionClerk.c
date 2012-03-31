#include "syscall.h"
#define MAX_CC 5 /*Maximum number of TicketClerks*/

	enum CCSTATE {CCBUSY , CCNOT_BUSY};
	enum BREAKSTATE {ON_BREAK,OFF_BREAK};

	int concessionClerkLineLockId;		/*lock and CV to enter into concession clerk line*/
	int concessionClerkLineCVId[MAX_CC];
	int concessionClerkLineCountMVId;
	int concessionClerkStateMVId;
	int managerCustomerCCFlagMVId;

	int concessionClerkLockId[MAX_CC];	/*lock and CV to interact with concession clerk*/
	int concessionClerkCVId[MAX_CC];
	int numberOfPopcornForCCMVId;

	int numberOfSodaForCCMVId;
	int amt2CCMVId;
	int totalAmtByCCMVId;

	int concessionClerkManagerLockId;
	int concessionClerkManagerBreakLockId;
	int concessionClerkManagerBreakCVId[MAX_CC];	/*lock between manager and concession clerk to send him on breaks*/

	int concessionClerkIndexLockId;	/*lock between concession clerks to take their index after they are forked*/
	int nextCCIndexMVId;
	int CCBreakStateMVId;

	int mgrMovieTechExitMVId;

	int myIndex;

	char buff[1];
	char *name = "";
	char *name1 = "";
	int myints[20];
	int yourints[20];

	int myexp ( int count )
	{
	  int i, val=1;
	  for (i=0; i<count; i++ )
	  {
	    val = val * 10;
	  }
	  return val;
	}

	void itoa( char arr[], int size, int val )
	{
	  int i, max, dig, subval, loc;
	  for (i=0; i<size; i++ )
	  {
	    arr[i] = '\0';
	  }

	  for ( i=1; i<=2; i++ )
	  {
	    if (( val / myexp(i) ) == 0 )
	    {
	      max = i-1;
	      break;
	    }
	  }

	  subval = 0;
	  loc = 0;
	  for ( i=max; i>=0; i-- )
	  {
	    dig = 48 + ((val-subval) / myexp(i));
	    subval = (dig-48) * myexp(max);
	    arr[loc] = dig;
	    loc++;
	  }
	  return;
	}

	void CreateMVs()
	{
		nextCCIndexMVId = CreateMV("nextCCIndex", 11, 1);
		totalAmtByCCMVId = CreateMV("totalAmtByCC", 12, MAX_CC);
		amt2CCMVId = CreateMV("amt2CCMV", 8, MAX_CC);
		numberOfSodaForCCMVId = CreateMV("numberOfSodaForCC", 17, MAX_CC);
		numberOfPopcornForCCMVId = CreateMV("numberOfPopcornForCC", 20, MAX_CC);
		managerCustomerCCFlagMVId = CreateMV("managerCustomerCCFlag", 21, MAX_CC);
		CCBreakStateMVId = CreateMV("CCBreakState", 12, MAX_CC);
		concessionClerkStateMVId = CreateMV("ccState", 7, MAX_CC);
		concessionClerkLineCountMVId = CreateMV("ccLineCount", 11, MAX_CC);

		mgrMovieTechExitMVId = CreateMV("mgrMovieTechExit", 16, 1);
	}

	void CreateCCLockAndCV()
	{

		concessionClerkLineLockId = CreateLock("ccLineLock", 10);
		concessionClerkManagerBreakLockId = CreateLock("ccManagerBreakLock", 18);
		concessionClerkManagerLockId = CreateLock("ccManagerLock", 13); /* lock for updating the individual amount*/
		concessionClerkManagerBreakCVId[myIndex] = CreateCV("ccManagerBreakCV", 16);

		itoa(buff,1,myIndex);
		name = "ccLineCV";
		*(name + 8) =  buff[0];
		concessionClerkLineCVId[myIndex] = CreateCV(name, 9);

		name = "ccLock";
		*(name + 6) =  buff[0];
		concessionClerkLockId[myIndex] = CreateLock(name, 7);

		name = "ccCV";
		*(name + 4) =  buff[0];
		concessionClerkCVId[myIndex] = CreateCV(name, 5);

	}

int main()
{

	CreateMVs();
	concessionClerkIndexLockId = CreateLock("ccIndexLock", 11);

	Acquire("ccIndexLock", 11);	/*concession clerks acquire index lock to find their index*/
	myIndex = GetMV("nextCCIndex", 11, 0);
	SetMV("nextCCIndex", 11, 0, myIndex + 1);
	Release("ccIndexLock", 11);

	CreateCCLockAndCV();

	itoa(buff,1,myIndex);
	name = "ccManagerBreakCV";
	*(name + 16) =  buff[0];
	concessionClerkManagerBreakCVId[myIndex] = CreateCV(name, 17);


	while(1)
	{
		Acquire("ccManagerBreakLock", 18);		/*Acquire to see if manager has sent clerk on break*/
		Acquire("ccLineLock", 10);			/*Acquire LineLock to see if anybody is in line*/

		if(GetMV("CCBreakState", 12, myIndex) == ON_BREAK)	/*check if manager has told you to go on break*/
		{
			Release("ccLineLock", 10);
			WriteToConsole("\nConcessionClerk %d is going on break.\n\0",myIndex, -1, -1);
			Wait(name, 17, "ccManagerBreakLock", 18);	/*wait for manager to call you Off break*/
			Acquire("ccLineLock", 10);
		}
		else
		{
			SetMV("managerCustomerCCFlag", 21, myIndex, 0);
		}

		SetMV("ccState", 7, myIndex, CCNOT_BUSY);
		Release("ccManagerBreakLock", 18);	/*release the break lock acquired with manager*/


		if (GetMV("ccLineCount", 11, myIndex) > 0)
		{
			/*make yourself busy*/
			SetMV("ccState", 7, myIndex, CCBUSY);
			WriteToConsole("\nConcessionClerk %d has a line length %d and is signaling a customer.\n\0",myIndex, GetMV("concessionClerkLineCount", 24, myIndex), -1);

			  /* as I am going to service this customer, decrement line count by 1*/
			SetMV("ccLineCount", 11, myIndex, (GetMV("ccLineCount", 11, myIndex) - 1) );

			/*wake up the customer to be serviced*/

			name = "ccLineCV";
			*(name + 8) =  buff[0];
			Signal(name,9,"ccLineLock",10);
		}
		else
		{
			WriteToConsole("\nConcessionClerk %d has no one in line. I am available for a customer.\n\0",myIndex, -1, -1);
			/*no one in line so i am not busy, so make myself not busy*/
			SetMV("ccState", 7, myIndex, CCNOT_BUSY);
		}

		/*acquire a lock to start interaction with head customer TT game*/

		name = "ccLock";
		*(name + 6) =  buff[0];
		Acquire(name,7);
		/* this lock is acquired before releasing concessionClerkLineLock to aviod contex switching problems*/

		/*as a customer to interact is found then we dont need this lock currently*/
		Release("ccLineLock", 10);

		/*wait for customer to come to my counter and order number of popcorns*/

		name1 = "ccCV";
		*(name1 + 4) =  buff[0];
		/*wait for customer to come to my counter*/
		Wait(name1,5, name,7);

		if(GetMV("mgrMovieTechExit", 16, 0) == 1)
		{
			Exit(0);
		}

		/*calculate total amount to be paid by the customer, store it in monitor variable and share it with customer*/

		SetMV("amt2CCMV", 8, myIndex, (GetMV("numberOfPopcornForCC", 20, myIndex)) * 5);

		/*tell the customer to give the soda order*/
		Signal(name1,5, name,7);

		/*wait for customer to come to my counter and order number of sodas*/
		Wait(name1,5, name,7);

		/*calculate total amount to be paid by the customer, store it in monitor variable and share it with customer*/

		SetMV("amt2CCMV", 8, myIndex,  (GetMV("amt2CCMV", 8, myIndex)) + (GetMV("numberOfSodaForCC", 17, myIndex)) * 4);

		WriteToConsole("\nConcessionClerk %d has an order for %d popcorn and %d soda. The cost is %d.\n\0",
		(myIndex * 1000000) + (GetMV("numberOfPopcornForCC", 20, myIndex) * 10000) + (GetMV("numberOfSodaForCC", 17, myIndex) * 100) + ((5 * GetMV("numberOfPopcornForCC", 20, myIndex) + 4 * GetMV("numberOfSodaForCC", 17, myIndex))),
		-5, 4);
		/* 4  or more arguments so need to pass arguments using IInd method*/

		/*give customer the bill to be paid and the food order(soda and popcorn)*/
		Signal(name1,5, name,7);

		/*wait for the customer to make payment*/
		Wait(name1,5, name,7);

		Acquire("ccManagerLock", 13);
		/* Customer made the payment, add it to the total food collection in mutually exclusive manner*/

		SetMV("totalAmtByCC", 12, myIndex, (GetMV("totalAmtByCC", 12, myIndex)) + GetMV("amt2CCMV", 8, myIndex));

		Release("ccManagerLock", 13);
		/*added my amount to total food collection*/

		WriteToConsole("\nConcessionClerk %d has been paid for the order.\n\0",myIndex, -1, -1);

		/*as interaction is over, release the lock*/
		Release(name,7);
	}

}
