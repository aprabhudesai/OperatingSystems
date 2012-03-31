//	Simple test cases for the threads assignment.
//

#include "copyright.h"
#include "system.h"
#ifdef CHANGED
#include "synch.h"
#endif

#ifdef CHANGED

// ----------------------------------------------------------------------------------------------
//	VARIABLE DECLARATIONS
//	1. Locks
//	2. Condition Variables
//	3. Monitor Variable
//	4. Other Global declarations
//-----------------------------------------------------------------------------------------------

//Global Variables for Movie Theater
struct CustomerData
{
	int customerNumber;
	int groupNumber;
	int groupSize; //This value is used by the group head
	int mySeat;
};

#define MAX_TC 5
#define MAX_CC 5
#define MAX_TT 3
#define MAX_GROUPS 10

//+++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++
// Interaction: Customer with Ticket Clerk

Lock *ticketClerkLineLock;// Deciding the line to get into
Condition *ticketClerkLineCV[MAX_TC];//To wait on the line lock to start interaction with customer
int ticketClerkLineCount[MAX_TC] = {0};
enum TCSTATE {TCBUSY , TCNOT_BUSY, ON_BREAK, OFF_BREAK};
TCSTATE ticketClerkState[MAX_TC] = {TCBUSY};
int managerCustomerFlag[MAX_TC] = {0};

int numberOfTicketsForTC[MAX_TC] = {0};
Lock *ticketClerkLock[MAX_TC];
Condition *ticketClerkCV[MAX_TC];
int amt2TC[MAX_TC] = {0}, totalAmtByTC[MAX_TC] = {0};

Lock *groupTicketLock[MAX_GROUPS];
Condition *groupTicketLockCV[MAX_GROUPS];
enum TICKETSTATUS {BOOKED, NOT_BOOKED};
TICKETSTATUS hasTickets[MAX_GROUPS] = {NOT_BOOKED};

//Take Food Order from Group members by Head Customer
Lock *groupFoodLock[MAX_GROUPS];
Condition *groupFoodLockCV[MAX_GROUPS];
int numberOfSoda[MAX_GROUPS], numberOfPopcorn[MAX_GROUPS];
int groupMemberCountForFood[MAX_GROUPS];

//For mutual exclusion so that only one member customer can specify food order to Head Customer
Lock *intraGroupFoodLock[MAX_GROUPS];

//Food Ordered or not ordered communication between Head Customer and group members
Lock *groupFoodWaitLock[MAX_GROUPS];
Condition *groupFoodWaitCV[MAX_GROUPS];
enum ORDERSTATE {ORDER_NOTRECEIVED,ORDER_RECEIVED};
ORDERSTATE hasOrdered[MAX_GROUPS] = {ORDER_NOTRECEIVED};
int groupFoodWaitFlag[MAX_GROUPS] = {0};

Lock *concessionClerkLineLock;
Condition *concessionClerkLineCV[MAX_CC];
int concessionClerkLineCount[MAX_CC] = {0};
enum CCSTATE {CCBUSY , CCNOT_BUSY};
CCSTATE concessionClerkState[MAX_CC] = {CCBUSY};

Lock *concessionClerkLock[MAX_CC];
Condition *concessionClerkCV[MAX_CC];
int numberOfPopcornForCC[MAX_CC] = {0};
int numberOfSodaForCC[MAX_CC] = {0};
int amt2CC[MAX_CC] = {0}, totalAmtByCC[MAX_CC] = {0};

Lock *groupTicketSubmitLock[MAX_GROUPS];
Condition *groupTicketSubmitCV[MAX_GROUPS];
enum SUBMITSTATE {TICKETS_ACCEPTED,TICKETS_NOTACCEPTED};
SUBMITSTATE hasSubmitted[MAX_GROUPS] = {TICKETS_NOTACCEPTED};
int groupTicketSubmitFlag[MAX_GROUPS] = {0};

Lock *ticketTakerLineLock;
Condition *ticketTakerLineCV[MAX_TT];
int ticketTakerLineCount[MAX_TT] = {0};
enum TTWORKSTATE {TTBUSY , TTNOT_BUSY};
TTWORKSTATE ticketTakerState[MAX_TT] = {TTBUSY};
int lineTicketMessage[MAX_TT]={0};

Lock *ticketTakerLock[MAX_TT];
Condition *ticketTakerCV[MAX_TT];
int numberOfTicketsForTT[MAX_TT] = {0};
int acceptMessage[MAX_TT]={0};



//+++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++

Lock *movieSeatManagerLock;
int totalOccupiedSeats = 0;
int totalMovieRoomExitCount = 0;

Lock *movieSeatLock;

enum SEATSTATE {SEATS_FREE, SEATS_OCCUPIED};
SEATSTATE seats[5][5]= {SEATS_FREE};
int myOccupiedSeats[MAX_GROUPS][5]= {-1};
int numberOfFreeSeats[5] = {5,5,5,5,5}; // counter for the free seats per row

int groupMemberCountForSeats[MAX_GROUPS];

Lock *groupSeatWaitLock[MAX_GROUPS];
Condition *groupSeatWaitCV[MAX_GROUPS];

//+++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++

Lock *movieTechLock;
Condition *movieTechCV;
enum MOVIESTATE {OVER, NOT_OVER, FIRST_MOVIE};
MOVIESTATE movieFinishState = FIRST_MOVIE;
enum CUSTOMERMOVIEWAITINGSTATE {NOT_WAITING, WAITING};
CUSTOMERMOVIEWAITINGSTATE movieTechState = NOT_WAITING;

Lock *groupExitLock[MAX_GROUPS];
Condition *groupExitCV[MAX_GROUPS];
int groupExitCount[MAX_GROUPS] = {0};

Lock *groupRegroupLock[MAX_GROUPS];
Condition *groupRegroupCV[MAX_GROUPS];
enum GROUPSTATE {REGROUPED, NOT_REGROUPED};
GROUPSTATE groupRegroupState[MAX_GROUPS] = {NOT_REGROUPED};
int groupRegroupFlag[MAX_GROUPS] = {0};

//+++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++

Lock *groupTheaterExitLock[MAX_GROUPS];
Condition *groupTheaterExitCV[MAX_GROUPS];
int groupTheaterExitCount[MAX_GROUPS] = {0};

//+++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++
Lock *movieTechManagerLock;
Condition *movieTechManagerCV;
int commandToStart = 0;

//+++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++

Lock *ticketClerkManagerLock;
Condition *ticketClerkManagerCV;

int ticketClerkBreakCount = 0;
enum BREAKSTATUS {NO, YES};
BREAKSTATUS onBreak = NO;

//+++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++

Lock *mgrTTCustomerCommonLock;
Condition *mgrTTCustomerCommonCV;
enum MOVIEMGRSTATE {STARTING, NOT_STARTING};
MOVIEMGRSTATE movieManagerState = {NOT_STARTING};

Lock *ttStateLock;
enum TTSTATE {NOT_ACCEPTING, ACCEPTING};
TTSTATE ttTicketAcceptState = NOT_ACCEPTING;
int totalAcceptedSeats = 0;
int seatFullMessage = 0;

Lock *customerManagerExitLock;
int allCustomerExitedCount = 0;
int totalCustomerInCount = 0;


int totalTicketClerkAmount = 0;
int totalConcessionClerkAmount = 0;
Lock *concessionClerkManagerLock;
//===========================================================
//	FUNCTION: Interaction of HeadCustomer with Ticket Clerk
//===========================================================
void interactionForTicketsWithTC(CustomerData * current)
{
	int myTicketClerk = -1;

	int myGroupSize = current -> groupSize;

	while(myTicketClerk == -1)
	{
		//printf("\nHeadCustomer[%d] acquired line lock\n",current->groupNumber);
		ticketClerkLineLock -> Acquire();

		//printf("\nHeadCustomer[%d] Checks if any TC is Free \n",current->groupNumber);
		//Check if any TicketClerk is not busy
		for(int i = 0; i < MAX_TC; i++)
		{
			if(ticketClerkState[i] == TCNOT_BUSY)
			{
				//found a free TC
				//printf("\nHeadCustomer[%d] found Free TC[%d]\n",current->groupNumber,i);
				myTicketClerk = i;
				ticketClerkState[i] = TCBUSY;
				break;
			}
		}

		//If all are on break - condition has to be used

		//If no free TC available then check for shortest line of TC
		if(myTicketClerk == -1)
		{
			printf("\nHeadCustomer[%d] did not find free TC\n",current->groupNumber);
			int shortestTCLine = 0; //by default the shortest line is the 1st one
			int shortestTCLineLength = ticketClerkLineCount[0];
			for(int i = 1; i < MAX_TC; i++)
			{
				//printf("\nCheck shortestTCLineLength: %d ticketClerkLineCount[i]: %d && ticketClerkState[i]: %d\n",shortestTCLineLength,ticketClerkLineCount[i],ticketClerkState[i]);
				if ((shortestTCLineLength > ticketClerkLineCount[i]) && (ticketClerkState[i] == TCBUSY))
				{
					//printf("\nHeadCustomer[%d] found shortest tc line TC[%d]\n",current->groupNumber,i);
					shortestTCLineLength = ticketClerkLineCount[i];
					shortestTCLine = i;
				}
			}
				myTicketClerk = shortestTCLine;
				//printf("\nHeadCustomer[%d] entered TC[%d]\n",current->groupNumber,myTicketClerk);
				//get into the line
				ticketClerkLineCount[myTicketClerk]++;
				//printf("\nHeadCustomer[%d] waiting in TC[%d] for its turn \n",current->groupNumber,myTicketClerk);
				ticketClerkLineCV[myTicketClerk] -> Wait(ticketClerkLineLock);
				printf("\nHeadCustomer[%d] got signal from TC[%d] for its turn \n",current->groupNumber,myTicketClerk);

				if(ticketClerkState[myTicketClerk] == ON_BREAK || managerCustomerFlag[myTicketClerk] == 1)
				{
					ticketClerkLineCount[myTicketClerk]--;
					myTicketClerk = -1;
				}
				else
				{
					//printf("\nHeadCustomer[%d] releases LINE LOCK as it is serviced by TC[%d] for its turn \n",current->groupNumber,myTicketClerk);
					//release lock as I have come into the shortest line
				}
			}

		ticketClerkLineLock ->Release();
	}

	//printf("\nHeadCustomer[%d] acquired TC[%d] interaction Lock\n",current->groupNumber,myTicketClerk);

	//acquire then new lock to start interaction with ticket clerk like TT game
	ticketClerkLock[myTicketClerk] -> Acquire();

	//printf("\nHeadCustomer[%d] asks and signals for tickets to TC[%d]\n",current->groupNumber,myTicketClerk);
	//pass no. of tickets and wake up clerk
	numberOfTicketsForTC[myTicketClerk] = myGroupSize;

	//signal the TC regarding no. of tickets
	ticketClerkCV[myTicketClerk] -> Signal(ticketClerkLock[myTicketClerk]);

	//printf("\nHeadCustomer[%d] waiting for amount for Tickets by TC[%d]\n",current->groupNumber,myTicketClerk);

	//printf("\nHeadCustomer[%d] ticket Status %d\n",current->groupNumber,hasTickets[current -> groupNumber]);
	//wait for response from TC
	ticketClerkCV[myTicketClerk] -> Wait(ticketClerkLock[myTicketClerk]);

	//printf("\nHeadCustomer[%d] pays ticket amount to TC[%d]\n",current->groupNumber,myTicketClerk);
	//pay the amount by signalling the TC
	ticketClerkCV[myTicketClerk] -> Signal(ticketClerkLock[myTicketClerk]);

	//printf("\nHeadCustomer[%d] waits for ticket from TC[%d]\n",current->groupNumber,myTicketClerk);
	ticketClerkCV[myTicketClerk] -> Wait(ticketClerkLock[myTicketClerk]);

	//hasTickets[MAX_GROUPS] is a monitor variable
	hasTickets[current -> groupNumber] = BOOKED;

	printf("\nHeadCustomer[%d] received tickets from TC[%d]\n",current->groupNumber,myTicketClerk);
	//-------end of booking of tickets section---------//

	/*
	 Lock* groupTicketLock{MAX_GROUPS]
	 Condition* groupTicketLockCV[MAX_GROUPS]
	 enum ticketStatus {BOOKED, NOT_BOOKED}
	 ticketStatus hasTickets[MAX_GROUPS]
	 */

	//Interaction: Head Customer with group members
	//Acquire group ticket lock to start with interaction
	//printf("\nHeadCustomer[%d] acquired interaction lock with members of Group\n",current->groupNumber);
	groupTicketLock[current -> groupNumber] -> Acquire();

	//printf("\nHeadCustomer[%d] released interaction lock with TC[%d]\n",current->groupNumber,myTicketClerk);
	//release the TC lock acquired above for booking tickets
	ticketClerkLock[myTicketClerk] -> Release();

	if(current -> groupSize > 1)
	{
		//signal waiting group members about the booked tickets by broadcasting
		//printf("\nHeadCustomer[%d] broadcasts all members of Group about booked tickets\n",current->groupNumber);
		groupTicketLockCV[current -> groupNumber] -> Broadcast(groupTicketLock[current -> groupNumber]);
	}
	//For Later Logic
	//printf("\nHeadCustomer[%d] acquires food interaction lock with members of Group\n",current->groupNumber);
	//Acquire Food Lock for Head Customer to interact with group members
	groupFoodLock[current -> groupNumber] -> Acquire();

	//release the TC lock as the tickets are bought
	//printf("\nHeadCustomer[%d] releases interaction lock with members of Group\n",current->groupNumber);
	groupTicketLock[current -> groupNumber] -> Release();

}

//==============================================================
//	FUNCTION: Interaction of HeadCustomer with Concession Clerk
//				to order Food for his entire group
//==============================================================
void interactWithConcessionClerksToOrderFood(CustomerData *current)
{
	int myConcessionClerk = -1;

	int myGroupSize = current -> groupSize;

	//printf("\nHeadCustomer[%d] acquired CC line lock\n",current->groupNumber);
	concessionClerkLineLock -> Acquire();

	//printf("\nHeadCustomer[%d] Checks if any CC is Free \n",current->groupNumber);
	//Check if any ConcessionClerk is not busy
	for(int i = 0; i < MAX_CC; i++)
	{
		if(concessionClerkState[i] == CCNOT_BUSY)
		{
			//found a free CC
			//printf("\nHeadCustomer[%d] found Free CC[%d]\n",current->groupNumber,i);
			myConcessionClerk = i;
			concessionClerkState[i] = CCBUSY;
			break;
		}
	}

	//If all are on break - condition has to be used

	//If no free CC available then check for shortest line of CC
	if(myConcessionClerk == -1)
	{
		printf("\nHeadCustomer[%d] did not find free CC\n",current->groupNumber);
		int shortestCCLine = 0; //by default the shortest line is the 1st one
		int shortestCCLineLength = concessionClerkLineCount[0];
		for(int i = 1; i < MAX_CC; i++)
		{
			if ((shortestCCLineLength > concessionClerkLineCount[i]) && (concessionClerkState[i] == CCBUSY))
			{
				//printf("\nHeadCustomer[%d] found shortest cc line CC[%d]\n",current->groupNumber,i);
				shortestCCLineLength = concessionClerkLineCount[i];
				shortestCCLine = i;
			}
		}
		// I have the shortest line
		myConcessionClerk = shortestCCLine;
		printf("\nHeadCustomer[%d] entered CC[%d]\n",current->groupNumber,myConcessionClerk);
		//get into the line
		concessionClerkLineCount[myConcessionClerk]++;
		//printf("\nHeadCustomer[%d] waiting in CC[%d] for its turn \n",current->groupNumber,myConcessionClerk);
		concessionClerkLineCV[myConcessionClerk] -> Wait(concessionClerkLineLock);
	}
	//printf("\nHeadCustomer[%d] got signal from CC[%d] for its turn \n",current->groupNumber,myConcessionClerk);
	//printf("\nHeadCustomer[%d] releases CC LINE LOCK as it is serviced by CC[%d] for its turn \n",current->groupNumber,myConcessionClerk);
	//release lock as I have come into the shortest line
	concessionClerkLineLock ->Release();

	//acquire then new lock to start interaction with Concession clerk like TT game
	concessionClerkLock[myConcessionClerk] -> Acquire();
	//printf("\nHeadCustomer[%d] acquired CC[%d] interaction Lock\n",current->groupNumber,myConcessionClerk);

	//printf("\nHeadCustomer[%d] asks and tells order for popcorn to CC[%d]\n",current->groupNumber,myConcessionClerk);
	//pass no. of popcorn and wake up clerk
	numberOfPopcornForCC[myConcessionClerk] = numberOfPopcorn[current->groupNumber];

	//signal the CC regarding no. of popcorns
	concessionClerkCV[myConcessionClerk] -> Signal(concessionClerkLock[myConcessionClerk]);

	//wait for CC to give acknowledgement for popcorn order
	concessionClerkCV[myConcessionClerk] -> Wait(concessionClerkLock[myConcessionClerk]);

	//printf("\nHeadCustomer[%d] asks and tells order for soda to CC[%d]\n",current->groupNumber,myConcessionClerk);
	//pass no. of sodas and wake up clerk
	numberOfSodaForCC[myConcessionClerk] = numberOfSoda[current->groupNumber];

	//signal the CC regarding no. of soda
	concessionClerkCV[myConcessionClerk] -> Signal(concessionClerkLock[myConcessionClerk]);

	//printf("\nHeadCustomer[%d] waiting for CC[%d] to give popcorn and soda\n",current->groupNumber,myConcessionClerk);
	//wait for CC to give acknowledgement for soda order
	concessionClerkCV[myConcessionClerk] -> Wait(concessionClerkLock[myConcessionClerk]);

	printf("\nHeadCustomer[%d] pays total Food amount to CC[%d] as Popcorn and Soda Received\n",current->groupNumber,myConcessionClerk);
	//pay the amount by signalling the CC
	concessionClerkCV[myConcessionClerk] -> Signal(concessionClerkLock[myConcessionClerk]);

	concessionClerkLock[myConcessionClerk] -> Release();
}

//===========================================================
//	FUNCTION: Interaction of HeadCustomer with Ticket Taker
//				to submit tickets
//===========================================================
void interactWithTicketTakerToGiveTickets(CustomerData *current)
{
	int myTicketTaker = -1;
	int myGroupSize = current -> groupSize;

	while(myTicketTaker == -1)
		{
			mgrTTCustomerCommonLock -> Acquire();

			ttStateLock -> Acquire();

			if(movieManagerState == NOT_STARTING || seatFullMessage == 1)
			//if(movieManagerState == NOT_STARTING)
			{
				ttStateLock -> Release();
				mgrTTCustomerCommonCV -> Wait(mgrTTCustomerCommonLock);

			}
			else
			{
				ttStateLock -> Release();
			}
			mgrTTCustomerCommonLock -> Release();

			//printf("\nHeadCustomer[%d] acquired TT line lock\n",current->groupNumber);
			ticketTakerLineLock -> Acquire();

			//printf("\nHeadCustomer[%d] Checks if any TT is Free \n",current->groupNumber);
			//Check if any TicketTaker is not busy
			for(int i = 0; i < MAX_TT; i++)
			{
				if(ticketTakerState[i] == TTNOT_BUSY)
				{
					//found a free TT
					printf("\nHeadCustomer[%d] found Free TT[%d]\n",current->groupNumber,i);
					myTicketTaker = i;
					ticketTakerState[i] = TTBUSY;
					break;
				}
			}

			//If all are on break - condition has to be used

			//If no free TT available then check for shortest line of TT
			if(myTicketTaker == -1)
			{
				printf("\nHeadCustomer[%d] did not find free TT\n",current->groupNumber);
				int shortestTTLine = 0; //by default the shortest line is the 1st one
				int shortestTTLineLength = ticketTakerLineCount[0];
				for(int i = 1; i < MAX_TT; i++)
				{
					if ((shortestTTLineLength > ticketTakerLineCount[i]) && (ticketTakerState[i] == TTBUSY))
					{
						printf("\nHeadCustomer[%d] found shortest tt line TT[%d]\n",current->groupNumber,i);
						shortestTTLineLength = ticketTakerLineCount[i];
						shortestTTLine = i;
					}
				}
				// I have the shortest line
				myTicketTaker = shortestTTLine;
				printf("\nHeadCustomer[%d] entered TT[%d]\n",current->groupNumber,myTicketTaker);
				
				if(lineTicketMessage[myTicketTaker] == 1)
				{
					
					
					printf("\nHeadCustomer[%d] acquire mgrTTCustomerCommon lock to wait for signal for next movie start, TT[%d] is not accepting\n",current->groupNumber,myTicketTaker);
					printf("\nHeadCustomer[%d] releases ttStateLock after check TT[%d] ttTicketAcceptState\n",current->groupNumber,myTicketTaker);

					printf("\nHeadCustomer[%d] releases tickettakerlinelock as need to wait for next movie signal and TT[%d] is not accepting\n",current->groupNumber,myTicketTaker);
					ticketTakerLineLock -> Release();
					myTicketTaker = -1;
					mgrTTCustomerCommonLock -> Acquire();
					mgrTTCustomerCommonCV -> Wait(mgrTTCustomerCommonLock);
					printf("\nHeadCustomer[%d] receives signal from mgrTTCustomerCommonCV for next movie start\n",current->groupNumber);
					mgrTTCustomerCommonLock -> Release();
					continue;
				}
				else
				{
					//get into the line
					ticketTakerLineCount[myTicketTaker]++;
				
					printf("\nHeadCustomer[%d] waiting in TT[%d] for its turn \n",current->groupNumber,myTicketTaker);
					ticketTakerLineCV[myTicketTaker] -> Wait(ticketTakerLineLock);
				}
				//printf("\nHeadCustomer[%d] acquires ttStateLock to check TT[%d] ttTicketAcceptState\n",current->groupNumber,myTicketTaker);
				if(lineTicketMessage[myTicketTaker] == 1)
				{
					
					
					printf("\nHeadCustomer[%d] acquire mgrTTCustomerCommon lock to wait for signal for next movie start, TT[%d] is not accepting\n",current->groupNumber,myTicketTaker);
					printf("\nHeadCustomer[%d] releases ttStateLock after check TT[%d] ttTicketAcceptState\n",current->groupNumber,myTicketTaker);

					printf("\nHeadCustomer[%d] releases tickettakerlinelock as need to wait for next movie signal and TT[%d] is not accepting\n",current->groupNumber,myTicketTaker);
					ticketTakerLineLock -> Release();
					myTicketTaker = -1;
					mgrTTCustomerCommonLock -> Acquire();
					mgrTTCustomerCommonCV -> Wait(mgrTTCustomerCommonLock);
					printf("\nHeadCustomer[%d] receives signal from mgrTTCustomerCommonCV for next movie start\n",current->groupNumber);
					mgrTTCustomerCommonLock -> Release();
					continue;
				}
			}
			//printf("\nHeadCustomer[%d] got signal from TT[%d] for its turn \n",current->groupNumber,myTicketTaker);
			
			//printf("\nHeadCustomer[%d] releases TT LINE LOCK as it is serviced by TT[%d] for its turn \n",current->groupNumber,myTicketTaker);
			//release lock as I have come into the shortest line
			ticketTakerLineLock ->Release();

			//acquire then new lock to start interaction with Concession clerk like TT game
			ticketTakerLock[myTicketTaker] -> Acquire();
			//printf("\nHeadCustomer[%d] acquired TT[%d] interaction Lock\n",current->groupNumber,myTicketTaker);

			printf("\nHeadCustomer[%d] asks and tells no of tickets to TT[%d]\n",current->groupNumber,myTicketTaker);
			//pass no. of tickets and wake up Ticket Taker
			numberOfTicketsForTT[myTicketTaker] = myGroupSize;

			//signal the TT regarding no. of tickets
			ticketTakerCV[myTicketTaker] -> Signal(ticketTakerLock[myTicketTaker]);

			//wait for TT to give acknowledgement for Ticket Accepted or not
			ticketTakerCV[myTicketTaker] -> Wait(ticketTakerLock[myTicketTaker]);
			printf("HeadCustomer[%d] gets reply for given tickets from TT[%d]",current->groupNumber,myTicketTaker);

			if(acceptMessage[myTicketTaker] == 1)
			{
				
				
				
				printf("\nHeadCustomer[%d] releases ttStateLock after check TT[%d] ttTicketAcceptState\n",current->groupNumber,myTicketTaker);
				printf("\nHeadCustomer[%d] releases tickettakerlinelock as need to wait for next movie signal and TT[%d] is not accepting\n",current->groupNumber,myTicketTaker);
				ticketTakerLock[myTicketTaker] -> Release();
				myTicketTaker = -1;
				mgrTTCustomerCommonLock -> Acquire();
				printf("\nHeadCustomer[%d] acquire mgrTTCustomerCommon lock to wait for signal for next movie start, TT[%d] is not accepting\n",current->groupNumber,myTicketTaker);
				mgrTTCustomerCommonCV -> Wait(mgrTTCustomerCommonLock);
				printf("\nHeadCustomer[%d] receives signal from mgrTTCustomerCommonCV for next movie start\n",current->groupNumber);
				mgrTTCustomerCommonLock -> Release();
				continue;
			}
			printf("\nHeadCustomer[%d] finds TT[%d] accepted tickets so releases ticketTakerLock\n",current->groupNumber,myTicketTaker);
			ticketTakerLock[myTicketTaker] -> Release();
		}
}

//================================================================
//	FUNCTION: Head Customer allocates seats for his group members
//================================================================
void allocateSeats(int minRow, int maxRow, int totalMemCount,int grpNo)
{
	int memNumber=0;
	for(int i=minRow; i <= maxRow && memNumber < totalMemCount; i++)
	{
		for(int j=0;j <5 && memNumber < totalMemCount; j++)
		{
			if(seats[i][j]== SEATS_FREE)
			{
				seats[i][j]= SEATS_OCCUPIED;
				myOccupiedSeats[grpNo][memNumber] = i * 5 + j + 1;
				printf("\nI = %d, J = %d, numberoffreeseats = %d, Seatcheck = %d", i, j,numberOfFreeSeats[i],myOccupiedSeats[grpNo][memNumber]);
				totalOccupiedSeats++;
				numberOfFreeSeats[i]--;
				memNumber++;
			}
		}
	}
}

//===========================================================
//	FUNCTION: Interaction of HeadCustomer with group members
//				to inform the about the seat allocation
//===========================================================
void interactWithGroupMembersForSeatAllocation(CustomerData *myData)
{
	int myGroupNumber = myData -> groupNumber;

	while(groupMemberCountForSeats[myGroupNumber] > 1)
	{
		//waiting for all group members to come for occupying seats
		//printf("\nHeadCustomer[%d] is waiting for his Member Count before wait[%d] to give up seat wait lock\n",myGroupNumber,groupMemberCountForSeats[myGroupNumber]);
		groupSeatWaitCV[myGroupNumber] -> Wait(groupSeatWaitLock[myGroupNumber]);
		//group member placed order so decrement count and wait for next group member to share the seat number allocated to the next group member
	}

	//release seat wait lock as the order taken from group members
	groupSeatWaitLock[myGroupNumber] -> Release();
	printf("\nHeadCustomer[%d] releases seat wait lock\n", myGroupNumber);
}

//===========================================================
//	FUNCTION: Logic to implement seat Allocation
//				by head customer
//===========================================================
void movieSeatAllocation(CustomerData *myData)
{
	//initialize to -1 for furthur check of seats/rows in theatre
	int myRow = -1;
	int myGroupNumber = myData -> groupNumber;

	//acquire movieseatlock shared with other Head customers and Movie Technician, to share/update total theatre seat count
	//printf("\nHeadCustomer[%d] acquires movieseatlock to be eligible to access theatre seats/total count\n",myGroupNumber);
	movieSeatLock -> Acquire();

	//not needed in the logic
	/*if(movieSeatLockState == INUSE)
	 {
	 movieSeatCV -> Wait(movieSeatLock);
	 }*/

	//acquire movieseatmanagerlock shared with other Head customers and Manager
	//printf("\nHeadCustomer[%d] acquires movieseatmanagerlock to be eligible to update total seat count\n",myGroupNumber);
	movieSeatManagerLock -> Acquire();
	//=======================================================================
	//find the row with maximum seats available for the group to be seated together
	//printf("\nHeadCustomer[%d] entering loops to fnd the row which can accomodate the group together\n",myGroupNumber);
	for(int i=0; i<5; i++)
	{
		if(numberOfFreeSeats[i] >= myData -> groupSize)
		{
			printf("\nseatcheck:  i=%d\n",i);
			//printf("\nHeadCustomer[%d] found Row [%d] which can accomodate all group members together\n",myGroupNumber,i);
			myRow = i;
			//row which can allocate all group members can allocate together for seat occupy logic
			printf("\nHeadCustomer[%d] found Row [%d ] in theatre which can accomodate all group members together\n",myGroupNumber,myRow);
			allocateSeats(i,i,myData -> groupSize,myGroupNumber);
			break;
		}
	}
	if(myRow == -1)
	{
		//printf("\nHeadCustomer[%d] did not find Row in theatre which can accomodate all group members together\n",myGroupNumber);

		//printf("\nHeadCustomer[%d] is now searching for 2 consequtive rows which can accomodate all group members\n",myGroupNumber);
		//find two consequtive rows which can accomodate all group members
		for(int i=0; i<4; i++)
		{
			//this condition is to check if free seats in 2 consequtive rows can allocate group members
			if((numberOfFreeSeats[i] + numberOfFreeSeats[i+1]) >= myData->groupSize)
			{
				//2 consequtive rows found that can accomodate all group members
				printf("\nHeadCustomer[%d] found available seats in Row [%d] [%d] to accomodate 2 consequtive rows\n",myGroupNumber,i,i+1);
				myRow = 0;// myRow is set to zero i.e. rows found
				printf("\nHeadCustomer[%d] calls allocate seats to allocate group members in Row [%d] [%d] \n",myGroupNumber,i,i+1);
				allocateSeats(i , i+1,myData->groupSize,myData->groupNumber);
				printf("\nHeadCustomer[%d] has allocated seats for group members in Row [%d] [%d] \n",myGroupNumber,i,i+1);
				break;
			}
		}
		if(myRow == -1)
		{
			//printf("\nHeadCustomer[%d] did not find 2 Rows in theatre which can accomodate all group members together\n",myGroupNumber);

			//printf("\nHeadCustomer[%d] is now searching for 3 consequtive rows which can accomodate all group members\n",myGroupNumber);
			//find three consequtive rows which can accomodate all group members
			for(int i=0; i<3; i++)
			{
				//this condition is to check if free seats in 3 consequtive rows can allocate group members
				//also we need to check the middle row has some free seats to ensure that GM is allocated seats in consequtive rows w/o skipping any row in between
				if((numberOfFreeSeats[i] + numberOfFreeSeats[i+1]+ numberOfFreeSeats[i+2]) >= myData->groupSize && numberOfFreeSeats[i+1]!=0)
				{
					//3 consequtive rows found that can accomodate all group members
					printf("\nHeadCustomer[%d] found seats in Row [%d] [%d] [%d] to accomodate 3 consequtive rows\n",myGroupNumber,i,i+1,i+2);
					myRow = 0;
					printf("\nHeadCustomer[%d] calls allocate seats to allocate group members in Row [%d] [%d] [%d] \n",myGroupNumber,i,i+1,i+2);
					allocateSeats(i , i+2,myData->groupSize,myData->groupSize);
					printf("\nHeadCustomer[%d] has allocated seats for group members in Row [%d] [%d] [%d] \n",myGroupNumber,i,i+1,i+2);
					break;
				}
			}
		}
		if(myRow == -1)
		{
			//printf("\nHeadCustomer[%d] did not find 3 Rows in theatre which can accomodate all group members together\n",myGroupNumber);

			//printf("\nHeadCustomer[%d] is now searching for 4 consequtive rows which can accomodate all group members\n",myGroupNumber);
			//find four consequtive rows which can accomodate all group members
			for(int i=0; i<2; i++)
			{
				//this condition is to check if free seats in 4 consequtive rows can allocate group members
				//also we need to check the middle 2 rows have some free seats to ensure that GM is allocated seats in consequtive rows w/o skipping any row in between
				if((numberOfFreeSeats[i] + numberOfFreeSeats[i+1] + numberOfFreeSeats[i+2]+ numberOfFreeSeats[i+3]) >= myData->groupSize && numberOfFreeSeats[i+1]!=0 && numberOfFreeSeats[i+2]!=0)
				{
					//3 consequtive rows found that can accomodate all group members
					printf("\nHeadCustomer[%d] found seats in Row [%d] [%d] [%d] [%d] to accomodate 4 consequtive rows\n",myGroupNumber,i,i+1,i+2,i+3);
					myRow = 0;
					printf("\nHeadCustomer[%d] calls allocate seats to allocate group members in Row [%d] [%d] [%d] [%d] \n",myGroupNumber,i,i+1,i+2,i+3);
					allocateSeats(i , i+3,myData->groupSize,myData->groupSize);
					printf("\nHeadCustomer[%d] has allocated seats for group members in Row [%d] [%d] [%d] [%d] \n",myGroupNumber,i,i+1,i+2,i+3);
					break;
				}
			}
		}
		if(myRow == -1)
		{
			//as no consequtive rows have seats available for group members, random allocation is done here
			//printf("\nHeadCustomer[%d] calls allocate seats to allocate group members in random order\n",myGroupNumber);
			allocateSeats(0,4,myData->groupSize,myData->groupSize);
		}
	}

	myData -> mySeat = myOccupiedSeats[myGroupNumber][myData -> customerNumber];
	printf("\nHeadCustomer[%d] of GroupNo %d occupied Seat [%d] and now waiting for the Movie\n",myData->customerNumber,myGroupNumber,myData -> mySeat);

	if(myData -> groupSize > 1)
	interactWithGroupMembersForSeatAllocation(myData);

	movieSeatLock -> Release();
	//printf("\nHeadCustomer[%d] releases movieseatlock to be eligible to access theatre seats/total count\n",myGroupNumber);

	//release the lock taken with manager
	movieSeatManagerLock -> Release();
	//printf("\nHeadCustomer[%d] releases movieseatmanagerlock to be eligible to update total seat count\n",myGroupNumber);
}


//===============================================================
//	FUNCTION: All customers may go to bathroom and then Exit here
//===============================================================
void customersExit(CustomerData *myData) {

	int myGroupNumber = myData -> groupNumber;
	if((rand()%2) == 1)
	printf("\nMember[%d] of Group [%d] goes to Sandas\n", myData->customerNumber,myData->groupNumber);

	groupTheaterExitLock[myGroupNumber] -> Acquire();
	groupTheaterExitCount[myGroupNumber]++;
	printf("\nMember[%d] of Group [%d] GROUP COUNT [%d] group size [%d] all to EXIT THEATER\n", myData->customerNumber,myData->groupNumber,groupTheaterExitCount[myGroupNumber],myData -> groupSize);
	if(groupTheaterExitCount[myGroupNumber] == myData -> groupSize) {
		printf("\nMember[%d] of Group [%d] broadcasts all to EXIT THEATER\n", myData->customerNumber,myData->groupNumber);
		groupTheaterExitCV[myGroupNumber] -> Broadcast(groupTheaterExitLock[myGroupNumber]);
	}
	else {
		printf("\nMember[%d] of Group [%d] WAITS to EXIT THEATER\n", myData->customerNumber,myData->groupNumber);
		groupTheaterExitCV[myGroupNumber] -> Wait(groupTheaterExitLock[myGroupNumber]);
	}
	groupTheaterExitLock[myGroupNumber] -> Release();
	customerManagerExitLock -> Acquire();
	allCustomerExitedCount++;
	customerManagerExitLock -> Release();
	printf("\nMember[%d] of Group [%d] Exits the Theater - Movie Khatam Paisa Hajam\n", myData->customerNumber,myData->groupNumber);
}

//================================================================
//	FUNCTION: Customers entered Theater, now watch movie and then
//				exit theater when movie is over
//================================================================
void watchMovieAndExit(CustomerData *myData)
{
	int groupNum = myData -> groupNumber;

	//printf("\nHeadCustomer[%d] acquires groupExitLock to interaction with GM\n",groupNum);
	groupExitLock[groupNum] -> Acquire();

	//groupSeatWaitLock[groupNum] ->Release();
	//printf("\nHeadCustomer[%d] releases groupSeatWaitLock\n",groupNum);

	printf("\nHeadCustomer[%d] acquires movieTechLock to check if movie is over\n",groupNum);
	movieTechLock -> Acquire();

	if(movieFinishState != OVER)
	{
		printf("\nHeadCustomer[%d] waits as movie is not over\n",groupNum);
		movieTechCV -> Wait(movieTechLock);
		printf("\nHeadCustomer[%d] gets signal of movie getting over\n",groupNum);
	}

	movieTechLock -> Release();
	//printf("\nHeadCustomer[%d] releases movieTechLock\n",groupNum);

	myData -> mySeat = 0;

	printf("\nHeadCustomer[%d] now will check if all GM have gathered to Exit\n",groupNum);
	while(groupExitCount[groupNum] > 1)
	{
		//printf("\nHeadCustomer[%d] now waiting for GM to come to Exit\n",groupNum);
		groupExitCV[groupNum] -> Wait(groupExitLock[groupNum]);
	}
	printf("\nHeadCustomer[%d] found all GM have come to Exit\n",groupNum);
	groupExitLock[groupNum] -> Release();

	//printf("\nHeadCustomer[%d] releases groupExitLock as all Members gathered to exit\n",groupNum);

	//printf("\nHeadCustomer[%d] acquire groupRegroupLock to convey exit\n",groupNum);
	groupRegroupLock[groupNum] -> Acquire();

	printf("\nHeadCustomer[%d] changes state to REGROUPED to exit theater\n",groupNum);
	groupRegroupState[groupNum] = REGROUPED;

	if(groupRegroupFlag[groupNum] == 1)
	{
		//printf("\nHeadCustomer[%d] sends Exit Signal to waiting GMs to Exit\n",groupNum);
		groupRegroupCV[groupNum] -> Broadcast(groupRegroupLock[groupNum]);
	}

	groupRegroupLock[groupNum] -> Release();
	printf("\nHeadCustomer[%d] releases groupRegroupLock and Theater Exited\n",groupNum);
	// Acquire mgr lock and increment the exit count

	movieSeatManagerLock -> Acquire();

	totalMovieRoomExitCount += myData -> groupSize;

	movieSeatManagerLock -> Release();
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	printf("\nHeadCustomer[%d] may go towards Optional Bathroom and EXIT theater\n",groupNum);
	customersExit(myData);
}

//===========================================================
//	FUNCTION: Interaction of HeadCustomer Group Members to take
//				Food order from each member
//===========================================================
void interactWithGroupMembersForFoodOrder(CustomerData *myData)
{
	//int groupMemberCount = myData -> groupSize;
	int myGroupNumber = myData -> groupNumber;

	while(groupMemberCountForFood[myGroupNumber] > 1)
	{
		//waiting for order from every group member
		printf("\nHeadCustomer[%d] is waiting for his Member Count before wait[%d] to order food\n",myGroupNumber,groupMemberCountForFood[myGroupNumber]);
		groupFoodLockCV[myGroupNumber] -> Wait(groupFoodLock[myGroupNumber]);
		//group member placed order so decrement count and wait for next group member to tell the food order
	}

	//order soda and popcorn [handle condition if I want to order or not]

	//head customer's soda and popcorn order
	numberOfSoda[myGroupNumber] += 1;
	numberOfPopcorn[myGroupNumber] += 1;

	//release food lock as the order taken from group members
	//printf("\nHeadCustomer[%d] releases food lock\n", myGroupNumber);
	printf("\nTotal Soda for %d = %d\n",myGroupNumber,numberOfSoda[myGroupNumber]);
	printf("\nTotal Popcorn for %d = %d\n",myGroupNumber,numberOfPopcorn[myGroupNumber]);

	groupFoodLock[myGroupNumber] -> Release();

	interactWithConcessionClerksToOrderFood(myData);

	//Acquire lock to inform Group members about Food Order is complete
	groupFoodWaitLock[myGroupNumber] -> Acquire();
	//printf("\nHeadCustomer[%d] acquires food Wait lock with members of Group\n",myGroupNumber);

	//Order Received, hence change the status for Group members reference
	hasOrdered[myGroupNumber] = ORDER_RECEIVED;
	printf("\nHeadCustomer[%d] changed the food order status to ORDER_RECEIVED for group members\n",myGroupNumber);

	if(groupFoodWaitFlag[myGroupNumber] == 1) {
		printf("\nHeadCustomer[%d] informs all its waiting group member about Food order completion\n",myGroupNumber);
		groupFoodWaitCV[myGroupNumber] -> Broadcast(groupFoodWaitLock[myGroupNumber]);
	}
	//printf("\nHeadCustomer[%d] releases food Wait lock with members of Group\n",myGroupNumber);
	groupFoodWaitLock[myGroupNumber] -> Release();

	interactWithTicketTakerToGiveTickets(myData);

	//Acquire lock to inform Group members about Ticket Submitted is complete
	groupTicketSubmitLock[myGroupNumber] -> Acquire();
	//printf("\nHeadCustomer[%d] acquires ticket submit lock with members of Group\n",myGroupNumber);

	//Tickets Submitted, hence change the status for Group members reference
	hasSubmitted[myGroupNumber] = TICKETS_ACCEPTED;
	printf("\nHeadCustomer[%d] changed the Ticket Submitted status to TICKETS_ACCEPTED for group members\n",myGroupNumber);

	if(groupTicketSubmitFlag[myGroupNumber] == 1) {
		//printf("\nHeadCustomer[%d] informs all its waiting group member about Ticket Submit completion\n",myGroupNumber);
		groupTicketSubmitCV[myGroupNumber] -> Broadcast(groupTicketSubmitLock[myGroupNumber]);
	}

	//printf("\nHeadCustomer[%d] acquires groupseatwaitlock to interact about seats with members of Group\n",myGroupNumber);
	groupSeatWaitLock[myGroupNumber] -> Acquire();

	groupTicketSubmitLock[myGroupNumber] -> Release();
	//printf("\nHeadCustomer[%d] releases ticket submit lock with members of Group\n",myGroupNumber);
}

//===========================================================
//	FUNCTION: Group Member function to perform all functions
//				to be done by them
//===========================================================
void customerGroupMember(int currentGM)
{
	CustomerData *myData = (CustomerData *) currentGM;
	int myGroupNumber = myData -> groupNumber;

	//Acquire group ticket lock to check if the head customer bought the tickets
	groupTicketLock[myGroupNumber] -> Acquire();
	//printf("\nGM[%d] of Group %d waiting acquired lock for interaction with HeadCustomer[%d]\n",myData->customerNumber,myGroupNumber,myGroupNumber);

	//printf("Group Number %d has Ticket status = %d",myGroupNumber,hasTickets[myGroupNumber]);
	//check if group head has bought tickets
	if (hasTickets[myGroupNumber] != BOOKED)
	{
		//printf("\nGM[%d] of Group %d waiting for tickets to be booked by HeadCustomer[%d]\n",myData->customerNumber,myGroupNumber,myGroupNumber);
		//if tickets are not booked wait for the head customer
		groupTicketLockCV[myGroupNumber] -> Wait(groupTicketLock[myGroupNumber]);
	}
	printf("\nGM[%d] of Group %d released lock as tickets booked by HeadCustomer[%d]\n",myData->customerNumber,myGroupNumber,myGroupNumber);

	//tickets are booked as release the lock, if food needed then go the food counter after this
	groupTicketLock[myGroupNumber] -> Release();

	//printf("\nGM[%d] of Group %d acquires intra group lock while placing order to HeadCustomer[%d]\n",myData->customerNumber,myGroupNumber,myGroupNumber);
	//Acquire Intra group Lock to have mutual exclusion in food order
	intraGroupFoodLock[myGroupNumber] -> Acquire();

	//printf("\nGM[%d] of Group %d acquired groupFoodLock to interact with HeadCustomer[%d]\n",myData->customerNumber,myGroupNumber,myGroupNumber);
	//Acquire this lock to place order to Head Customer
	groupFoodLock[myGroupNumber] -> Acquire();

	//Place order [Logic has to be implemented for random order of soda and food]
	numberOfSoda[myGroupNumber] += 1;
	numberOfPopcorn[myGroupNumber] += 1;

	//Order is placed so decrement the count and signal head customer
	groupMemberCountForFood[myGroupNumber]--;
	printf("\nGM[%d] of Group %d places order to HeadCustomer[%d]\n",myData->customerNumber,myGroupNumber,myGroupNumber);
	groupFoodLockCV[myGroupNumber] -> Signal(groupFoodLock[myGroupNumber]);

	//printf("\nGM[%d] of Group %d releases groupFoodLock as food order given to HeadCustomer[%d]\n",myData->customerNumber,myGroupNumber,myGroupNumber);

	groupFoodLock[myGroupNumber] -> Release();

	//printf("\nGM[%d] of Group %d releases intra group lock\n",myData->customerNumber,myGroupNumber);
	//release intra lock so that next waiting customer can order to head customer
	intraGroupFoodLock[myGroupNumber] -> Release();

	//Acquire lock to check if Head Customer has received the Order
	groupFoodWaitLock[myGroupNumber] -> Acquire();
	//printf("\nGM[%d] of Group %d acquired group Food Wait lock\n",myData->customerNumber,myGroupNumber);

	//
	if(hasOrdered[myGroupNumber]== ORDER_NOTRECEIVED) {
		//printf("\nGM[%d] of Group %d is waiting for Food Order confirmation to be received from Head Customer\n",myData->customerNumber,myGroupNumber);
		groupFoodWaitFlag[myGroupNumber] = 1; //this is for Head customer's Reference that atleast one group member is waiting for Order Confirmation
		groupFoodWaitCV[myGroupNumber] -> Wait(groupFoodWaitLock[myGroupNumber]);
		printf("\nGM[%d] of Group %d : Food Order confirmation HAS BEEN received from Head Customer\n",myData->customerNumber,myGroupNumber);
	}
	//printf("\nGM[%d] of Group %d released group Food Wait lock\n",myData->customerNumber,myGroupNumber);
	groupFoodWaitLock[myGroupNumber] -> Release();

	//Acquire lock to check if Head Customer has given tickets to Ticket Taker
	groupTicketSubmitLock[myGroupNumber] -> Acquire();
	//printf("\nGM[%d] of Group %d acquired group Ticket Submit Wait lock\n",myData->customerNumber,myGroupNumber);

	//
	if(hasSubmitted[myGroupNumber]== TICKETS_NOTACCEPTED) {
		//printf("\nGM[%d] of Group %d is waiting for Ticket Submit confirmation to be received from Head Customer\n",myData->customerNumber,myGroupNumber);
		groupTicketSubmitFlag[myGroupNumber] = 1; //this is for Head customer's Reference that atleast one group member is waiting for Order Confirmation
		groupTicketSubmitCV[myGroupNumber] -> Wait(groupTicketSubmitLock[myGroupNumber]);
		//printf("\nGM[%d] of Group %d : ticket submit confirmation HAS BEEN received from Head Customer\n",myData->customerNumber,myGroupNumber);
	}
	//printf("\nGM[%d] of Group %d released ticket submit Wait lock\n",myData->customerNumber,myGroupNumber);
	groupTicketSubmitLock[myGroupNumber] -> Release();

	//printf("\nGM[%d] of Group %d acquired groupSeatWaitLock to interact with HeadCustomer[%d]\n",myData->customerNumber,myGroupNumber,myGroupNumber);
	//Acquire this lock to receive the seat number allocated by Head Customer
	groupSeatWaitLock[myGroupNumber] -> Acquire();

	//The seat allocated is received so decrement the counter of group members needed to communicate with HC
	groupMemberCountForSeats[myGroupNumber]--;

	myData -> mySeat = myOccupiedSeats[myGroupNumber][myData -> customerNumber];
	printf("\nGM[%d] allocated Seat [%d] assigned by HeadCustomer[%d] and now waiting for the Movie\n",myData->customerNumber,myData -> mySeat,myGroupNumber);

	//printf("\nGM[%d] of Group %d Signals to HeadCustomer[%d] confirming that seat is allocated\n",myData->customerNumber,myGroupNumber,myGroupNumber);
	groupSeatWaitCV[myGroupNumber] -> Signal(groupSeatWaitLock[myGroupNumber]);

	groupSeatWaitLock[myGroupNumber] -> Release();
	//printf("\nGM[%d] of Group %d releases groupSeatWaitLock to interact with HeadCustomer[%d]\n",myData->customerNumber,myGroupNumber,myGroupNumber);

	//take movie start-stop lock and wait on that
	//printf("\nGM[%d] of Group %d acquires movietechlock to interact with Movie Technician\n",myData->customerNumber,myGroupNumber);
	movieTechLock -> Acquire();

	if(movieFinishState != OVER)
	{
		//printf("\nGM[%d] of Group %d finds that movie is not over\n",myData->customerNumber,myGroupNumber);
		printf("\nGM[%d] of Group %d waiting for movie to get start\n",myData->customerNumber,myGroupNumber);
		movieTechCV -> Wait(movieTechLock);
	}

	movieTechLock -> Release();
	//printf("\nGM[%d] of Group %d releases movietechlock to interact with Movie Technician\n",myData->customerNumber,myGroupNumber);

	myData -> mySeat = 0;

	//printf("\nGM[%d] of Group %d acquires movieexitLock to interact with HeadCustomer[%d]\n",myData->customerNumber,myGroupNumber,myGroupNumber);
	groupExitLock[myGroupNumber]->Acquire();

	groupExitCount[myGroupNumber]--;

	groupExitCV[myGroupNumber] -> Signal(groupExitLock[myGroupNumber]);
	//printf("\nGM[%d] of Group %d Signals to HeadCustomer[%d] regarding waiting to exit theater\n",myData->customerNumber,myGroupNumber,myGroupNumber);

	groupExitLock[myGroupNumber]-> Release();
	//printf("\nGM[%d] of Group %d releases movieexitLock to interact with HeadCustomer[%d]\n",myData->customerNumber,myGroupNumber,myGroupNumber);

	//printf("\nGM[%d] of Group %d acquires groupRegroupLock to interact with HeadCustomer[%d]\n",myData->customerNumber,myGroupNumber,myGroupNumber);
	groupRegroupLock[myGroupNumber]-> Acquire();

	if(groupRegroupState[myGroupNumber] == NOT_REGROUPED)
	{
		//printf("\nGM[%d] of Group %d waits for HeadCustomer[%d] as not regrouped\n",myData->customerNumber,myGroupNumber,myGroupNumber);
		groupRegroupFlag[myGroupNumber] = 1;
		groupRegroupCV[myGroupNumber] -> Wait(groupRegroupLock[myGroupNumber]);
	}

	groupRegroupLock[myGroupNumber] -> Release();
	//printf("\nGM[%d] of Group %d releases groupRegroupLock to interact with HeadCustomer[%d]\n",myData->customerNumber,myGroupNumber,myGroupNumber);

	printf("\nGM[%d] of Group %d may go to bathroom and EXIT theater\n",myData->customerNumber,myGroupNumber);
	customersExit(myData);
}

//===========================================================
//	FUNCTION: Head Customer starts here
//===========================================================
void headCustomer(int currentHC)
{
	CustomerData * current = (CustomerData *) currentHC;
	//printf("\nHeadCustomer[%d] ENTERING interactionForTicketsWithTC to decide and interact with TC\n",current->groupNumber);
	interactionForTicketsWithTC(current);
	printf("\nHeadCustomer[%d] EXITING interactionForTicketsWithTC to decide and interact with TC\n",current->groupNumber);

	//printf("\nHeadCustomer[%d] ENTERING interactWithGroupMembersForFoodOrder to take food order\n",current->groupNumber);
	interactWithGroupMembersForFoodOrder(current);
	printf("\nHeadCustomer[%d] EXITING interactWithGroupMembersForFoodOrder as food order taken\n",current->groupNumber);

	//printf("\nHeadCustomer[%d] ENTERING movieSeatAllocation Function to acquire free seat numbers in theater\n",current->groupNumber);
	movieSeatAllocation(current);
	printf("\nHeadCustomer[%d] EXITING movieSeatAllocation Function as acquired the seats in theater\n",current->groupNumber);

	printf("\nHeadCustomer[%d] watching movie\n",current->groupNumber);
	watchMovieAndExit(current);
	printf("\nHeadCustomer[%d] EXITING as movie is over\n",current->groupNumber);
}

//===========================================================
//	FUNCTION: Concession starts here
//===========================================================
void concessionClerk(int myIndex)
{
	int loopCount = 0;

	while( loopCount < 5)
	{
		loopCount++;

		//acquire line lock to check if anyone is waiting in line
		//printf("\nCC[%d] acquires line lock\n",myIndex);
		concessionClerkLineLock -> Acquire();

		concessionClerkState[myIndex] = CCNOT_BUSY;

		//printf("\nCC[%d] Checks if customers waiting in line\n",myIndex);
		if (concessionClerkLineCount[myIndex] > 0)// Yes I have customers
		{
			//make myself busy
			printf("\nCC[%d] finds %d customers waiting in line\n",myIndex,concessionClerkLineCount[myIndex]);
			concessionClerkState[myIndex] = CCBUSY;

			concessionClerkLineCount[myIndex]--;// as I am going to service this customer, decrement line count by 1

			//printf("\nCC[%d] processes first customer in line by signaling it\n",myIndex);
			//wake up the customer to be serviced
			concessionClerkLineCV[myIndex] -> Signal(concessionClerkLineLock);
		}
		else
		{
			printf("\nCC[%d] has no customers waiting in line, so make CC NOT BUSY\n",myIndex);
			//no one in line so i am not busy, so make myself not busy
			concessionClerkState[myIndex] = CCNOT_BUSY;
		}

		//acquire a lock to start interaction with head customer TT game

		//printf("\nCC[%d] acquires interaction lock\n",myIndex);
		concessionClerkLock[myIndex] -> Acquire();
		// this lock is aacquired before releasing concessionClerkLineLock to aviod contex switching problems

		//printf("\nCC[%d] releases line lock\n",myIndex);
		//as a customer to interact is found then we dont need this lock currently
		concessionClerkLineLock -> Release();

		//printf("\nCC[%d] waiting for customer to come to counter and order popcorn\n",myIndex);
		//wait for customer to come to my counter and order number of popcorns
		concessionClerkCV[myIndex] -> Wait(concessionClerkLock[myIndex]);
		//printf("\nCC[%d] got signal from Customer regarding number of popcorns\n",myIndex);

		//calculate total amount to be paid by the customer, store it in monitor variable and share it with customer
		amt2CC[myIndex] = 5 * numberOfPopcornForCC[myIndex];

		//printf("\nCC[%d] asks customer to give Soda Order\n",myIndex);
		//tell the customer to give the soda order
		concessionClerkCV[myIndex] -> Signal(concessionClerkLock[myIndex]);

		//printf("\nCC[%d] waiting for customer to come to order soda\n",myIndex);
		//wait for customer to come to my counter and order number of sodas
		concessionClerkCV[myIndex] -> Wait(concessionClerkLock[myIndex]);
		//printf("\nCC[%d] got signal from Customer regarding number of sodas\n",myIndex);

		//calculate total amount to be paid by the customer, store it in monitor variable and share it with customer
		amt2CC[myIndex] += 4 * numberOfSodaForCC[myIndex];

		//give customer the bill to be paid and the food order(soda and popcorn)
		concessionClerkCV[myIndex] -> Signal(concessionClerkLock[myIndex]);

		//printf("\nCC[%d] waits for amount %d for Popcorn+Soda Order\n",myIndex,amt2CC[myIndex]);
		//wait for the customer to make payment
		concessionClerkCV[myIndex] -> Wait(concessionClerkLock[myIndex]);

		concessionClerkManagerLock -> Acquire();
		//Yohoo , customer made the payment, add it to the total ticket collection
		totalAmtByCC[myIndex] += amt2CC[myIndex];

		concessionClerkManagerLock -> Release();
		//Yohoo , customer made the payment, add it to the total ticket collection
		

		printf("\nCC[%d] Got money from Customer\n",myIndex);

		//printf("\nCC[%d] releases interaction lock\n",myIndex);
		//as interaction is over, release the lock
		concessionClerkLock[myIndex] -> Release();
	}
}

//===========================================================
//	FUNCTION: Ticket Taker starts here
//===========================================================
void ticketTaker(int myIndex)//myIndex is used to identify the TT to interact with
{
	int loopCount = 0;

	ticketTakerLineLock -> Acquire();
	ticketTakerState[myIndex] = TTNOT_BUSY;
	ticketTakerLineLock -> Release();

	while(true)
		{
			mgrTTCustomerCommonLock -> Acquire();

			ttStateLock -> Acquire();

			if(movieManagerState == NOT_STARTING || seatFullMessage == 1)
			{
				printf("\nTT[%d] Finds state as not accepting or Movie Manager has not yet asked to start Movie\n",myIndex);
				ttStateLock -> Release();
				
				ticketTakerLineLock -> Acquire();
				ticketTakerLineCount[myIndex] = 0;
				lineTicketMessage[myIndex] = 1;
				ticketTakerLineCV[myIndex] -> Broadcast(ticketTakerLineLock);
				ticketTakerLineLock -> Release();
				
				mgrTTCustomerCommonCV -> Wait(mgrTTCustomerCommonLock);
				printf("\nTT[%d] get signal from Manager for Movie Starting\n",myIndex);
				ticketTakerLineLock -> Acquire();
					lineTicketMessage[myIndex] = 0;
				ticketTakerLineLock -> Release();
			}
			else
			{
				printf("\nTT[%d] Finds that its ttticketacceptstate as accepting or moviemanager state is starting\n",myIndex);
				ttStateLock -> Release();
			}

			mgrTTCustomerCommonLock -> Release();

			//acquire line lock to check if anyone is waiting in line
			//printf("\nTT[%d] acquires line lock\n",myIndex);
			ticketTakerLineLock -> Acquire();

			printf("\nTT[%d] Checks if customers waiting in line\n",myIndex);
			if (ticketTakerLineCount[myIndex] > 0)// Yes I have customers
			{
				ttStateLock -> Acquire();
				if(seatFullMessage == 1)
				{
					lineTicketMessage[myIndex] = 1;
					
					//recent change
					ticketTakerLineCount[myIndex] = 0;
					
					ticketTakerLineCV[myIndex] -> Broadcast(ticketTakerLineLock);
					
					//recent change
					//ticketTakerState[myIndex] = TTNOT_BUSY;
					
					
					
					ticketTakerLineLock -> Release();
					
					ttStateLock -> Release();
					
					//mgrTTCustomerCommonLock -> Acquire();
					//mgrTTCustomerCommonCV -> Wait(mgrTTCustomerCommonLock);
					//mgrTTCustomerCommonLock -> Release();
					continue;
				}
				else
				{	
					//make myself busy
					printf("\nTT[%d] finds %d customers waiting in line\n",myIndex,ticketTakerLineCount[myIndex]);
					ticketTakerState[myIndex] = TTBUSY;

					ticketTakerLineCount[myIndex]--;// as I am going to service this customer, decrement line count by 1

					printf("\nTT[%d] processes first customer in line by signaling it\n",myIndex);
					//wake up the customer to be serviced
					lineTicketMessage[myIndex] = 0;
					ticketTakerLineCV[myIndex] -> Signal(ticketTakerLineLock);
					ttStateLock -> Release();
				}
				
			}
			else
			{
				printf("\nTT[%d] has no customers waiting in line, so make TT NOT BUSY\n",myIndex);
				//no one in line so i am not busy, so make myself not busy
				ticketTakerState[myIndex] = TTNOT_BUSY;
			}

			//acquire a lock to start interaction with head customer TT game

			//printf("\nTT[%d] acquires interaction lock\n",myIndex);
			ticketTakerLock[myIndex] -> Acquire();
			// this lock is aacquired before releasing ticketTakerLineLock to aviod contex switching problems

			//printf("\nTT[%d] releases line lock\n",myIndex);
			//as a customer to interact is found then we dont need this lock currently
			ticketTakerLineLock -> Release();

			//printf("\nTT[%d] waiting for customer to come to counter and get tickets checked\n",myIndex);
			//wait for customer to come to my counter and order number of popcorns
			ticketTakerCV[myIndex] -> Wait(ticketTakerLock[myIndex]);
			printf("\nTT[%d] got the tickets from head customer\n",myIndex);

			ttStateLock -> Acquire();

			printf("\nTotal accepted seats = % d for TT[%d] and numberOfTicketsForTT[%d] = %d and tt accept state = %d\n",totalAcceptedSeats,myIndex,myIndex,numberOfTicketsForTT[myIndex],ttTicketAcceptState);
			if(seatFullMessage == 0)
			{
				if(totalAcceptedSeats + numberOfTicketsForTT[myIndex] <= 25)
				{
					totalAcceptedSeats += numberOfTicketsForTT[myIndex];
					printf("\nTT[%d] updates total accepted seats to %d\n",myIndex,totalAcceptedSeats);
					printf("\nTT[%d] informs head customer as tickets are accepted\n",myIndex);

					acceptMessage[myIndex] = 0;
					ticketTakerCV[myIndex] -> Signal(ticketTakerLock[myIndex]);
					if(totalAcceptedSeats == 25)
					{
						seatFullMessage = 1;
						
						ticketTakerLineLock -> Acquire();
						ticketTakerLineCount[myIndex] = 0;
						lineTicketMessage[myIndex] = 1;
						ticketTakerLineCV[myIndex] -> Broadcast(ticketTakerLineLock);
						ticketTakerLineLock -> Release();
						
						ttStateLock -> Release();
						
						ticketTakerLock[myIndex] -> Release();
						
						//mgrTTCustomerCommonLock -> Acquire();
						//mgrTTCustomerCommonCV -> Wait(mgrTTCustomerCommonLock);
						//mgrTTCustomerCommonLock -> Release();
						continue;
					}
					
						
				}
				else
				{
					seatFullMessage = 1;
					acceptMessage[myIndex] = 1;
					ticketTakerCV[myIndex] -> Signal(ticketTakerLock[myIndex]);
					
					ticketTakerLineLock -> Acquire();
					ticketTakerLineCount[myIndex] = 0;
					lineTicketMessage[myIndex] = 1;
					ticketTakerLineCV[myIndex] -> Broadcast(ticketTakerLineLock);
					ticketTakerLineLock -> Release();
					
					ttStateLock -> Release();
					
					ticketTakerLock[myIndex] -> Release();
					
					//mgrTTCustomerCommonLock -> Acquire();
					//mgrTTCustomerCommonCV -> Wait(mgrTTCustomerCommonLock);
					//mgrTTCustomerCommonLock -> Release();
					continue;
				}
			}
			else
			{
				acceptMessage[myIndex] = 1;
				ticketTakerCV[myIndex] -> Signal(ticketTakerLock[myIndex]);

				ticketTakerLineLock -> Acquire();
				lineTicketMessage[myIndex] = 1;
				ticketTakerLineCount[myIndex] = 0;
				ticketTakerLineCV[myIndex] -> Broadcast(ticketTakerLineLock);
				ticketTakerLineLock -> Release();
				
				ttStateLock -> Release();
				
				ticketTakerLock[myIndex] -> Release();
				
				
				//mgrTTCustomerCommonLock -> Acquire();
				//mgrTTCustomerCommonCV -> Wait(mgrTTCustomerCommonLock);
				//mgrTTCustomerCommonLock -> Release();
				continue;
			}
			ttStateLock -> Release();
			ticketTakerLock[myIndex] -> Release();
			
			
		}
}

//===========================================================
//	FUNCTION: Ticket Clerk Starts here
//===========================================================
void ticketClerk(int myIndex)//myIndex is used to indentify the TC to interact with
{
	int loopCount = 0;
	while(loopCount < 5)
	{
		loopCount++;

		//printf("\nTC[%d] acquires line lock\n",myIndex);
		ticketClerkLineLock -> Acquire();

		if(ticketClerkState[myIndex] == ON_BREAK)
				{
					ticketClerkManagerLock -> Acquire();
					//printf("\nTicketClerk[%d] Acquiring ClerkManagerLock to check if he is allowed to go on break\n", myIndex);
					ticketClerkBreakCount++;	// Going on break
					//ticketClerkLineLock -> Acquire();
					//printf("\nTicketClerk[%d] releasing LineLock before going on break\n", myIndex);
					ticketClerkLineCV[myIndex] -> Broadcast(ticketClerkLineLock);
					printf("\nTicketClerk[%d] broadcasting all customer in his line to go to other line as he is going to break\n", myIndex);
					ticketClerkLineLock -> Release();
					//printf("\nTicketClerk[%d] releasing LineLock before going on break\n", myIndex);

					printf("\nTicketClerk[%d] waiting for the manager to call him off break\n", myIndex);
					ticketClerkManagerCV -> Wait(ticketClerkManagerLock);
					ticketClerkBreakCount--;
					ticketClerkState[myIndex] = OFF_BREAK;
					ticketClerkLineLock -> Acquire();
					ticketClerkManagerLock -> Release();
					printf("\n TicketClear[%d] releasing ClerkManagerLock after coming off break\n", myIndex);
				}


		//acquire lock to check if anyone is waiting in line
		//ticketClerkLineLock -> Acquire();
		ticketClerkState[myIndex] = TCNOT_BUSY;

		//printf("\nTC[%d] Checks if customers waiting in line\n",myIndex);
		if (ticketClerkLineCount[myIndex] > 0)// Yes I have customers
		{
			//make myself busy
			//printf("\nTC[%d] finds %d customers waiting in line\n",myIndex,ticketClerkLineCount[myIndex]);
			ticketClerkState[myIndex] = TCBUSY;

			ticketClerkLineCount[myIndex]--;// as I am going to service this customer, decrement line count by 1

			//printf("\nTC[%d] processes first customer in line by signaling it\n",myIndex);
			//wake up the customer to be serviced
			ticketClerkLineCV[myIndex] -> Signal(ticketClerkLineLock);
		}
		else
		{
			printf("\nTC[%d] has no customers waiting in line, so make TC NOT BUSY\n",myIndex);
			//no one in line so i am not busy, so make myself not busy
			ticketClerkState[myIndex] = TCNOT_BUSY;
		}

		//acquire a lock to start interaction with customer TT game

		//printf("\nTC[%d] acquires interaction lock\n",myIndex);
		ticketClerkLock[myIndex] -> Acquire();
		// this lock is aacquired before releasing ticketClerkLineLock to aviod contex switching problems

		//printf("\nTC[%d] releases line lock\n",myIndex);
		//as a customer to interact is found then we dont need this lock currently
		ticketClerkLineLock -> Release();

		//printf("\nTC[%d] waiting for customer to come for interaction on interaction lock\n",myIndex);
		//wait for customer to come to my counter
		ticketClerkCV[myIndex] -> Wait(ticketClerkLock[myIndex]);
		//printf("\nTC[%d] got signal from Customer regarding number of tickets\n",myIndex);

		//calculate total amount to be paid by the customer, store it in monitor variable and share it with customer
		amt2TC[myIndex] = 12 * numberOfTicketsForTC[myIndex];

		//printf("\nTC[%d] asks customer to pay the amount %d for %d tickets\n",myIndex,amt2TC[myIndex],numberOfTicketsForTC[myIndex]);
		//tell the customer the amount to be paid
		ticketClerkCV[myIndex] -> Signal(ticketClerkLock[myIndex]);

		//printf("\nTC[%d] waits for amount %d for %d tickets\n",myIndex,amt2TC[myIndex],numberOfTicketsForTC[myIndex]);
		//wait for the customer to make payment
		ticketClerkCV[myIndex] -> Wait(ticketClerkLock[myIndex]);

		ticketClerkManagerLock -> Acquire();
		//Yohoo , customer made the payment, add it to the total ticket collection
		totalAmtByTC[myIndex] += amt2TC[myIndex];

		ticketClerkManagerLock -> Release();

		printf("\nTC[%d] gave tickets to Customer\n",myIndex);
		//provide customer the tickets
		ticketClerkCV[myIndex] -> Signal(ticketClerkLock[myIndex]);

		//printf("\nTC[%d] releases interaction lock\n",myIndex);
		//as interaction is over, release the lock
		ticketClerkLock[myIndex] -> Release();
	}
}

void randomYield(int waitCount)
{
	while(waitCount-- > 0)
	{
		currentThread -> Yield();

	}
}

//===========================================================
//	FUNCTION: Movie Technician Starts here
//===========================================================
void movieTechnician(int noUse) {

	while(true)
	{
		printf("\nMovie Technician Acquires movieTechManagerLock to interact with Manager to check when to start movie\n");
		movieTechManagerLock -> Acquire();

		if(commandToStart == 0)//commandtostart must be 1 to start movie
		{
			printf("\nMovie Tech waits for signal from Manager to start movie\n");
			movieTechManagerCV -> Wait(movieTechManagerLock);
		}

		int simulationTime = (rand() % (300 - 200 + 1) + 200);

		randomYield(simulationTime);

		printf("\nMovie Technician Acquires movieTechLock to change state to movie Over\n");
		movieTechLock -> Acquire();
		movieFinishState = OVER;

		commandToStart = 0;
		printf("\nMovie Tech gets signal from Manager and starts movie\n");
		movieTechManagerLock -> Release();

		printf("\nMovie Technician signals all customers watching movie inside theater that movie is over\n");
		movieTechCV -> Broadcast(movieTechLock);
		movieTechLock -> Release();
		printf("\nMovie Technician releases movieTechLock as all customers informed\n");
	}

}

//===========================================================
//	FUNCTION: Manager Starts here
//===========================================================
void theaterManager(int noUse) {

	int random=0;
	int loop = 0;
	while(true)
	{

		int simulationTime = (rand() % (600 - 200 + 1) + 400);
		randomYield(simulationTime);

		for(int i = 0;i < MAX_TC; i++)
		{
			ticketClerkManagerLock -> Acquire();	// Ticket clerk lock acquired for interaction
			printf("\nTheater Manager Acquires ticketClerkManagerLock to interact with movie Technician\n");
			ticketClerkLineLock -> Acquire();		// lock for checking the line count of ticket clerk
			printf("\nTheater Manager Acquires ticketClerkLineLock to interact with movie Technician\n");

			if((ticketClerkState[i] != ON_BREAK) || ticketClerkState[i] == TCNOT_BUSY)
			{
				printf("\nManager Finds ticketClerkState[%d] == OFF_BREAK) || ticketClerkState[%d] == TCNOT_BUSY\n",i,i);
				//determine whether TC should go on break randomly
				//set ONBREAK = TRUE
				random = rand() % 10;
				if(((ticketClerkLineCount[i] == 0 && random < 2)))
				{
					ticketClerkState[i] = ON_BREAK;
					printf("\nSending TicketClerk[%d] on break", i);
				}
			}
			else if(ticketClerkLineCount[i] > 5 && ticketClerkBreakCount > 0)
			{
				printf("\nManager Finds ticketClerkLineCount[%d] > 5 && ticketClerkBreakCount = %d which is greater than 0\n",i,ticketClerkBreakCount);
				printf("\nSo, Manager signals some TC to come OFF BREAK\n");
				ticketClerkManagerCV -> Signal(ticketClerkManagerLock);
				managerCustomerFlag[i] = 1;
				printf("\nAs new TC comes OFF BREAK, Manager Informs all customers standing in TC[%d] to go into another line\n",i);
				ticketClerkLineCV[i] -> Broadcast(ticketClerkLineLock);
				//ticketClerkManagerLock -> Release();	//Ticket clerk interaction lock released
				printf("\nCalling one TicketClerk off break");
			}
			printf("\nManager releases ticketClerkLineLock\n");
			ticketClerkLineLock -> Release();		// releasing the lock to check the line count of ticket clerk
			printf("\nManager releases ticketClerkManagerLock\n");
			ticketClerkManagerLock -> Release();
		}

		randomYield(simulationTime);

		//===============================
		randomYield(simulationTime);
		randomYield(simulationTime);
		randomYield(simulationTime);
		randomYield(simulationTime);


		movieTechManagerLock -> Acquire();
		printf("\nManager Acquires movieTechManagerLock to check ability to signal Movie Starting to MT, TT and Customers, checks MT - MV\n");

		ttStateLock -> Acquire();
		printf("\ntotalMovieRoomExitCount: %d  totalAcceptedSeats: %d && movieFinishState: %d\n",totalMovieRoomExitCount,totalAcceptedSeats,movieFinishState);
		if(movieFinishState == FIRST_MOVIE)
		{
			mgrTTCustomerCommonLock -> Acquire();
			printf("\nManager acquires mgrTTCustomerCommonLock: STARTING FIRST MOVIE\n");
			seatFullMessage = 0;
			movieManagerState = STARTING;
			
			ticketTakerLineLock -> Acquire();
			for(int k=0; k< MAX_TT ;k++)
			{
				lineTicketMessage[k] = 0;
			}
			ticketTakerLineLock -> Release();
			
			printf("\nManager makes movieManagerState = STARTING : FIRST MOVIE\n");
			mgrTTCustomerCommonCV -> Broadcast(mgrTTCustomerCommonLock);
			printf("\nManager BroadCasts: STARTING FIRST MOVIE to TT,Customers, MovieTech : FIRST MOVIE\n");
			printf("\nManager makes movieFinishState = OVER : FIRST MOVIE\n");
			//movieFinishState = OVER;
			printf("\nManager RELEASES mgrTTCustomerCommonLock\n");
			mgrTTCustomerCommonLock -> Release();
		}
		else if(totalMovieRoomExitCount == totalAcceptedSeats && movieFinishState == OVER && totalAcceptedSeats!=0)
		{

			printf("\nManager Finds totalMovieRoomExitCount and totalAcceptedSeats = %d && movieFinishState == OVER\n",totalAcceptedSeats);
			for(int i=0;i < 5; i++)
			{
				for(int j =0; j <5 ;j++)
				{
					seats[i][j] = SEATS_FREE;
				}
				numberOfFreeSeats[i] = 5;
			}
			printf("\nManager acquires movieSeatManagerLock to change totalOccupiedSeats and totalAcceptedSeats to zero\n");
			movieSeatManagerLock -> Acquire();
			printf("\nMaking the total occupied seats as 0 as the movie is over and all the customers have exited the movie room\n");
			totalOccupiedSeats = 0;
			printf("\nMaking the total accepted seats by TTs as 0 as the movie is over and all the customers have exited the movie room\n");
			totalAcceptedSeats = 0;
			
			printf("\nMaking the seat full message as 0 as the movie is over and all the customers have exited the movie room\n");
			seatFullMessage = 0;
			movieSeatManagerLock -> Release();
			totalMovieRoomExitCount = 0;
			
			mgrTTCustomerCommonLock -> Acquire();
			printf("\nManager acquires mgrTTCustomerCommonLock to change movieManagerState = STARTING;\n");
			movieManagerState = STARTING;
			
			ticketTakerLineLock -> Acquire();
			for(int k=0; k< MAX_TT ;k++)
			{
				lineTicketMessage[k] = 0;
			}
			ticketTakerLineLock -> Release();
			
			printf("\nManager BroadCasts: STARTING notFirst MOVIE to TT,Customers, MovieTech\n");
			mgrTTCustomerCommonCV -> Broadcast(mgrTTCustomerCommonLock);
			printf("\nManager RELEASES mgrTTCustomerCommonLock\n");
			mgrTTCustomerCommonLock -> Release();
		}
		ttStateLock -> Release();
		printf("\nManager releases movieTechManagerLock after check ability to signal Movie Starting to MT, TT and Customers, checks MT - MV\n");
		movieTechManagerLock -> Release();

		randomYield(simulationTime);

		randomYield(simulationTime);

		randomYield(simulationTime);

		randomYield(simulationTime);

		ticketClerkManagerLock -> Acquire();
		//Yohoo , customer made the payment, add it to the total ticket collection
		
		for(int i=0;i<MAX_TC;i++)
		{
			totalTicketClerkAmount += totalAmtByTC[i];
			printf("\nTotal amount collected by TC[%d] at the moment = %d\n",i,totalAmtByTC[i]);
		}
		
		ticketClerkManagerLock -> Release();
		
		randomYield(simulationTime);

		randomYield(simulationTime);

		randomYield(simulationTime);

		randomYield(simulationTime);

		concessionClerkManagerLock -> Acquire();
		for(int i=0;i<MAX_CC;i++)
		{
			totalConcessionClerkAmount += totalAmtByCC[i];
			printf("\nTotal amount collected by CC[%d] at the moment = %d\n",i,totalAmtByCC[i]);
		}
		
		concessionClerkManagerLock -> Release();
		
		randomYield(simulationTime);

		randomYield(simulationTime);

		randomYield(simulationTime);

		randomYield(simulationTime);
		//======================================

		/*ttStateLock -> Acquire(); // acquire tt lock to check if all TTs have stopped accepting tickets
		int ttStateCheckCount = 0;
		for(int i=0 ; i < MAX_TT ; i++)
		{
			if(ttTicketAcceptState == NOT_ACCEPTING)
			{
				ttStateCheckCount++;
			}
		}*/

		movieTechManagerLock -> Acquire();
		printf("\nTheater Manager Acquires movieTechManagerLock to interact with movie Technician\n");
		movieSeatManagerLock -> Acquire();
		printf("\nTheater Manager Acquires movieSeatManagerLock to interact with movie Technician\n");

		//ttStateLock -> Acquire();

		printf("\ncommandToStart == %d && totalOccupiedSeats = %d totalAcceptedSeats = %d seatFullMessage = %d\n",commandToStart,totalOccupiedSeats,totalAcceptedSeats,seatFullMessage);

		if(commandToStart == 0 && totalOccupiedSeats == totalAcceptedSeats && (movieFinishState == OVER || movieFinishState == FIRST_MOVIE) && seatFullMessage == 1 && totalAcceptedSeats!=0)
		{
			printf("\nTheater Manager asks movie Tech to start a movie\n");
			commandToStart = 1;
			movieFinishState = NOT_OVER;
			
			movieTechManagerCV -> Signal(movieTechManagerLock);
			mgrTTCustomerCommonLock -> Acquire();
			movieManagerState = NOT_STARTING;
			mgrTTCustomerCommonLock -> Acquire();
		}
		printf("\nTheater Manager releases ttStateLock to interact with movie Technician\n");
		//ttStateLock -> Release();
		printf("\nTheater Manager releases movieSeatManagerLock to interact with movie Technician\n");
		movieSeatManagerLock -> Release();
		printf("\nTheater Manager releases movieTechManagerLock to interact with movie Technician\n");
		movieTechManagerLock -> Release();
		printf("\nTheater Manager Releases movieTechManagerLock\n");

		//======================

		customerManagerExitLock -> Acquire();
		printf("\nTheater Manager Acquires customerManagerExitLock to interact with movie Technician\n");
		printf("\nExit Count = %d Customer Incount = %d\n",allCustomerExitedCount,totalCustomerInCount);
		if(allCustomerExitedCount == totalCustomerInCount)
		{
			printf("\nTheater Manager finds allCustomerExitedCount == totalCustomerInCount\n");
			printf("\nTheater Manager releases customerManagerExitLock to interact with movie Technician\n");
			customerManagerExitLock -> Release();
			printf("\nManager Exited Work Complete\n");
			break;
		}
		printf("\nTheater Manager releases customerManagerExitLock to interact with movie Technician\n");
		customerManagerExitLock -> Release();


		randomYield(simulationTime);
		randomYield(simulationTime);
		randomYield(simulationTime);
	}
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++
void TestSuite() {

	//int numberOfGroups = rand()%10 + 1;

	int numberOfGroups = 10;

	//Create Locks, Condition Variables and initialize monitor variables
	ticketClerkManagerLock = new Lock("ticketClerkManagerLock");
	ticketClerkManagerCV = new Condition("ticketClerkManagerCV");

	concessionClerkManagerLock = new Lock("concessionClerkManagerLock");
	
	movieTechManagerLock = new Lock("movieTechManagerLock");
	movieTechManagerCV = new Condition("movieTechManagerCV");

	movieSeatManagerLock = new Lock("movieSeatManagerLock");

	ticketClerkLineLock = new Lock("ticketClerkLineLock");

	movieSeatLock = new Lock("movieSeatLock");

	movieTechLock = new Lock("movieTechLock");

	movieTechCV = new Condition("movieTechCV");

	mgrTTCustomerCommonLock = new Lock("mgrTTCustomerCommonLock");
	mgrTTCustomerCommonCV = new Condition("mgrTTCustomerCommonCV");
	ttStateLock = new Lock("ttStateLock");

	customerManagerExitLock = new Lock("customerManagerExitLock");

	for (int i=0;i<MAX_TC;i++)
	{
		ticketClerkLineCV[i] = new Condition("ticketClerkLineCV");
		ticketClerkLock[i] = new Lock("ticketClerkLock");
		ticketClerkCV[i] = new Condition("ticketClerkCV");
		ticketClerkState[i] = TCBUSY;
	}

	concessionClerkLineLock = new Lock("concessionClerkLineLock");
	for (int i=0;i<MAX_CC;i++)
	{
		concessionClerkLineCV[i] = new Condition("concessionClerkLineCV");
		concessionClerkLock[i] = new Lock("concessionClerkLock");
		concessionClerkCV[i] = new Condition("concessionClerkCV");
		concessionClerkState[i] = CCBUSY;
	}

	ticketTakerLineLock = new Lock("ticketTakerLineLock");
	ttTicketAcceptState = NOT_ACCEPTING;
	for (int i=0;i<MAX_TT;i++)
	{
		ticketTakerLineCV[i] = new Condition("ticketTakerLineCV");
		ticketTakerLock[i] = new Lock("ticketTakerLock");
		ticketTakerCV[i] = new Condition("ticketTakerCV");
		ticketTakerState[i] = TTBUSY;

	}

	for(int i=0;i<numberOfGroups;i++)
	{
		groupTicketLock[i] = new Lock("groupTicketLock");
		groupTicketLockCV[i] = new Condition("groupTicketLockCV");
		hasTickets[i] = NOT_BOOKED;

		groupFoodLock[i] = new Lock("groupFoodLock");
		groupFoodLockCV[i] = new Condition("groupFoodLockCV");
		intraGroupFoodLock[i] = new Lock("intraGroupFoodLock");
		numberOfSoda[i] = 0;
		numberOfPopcorn[i] = 0;

		groupFoodWaitLock[i]=new Lock("groupFoodWaitLock");
		groupFoodWaitCV[i]=new Condition("groupFoodWaitLock");
		hasOrdered[i]=ORDER_NOTRECEIVED;
		groupFoodWaitFlag[i]=0;

		groupTicketSubmitLock[i] = new Lock("groupTicketSubmitLock");
		groupTicketSubmitCV[i] = new Condition("groupTicketSubmitLock");
		hasSubmitted[i] = TICKETS_NOTACCEPTED;
		groupTicketSubmitFlag[i] = 0;

		groupSeatWaitLock[i] = new Lock("groupSeatWaitLock");
		groupSeatWaitCV[i] = new Condition("groupSeatWaitCV");

		groupExitLock[i] = new Lock("groupExitLock");
		groupExitCV[i] = new Condition("groupExitCV");

		groupRegroupLock[i] = new Lock("groupRegroupLock");
		groupRegroupCV[i] = new Condition("groupRegroupCV");
		groupRegroupState[i] = NOT_REGROUPED;

		groupTheaterExitLock[i] = new Lock("groupTheaterExitLock");
		groupTheaterExitCV[i] = new Condition("groupTheaterExitCV");
	}

	//Fork all required Threads
	Thread *managerThread = new Thread("Manager");
	managerThread -> Fork (theaterManager,0);

	Thread *movieTechThread = new Thread("Movie Technician");
	movieTechThread -> Fork (movieTechnician,0);

	for(int i=0; i<MAX_TC; i++) {
		Thread *tcThread = new Thread("Ticket Clerk");
		tcThread -> Fork (ticketClerk, i);
	}

	for(int i=0; i<MAX_CC; i++) {
		Thread *ccThread = new Thread("Concession Clerk");
		ccThread -> Fork (concessionClerk, i);
	}

	for(int i=0; i<MAX_TT; i++) {
		Thread *ttThread = new Thread("Ticket Taker");
		ttThread -> Fork (ticketTaker, i);
	}

	printf("\n### Created Ticket Clerk ###\n");
	for ( int i=0; i<numberOfGroups; i++ ) {
		//Generate the size of the group, not counting head customer
		//int groupSize = (rand()%5) + 1;
		int groupSize = 5;

		int nextCustomerNumber = 0;
		//Create and fork the head customer thread
		//The head customer of a group needs a group number, group size,
		//and a customer number. I will store all 3 values in a struct pointer and pass
		//it as the second argument to Fork
		CustomerData *current = new CustomerData;
		current->customerNumber = nextCustomerNumber;
		nextCustomerNumber++;//don't need a lock as only this main thread does this
		current->groupNumber = i;
		current->groupSize = groupSize;
		groupMemberCountForFood[i]=groupSize;

		customerManagerExitLock -> Acquire();
		totalCustomerInCount++;
		customerManagerExitLock -> Release();

		Thread *t = new Thread("Head Customer");
		t->Fork( headCustomer, (int) current );

		printf("\n### Created Group Leader %d ###\n",i);

		//now make all the regular Customers for this group
		for ( int j=1; j<groupSize; j++ ) {
			CustomerData *currentGM = new CustomerData;
			currentGM->customerNumber = nextCustomerNumber;
			nextCustomerNumber++;
			currentGM->groupNumber = i;
			currentGM->groupSize = groupSize;
			customerManagerExitLock -> Acquire();
			totalCustomerInCount++;
			customerManagerExitLock -> Release();
			Thread *tGM = new Thread("Group Member");
			tGM->Fork(customerGroupMember, (int) currentGM );
		}
	}

}
#endif
