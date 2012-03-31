#include "syscall.h"

#define MAX_TT 3 /*Maximum number of TicketTakers*/

void ticketTaker();

enum TTWORKSTATE {TTBUSY , TTNOT_BUSY};
enum MOVIEMGRSTATE {STARTING, NOT_STARTING};
enum TTSTATE {NOT_ACCEPTING, ACCEPTING};

int ticketTakerLineLockId;	/*lock and CV to enter ticket taker's line*/
int ticketTakerLineCVId[MAX_TT];
/*int ticketTakerLineCount[MAX_TT] = {0};*/
/*enum TTWORKSTATE {TTBUSY , TTNOT_BUSY}
ticketTakerState[MAX_TT] = {TTBUSY, TTBUSY, TTBUSY}; */

/* int lineTicketMessage[MAX_TT]={0}; */

int ticketTakerLockId[MAX_TT];
int ticketTakerCVId[MAX_TT];	/*lock and cv to interact with ticket taker*/
/*int numberOfTicketsForTT[MAX_TT] = {0,0,0};
int acceptMessage[MAX_TT]={0,0,0};*/

int mgrTTCustomerCommonLockId;
int mgrTTCustomerCommonCVId;	
/* lock and CV between manager and ticket taker and customers to wait for manager to say when a movie is starting*/
/*enum MOVIEMGRSTATE {STARTING, NOT_STARTING}
movieManagerState = {NOT_STARTING};*/

int ttStateLockId;		/* lock between tickettakers, customers and manager to see if tickets are being accepted or not */
/*enum TTSTATE {NOT_ACCEPTING, ACCEPTING}
ttTicketAcceptState = NOT_ACCEPTING;
int totalAcceptedSeats = 0;
int seatFullMessage = 0;*/

int ticketTakerIndexLockId;	/*lock between ticket takers to take their index after they are forked*/
/*int nextTTIndex = 0;*/

/*--------------------------------*/
/**
*	New Entries for all MVs
*
*/
/* Ths is Array of MAX_TT */
int ticketTakerStateMVId;
int lineTicketMessageMVId;
int numberOfTicketsForTTMVId;
int acceptMessageMVId;
int ticketTakerLineCountMVId;
int mgrMovieTechExitMVId;

/* Only one entity */
int movieManagerStateMVId;
int ttTicketAcceptStateMVId;
int totalAcceptedSeatsMVId;
int seatFullMessageMVId;

int nextTTIndexMVId;

/*--------------------------------*/
/* Local Global Variable - Not Monitor Variable */
int myIndex;
char buff[1];
char *ticketTakerLineCVName = "ticketTakerLineCV";
char *ticketTakerCVName = "ticketTakerCV";
char *ticketTakerLockName = "ticketTakerLock";
/* itoa --------------- */

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

void randomYield(int waitCount)
{
	while(waitCount-- > 0)
	{
		Yield();
	}
}

/* --------------------*/

void ticketTaker()/*myIndex is used to identify the TT to interact with*/
{
	int temp = 0,temp1 = 0, temp2 = 0;

	while(1)
	{
			Acquire("mgrTTCustLock",13);	/*acquire common lock to see if manager has ask to start collecting tickets*/
			/*Acquire(ttStateLockId); */		/*acquire stateLock lock to see if seats are not full*/
			
			/*if(movieManagerState == NOT_STARTING || seatFullMessage == 1)	/*if seats are alrady full or manager has not yet told to collect tickets*/
			if(GetMV("movieManagerState",17,0) == NOT_STARTING)	/*if seats are alrady full or manager has not yet told to collect tickets*/
			{																/*then wait for managers command*/
				Acquire("ticketTakerLineLock",19);
				SetMV("ticketTakerLineCount",20,myIndex,0);			/*initially clerking the line count*/
				SetMV("lineTicketMessage",17,myIndex,1);				/*make lineTicket message as not accepting*/
				Broadcast(ticketTakerLineCVName,18,"ticketTakerLineLock",19);	/*broadcast to everyone in line*/
				Release("ticketTakerLineLock",19);
				
				/*Release(ttStateLockId);*/
				Wait("mgrTTCustCV",11,"mgrTTCustLock",13);	/*wait here fpr managers command*/
				Release("mgrTTCustLock",13);	/*release interaction lock with manager*/
				Acquire("ticketTakerLineLock",19);
					SetMV("lineTicketMessage",17,myIndex,0);		/*after getting command to start collecting tickets, set line Message as tickets are being accepted*/
				Release("ticketTakerLineLock",19);
				SetMV("ticketTakerState",16,myIndex,TTNOT_BUSY);
			}
			else
			{
				/*Release(ttStateLockId);*/	/*release the lock as it is not required*/
				Release("mgrTTCustLock",13);	/*release interaction lock with manager*/
			}
			

			Acquire("ticketTakerLineLock",19);

			if (GetMV("ticketTakerLineCount",20,myIndex) > 0)/* Yes I have customers*/
			{
				Acquire("ttStateLock",11);	/*acquire lock to see if seats are full or not*/
				if(GetMV("seatFullMessage",15,0) == 1)	/* check if seats are available in movie hall*/
				{
					SetMV("lineTicketMessage",17,myIndex,1);	/*if seats are no more available, then set line message as not accepting*/

					/*recent change*/
					SetMV("ticketTakerLineCount",20,myIndex,0);	/*clear line count */
					
					Broadcast(ticketTakerLineCVName,18, "ticketTakerLineLock",19);	/*inform all customers in line to go back to lobby*/
					
					/*recent change*/
					/*ticketTakerState[myIndex] = TTNOT_BUSY;*/
					
					Release("ticketTakerLineLock",19);	/*release lock after infroming all customers*/
					
					Release("ttStateLock",11);	/*releasse lock acuired to see if seats are full or not*/
					
					continue;
				}
				else
				{	
					/* tickets are being accepted at this point, so start interaction with the first customer in line*/
					/*make myself busy*/
					SetMV("ticketTakerState",16,myIndex,TTBUSY);
					WriteToConsole("\nTicketTaker %d has a line length %d and is signaling a customer.\n\0",myIndex, GetMV("ticketTakerLineCount",20,myIndex), -1);
					temp = GetMV("ticketTakerLineCount",20,myIndex);
					SetMV("ticketTakerLineCount",20,myIndex,temp - 1);/* as I am going to service this customer, decrement line count by 1*/

					/*wake up the customer to be serviced*/
					SetMV("lineTicketMessage",17,myIndex,0);
					Signal(ticketTakerLineCVName,18, "ticketTakerLineLock",19);		/*call the first customer in your line*/
					Release("ttStateLock",11);
				}
				
			}
			else
			{
				WriteToConsole("\nTicketTaker %d has no one in line. I am available for a customer.\n\0",myIndex, -1, -1);
				/*no one in line so i am not busy, so make myself not busy*/
				SetMV("ticketTakerState",16,myIndex,TTNOT_BUSY);
			}

			/*acquire a lock to start interaction with head customer TT game*/

			Acquire(ticketTakerLockName,16);
			/* this lock is aacquired before releasing ticketTakerLineLock to aviod contex switching problems*/

			/*as a customer to interact is found then we dont need this lock currently*/
			Release("ticketTakerLineLock",19);

			/*wait for customer to come to my counter and order number of popcorns*/
			Wait(ticketTakerCVName,14, ticketTakerLockName,16);

			if(GetMV("mgrMovieTechExit", 16, 0) == 1)
			{
				Exit(0);
			}

			WriteToConsole("\nTicketTaker %d has received %d tickets.\n\0",myIndex, GetMV("numberOfTicketsForTT",20,myIndex), -1);

			Acquire("ttStateLock",11);	/*again check if seats are being accepted or not*/
			if(GetMV("seatFullMessage",15,0) == 0)
			{
				temp = GetMV("totalAcceptedSeats",18,0);
				temp1 = GetMV("numberOfTicketsForTT",20,myIndex);
				if( temp + temp1 <= 25)	/*if this group can be accommodated then accept tickets or else reject it*/
				{ 
					/* totalAcceptedSeats += numberOfTicketsForTT[myIndex]; */
					SetMV("totalAcceptedSeats",18,0,temp + temp1);
					WriteToConsole("\nTicketTaker %d is allowing the group into the theater. The number of tickets taken is %d.\n\0",myIndex,GetMV("numberOfTicketsForTT",20,myIndex), -1);

					SetMV("acceptMessage",13,myIndex,0);	/*message set to 0 to inform customer that his tickets have been accepted*/
					Signal(ticketTakerCVName,14,ticketTakerLockName,16);	/*signal customer that the tickets have been accepted*/
					Release(ticketTakerLockName,16);	/* release interaction lock with customer*/

					if(temp == 25)
					{
						SetMV("seatFullMessage",15,0,1); /*if the theater is full after accepting this group then inform customer to go to lobby and wait*/
						Release("ttStateLock",11);	/*release statelock*/
						Acquire("ticketTakerLineLock",19);
						SetMV("ticketTakerLineCount",20,myIndex,0);		/*clear line count*/
						SetMV("lineTicketMessage",17,myIndex,1);			/*set line message as ticket not being accepted*/
						Broadcast(ticketTakerLineCVName,18, "ticketTakerLineLock",19);
						Release("ticketTakerLineLock",19);
						WriteToConsole("\nTicketTaker %d has stopped taking tickets.\n\0",myIndex, -1, -1);
					}
					else
					{
						Release("ttStateLock",11);	/*release lock transaction was successful*/
					}
				}
				else
				{
					SetMV("seatFullMessage",15,0,1);	/*if the theater is full after accepting this group then inform customer to go to lobby and wait*/
					Release("ttStateLock",11);		/*release line lock*/
					SetMV("acceptMessage",13,myIndex,1);
					Signal(ticketTakerCVName,14, ticketTakerLockName,16);	/*inform customer that his tickets has been rejected*/
					Release(ticketTakerLockName,16);	/*release interaction lock with customer*/

					Acquire("ticketTakerLineLock",19);	/*acquire line lock to inform others in line*/
					SetMV("ticketTakerLineCount",20,myIndex,0);	/*clear line count*/
					SetMV("lineTicketMessage",17,myIndex,1);		/*set line message as ticket not being accepted*/
					Broadcast(ticketTakerLineCVName,18, "ticketTakerLineLock",19);
					Release("ticketTakerLineLock",19);
					WriteToConsole("\nTicketTaker %d is not allowing the group into the theater. The number of tickets taken is %d and the group size is %d.\n\0",myIndex, GetMV("totalAcceptedSeats",18,0), GetMV("numberOfTicketsForTT",20,myIndex));
					WriteToConsole("\nTicketTaker %d has stopped taking tickets.\n\0",myIndex, -1, -1);
					continue;
				}
			}
			else
			{
				
				SetMV("acceptMessage",13,myIndex,1);	/*if the theater is full after accepting this group then inform customer to go to lobby and wait*/
				Release("ttStateLock",11);		/*release line lock*/
				Signal(ticketTakerCVName,14, ticketTakerLockName,16);	/*inform customer that his tickets has been rejected*/
				Release(ticketTakerLockName,16);	/*release interaction lock with customer*/

				Acquire("ticketTakerLineLock",19);		/*acquire line lock to inform others in line*/
				SetMV("lineTicketMessage",17,myIndex,1);			/*set line message as ticket not being accepted*/
				SetMV("ticketTakerLineCount",20,myIndex,0);		/*clear line count*/
				Broadcast(ticketTakerLineCVName,18, "ticketTakerLineLock",19);
				WriteToConsole("\nTicketTaker %d is not allowing the group into the theater. The number of tickets taken is %d and the group size is %d.\n\0",myIndex,GetMV("totalAcceptedSeats",18,0), GetMV("numberOfTicketsForTT",20,myIndex));
				WriteToConsole("\nTicketTaker %d has stopped taking tickets.\n\0",myIndex,-1, -1);
				Release("ticketTakerLineLock",19);
				continue;
			}
			randomYield(20);	/*was a trial but important one*/
		}
}

void CreateMonitors(){
	/* Ths is Array of MAX_TT */
	ticketTakerStateMVId = CreateMV("ticketTakerState",16,MAX_TT);
	lineTicketMessageMVId = CreateMV("lineTicketMessage",17,MAX_TT);
	numberOfTicketsForTTMVId = CreateMV("numberOfTicketsForTT",20,MAX_TT);
	acceptMessageMVId = CreateMV("acceptMessage",13,MAX_TT);
	ticketTakerLineCountMVId = CreateMV("ticketTakerLineCount",20,MAX_TT);;

	/* Only one entity */
	movieManagerStateMVId = CreateMV("movieManagerState",17,1);
	ttTicketAcceptStateMVId = CreateMV("ttTicketAcceptState",19,1);
	totalAcceptedSeatsMVId = CreateMV("totalAcceptedSeats",18,1);
	seatFullMessageMVId = CreateMV("seatFullMessage",15,1);

	nextTTIndexMVId = CreateMV("nextTTIndex",11,1);

	mgrMovieTechExitMVId = CreateMV("mgrMovieTechExit", 16, 1);
}

int main(){
	int i;
	
	CreateMonitors();

	mgrTTCustomerCommonLockId = CreateLock("mgrTTCustLock", 13);
	mgrTTCustomerCommonCVId = CreateCV("mgrTTCustCV", 11);
	ttStateLockId = CreateLock("ttStateLock", 11);

	ticketTakerLineLockId = CreateLock("ticketTakerLineLock", 19);
	SetMV("ttTicketAcceptState",19,0,NOT_ACCEPTING);

	ticketTakerIndexLockId = CreateLock("ticketTakerIndexLock", 20);
	
	Acquire("ticketTakerIndexLock",20);
		myIndex = GetMV("nextTTIndex",11,0);
		SetMV("nextTTIndex",11,0,myIndex + 1);
	Release("ticketTakerIndexLock",20);
	
	itoa(buff,1,myIndex);
	/* *(name + 25) =  buff[0];*/
	*(ticketTakerLineCVName + 17) = buff[0];  /* 18 */
	*(ticketTakerCVName + 13) = buff[0];   /* 14 */
	*(ticketTakerLockName + 15) = buff[0]; /* 16 */
	
	
	/*create all locks and CV for ticket taker */
		ticketTakerLineCVId[myIndex] = CreateCV(ticketTakerLineCVName, 18);
		ticketTakerLockId[myIndex] = CreateLock(ticketTakerLockName, 16);
		ticketTakerCVId[myIndex] = CreateCV(ticketTakerCVName, 14);
		ticketTaker();
		/*ticketTakerState[myIndex] = TTBUSY;*/
}



