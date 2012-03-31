#include "syscall.h"

#define MAX_TC 5 /*Maximum number of TicketClerks*/
#define MAX_CC 5 /*Maximum number of ConcessionClerks*/
#define MAX_TT 3 /*Maximum number of TicketTakers*/
#define MAX_GROUPS 8	/* Maximum number of Customer Groups*/
struct CustomerData	/*Customer Data Structure*/
{
	int customerNumber;
	int groupNumber;
	int groupSize; /*This value is used by the group head*/
	int mySeat;
};

void customersExit(struct CustomerData *myData);
void interactionForTicketsWithTC(struct CustomerData * current);
void interactWithTicketTakerToGiveTickets(struct CustomerData *current);
void interactWithConcessionClerksToOrderFood(struct CustomerData *current);
void interactWithGroupMembersForFoodOrder(struct CustomerData *myData);
void allocateSeats(int totalMemCount,int grpNo);
void interactWithGroupMembersForSeatAllocation(struct CustomerData *myData);
void movieSeatAllocation(struct CustomerData *myData);
void watchMovieAndExit(struct CustomerData *myData);

int movieSeatManagerLockId;
int customerManagerExitLockId;
int movieSeatLockId;
int totalOccupiedSeatsMVId;
int seatsMVId;
int customerNumberIndexLockId;


int totalMovieRoomExitCountMVId;

int ticketClerkLineLockId;	/* For mutual exclusion to enter Ticket Taker Line*/
int ticketClerkLineCVId[MAX_TC];	/*To wait on the line lock to start interaction with customer*/

enum TCSTATE {TCBUSY,TCNOT_BUSY};
enum BREAKSTATE {ON_BREAK,OFF_BREAK};
enum ORDERSTATE {ORDER_NOTRECEIVED,ORDER_RECEIVED};
enum SUBMITSTATE {TICKETS_ACCEPTED,TICKETS_NOTACCEPTED};
enum CUSTOMERMOVIEWAITINGSTATE {NOT_WAITING, WAITING};
enum GROUPSTATE {REGROUPED, NOT_REGROUPED};
enum SEATSTATE {SEATS_FREE, SEATS_OCCUPIED};
enum MOVIESTATE {OVER, NOT_OVER, FIRST_MOVIE};

enum CCSTATE {CCBUSY , CCNOT_BUSY};

int ticketClerkLockId[MAX_TC];	/*A lock between TicketClerk and HeadCustomer for mutual exclusion during the interaction*/
int ticketClerkCVId[MAX_TC];	/*CV to wait on a TicketClerk lock to begin interaction*/

int groupTicketLockId[MAX_GROUPS];	/*lock for interaction with group members after receiving tickets*/
int groupTicketLockCVId[MAX_GROUPS];	/*CV for group members to wait for Head customer*/
enum TICKETSTATUS {BOOKED, NOT_BOOKED};

int concessionClerkLineLockId;
int concessionClerkLineCVId[MAX_CC];
int concessionClerkLineCount[MAX_CC] = {0,0,0,0,0};
int concessionClerkLockId[MAX_CC];	/*lock and CV to interact with concession clerk*/
int concessionClerkCVId[MAX_CC];

int ticketTakerLineLockId;
int ticketTakerLineCVId[MAX_TT];
int ticketTakerLineCount[MAX_TT] = {0,0,0};
int ticketTakerLockId[MAX_TT];	/*lock and CV to interact with concession clerk*/
int ticketTakerCVId[MAX_TT];

enum TTSTATE {NOT_ACCEPTING, ACCEPTING};
int mgrTTCustomerCommonLockId;
int movieManagerStateMVId;
int mgrTTCustomerCommonCVId;

enum TTWORKSTATE {TTBUSY , TTNOT_BUSY};

int ticketTakerStateMVId;
int lineTicketMessageMVId;
int numberOfTicketsForTTMVId;
int acceptMessageMVId;

enum MOVIEMGRSTATE {STARTING, NOT_STARTING};

int nextHCIndexLockId;
int hasTicketsLockId;
int numberOfGroupsMVId;
int nextHCIndexMVId;
int groupMembersCountMVId;

int ticketClerkBreakStateMVId;
int ticketClerkStateMVId;
int ticketClerkLineCountMVId;
int managerCustomerTCFlagMVId;
int numberOfTicketsForTCMVId;


int ccBreakStateMVId;
int concessionClerkStateMVId;
int concessionClerkLineCountMVId;
int managerCustomerCCFlagMVId;
int numberOfPopcornForCCMVId;
int numberOfPopcorn;
int numberOfSodaForCCMVId;
int numberOfSoda;

int remainingCountMVId;
int numberOfSodaMVId;
int numberOfPopcornMVId;
int groupMemberCountForFoodMVId;
int hasTicketsMVId;
int allCustomerExitedCountMVId;		/* lock for manager to check if all customers who entered have exited the theater , if yes then stop the simulation*/
int totalCustomerInCountMVId;
int hasOrderedMVId;
int groupFoodWaitFlagMVId;
int hasSubmittedMVId;
int groupTicketSubmitFlagMVId;
int groupMemberCountForSeatsMVId;
int movieFinishStateMVId;
int movieTechStateMVId;
int groupExitCountMVId;
int groupRegroupStateMVId;
int groupTheaterExitCountMVId;
int groupMemCountMVId;
int groupMembersCountMVId;
int memberCountForGroupMembersMVId;
int groupRegroupFlagMVId;

int myGroupNumber;
int groupSize;
int myGroupSize;

int myOccupiedSeats[MAX_GROUPS][5]= {-1};

/*Take Food Order from Group members by Head Customer*/

int groupFoodLockId[MAX_GROUPS], groupFoodLockCVId[MAX_GROUPS];

int groupTicketLockId[MAX_GROUPS];	/*lock for interaction with group members after receiving tickets*/
int groupTicketLockCVId[MAX_GROUPS];	/*CV for group members to wait for Head customer*/

/*For mutual exclusion so that only one member customer can specify food order to Head Customer*/
int intraGroupFoodLockId[MAX_GROUPS];

int customerManagerExitLockId;

/*Food Ordered or not ordered communication between Head Customer and group members*/
int groupFoodWaitLockId[MAX_GROUPS];
int groupFoodWaitCVId[MAX_GROUPS];	/*lock and CV to wait for head customer to order food*/

int groupTicketSubmitLockId[MAX_GROUPS];
int groupTicketSubmitCVId[MAX_GROUPS];	/*lock and CV for group members to wait for Head customer to submit tickets to ticket taker*/

int groupSeatWaitLockId[MAX_GROUPS];	/*lock and CV to wait for Head Customer to allocate seats to his group members*/
int groupSeatWaitCVId[MAX_GROUPS];

int movieTechLockId;
int movieTechCVId;

int groupExitLockId[MAX_GROUPS];
int groupExitCVId[MAX_GROUPS];	/*lock and CV for group members to wait for manager to tell them to exit movie room*/

int groupRegroupLockId[MAX_GROUPS];
int groupRegroupCVId[MAX_GROUPS];

int groupTheaterExitLockId[MAX_GROUPS];
int groupTheaterExitCVId[MAX_GROUPS];	/*lock and CV for group members to wait for manager to tell them to exit theater*/

char buff[1];
char buff1[1];
char buff2[1];
char buff3[1];
char buff4[1];
char *name = "";
char *name1 = "";		
char *name2 = "";
char *name3 = "";
char *name4 = "";
char *name5 = "";
char *name6 = "";
char *name7 = "";
char *name8 = "";

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
/*===========================================================*/
/*	FUNCTION: Interaction of HeadCustomer with Ticket Taker*/
/*				to submit tickets*/
/*===========================================================*/
void interactWithTicketTakerToGiveTickets(struct CustomerData *current)
{
	int myTicketTaker = -1, i, shortestTTLine, shortestTTLineLength, ttlineCnt;
	myGroupSize = current -> groupSize;

	while(myTicketTaker == -1)
	{
			Acquire("mgrTTCustLock",13); /*acquire managers lock to check if manager has told that a movie is starting*/
			if(GetMV("movieManagerState",17,0) == NOT_STARTING)	/*if not accepting then wait for managers signal for next movie*/
			{
				Wait("mgrTTCustCV",11, "mgrTTCustLock",13);
				Release("mgrTTCustLock",13);
			}
			else															/* tickets are being accepted, so release the locks which others can use*/
			{
				Release("mgrTTCustLock",13);
			}

			/*HeadCustomer gets in line here*/
			Acquire("ticketTakerLineLock",19);

			/*Check if any TicketTaker is not busy*/
			for(i = 0; i < MAX_TT; i++)
			{
				if(GetMV("ticketTakerState",16,i) == TTNOT_BUSY)
				{
					/*found a free TT, so make it your TicketTaker*/
					myTicketTaker = i;
					itoa(buff2,1,myTicketTaker);
					SetMV("ticketTakerState",16,i,TTBUSY);
					break;
				}
			}

			/*If all are on break - condition has to be used*/

			/*If no free TT available then check for shortest line of TT*/
			if(myTicketTaker == -1)
			{
				/*nHeadCustomerdid not find free TT*/
				shortestTTLine = 0; /*by default the shortest line is the 1st one*/
				shortestTTLineLength = GetMV("ticketTakerLineCount",20,0);
				for(i = 1; i < MAX_TT; i++)
				{
					if (shortestTTLineLength > GetMV("ticketTakerLineCount",20,i) && (GetMV("ticketTakerState",16,i) == TTBUSY))
					{
						/*nHeadCustomer found shortest TicketTakerLine*/
						shortestTTLineLength = GetMV("ticketTakerLineCount",16,i);
						shortestTTLine = i;
					}
				}
				/* I have the shortest line*/
				myTicketTaker = shortestTTLine;
				itoa(buff2,1,myTicketTaker);

				/*get in that line*/
				WriteToConsole("\nCustomer %d in Group %d is getting in TicketTaker line %d.\n\0",current->customerNumber, current->groupNumber,myTicketTaker);
				
				if(GetMV("lineTicketMessage",17,myTicketTaker) == 1)	/*before getting in that line check if tickets are being accepted in that line*/
				{
					WriteToConsole("\nCustomer %d in Group %d sees TicketTaker %d is no longer taking tickets. Going to the lobby.\n\0",current->customerNumber, current->groupNumber,myTicketTaker);
					
					WriteToConsole("\nCustomer %d in Group %d is in the lobby.\n\0",current->customerNumber, current->groupNumber, -1);
					Release("ticketTakerLineLock",19);
					myTicketTaker = -1;
					Acquire("mgrTTCustLock",13);
					Wait("mgrTTCustCV",11, "mgrTTCustLock",13);	/*if not accepting then wait for managers signal for next movie*/
					Release("mgrTTCustLock",13);

					WriteToConsole("\nCustomer %d in Group %d is leaving the lobby.\n\0",current->customerNumber, current->groupNumber, -1);
					continue;	/*got signal from manager now start again*/
				}
				else
				{
					ttlineCnt = GetMV("ticketTakerLineCount",20,myTicketTaker);
					ttlineCnt++;
					/*get into the line*/
					SetMV("ticketTakerLineCount",20,myTicketTaker,ttlineCnt);

					name4 = "ticketTakerLineCV";
					*(name4+17) = buff2[0];
					
					/*wait for your turn*/
					Wait(name4,18, "ticketTakerLineLock",19);
				}

				if(GetMV("lineTicketMessage",17,myTicketTaker) == 1)	/*before starting interaction with TicketTaker, again check if tickets are being accepted*/
				{
					/*if not then go to lobby and wait for managers signal for next movie*/
					WriteToConsole("\nCustomer %d in Group %d sees TicketTaker %d is no longer taking tickets. Going to the lobby.\n\0",current->customerNumber, current->groupNumber,myTicketTaker);
					WriteToConsole("\nCustomer %d in Group %d is in the lobby.\n\0",current->customerNumber, current->groupNumber, -1);

					Release("ticketTakerLineLock",19);	/*release line lock before going to lobby*/
					myTicketTaker = -1;

					Acquire("mgrTTCustLock",13);
					Wait("mgrTTCustCV",11, "mgrTTCustLock", 13);
					Release("mgrTTCustLock",13);	/*received signal here, so leave lobby and start over again*/
					WriteToConsole("\nCustomer %d in Group %d is leaving the lobby.\n\0",current->customerNumber, current->groupNumber, -1);
					continue;
				}
			}

			/* At this point tickets are being accepted*/

			/*release lock as I have come into the shortest line*/
			Release("ticketTakerLineLock",19);
			WriteToConsole("\nCustomer %d in Group %d is walking up to TicketTaker %d to give %d tickets.\n\0",
			(((current->customerNumber) * 1000000) + ((current->groupNumber) * 10000) + (myTicketTaker * 100) + myGroupSize), -5, 4);	/* 4  or more arguments so need to pass arguments using IInd method*/
			
			name4 = "ticketTakerLock";
			*(name4 + 15) = buff2[0];
			/*acquire then new lock to start interaction with Concession clerk*/
			Acquire(name4,16);

			/*pass no. of tickets and wake up Ticket Taker*/
			SetMV("numberOfTicketsForTT",20,myTicketTaker,myGroupSize);

			name5 = "ticketTakerCV";
			*(name5 + 13) = buff2[0];
			/*signal the no. of tickets*/
			Signal(name5,14, name4 , 16);

			/*wait for acknowledgement from TicketTaker to see if Ticket Accepted or not*/
			Wait(name5,14, name4 , 16);

			if(GetMV("acceptMessage",13,myTicketTaker) == 1)	/*if tickets are not accepted then go to lobby and wait for managers signal for next movie*/
			{
				
				WriteToConsole("\nCustomer %d in Group %d sees TicketTaker %d is no longer taking tickets. Going to the lobby.\n\0",current->customerNumber, current->groupNumber,myTicketTaker);
				WriteToConsole("\nCustomer %d in Group %d is in the lobby.\n\0",current->customerNumber, current->groupNumber, -1);

				Release(name4, 16);
				myTicketTaker = -1;

				/*wait here for managers signal*/
				Acquire("mgrTTCustLock",13);
				Wait("mgrTTCustCV",11, "mgrTTCustLock",13);

				/*HeadCustomer receives signal Manager that the next movie is starting*/
				Release("mgrTTCustLock",13);

				/*got the signal, now start over again*/
				WriteToConsole("\nCustomer %d in Group %d is leaving the lobby.\n\0",current->customerNumber, current->groupNumber, -1);
				continue;
			}

			/*Tickets are accepted here so release the lock and proceed to Movie hall*/

			WriteToConsole("\nCustomer %d in Group %d is leaving TicketTaker %d.\n\0",current->customerNumber, current->groupNumber,myTicketTaker);
			Release(name4,16);
		}
}
/*------------------------------------------------end of interaction with TicketTaker--------------------------------------------------*/

/*===========================================================*/
/*	FUNCTION: Interaction of HeadCustomer Group Members to take*/
/*				Food order from each member*/
/*===========================================================*/
void interactWithGroupMembersForFoodOrder(struct CustomerData *myData)
{
	int temp_popcorn, temp_soda, random_popcorn_order, random_soda_order;
	/*myGroupNumber = myData -> groupNumber;*/

	itoa(buff3,1,myGroupNumber);
	while(GetMV("groupMemberCountForFood",23,myGroupNumber) > 1)	/*Interact with all members*/
	{

		name6 = "groupFoodLockCV";
		*(name6 + 15) = buff3[0];
		
		name7 = "groupFoodLock";
		*(name7 + 13) = buff3[0];
		/*waiting for order from every group member*/
		Wait(name6,16, name7,14);
		/*group member placed order so decrement count and wait for next group member to tell the food order*/
	}

	/*order soda and popcorn [handle condition if I want to order or not]*/
	temp_popcorn = 0;
	temp_soda = 0;
	random_popcorn_order = Random(100);	/*random value for popcorn between 0-99*/
	if(random_popcorn_order > 25)				/* 75 percent chance of order*/
	{
		temp_popcorn = 1;
	}
	else
	{
		temp_popcorn = 0;
	}

	random_soda_order = Random(100);		/*random value for soda between 0-99*/
	if(random_soda_order > 25)					/* 75 percent chance of order*/
	{
		temp_soda = 1;
	}
	else
	{
		temp_soda = 0;
	}
	WriteToConsole("\nCustomer %d in Group %d wants %d popcorn and %d soda.\n\0", 
	(((myData->customerNumber) * 1000000) + ((myGroupNumber) * 10000) +  ((temp_popcorn) * 100) + temp_soda), -5, 4);	/* 4  or more arguments so need to pass arguments using IInd method*/
	
	SetMV("numberOfPopcorn", 15, myGroupNumber, GetMV("numberOfPopcorn", 15, myGroupNumber) + temp_popcorn);
	SetMV("numberOfSoda", 12, myGroupNumber, GetMV("numberOfSoda", 12, myGroupNumber) + temp_soda);
	
	/*release food lock as the order taken from group members*/
	Release(name7,14);

	interactWithConcessionClerksToOrderFood(myData);

	name6 = "groupFoodWaitLock";
	*(name6+17) = buff3[0];
	/*Acquire lock to inform Group members about Food Order is complete*/
	Acquire(name6,18);

	name7 = "groupFoodWaitCV";
	*(name7+15) = buff3[0];
	/*Order Received, hence change the status for Group members reference*/
	SetMV("hasOrdered",10,myGroupNumber,ORDER_RECEIVED);

	if(GetMV("groupFoodWaitFlag",17,myGroupNumber) == 1)
	{
	/*	HeadCustomer informs all its waiting group member about Food order completion*/
		Broadcast(name7,16,name6,18);
	}
	/*HeadCustomer releases food Wait lock with members of Group*/
	Release(name6, 18);
	
	interactWithTicketTakerToGiveTickets(myData);	/* got the order from members now interact with COncessionClerk*/

	name6 = "groupTSLock";
	*(name6+11) = buff3[0];
	
	name7 = "groupTSCV";
	*(name7+9) = buff3[0];
	/*Acquire lock to inform Group members about Ticket Submitted is complete*/
	Acquire(name6,12);

	/*Tickets Submitted, hence change the status for Group members reference*/
	SetMV("hasSubmitted",12,myGroupNumber,TICKETS_ACCEPTED);

	if(GetMV("groupTicketSubmitFlag",21,myGroupNumber) == 1)
	{
		/*nHeadCustomer informs all its waiting group member about Ticket Submitted to TicketTaker*/
		Broadcast(name7,10,name6,12);
	}
	
	name8 = "groupSeatWaitLock";
	*(name8 + 17) = buff3[0];
	/*nHeadCustomer acquires groupseatwaitlock to interact about seats with members of Group*/
	Acquire(name8,18);
	
	Release(name6,12);
	/*HeadCustomer[%d] releases ticket submit lock with members of Group*/
}

/*==============================================================*/
/*	FUNCTION: Interaction of HeadCustomer with Concession Clerk*/
/*				to order Food for his entire group*/
/*==============================================================*/
void interactWithConcessionClerksToOrderFood(struct CustomerData *current)
{
	int myConcessionClerk = -1, i, shortestCCLine, shortestCCLineLength,ccLineCount;

	myGroupSize = current -> groupSize;

	/*search a Concession Clerk to order food*/
	while(myConcessionClerk == -1)
	{
		/*acquired ConcessionClerk line lock*/
		Acquire("ccLineLock",10);
		for(i = 0; i < MAX_CC; i++)	/*Check if any concessionClerk is not busy*/
		{
			if(GetMV("CCBreakState",12,i) == ON_BREAK)
			{
				WriteToConsole("\nCustomer %d in Group %d sees ConcessionClerk %d is on break.\n\0",current->customerNumber, current->groupNumber,i);
			}
			if(GetMV("ccState",7,i) == CCNOT_BUSY && GetMV("CCBreakState",12,i) == OFF_BREAK)
			{
				/*found a free CC, so make it your concession clerk*/
				myConcessionClerk = i;
				itoa(buff1,1,myConcessionClerk);
				SetMV("ccState",7,i,CCBUSY);
				break;
			}
		}
		/*If all are on break - condition has to be used*/

		/*If no free CC available then check for shortest line of CC*/
		if(myConcessionClerk == -1)
		{
			/*HeadCustomer did not find free CC*/
			/*shortestCCLine = 0; *//*by default the shortest line is the 1st one*/

			/*	shortestCCLineLength = concessionClerkLineCount[0];	*/
			shortestCCLineLength = 5000;
			for(i = 0; i < MAX_CC; i++)
			{
				if ((shortestCCLineLength >= GetMV("ccLineCount",11,i)) && (/*concessionClerkState[i] == CCBUSY &&*/ GetMV("CCBreakState",12,i) == OFF_BREAK))
				{
					/*found shortest ConcessionClerk line so get in that line*/
					shortestCCLineLength = GetMV("ccLineCount",11,i);
					shortestCCLine = i;
				}
			}
			
				myConcessionClerk = shortestCCLine;
				itoa(buff1,1,myConcessionClerk);
				/*get into the line*/
				ccLineCount = GetMV("ccLineCount",11,i);
				ccLineCount++;
				SetMV("ccLineCount",11,i,ccLineCount);
				/*waiting in ConcessionClerk line for your turn*/
				WriteToConsole("\nCustomer %d in Group %d is getting in ConcessionClerk line %d \n\0", current->customerNumber, current->groupNumber, myConcessionClerk);
				
				name2 = "ccLineCV";
				*(name2+8) = buff1[0];
				Wait(name2,9, "ccLineLock",10);

				if(GetMV("managerCustomerCCFlag",21,myConcessionClerk) == 1 || GetMV("managerCustomerCCFlag",21,myConcessionClerk) ==  2)
				{
					if(GetMV("managerCustomerCCFlag",21,myConcessionClerk) == 2)
					{
						SetMV("managerCustomerCCFlag",21,myConcessionClerk,0);
					}
					myConcessionClerk = -1;
					Release("ccLineLock",10);
					continue;
				}
			}

		/*Release the line lock as you have shortest line*/
		Release("ccLineLock",10);
	}
	name2 = "ccLock";
	*(name2 + 6) =  buff1[0];
	*(name2 + 7) = '\0';
	/*acquire then new lock to start interaction with Concession clerk*/
	Acquire(name2,7);

	/*pass popcorn order and wake up clerk*/
	SetMV("numberOfPopcornForCC",20,myConcessionClerk,GetMV("numberOfPopcorn",15,current -> groupNumber));

	name3 = "ccCV";
	*(name3 + 4) =  buff1[0];
	/*signal the ConcessionClerk regarding no. of popcorns*/
	Signal(name3,5,name2,7);

	/*wait for ConcessionClerk to give acknowledgement for popcorn order*/
	Wait(name3,5,name2,7);

	/*pass no. of sodas and wake up clerk*/
	SetMV("numberOfSodaForCC",17,myConcessionClerk,GetMV("numberOfSoda",15,current -> groupNumber));

	WriteToConsole("\nCustomer %d in Group %d is walking up to ConcessionClerk %d to buy %d popcorn and %d soda.\n\0", 
	(((current -> customerNumber) * 100000000) + ((current->groupNumber) * 1000000) +  ((myConcessionClerk) * 10000) + 
	((GetMV("numberOfPopcornForCC",20,myConcessionClerk)) * 100) + GetMV("numberOfSodaForCC",17,myConcessionClerk)), -5, 5);	/* 5 arguments*/

	/*tell the ConcessionClerk regarding no. of soda*/
	Signal(name3,5,name2,7);

	/*wait for ConcessionClerk to give acknowledgement for soda order*/
	Wait(name3,5,name2,7);

	WriteToConsole("\nCustomer %d in Group %d in ConcessionClerk line %d is paying %d for food.\n\0", 
	( ((current->customerNumber) * 1000000) +((current->groupNumber) * 10000) + ((myConcessionClerk * 100)) + (5 * GetMV("numberOfPopcornForCC",20,myConcessionClerk) +  4 * GetMV("numberOfSodaForCC",17,myConcessionClerk))), -5, 4);	/* 4  or more arguments so need to pass arguments using IInd method*/

	/*pay the amount by signalling the CC*/
	Signal(name3,5,name2,7);

	WriteToConsole("\nCustomer %d in Group %d is leaving ConcessionClerk %d.\n\0", current->customerNumber, current->groupNumber, myConcessionClerk);
	Release(name2,7);
	/*------------------------------------------------end of interaction with ConcessionClerk--------------------------------------------------*/
}

/*==============================================================*/
/*	FUNCTION: Interaction of HeadCustomer with Ticket Clerk*/
/*				to order Tickets for his entire group*/
/*==============================================================*/			
void interactionForTicketsWithTC(struct CustomerData * current)
{
	int i, myTicketClerk = -1, shortestTCLineLength, shortestTCLine,linecnt;
	
	myGroupSize = current -> groupSize;
	while(myTicketClerk == -1)
	{
		/*HeadCustomer acquired line lock*/
		Acquire("tcLineLock", 10);

		/*Check if any TicketClerk is not busy*/
		for(i = 0; i < MAX_TC; i++)
		{
			if(GetMV("TCBreakState",12,i) == ON_BREAK)
			{
				WriteToConsole("\nCustomer %d in Group %d sees TicketClerk %d is on break\n\0", current->customerNumber, current->groupNumber, i);
				continue;
			}
			/*if(ticketClerkState[i] == TCNOT_BUSY)*/
			if(GetMV("ticketClerkState",16,i) == TCNOT_BUSY && GetMV("TCBreakState",12,i) == OFF_BREAK)
			{
				/*found a free TC, so make it your TicketClerk and change its status to BUSY*/
				myTicketClerk = i;
				itoa(buff,1,myTicketClerk);
				SetMV("ticketClerkState",16,i,TCBUSY);
				WriteToConsole("\nCustomer %d in Group %d is getting in TicketClerk line %d\n\0",current->customerNumber, current->groupNumber, myTicketClerk);
				break;
			}
		}

		/*If all are on break - condition has to be used*/

		/*If no free TC available then check for shortest line of TC*/
		if(myTicketClerk == -1)
		{
			/*shortestTCLine = 0;*/ /*by default the shortest line is the 1st one*/
			/*shortestTCLineLength = ticketClerkLineCount[0];*/
			shortestTCLineLength = 5000;
			for(i = 0; i < MAX_TC; i++)
			{
				if ((shortestTCLineLength >= GetMV("ticketClerkLineCount",20,i)) && (/*ticketClerkState[i] == TCBUSY && */GetMV("TCBreakState",12,i) == OFF_BREAK))
				{
					/*HeadCustomer found TC with shortest line*/
					shortestTCLineLength = GetMV("ticketClerkLineCount",20,i);
					shortestTCLine = i;
				}
			}
			WriteToConsole("\nHead Customer %d found TicketClerk %d in shortest line\n\0", current -> customerNumber,shortestTCLine,-1);
			myTicketClerk = shortestTCLine;
			itoa(buff,1,myTicketClerk);
			/*get into the line*/
			linecnt = GetMV("ticketClerkLineCount",20,myTicketClerk);
			linecnt += 1;
			SetMV("ticketClerkLineCount",20,myTicketClerk,linecnt);
			/*waiting in TC for its turn*/
			
			name = "ticketClerkLineCV";
			*(name+17) = buff[0];
			Wait(name, 18,"tcLineLock", 10);
			WriteToConsole("\nCustomer %d in Group %d is getting in TicketClerk line %d\n\0",current->customerNumber, current->groupNumber, myTicketClerk);
		
			/*if managerCustomerTCFlag[myTicketClerk] == 1 then current TC is going on Break and customer needs to get in another line*/
				if(GetMV("managerCustomerTCFlag",21,myTicketClerk) == 1 || GetMV("managerCustomerTCFlag",21,myTicketClerk) ==  2)
				{
					if(GetMV("managerCustomerTCFlag",21,myTicketClerk) == 2) /*if 2 then current TC has more than 5 customers and so other TC has come OFF BREAK*/
					{
						SetMV("managerCustomerTCFlag",21,myTicketClerk,0);  /*initialize current TC flag to 0, OFF_BREAK*/
					}
					myTicketClerk = -1;
					Release("tcLineLock", 10);
					continue;
				}
			}

		Release("tcLineLock", 10);
	}

	name = "ticketClerkLock";
	*(name + 15) =  buff[0];
	/**(name + 16) = '\0';*/
	/*acquire then new lock to start interaction with ticket clerk like TT game*/
	Acquire(name,16);

	/*pass no. of tickets and wake up clerk*/

	SetMV("numberOfTicketsForTC",20, myTicketClerk, myGroupSize);

	WriteToConsole("\nCustomer %d in Group %d is walking up to TicketClerk %d to buy %d tickets\n\0",
	(((current->customerNumber) * 1000000) + ((current->groupNumber) * 10000) + (myTicketClerk * 100) + myGroupSize), -5, 4);	/* 4  or more arguments so need to pass arguments using IInd method*/

	name1 = "ticketClerkCV";
	*(name1 + 13) =  buff[0];
	/*signal the TC regarding no. of tickets*/
	Signal(name1,14, name,16);

	/*wait for response from TC*/
	Wait(name1,14, name,16);

	/*pay the amount by signalling the TC*/
	WriteToConsole("\nCustomer %d in Group %d in TicketClerk line %d is paying %d for tickets\n\0",
	(((current->customerNumber) * 1000000) + ((current->groupNumber) * 10000) + (myTicketClerk * 100) + (myGroupSize * 12) ), -5, 4);	/* 4  or more arguments so need to pass arguments using IInd method*/

	Signal(name1,14, name,16);

	/*wait here for ticket*/
	Wait(name1,14, name,16);

	/*hasTickets[MAX_GROUPS] is a monitor variable*/
	SetMV("hasTickets",10,current -> groupNumber,BOOKED);

	WriteToConsole("\nCustomer %d in Group %d is leaving TicketClerk %d\n\0",current->customerNumber, current->groupNumber, myTicketClerk);

/*------------------------------------------------end of booking of tickets section--------------------------------------------------*/
	/*Interaction: Head Customer with group members*/
	/*Acquire group ticket lock to start with interaction*/
	name2 = "grpTLock";
	itoa(buff1,1,current -> groupNumber);
	*(name2 + 8) = buff1[0];
	
	name3 = "grpTCV";
	*(name3 + 6) = buff1[0];
	
	Acquire(name2, 9);
	/*release the TC lock acquired above for booking tickets*/
	Release(name,16);
	if(current -> groupSize > 1)	/*inform only if there are other members in your group*/
	{
		/*signal waiting group members about the booked tickets by broadcasting*/
		Broadcast(name3,7,name2,9);
		WriteToConsole("\nHead customer %d of group %d is informing all his group members as tickets received \n\0", current->customerNumber, current -> groupNumber, -1);
	}
	/*------------------------------------------------informed all the group members tickets booked-------------------------------------------*/
	/*Acquire Food Lock for Head Customer to interact with group members*/
	name3 = "groupFoodLock";
	*(name3 + 13) = buff1[0];
	
	Acquire(name3,14);
	/*release the TC lock as the tickets are bought*/
	Release(name2,9);	
}

/*===========================================================*/
/*	FUNCTION: Interaction of HeadCustomer with group members*/
/*				to inform the about the seat allocation*/
/*===========================================================*/
void interactWithGroupMembersForSeatAllocation(struct CustomerData *myData)
{
	itoa(buff,1,myGroupNumber);
	
	name1 = "groupSeatWaitCV";
	*(name1 + 15) = buff[0];
	name = "groupSeatWaitLock";
	*(name+17) = buff[0];
	while(GetMV("GMCountForSeats",15,myGroupNumber) > 1)	/* wait for all group members to occupy the seats*/
	{
		Wait(name1,16, name,18);
	}
	/*release seat wait lock as all group members have occupied seats*/

	Release(name,18);
	/*HeadCustomer releases seat wait lock*/
}

/*================================================================*/
/*	FUNCTION: Head Customer allocates seats for his group members*/
/*================================================================*/
void allocateSeats(int totalMemCount,int grpNo)
{
	int memNumber=0, i,temp = -1,counter = 0;
	for(i=0; i<25; i++)
	{
		if(GetMV("seats",5,i) == SEATS_FREE)
		{
			SetMV("seats",5,i,SEATS_OCCUPIED);
			counter++;
			temp = GetMV("totalOccupiedSeats",18,0);
			SetMV("totalOccupiedSeats",18,0,temp+1);
			if(counter == 5)
				break;
		}
	}	
}

/*===========================================================*/
/*	FUNCTION: Logic to implement seat Allocation*/
/*				by head customer*/
/*===========================================================*/
void movieSeatAllocation(struct CustomerData *myData)
{
	int myRow = -1, i;
	
	/*initialize to -1 for furthur check of seats/rows in theatre*/
	/*myGroupNumber = myData -> groupNumber;*/
	
	Acquire("movieSeatManagerLock",20);	/* To access and update total seat count*/

	/*acquire movieseatlock shared with other Head customers and Movie Technician, to share/update total theater seat count*/
	Acquire("movieSeatLock",13);

	/*acquire movieseatmanagerlock shared with other Head customers and Manager*/

	/*Acquire("movieSeatManagerLock",20); */	/* To access and update total seat count*/
	/*=======================================================================*/
	allocateSeats(myData->groupSize,myData->groupNumber);
	
	/* myData -> mySeat = myOccupiedSeats[myGroupNumber][myData -> customerNumber]; */	/*seat number of head customer*/
	WriteToConsole("\nCustomer %d in group %d is sitting in a theater room seat\n\0", myData->customerNumber, myGroupNumber, -1);

	if(myData -> groupSize > 1)		/*if there are any members in your group then allocate seats to them also*/
		interactWithGroupMembersForSeatAllocation(myData);

	/*Release("movieSeatManagerLock",20);*/
	Release("movieSeatLock",13);
	Release("movieSeatManagerLock",20);
	/*release the lock taken with manager*/	
}


/*================================================================*/
/*	FUNCTION: Customers entered Theater, now watch movie and then*/
/*				exit theater when movie is over*/
/*================================================================*/
void watchMovieAndExit(struct CustomerData *myData)
{
	int groupNum,temp = -1;
	groupNum = myData -> groupNumber;
	
	itoa(buff,1,groupNum);
	name = "groupExitLock";
	*(name+13) = buff[0];
	
	/*nHeadCustomer acquires groupExitLock to interaction with group members*/
	Acquire(name,14);

	/*HeadCustomer acquires movieTechLock to check if movie is over*/
	Acquire("movieTechLock",13);
	temp = GetMV("movieFinishState",16,0);
	if(temp == NOT_OVER || temp == FIRST_MOVIE)
	{
			/*	waits as movie is not over*/
		Wait("movieTechCV",11,"movieTechLock",13);
			/*	got signal of movie getting*/
	}

	Release("movieTechLock",13);
	/*releases movieTechLock*/

	myData -> mySeat = 0;	/*before exiting make seat = 0*/

	WriteToConsole("\nCustomer %d in group %d is getting out of a theater room seat\n\0", myData->customerNumber, groupNum, -1);

	/*nHeadCustomer will now check if all group members have gathered*/
	name1 = "groupExitCV";
	*(name1 + 11) = buff[0];
	
	while(GetMV("groupExitCount",14,groupNum) > 1)
	{
		Wait(name1,12,name,14);	/* wait for all members*/
	}
	/*all members arrived, so now its OK to exit*/
	Release(name,14);
	/*releases groupExitLock as all Members gathered to exit*/
	
	name = "groupRegroupLock";
	*(name+16) = buff[0];
	
	/*HeadCustomer acquire groupRegroupLock to convey exit*/
	Acquire(name,17);

	
	SetMV("groupRegroupState",17,groupNum,REGROUPED);
	if(GetMV("groupRegroupFlag",16,groupNum) == 1)
	{
		name1 = "groupRegroupCV";
		*(name1 + 14) = buff[0];
		WriteToConsole("\nHeadCustomer %d of Group %d has told the group to proceed.\n\0",myData->customerNumber, groupNum, -1);
		Broadcast(name1,15, name,17);		/*Broadcast everybody to exit movie hell*/
	}

	Release(name,17);
	
	WriteToConsole("\nHeadCustomer %d of group %d is waiting for the group to form.\n\0",myData->customerNumber, groupNum, -1);
	
	/* Acquire mgr lock and increment the exit count*/

	Acquire("movieSeatManagerLock",20);
		temp = GetMV("totalMovieRoomExitCount",23,0);
		SetMV("totalMovieRoomExitCount",23,0,temp + myData -> groupSize);		/*increment the movie hall exit count*/
	Release("movieSeatManagerLock",20);
	/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

	/*Customers may proceed to exit or go to bathroom, if some one in a group goes to bathroom, others wait*/
	customersExit(myData);
}

/*===============================================================*/
/*	FUNCTION: All customers may go to bathroom and then Exit here*/
/*===============================================================*/
void customersExit(struct CustomerData *myData)
{
	int i, random,temp = -1;
	/*myGroupNumber = myData -> groupNumber;*/
	random = Random(100);
	if(random < 25)			
	{		/*25 percent chance of a person to go to bathroom*/
		WriteToConsole("\nCustomer %d in Group %d is going to the bathroom.\n\0",  myData->customerNumber, myData->groupNumber, -1);			
		randomYield(200);
		WriteToConsole("\nCustomer %d in Group %d is leaving the bathroom.\n\0",  myData->customerNumber, myData->groupNumber, -1);	
	}
	
	itoa(buff,1,myGroupNumber);

	name = "groupTELock";
	*(name + 11) = buff[0];
	
	Acquire(name,12);
	temp = GetMV("groupTheaterExitCount",21,myGroupNumber);
	SetMV("groupTheaterExitCount",21,myGroupNumber,temp+1);
	
	name1 = "groupTECV";
	*(name1 + 9) = buff[0];
	
	if(GetMV("groupTheaterExitCount",21,myGroupNumber) == myData -> groupSize)	/* All group members will wait, the last one will come and broadcast*/
	{
		Broadcast(name1,10, name,12);
	}
	else
	{
		Wait(name1,10, name,12);
	}
	Release(name,12);
	Acquire("customerManagerExitLock",23);
		temp = GetMV("allCustomerExitedCount",22,0);
		SetMV("allCustomerExitedCount",22,0,temp + 1);	/*increment the total exit count with the group size*/
	Release("customerManagerExitLock",23);

	WriteToConsole("\nCustomer %d in group %d has left the movie theater.\n\0", myData->customerNumber, myGroupNumber, -1);
	Exit(0);
}


void CreateMonitors()
{
	/*ticketClerkBreakStateMVId = CreateMV("TCBreakState",12,MAX_TC);
	ticketClerkStateMVId = CreateMV("ticketClerkState",16,MAX_TC);
	ticketClerkLineCountMVId = CreateMV("ticketClerkLineCount",20,MAX_TC);
	managerCustomerTCFlagMVId = CreateMV("managerCustomerTCFlag",21,1);
	numberOfTicketsForTCMVId = CreateMV("numberOfTicketsForTC",20,MAX_TC);
	hasTicketsMVId = CreateMV("hasTickets",10,MAX_GROUPS);
	numberOfGroupsMVId = CreateMV("numberOfGroups",14,1);
	ccBreakStateMVId = CreateMV("CCBreakState",12,MAX_CC);
	concessionClerkStateMVId = CreateMV("ccState",7,MAX_CC);
	concessionClerkLineCountMVId = CreateMV("ccLineCount",11,MAX_CC);
	managerCustomerCCFlagMVId = CreateMV("managerCustomerCCFlag",21,1);
	numberOfPopcornForCCMVId = CreateMV("numberOfPopcornForCC",20,MAX_CC);
	numberOfSodaForCCMVId = CreateMV("numberOfSodaForCC",17,MAX_CC);*/
	nextHCIndexMVId = CreateMV("nextHCIndex",11,1);
	
/*	memberCountForGroupMembersMVId = CreateMV("memCountForGrpMembers", 21, MAX_GROUPS);
	remainingCountMVId = CreateMV("remainingCount", 14, 1);
	numberOfSodaMVId = CreateMV("numberOfSoda", 12, MAX_GROUPS);
	numberOfPopcornMVId = CreateMV("numberOfPopcorn", 15, MAX_GROUPS);
	groupMemberCountForFoodMVId = CreateMV("groupMemberCountForFood", 23, MAX_GROUPS);
	allCustomerExitedCountMVId = CreateMV("allCustomerExitedCount", 22, 1);
	totalCustomerInCountMVId = CreateMV("totalCustomerInCount", 20, 1);
	hasOrderedMVId = CreateMV("hasOrdered", 10, MAX_GROUPS);
	groupFoodWaitFlagMVId = CreateMV("groupFoodWaitFlag", 17, MAX_GROUPS);
	hasSubmittedMVId = CreateMV("hasSubmitted", 12, MAX_GROUPS);
	groupTicketSubmitFlagMVId = CreateMV("groupTicketSubmitFlag", 21, MAX_GROUPS);
	groupMemberCountForSeatsMVId = CreateMV("GMCountForSeats",15, MAX_GROUPS);
	movieTechStateMVId = CreateMV("movieTechState", 14, 1);
	groupExitCountMVId = CreateMV("groupExitCount", 14, MAX_GROUPS);
	groupRegroupStateMVId = CreateMV("groupRegroupState", 17, MAX_GROUPS);
	groupRegroupFlagMVId = CreateMV("groupRegroupFlag", 16, MAX_GROUPS);
	groupTheaterExitCountMVId = CreateMV("groupTheaterExitCount", 21, MAX_GROUPS);
	groupMemCountMVId = CreateMV("groupMemCount", 13, 1);
	groupMembersCountMVId = CreateMV("groupMembersCount", 17, MAX_GROUPS);
	movieFinishStateMVId = CreateMV("movieFinishState", 16, 1);*/
	
	movieManagerStateMVId = CreateMV("movieManagerState",17,0);
	ticketTakerStateMVId = CreateMV("ticketTakerState",16,MAX_TT);
	lineTicketMessageMVId = CreateMV("lineTicketMessage",17,MAX_TT);
	numberOfTicketsForTTMVId = CreateMV("numberOfTicketsForTT",20,MAX_TT);
	acceptMessageMVId = CreateMV("acceptMessage",13,MAX_TT);
	totalMovieRoomExitCountMVId = CreateMV("totalMovieRoomExitCount",23,1);
	totalOccupiedSeatsMVId = CreateMV("totalOccupiedSeats",18,1);
	seatsMVId = CreateMV("seats",5,25);
}

void createTCLocksCV()
{
	ticketClerkLineLockId = CreateLock("tcLineLock", 10);
/*	ticketClerkLineCVId[0] = CreateCV("ticketClerkLineCV0", 18);
	ticketClerkLockId[0] = CreateLock("ticketClerkLock0", 16);
	ticketClerkCVId[0] = CreateCV("ticketClerkCV0", 14);
	ticketClerkLineCVId[1] = CreateCV("ticketClerkLineCV1", 18);
	ticketClerkLockId[1] = CreateLock("ticketClerkLock1", 16);
	ticketClerkCVId[1] = CreateCV("ticketClerkCV1", 14);
	ticketClerkLineCVId[2] = CreateCV("ticketClerkLineCV2", 18);
	ticketClerkLockId[2] = CreateLock("ticketClerkLock2", 16);
	ticketClerkCVId[2] = CreateCV("ticketClerkCV2", 14);
	ticketClerkLineCVId[3] = CreateCV("ticketClerkLineCV3", 18);
	ticketClerkLockId[3] = CreateLock("ticketClerkLock3", 16);
	ticketClerkCVId[3] = CreateCV("ticketClerkCV3", 14);
	ticketClerkLineCVId[4] = CreateCV("ticketClerkLineCV4", 18);
	ticketClerkLockId[4] = CreateLock("ticketClerkLock4", 16);
	ticketClerkCVId[4] = CreateCV("ticketClerkCV4", 14);*/
}

void createCCLocksCV()
{
	concessionClerkLineLockId = CreateLock("ccLineLock", 10);
/*	concessionClerkLineCVId[0] = CreateCV("ccLineCV0", 9);
	concessionClerkLockId[0] = CreateLock("ccLock0", 7);
	concessionClerkCVId[0] = CreateCV("ccCV0", 5);
	concessionClerkLineCVId[1] = CreateCV("ccLineCV1", 9);
	concessionClerkLockId[1] = CreateLock("ccLock1", 7);
	concessionClerkCVId[1] = CreateCV("ccCV1", 5);
	concessionClerkLineCVId[2] = CreateCV("ccLineCV2", 9);
	concessionClerkLockId[2] = CreateLock("ccLock2", 7);
	concessionClerkCVId[2] = CreateCV("ccCV2", 5);
	concessionClerkLineCVId[3] = CreateCV("ccLineCV3", 9);
	concessionClerkLockId[3] = CreateLock("ccLock3", 7);
	concessionClerkCVId[3] = CreateCV("ccCV3", 5);
	concessionClerkLineCVId[4] = CreateCV("ccLineCV4", 9);
	concessionClerkLockId[4] = CreateLock("ccLock4", 7);
	concessionClerkCVId[4] = CreateCV("ccCV4", 5);*/
}

void createTTLocksCV()
{
	ticketTakerLineLockId = CreateLock("ticketTakerLineLock", 19);
/*	ticketTakerLineCVId[0] = CreateCV("ticketTakerLineCV0", 18);
	ticketTakerLockId[0] = CreateLock("ticketTakerLock0", 16);
	ticketTakerCVId[0] = CreateCV("ticketTakerCV0", 14);
	ticketTakerLineCVId[1] = CreateCV("ticketTakerLineCV1", 18);
	ticketTakerLockId[1] = CreateLock("ticketTakerLock1", 16);
	ticketTakerCVId[1] = CreateCV("ticketTakerCV1", 14);
	ticketTakerLineCVId[2] = CreateCV("ticketTakerLineCV2", 18);
	ticketTakerLockId[2] = CreateLock("ticketTakerLock2", 16);
	ticketTakerCVId[2] = CreateCV("ticketTakerCV2", 14);*/
}

void createGroupLocksCV(int myGroupNumber)
{
	customerManagerExitLockId = CreateLock("customerManagerExitLock", 23);
	
	itoa(buff,1,myGroupNumber);
/*
	name = "grpTLock";
	*(name+8) = buff[0];
	groupTicketLockId[myGroupNumber] = CreateLock(name, 9);

	name = "grpTCV";
	*(name+6) = buff[0];
	groupTicketLockCVId[myGroupNumber] = CreateCV(name, 7);

	name = "groupFoodLock";
	*(name+13) = buff[0];
	groupFoodLockId[myGroupNumber] = CreateLock(name, 14);

	name = "groupFoodLockCV";
	*(name+15) = buff[0];
	groupFoodLockCVId[myGroupNumber] = CreateCV(name, 16);

	name = "intraGrpFoodLock";
	*(name+16) = buff[0];
	intraGroupFoodLockId[myGroupNumber] = CreateLock(name, 17);

	name = "groupFoodWaitLock";
	*(name+17) = buff[0];
	groupFoodWaitLockId[myGroupNumber] = CreateLock(name, 18);

	name = "groupFoodWaitCV";
	*(name+15) = buff[0];
	groupFoodWaitCVId[myGroupNumber] = CreateCV(name, 16);

	name = "groupTSLock";
	*(name+11) = buff[0];
	groupTicketSubmitLockId[myGroupNumber] = CreateLock(name, 12);

	name = "groupTSCV";
	*(name+9) = buff[0];
	groupTicketSubmitCVId[myGroupNumber] = CreateCV(name, 10);

	name = "groupSeatWaitLock";
	*(name+17) = buff[0];
	groupSeatWaitLockId[myGroupNumber] = CreateLock(name, 18);

	name = "groupSeatWaitCV";
	*(name+15) = buff[0];
	groupSeatWaitCVId[myGroupNumber] = CreateCV(name, 16);

	name = "groupExitLock";
	*(name+13) = buff[0];
	groupExitLockId[myGroupNumber] = CreateLock(name, 14);

	name = "groupExitCV";
	*(name+11) = buff[0];
	groupExitCVId[myGroupNumber] = CreateCV(name, 12);

	name = "groupRegroupLock";
	*(name+16) = buff[0];
	groupRegroupLockId[myGroupNumber] = CreateLock(name, 17);

	name = "groupRegroupCV";
	*(name+14) = buff[0];
	groupRegroupCVId[myGroupNumber] = CreateCV(name, 15);

	name = "groupTELock";
	*(name+11) = buff[0];
	groupTheaterExitLockId[myGroupNumber] = CreateLock(name, 12);

	name = "groupTECV";
	*(name+9) = buff[0];
	groupTheaterExitCVId[myGroupNumber] = CreateCV(name, 10);
*/
}

int main()
{
	int i;
	struct CustomerData current;
	int mynextHCIndex;
	
	int temp;
	temp=3200;
	for(i = 0 ; i < temp ; i++){
	}

	/*movieTechLockId = CreateLock("movieTechLock",13);
	movieSeatManagerLockId = CreateLock("movieSeatManagerLock",20);
	customerManagerExitLockId = CreateLock("customerManagerExitLock",23);
	movieSeatLockId = CreateLock("movieSeatLock",13);*/
	
	CreateMonitors();	
	createTCLocksCV();
	createCCLocksCV();
	createTTLocksCV();
	nextHCIndexLockId = CreateLock("nextHCIndexLock",15);
	Acquire("nextHCIndexLock",15);	/*head customer acquires index lock to find their index i.e group number*/
		mynextHCIndex = GetMV("nextHCIndex",11,0);
		SetMV("nextHCIndex",11,0,mynextHCIndex+1);	
		myGroupNumber = mynextHCIndex;
		myGroupSize = 5; /*GetMV("groupMembersCount",17,mynextHCIndex);*/
	Release("nextHCIndexLock",15);
	createGroupLocksCV(mynextHCIndex);

	current.groupNumber = myGroupNumber;	/*head customers create their data structure after finding required things*/
	current.groupSize = myGroupSize;
	current.customerNumber = 0;
	
	/*for(i = 0 ; i < GetMV("numberOfGroups",14,0) ; i++)
	{
		groupTicketLockId[i] = CreateLock("groupTicketLock", 15);				------- NOT REQUIRED
		groupTicketLockCVId[i] = CreateCV("groupTicketLockCV", 17);
	}*/
	mgrTTCustomerCommonLockId = CreateLock("mgrTTCustLock",13);
	mgrTTCustomerCommonCVId = CreateCV("mgrTTCustCV",11);
	
	Acquire("customerManagerExitLock", 23);
			/*increment in count of all customers*/
		SetMV("totalCustomerInCount", 20, 0, GetMV("totalCustomerInCount", 20, 0) + 1);
			/*increment remaining count for all customers*/
		SetMV("remainingCount", 14, 0, GetMV("remainingCount", 14, 0) + 1);
	Release("customerManagerExitLock", 23);
	
	WriteToConsole("\nCustomer %d in group %d has entered the movie theater.\n\0", current.customerNumber , current.groupNumber, -1);
	interactionForTicketsWithTC(&current);	/*Head customer goes to Ticket Clerk to buy tickets*/
	/*interact with group members */
	interactWithGroupMembersForFoodOrder(&current);	/* Head Customer goes to buy food from concession clerk*/
	
													/*Head customer then informs all the members that he got the food*/
													
													/*Head customer goes to TicketTaker after this to submit tickets*/
													
	movieSeatAllocation(&current);			/*Head allocate seats to all his group members*/

	watchMovieAndExit(&current);			/* Head customer exits movie hall and may go to bathroom before going out of theater*/
	Exit(0);
}
