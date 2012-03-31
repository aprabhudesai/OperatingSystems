#include "copyright.h"
#include "system.h"
#ifdef CHANGED
#include "synch.h"
#endif

#ifdef CHANGED

//=== Customer Data Structure used to store customer specific data ====
struct CustomerData
{
	int customerNumber;
	int groupNumber;
	int groupSize; //This value is used by the group head
	int mySeat;
};

// Define the maximum count of each entity in the application
#define MAX_TC 5 //Maximum number of TicketClerks
#define MAX_CC 5 //Maximum number of ConcessionClerks
#define MAX_TT 3 //Maximum number of TicketTakers
#define MAX_GROUPS 100	// Maximum number of Customer Groups
#define NUM_OF_GROUPS 10// Maximum number of Customer Groups

Lock *ticketClerkLineLock;	// For mutual exclusion to enter Ticket Taker Line
Condition *ticketClerkLineCV[MAX_TC];	//To wait on the line lock to start interaction with customer
int ticketClerkLineCount[MAX_TC] = {0};	//To maintain the count of customer in each clerk's line
enum TCSTATE {TCBUSY , TCNOT_BUSY};		//To check the state of TicketClerk if is busy or available
TCSTATE ticketClerkState[MAX_TC] = {TCBUSY};	//Initialize the state to busy so that if customers comes up before TicketClerk they will have to wait
int managerCustomerTCFlag[MAX_TC] = {0};	//Used to inform customers in line to join a new line
enum BREAKSTATE {ON_BREAK,OFF_BREAK};		//To check the Break status of TicketClerk if a TicketClerk is working or not
BREAKSTATE TCBreakState[MAX_TC] = {OFF_BREAK};	//Initializing break state to OFF_BREAK

int numberOfTicketsForTC[MAX_TC] = {0};	//A monitor used by the HeadCustomer to ask for required number of tickets from TicketClerk
Lock *ticketClerkLock[MAX_TC];	//A lock between TicketClerk and HeadCustomer for mutual exclusion during the interaction
Condition *ticketClerkCV[MAX_TC];	//CV to wait on a TicketClerk lock to begin interaction
int amt2TC[MAX_TC] = {0}, totalAmtByTC[MAX_TC] = {0};	//Used to keep the amount collected by TicketClerk during a transaction and total amount collected by each clerk

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
BREAKSTATE CCBreakState[MAX_TC] = {OFF_BREAK};
int managerCustomerCCFlag[MAX_CC] = {0};

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

Lock *groupTheaterExitLock[MAX_GROUPS];
Condition *groupTheaterExitCV[MAX_GROUPS];
int groupTheaterExitCount[MAX_GROUPS] = {0};

Lock *movieTechManagerLock;
Condition *movieTechManagerCV;
int commandToStart = 0;

Lock *ticketClerkManagerLock;
Condition *ticketClerkManagerCV;

Lock *concessionClerkManagerLock;

enum BREAKSTATUS {NO, YES};
BREAKSTATUS onBreak = NO;

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
int remainingCount = 0;

Lock *ticketClerkManagerBreakLock;
Condition *ticketClerkManagerBreakCV[MAX_TC];
int ticketClerkBreakCount = 0;

Lock *concessionClerkManagerBreakLock;
Condition *concessionClerkManagerBreakCV[MAX_TC];
int concessionClerkBreakCount = 0;

int amt2TCReadAmount[MAX_TC] = {0};
int numberOfTicketsForTCReadAmount[MAX_TC] = {0};
int totalAmtByTCReadAmount[MAX_TC] = {0};

int numberOfGroups  = 0;
int groupMemCount[MAX_GROUPS]={5};

int numberOfPopcornForCCTotalSale[MAX_CC] = {0};
int numberOfSodaForCCTotalSale[MAX_CC] = {0};
int amt2CCTotalSale[MAX_CC] = {0};

int numberOfTicketsForTCTotalSale[MAX_TC] = {0};
int amt2TCTotalSale[MAX_TC] = {0};
int totalAmtByTCTotalSale= 0;
int totalAmtByCCTotalSale = 0;

int numberOfTT = 0;
int numberOfTC = 0;
int numberOfCC = 0;
int numberOfTicketsForTCInteract[MAX_TC] = {0};

//+++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++


// Interaction: Customer with Ticket Clerk
//===========================================================
//===========================================================
//	FUNCTION: Interaction of HeadCustomer with Ticket Clerk
//===========================================================
void interactionForTicketsWithTC(CustomerData * current)
{
	int myTicketClerk = -1;

	int myGroupSize = current -> groupSize;

	while(myTicketClerk == -1)
	{
		//HeadCustomer acquired line lock
		ticketClerkLineLock -> Acquire();

		//Check if any TicketClerk is not busy
		for(int i = 0; i < MAX_TC; i++)
		{
			if(TCBreakState[i] == ON_BREAK)
			{
				printf("\nCustomer %d in Group %d sees TicketClerk %d is on break\n", current->customerNumber, current->groupNumber, i);
			}
			//if(ticketClerkState[i] == TCNOT_BUSY)
			if(ticketClerkState[i] == TCNOT_BUSY && TCBreakState[i] == OFF_BREAK)
			{
				//found a free TC, so make it your TicketClerk and change its status to BUSY
				myTicketClerk = i;
				ticketClerkState[i] = TCBUSY;
				break;
			}
		}

		//If all are on break - condition has to be used

		//If no free TC available then check for shortest line of TC
		if(myTicketClerk == -1)
		{
			int shortestTCLine = 0; //by default the shortest line is the 1st one
			int shortestTCLineLength = ticketClerkLineCount[0];
			for(int i = 1; i < MAX_TC; i++)
			{
				if ((shortestTCLineLength > ticketClerkLineCount[i]) && (ticketClerkState[i] == TCBUSY && TCBreakState[i] == OFF_BREAK))
				{
					//HeadCustomer found TC with shortest line
					shortestTCLineLength = ticketClerkLineCount[i];
					shortestTCLine = i;
				}
			}
				myTicketClerk = shortestTCLine;
				//get into the line
				ticketClerkLineCount[myTicketClerk]++;
				//waiting in TC for its turn
				ticketClerkLineCV[myTicketClerk] -> Wait(ticketClerkLineLock);
				printf("\nCustomer %d in Group %d is getting in TicketClerk line %d\n",current->customerNumber, current->groupNumber, myTicketClerk);

				//if managerCustomerTCFlag[myTicketClerk] == 1 then current TC is going on Break and customer needs to get in another line
				if(managerCustomerTCFlag[myTicketClerk] == 1 || managerCustomerTCFlag[myTicketClerk] ==  2)
				{
					if(managerCustomerTCFlag[myTicketClerk] == 2) //if 2 then current TC has more than 5 customers and so other TC has come OFF BREAK
					{
						managerCustomerTCFlag[myTicketClerk] = 0;  //initialize current TC flag to 0, OFF_BREAK
					}
					myTicketClerk = -1;
					ticketClerkLineLock -> Release();
					continue;
				}
			}

		ticketClerkLineLock ->Release();
	}

	//acquire then new lock to start interaction with ticket clerk like TT game
	ticketClerkLock[myTicketClerk] -> Acquire();

	//pass no. of tickets and wake up clerk
	numberOfTicketsForTC[myTicketClerk] = myGroupSize;

	printf("\nCustomer %d in Group %d is walking up to TicketClerk %d to buy %d tickets\n",current->customerNumber,current->groupNumber, myTicketClerk, numberOfTicketsForTC[myTicketClerk]);

	//signal the TC regarding no. of tickets
	ticketClerkCV[myTicketClerk] -> Signal(ticketClerkLock[myTicketClerk]);

	//wait for response from TC
	ticketClerkCV[myTicketClerk] -> Wait(ticketClerkLock[myTicketClerk]);

	//pay the amount by signalling the TC
	printf("\nCustomer %d in Group %d in TicketClerk line %d is paying %d for tickets\n",current->customerNumber,current->groupNumber, myTicketClerk, numberOfTicketsForTC[myTicketClerk]*12 );
	ticketClerkCV[myTicketClerk] -> Signal(ticketClerkLock[myTicketClerk]);

	//wait here for ticket
	ticketClerkCV[myTicketClerk] -> Wait(ticketClerkLock[myTicketClerk]);

	//hasTickets[MAX_GROUPS] is a monitor variable
	hasTickets[current -> groupNumber] = BOOKED;

	printf("\nCustomer %d in Group %d is leaving TicketClerk %d\n",current->customerNumber, current->groupNumber, myTicketClerk);

	//------------------------------------------------end of booking of tickets section--------------------------------------------------//

	//Interaction: Head Customer with group members
	//Acquire group ticket lock to start with interaction
	groupTicketLock[current -> groupNumber] -> Acquire();

	//release the TC lock acquired above for booking tickets
	ticketClerkLock[myTicketClerk] -> Release();

	if(current -> groupSize > 1)	//inform only if there are other members in your group
	{
		//signal waiting group members about the booked tickets by broadcasting
		groupTicketLockCV[current -> groupNumber] -> Broadcast(groupTicketLock[current -> groupNumber]);
	}

	//------------------------------------------------informed all the group members tickets booked-------------------------------------------//

	//Acquire Food Lock for Head Customer to interact with group members
	groupFoodLock[current -> groupNumber] -> Acquire();

	//release the TC lock as the tickets are bought
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

	//search a Concession Clerk to order food
	while(myConcessionClerk == -1)
	{
		//acquired ConcessionClerk line lock
		concessionClerkLineLock -> Acquire();

		for(int i = 0; i < numberOfCC; i++)	//Check if any concessionClerk is not busy
		{
			if(CCBreakState[i] == ON_BREAK)
			{
				printf("\nCustomer %d in Group %d sees ConcessionClerk %d is on break.\n",current->customerNumber, current->groupNumber,i);
			}
			if(concessionClerkState[i] == CCNOT_BUSY && CCBreakState[i] == OFF_BREAK)
			{
				//found a free CC, so make it your concession clerk
				myConcessionClerk = i;
				concessionClerkState[i] = CCBUSY;
				break;
			}
		}

		//If all are on break - condition has to be used

		//If no free CC available then check for shortest line of CC
		if(myConcessionClerk == -1)
		{
			//HeadCustomer did not find free CC
			int shortestCCLine = 0; //by default the shortest line is the 1st one

			int shortestCCLineLength = concessionClerkLineCount[0];
			for(int i = 1; i < numberOfCC; i++)
			{
				if ((shortestCCLineLength > concessionClerkLineCount[i]) && (concessionClerkState[i] == CCBUSY && CCBreakState[i] == OFF_BREAK))
				{
					//found shortest ConcessionClerk line so get in that line
					shortestCCLineLength = concessionClerkLineCount[i];
					shortestCCLine = i;
				}
			}
				myConcessionClerk = shortestCCLine;
				//get into the line
				concessionClerkLineCount[myConcessionClerk]++;

				//waiting in ConcessionClerk line for your turn
				printf("\nCustomer %d in Group %d is getting in ConcessionClerk line %d \n", current->customerNumber, current->groupNumber, myConcessionClerk);
				concessionClerkLineCV[myConcessionClerk] -> Wait(concessionClerkLineLock);

				if(managerCustomerCCFlag[myConcessionClerk] == 1 || managerCustomerCCFlag[myConcessionClerk] ==  2)
				{
					if(managerCustomerCCFlag[myConcessionClerk] == 2)
					{
						managerCustomerCCFlag[myConcessionClerk] = 0;
					}
					myConcessionClerk = -1;
					concessionClerkLineLock -> Release();
					continue;
				}
			}

		//Release the line lock as you have shortest line
		concessionClerkLineLock ->Release();
	}

	//acquire then new lock to start interaction with Concession clerk
	concessionClerkLock[myConcessionClerk] -> Acquire();





	//pass popcorn order and wake up clerk
	numberOfPopcornForCC[myConcessionClerk] = numberOfPopcorn[current->groupNumber];

	//signal the ConcessionClerk regarding no. of popcorns
	concessionClerkCV[myConcessionClerk] -> Signal(concessionClerkLock[myConcessionClerk]);

	//wait for ConcessionClerk to give acknowledgement for popcorn order
	concessionClerkCV[myConcessionClerk] -> Wait(concessionClerkLock[myConcessionClerk]);

	//pass no. of sodas and wake up clerk
	numberOfSodaForCC[myConcessionClerk] = numberOfSoda[current->groupNumber];

	printf("\nCustomer %d in Group %d is walking up to ConcessionClerk %d to buy %d popcorn and %d soda.\n", current->customerNumber, current->groupNumber, myConcessionClerk , numberOfPopcornForCC[myConcessionClerk], numberOfSodaForCC[myConcessionClerk]);

	//tell the ConcessionClerk regarding no. of soda
	concessionClerkCV[myConcessionClerk] -> Signal(concessionClerkLock[myConcessionClerk]);

	//wait for ConcessionClerk to give acknowledgement for soda order
	concessionClerkCV[myConcessionClerk] -> Wait(concessionClerkLock[myConcessionClerk]);

	printf("\nCustomer %d in Group %d in ConcessionClerk line %d is paying %d for food.\n", current->customerNumber, current->groupNumber, myConcessionClerk, 5 * numberOfPopcornForCC[myConcessionClerk] +  4 * numberOfSodaForCC[myConcessionClerk]);

	//pay the amount by signalling the CC
	concessionClerkCV[myConcessionClerk] -> Signal(concessionClerkLock[myConcessionClerk]);

	printf("\nCustomer %d in Group %d is leaving ConcessionClerk %d.\n", current->customerNumber, current->groupNumber, myConcessionClerk);
	concessionClerkLock[myConcessionClerk] -> Release();
	//------------------------------------------------end of interaction with ConcessionClerk--------------------------------------------------//
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
			ttStateLock -> Acquire();				//before approaching TicketTaker check if they are accepting the tickets
			mgrTTCustomerCommonLock -> Acquire();
				
			if(movieManagerState == NOT_STARTING || seatFullMessage == 1)	//if not accepting then wait for managers signal for next movie
			{
				ttStateLock -> Release();
				mgrTTCustomerCommonCV -> Wait(mgrTTCustomerCommonLock);
				mgrTTCustomerCommonLock -> Release();
			}
			else															// tickets are being accepted, so release the locks which others can use
			{
				ttStateLock -> Release();
				mgrTTCustomerCommonLock -> Release();
			}

			//HeadCustomer gets in line here
			ticketTakerLineLock -> Acquire();

			//Check if any TicketTaker is not busy
			for(int i = 0; i < numberOfTT; i++)
			{
				if(ticketTakerState[i] == TTNOT_BUSY)
				{
					//found a free TT, so make it your TicketTaker
					myTicketTaker = i;
					ticketTakerState[i] = TTBUSY;
					break;
				}
			}

			//If all are on break - condition has to be used

			//If no free TT available then check for shortest line of TT
			if(myTicketTaker == -1)
			{
				//nHeadCustomerdid not find free TT
				int shortestTTLine = 0; //by default the shortest line is the 1st one
				int shortestTTLineLength = ticketTakerLineCount[0];
				for(int i = 1; i < numberOfTT; i++)
				{
					if ((shortestTTLineLength > ticketTakerLineCount[i]) && (ticketTakerState[i] == TTBUSY))
					{
						//nHeadCustomer found shortest TicketTakerLine
						shortestTTLineLength = ticketTakerLineCount[i];
						shortestTTLine = i;
					}
				}
				// I have the shortest line
				myTicketTaker = shortestTTLine;

				//get in that line
				printf("\nCustomer %d in Group %d is getting in TicketTaker line %d.\n",current->customerNumber, current->groupNumber,myTicketTaker);
				
				if(lineTicketMessage[myTicketTaker] == 1)	//before getting in that line check if tickets are being accepted in that line
				{
					printf("\nCustomer %d in Group %d sees TicketTaker %d is no longer taking tickets. Going to the lobby.\n",current->customerNumber, current->groupNumber,myTicketTaker);
					
					printf("\nCustomer %d in Group %d is in the lobby.\n",current->customerNumber, current->groupNumber);
					ticketTakerLineLock -> Release();
					myTicketTaker = -1;
					mgrTTCustomerCommonLock -> Acquire();
					mgrTTCustomerCommonCV -> Wait(mgrTTCustomerCommonLock);	//if not accepting then wait for managers signal for next movie
					mgrTTCustomerCommonLock -> Release();

					printf("\nCustomer %d in Group %d is leaving the lobby.\n",current->customerNumber, current->groupNumber);
					continue;	//got signal from manager now start again
				}
				else
				{
					//get into the line
					ticketTakerLineCount[myTicketTaker]++;

					//wait for your turn
					ticketTakerLineCV[myTicketTaker] -> Wait(ticketTakerLineLock);
				}

				if(lineTicketMessage[myTicketTaker] == 1)	//before starting interaction with TicketTaker, again check if tickets are being accepted
				{
					//if not then go to lobby and wait for managers signal for next movie
					printf("\nCustomer %d in Group %d sees TicketTaker %d is no longer taking tickets. Going to the lobby.\n",current->customerNumber, current->groupNumber,myTicketTaker);
					printf("\nCustomer %d in Group %d is in the lobby.\n",current->customerNumber, current->groupNumber);

					ticketTakerLineLock -> Release();	//release line lock before going to lobby
					myTicketTaker = -1;

					mgrTTCustomerCommonLock -> Acquire();
					mgrTTCustomerCommonCV -> Wait(mgrTTCustomerCommonLock);
					mgrTTCustomerCommonLock -> Release();	//received signal here, so leave lobby and start over again

					printf("\nCustomer %d in Group %d is leaving the lobby.\n",current->customerNumber, current->groupNumber);
					continue;
				}
			}

			// At this point tickets are being accepted

			//release lock as I have come into the shortest line
			ticketTakerLineLock ->Release();
			printf("\nCustomer %d in Group %d is walking up to TicketTaker %d to give %d tickets.\n",current->customerNumber, current->groupNumber,myTicketTaker, myGroupSize);

			//acquire then new lock to start interaction with Concession clerk
			ticketTakerLock[myTicketTaker] -> Acquire();





			//pass no. of tickets and wake up Ticket Taker
			numberOfTicketsForTT[myTicketTaker] = myGroupSize;

			//signal the no. of tickets
			ticketTakerCV[myTicketTaker] -> Signal(ticketTakerLock[myTicketTaker]);

			//wait for acknowledgement from TicketTaker to see if Ticket Accepted or not
			ticketTakerCV[myTicketTaker] -> Wait(ticketTakerLock[myTicketTaker]);

			if(acceptMessage[myTicketTaker] == 1)	//if tickets are not accepted then go to lobby and wait for managers signal for next movie
			{
				
				printf("\nCustomer %d in Group %d sees TicketTaker %d is no longer taking tickets. Going to the lobby.\n",current->customerNumber, current->groupNumber,myTicketTaker);
				printf("\nCustomer %d in Group %d is in the lobby.\n",current->customerNumber, current->groupNumber);

				ticketTakerLock[myTicketTaker] -> Release();
				myTicketTaker = -1;

				//wait here for managers signal
				mgrTTCustomerCommonLock -> Acquire();
				mgrTTCustomerCommonCV -> Wait(mgrTTCustomerCommonLock);

				//HeadCustomer receives signal Manager that the next movie is starting
				mgrTTCustomerCommonLock -> Release();

				//got the signal, now start over again
				printf("\nCustomer %d in Group %d is leaving the lobby.\n",current->customerNumber, current->groupNumber);
				continue;
			}

			//Tickets are accepted here so release the lock and proceed to Movie hall

			printf("\nCustomer %d in Group %d is leaving TicketTaker %d.\n",current->customerNumber, current->groupNumber,myTicketTaker);
			ticketTakerLock[myTicketTaker] -> Release();
		}
}
//------------------------------------------------end of interaction with TicketTaker--------------------------------------------------//

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
				myOccupiedSeats[grpNo][memNumber] = i * 5 + j + 1;	//first allocated seat number will be (0*5) + 0 + 1 = 1
																	//second allocated seat number will be (0*5) + 1 + 1 = 2
				//seat numbers are stored here						//.....
																	//last accocated seat number will be (4*5) + 4 + 1 = 25
				printf("\nCustomer %d in Group %d has found the following seat: row %d seat %d\n",memNumber, grpNo, i+1, j+1 );
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

	while(groupMemberCountForSeats[myGroupNumber] > 1)	// wait for all group members to occupy the seats
	{
		groupSeatWaitCV[myGroupNumber] -> Wait(groupSeatWaitLock[myGroupNumber]);
	}
	//release seat wait lock as all group members have occupied seats

	groupSeatWaitLock[myGroupNumber] -> Release();
	//HeadCustomer releases seat wait lock
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

	//acquire movieseatlock shared with other Head customers and Movie Technician, to share/update total theater seat count
	movieSeatLock -> Acquire();

	//acquire movieseatmanagerlock shared with other Head customers and Manager
	movieSeatManagerLock -> Acquire();	// To access and update total seat count
	//=======================================================================
	//find the row with maximum seats available for the group to be seated together
	for(int i=0; i<5; i++)
	{
		if(numberOfFreeSeats[i] >= myData -> groupSize)
		{
			myRow = i;	//found row which can accommodate all group members
			allocateSeats(i,i,myData -> groupSize,myGroupNumber);	//Allocate seats in that row
			break;
		}
	}
	if(myRow == -1)
	{
		//now searching for 2 consecutive rows which can accommodate all group members
		for(int i=0; i<4; i++)
		{
			//this condition is to check if free seats in 2 consecutive rows can allocate group members
			if((numberOfFreeSeats[i] + numberOfFreeSeats[i+1]) >= myData->groupSize)
			{
				//2 consecutive rows found that can accommodate all group members
				myRow = 0;	// myRow is set to zero i.e. rows found
				allocateSeats(i , i+1,myData->groupSize,myData->groupNumber);
				break;
			}
		}
		if(myRow == -1)
		{
			//now searching for 3 consecutive rows which can accommodate all group members
			for(int i=0; i<3; i++)
			{
				//this condition is to check if free seats in 3 consecutive rows can allocate group members
				//also we need to check the middle row has some free seats to ensure that GM is allocated seats in consecutive rows w/o skipping any row in between
				if((numberOfFreeSeats[i] + numberOfFreeSeats[i+1]+ numberOfFreeSeats[i+2]) >= myData->groupSize && numberOfFreeSeats[i+1]!=0)
				{
					//3 consecutive rows found that can accommodate all group members
					myRow = 0;
					allocateSeats(i , i+2,myData->groupSize,myData->groupSize);
					break;
				}
			}
		}
		if(myRow == -1)
		{
			//now searching for 4 consecutive rows which can accommodate all group members
			for(int i=0; i<2; i++)
			{
				//this condition is to check if free seats in 4 consecutive rows can allocate group members
				//also we need to check the middle 2 rows have some free seats to ensure that GM is allocated seats in consecutive rows w/o skipping any row in between
				if((numberOfFreeSeats[i] + numberOfFreeSeats[i+1] + numberOfFreeSeats[i+2]+ numberOfFreeSeats[i+3]) >= myData->groupSize && numberOfFreeSeats[i+1]!=0 && numberOfFreeSeats[i+2]!=0)
				{
					//4 consecutive rows found that can accommodate all group members
					myRow = 0;
					allocateSeats(i , i+3,myData->groupSize,myData->groupSize);
					break;
				}
			}
		}
		if(myRow == -1)
		{
			//as no consecutive rows have seats available for group members, allocate where space is available
			allocateSeats(0,4,myData->groupSize,myData->groupSize);
		}
	}

	myData -> mySeat = myOccupiedSeats[myGroupNumber][myData -> customerNumber];	//seat number of head customer
	printf("\nCustomer %d in group %d is sitting in a theater room seat\n", myData->customerNumber, myGroupNumber);

	if(myData -> groupSize > 1)		//if there are any members in your group then allocate seats to them also
	interactWithGroupMembersForSeatAllocation(myData);

	movieSeatManagerLock -> Release();
	movieSeatLock -> Release();

	//release the lock taken with manager
	
}


//===============================================================
//	FUNCTION: All customers may go to bathroom and then Exit here
//===============================================================
void customersExit(CustomerData *myData)
{
	int myGroupNumber = myData -> groupNumber;
	if((rand()%100) < 25)			
	{		//25 percent chance of a person to go to bathroom
		printf("\nCustomer %d in Group %d is going to the bathroom.\n",  myData->customerNumber, myData->groupNumber);
		for(int i =0;i< 200; i++)
			currentThread -> Yield();
		printf("\nCustomer %d in Group %d is leaving the bathroom.\n",  myData->customerNumber, myData->groupNumber);	
	}

	groupTheaterExitLock[myGroupNumber] -> Acquire();
	groupTheaterExitCount[myGroupNumber]++;	

	if(groupTheaterExitCount[myGroupNumber] == myData -> groupSize)	// All group members will wait, the last one will come and broadcast
	{
		groupTheaterExitCV[myGroupNumber] -> Broadcast(groupTheaterExitLock[myGroupNumber]);
	}
	else
	{
		groupTheaterExitCV[myGroupNumber] -> Wait(groupTheaterExitLock[myGroupNumber]);
	}
	groupTheaterExitLock[myGroupNumber] -> Release();
	customerManagerExitLock -> Acquire();
	allCustomerExitedCount++;		//increment the total exit count with the group size
	customerManagerExitLock -> Release();

	printf("\nCustomer %d in group %d has left the movie theater.\n", myData->customerNumber, myGroupNumber);
}

//================================================================
//	FUNCTION: Customers entered Theater, now watch movie and then
//				exit theater when movie is over
//================================================================
void watchMovieAndExit(CustomerData *myData)
{
	int groupNum = myData -> groupNumber;

	//nHeadCustomer acquires groupExitLock to interaction with group members
	groupExitLock[groupNum] -> Acquire();


	//HeadCustomer acquires movieTechLock to check if movie is over
	movieTechLock -> Acquire();

	if(movieFinishState == NOT_OVER || movieFinishState == FIRST_MOVIE || movieFinishState == OVER)
	{
			//	waits as movie is not over
		movieTechCV -> Wait(movieTechLock);
			//	got signal of movie getting
	}

	movieTechLock -> Release();
	//releases movieTechLock

	myData -> mySeat = 0;	//before exiting make seat = 0

	printf("\nCustomer %d in group %d is getting out of a theater room seat\n", myData->customerNumber, groupNum);

	//nHeadCustomer will now check if all group members have gathered
	while(groupExitCount[groupNum] > 1)
	{
		groupExitCV[groupNum] -> Wait(groupExitLock[groupNum]);	// wait for all members
	}
	//all members arrived, so now its OK to exit
	groupExitLock[groupNum] -> Release();
	//releases groupExitLock as all Members gathered to exit

	//HeadCustomer acquire groupRegroupLock to convey exit
	groupRegroupLock[groupNum] -> Acquire();

	
	groupRegroupState[groupNum] = REGROUPED;

	if(groupRegroupFlag[groupNum] == 1)
	{
		printf("\nHeadCustomer %d of Group %d has told the group to proceed.\n",myData->customerNumber, groupNum);
		groupRegroupCV[groupNum] -> Broadcast(groupRegroupLock[groupNum]);		//Broadcast everybody to exit movie hell
	}

	groupRegroupLock[groupNum] -> Release();
	
	// Acquire mgr lock and increment the exit count

	movieSeatManagerLock -> Acquire();

	totalMovieRoomExitCount += myData -> groupSize;		//increment the movie hall exit count

	movieSeatManagerLock -> Release();
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	//Customers may proceed to exit or go to bathroom, if some one in a group goes to bathroom, others wait
	customersExit(myData);
}

//===========================================================
//	FUNCTION: Interaction of HeadCustomer Group Members to take
//				Food order from each member
//===========================================================
void interactWithGroupMembersForFoodOrder(CustomerData *myData)
{
	int myGroupNumber = myData -> groupNumber;

	while(groupMemberCountForFood[myGroupNumber] > 1)	//Interact with all members
	{
		//waiting for order from every group member
		groupFoodLockCV[myGroupNumber] -> Wait(groupFoodLock[myGroupNumber]);
		//group member placed order so decrement count and wait for next group member to tell the food order
	}

	//order soda and popcorn [handle condition if I want to order or not]
	int temp_popcorn = 0, temp_soda = 0;
	int random_popcorn_order = rand() % 100;	//random value for popcorn between 0-99
	if(random_popcorn_order < 75)				// 75 percent chance of order
	{
		temp_popcorn = 1;
	}
	else
	{
		temp_popcorn = 0;
	}

	int random_soda_order = rand() % 100;		//random value for soda between 0-99
	if(random_soda_order < 75)					// 75 percent chance of order
	{
		temp_soda = 1;
	}
	else
	{
		temp_soda = 0;
	}
	printf("\nCustomer %d in Group %d wants %d popcorn and %d soda.\n", myData->customerNumber,myGroupNumber, temp_popcorn, temp_soda);
	numberOfPopcorn[myGroupNumber] +=temp_popcorn;
	numberOfSoda[myGroupNumber] +=temp_soda;
	printf("\nCustomer %d in Group %d has %d popcorn and %d soda request from a group member.\n", myData->customerNumber,myGroupNumber, temp_popcorn, temp_soda);

	//release food lock as the order taken from group members
	groupFoodLock[myGroupNumber] -> Release();

	interactWithConcessionClerksToOrderFood(myData);

	//Acquire lock to inform Group members about Food Order is complete
	groupFoodWaitLock[myGroupNumber] -> Acquire();

	//Order Received, hence change the status for Group members reference
	hasOrdered[myGroupNumber] = ORDER_RECEIVED;

	if(groupFoodWaitFlag[myGroupNumber] == 1)
	{
	//	HeadCustomer informs all its waiting group member about Food order completion
		groupFoodWaitCV[myGroupNumber] -> Broadcast(groupFoodWaitLock[myGroupNumber]);
	}
	//HeadCustomer releases food Wait lock with members of Group
	groupFoodWaitLock[myGroupNumber] -> Release();

	interactWithTicketTakerToGiveTickets(myData);	// got the order from members now interact with COncessionClerk

	//Acquire lock to inform Group members about Ticket Submitted is complete
	groupTicketSubmitLock[myGroupNumber] -> Acquire();

	//Tickets Submitted, hence change the status for Group members reference
	hasSubmitted[myGroupNumber] = TICKETS_ACCEPTED;

	if(groupTicketSubmitFlag[myGroupNumber] == 1)
	{
		//nHeadCustomer informs all its waiting group member about Ticket Submitted to TicketTaker
		groupTicketSubmitCV[myGroupNumber] -> Broadcast(groupTicketSubmitLock[myGroupNumber]);
	}

	//nHeadCustomer acquires groupseatwaitlock to interact about seats with members of Group
	groupSeatWaitLock[myGroupNumber] -> Acquire();

	groupTicketSubmitLock[myGroupNumber] -> Release();
	//HeadCustomer[%d] releases ticket submit lock with members of Group
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

	


	//check if group head has bought tickets
	if (hasTickets[myGroupNumber] != BOOKED)
	{
		printf("\nCustomer %d of group %d is waiting for the HeadCustomer.\n", myData->customerNumber,myGroupNumber);
		//if tickets are not booked wait for the head customer
		groupTicketLockCV[myGroupNumber] -> Wait(groupTicketLock[myGroupNumber]);
	}
	printf("\nCustomer %d of group %d has been told by the HeadCustomer to proceed.\n", myData->customerNumber,myGroupNumber);
	//tickets are booked as release the lock, if food needed then go the food counter after this
	groupTicketLock[myGroupNumber] -> Release();

	//Acquire Intra group Lock to have mutual exclusion in food order
	intraGroupFoodLock[myGroupNumber] -> Acquire();

	//Acquire this lock to place order to Head Customer
	groupFoodLock[myGroupNumber] -> Acquire();
	
	printf("\nCustomer %d in Group %d is in the lobby.\n",myData->customerNumber, myData->groupNumber);
	int temp_popcorn = 0, temp_soda = 0;
	int random_popcorn_order = rand() % 100;		//random value between 0-99
	if(random_popcorn_order < 75)					//75 percent chance of an order
	{
		temp_popcorn = 1;
	}
		else
	{
		temp_popcorn = 0;
	}
	int random_soda_order = rand() % 100;			//random value between 0-99
	if(random_soda_order < 75)						//75 percent chance of an order
	{
		temp_soda = 1;
	}
	else
	{
		temp_soda = 0;
	}
	printf("\nCustomer %d in Group %d wants %d popcorn and %d soda.\n", myData->customerNumber,myGroupNumber, temp_popcorn, temp_soda);
	numberOfPopcorn[myGroupNumber] +=temp_popcorn;
	numberOfSoda[myGroupNumber] +=temp_soda;
	printf("\nCustomer %d in Group %d has %d popcorn and %d soda request from a group member.\n", myData->customerNumber,myGroupNumber, temp_popcorn, temp_soda);
	//Order is placed so decrement the count and signal head customer

	groupMemberCountForFood[myGroupNumber]--;
	groupFoodLockCV[myGroupNumber] -> Signal(groupFoodLock[myGroupNumber]);  //signal head customer every time

	groupFoodLock[myGroupNumber] -> Release();

	//release intragroup food lock so that next waiting customer can order to head customer
	intraGroupFoodLock[myGroupNumber] -> Release();

	//Acquire lock to check if Head Customer has received the Order
	groupFoodWaitLock[myGroupNumber] -> Acquire();
	

	//
	if(hasOrdered[myGroupNumber]== ORDER_NOTRECEIVED)	// wait if the head customer is interacting with concession clerk
	{
		groupFoodWaitFlag[myGroupNumber] = 1; 	//this is for Head customer's Reference that atleast one group member is waiting for Order Confirmation
		groupFoodWaitCV[myGroupNumber] -> Wait(groupFoodWaitLock[myGroupNumber]);
	}

	printf("\nCustomer %d in Group %d is leaving the lobby.\n",myData->customerNumber, myData->groupNumber);
	groupFoodWaitLock[myGroupNumber] -> Release();

	//Acquire lock to check if Head Customer has given tickets to Ticket Taker
	groupTicketSubmitLock[myGroupNumber] -> Acquire();

	



	if(hasSubmitted[myGroupNumber]== TICKETS_NOTACCEPTED) // wait if the head customer is interacting with TicketTaker to submit tickets
	{
		groupTicketSubmitFlag[myGroupNumber] = 1; //this is for Head customer's Reference that atleast one group member is waiting for Order Confirmation
		groupTicketSubmitCV[myGroupNumber] -> Wait(groupTicketSubmitLock[myGroupNumber]);
	}
	groupTicketSubmitLock[myGroupNumber] -> Release();


	//Acquire this lock to receive the seat number allocated by Head Customer
	groupSeatWaitLock[myGroupNumber] -> Acquire();

	//The seat allocated is received so decrement the counter of group members needed to communicate with HC
	groupMemberCountForSeats[myGroupNumber]--;

	myData -> mySeat = myOccupiedSeats[myGroupNumber][myData -> customerNumber];	// retrieve your seat number here

	printf("\nCustomer %d in group %d is sitting in a theater room seat\n", myData->customerNumber, myGroupNumber);

	groupSeatWaitCV[myGroupNumber] -> Signal(groupSeatWaitLock[myGroupNumber]);

	groupSeatWaitLock[myGroupNumber] -> Release();

	//take movie start-stop lock and wait on that
	//-----------------------------------------At this point every body is seated in movie hall--------------------------------------------//

	movieTechLock -> Acquire();	//Acquire movieTech lock to see if movie is over

	if(movieFinishState == NOT_OVER || movieFinishState == FIRST_MOVIE || movieFinishState == OVER)
	{
		movieTechCV -> Wait(movieTechLock);		// wait for movie to over
	}

	movieTechLock -> Release();	// release movietech lock

	myData -> mySeat = 0;

	//Group Member acquires movieexitLock to interact with HeadCustomer for Exit
	groupExitLock[myGroupNumber]->Acquire();

	groupExitCount[myGroupNumber]--;

	groupExitCV[myGroupNumber] -> Signal(groupExitLock[myGroupNumber]);
	printf("\nCustomer %d in group %d is getting out of a theater room seat\n", myData->customerNumber, myGroupNumber);

	groupExitLock[myGroupNumber]-> Release();
	//GroupMember releases movieexitLock after interacting with HeadCustomer

	//Acquire regroup lock and wait for members of group went to bathroom
	groupRegroupLock[myGroupNumber]-> Acquire();

	if(groupRegroupState[myGroupNumber] == NOT_REGROUPED)
	{
		printf("\nCustomer %d of group %d is waiting for the HeadCustomer.\n", myData->customerNumber,myGroupNumber);
		groupRegroupFlag[myGroupNumber] = 1;			//Wait for all the group memner before you can exit the movie hall
		groupRegroupCV[myGroupNumber] -> Wait(groupRegroupLock[myGroupNumber]);
	}

	groupRegroupLock[myGroupNumber] -> Release();		//release grgroup lock before proceeding
	printf("\nCustomer %d of group %d has been told by the HeadCustomer to proceed.\n", myData->customerNumber,myGroupNumber);

	
	customersExit(myData);
}

//===========================================================
//	FUNCTION: Head Customer starts here
//===========================================================
void headCustomer(int currentHC)
{
	CustomerData * current = (CustomerData *) currentHC;	//Head Customer is Active

	interactionForTicketsWithTC(current);	//Head customer goes to Ticket Clerk to buy tickets

	interactWithGroupMembersForFoodOrder(current);	// Head Customer goes to buy food from concession clerk
	
													//Head customer then informs all the members that he got the food
	
													//Head customer goes to TicketTaker after this to submit tickets
	
	movieSeatAllocation(current);			//Head allocate seats to all his group members

	watchMovieAndExit(current);			// Head customer exits movie hall and may go to bathroom before going out of theater
}

//===========================================================
//	FUNCTION: Concession starts here
//===========================================================
void concessionClerk(int myIndex)
{
	int loopCount = 0;

	while(true)
	{
		concessionClerkManagerBreakLock -> Acquire();		//Acquire to see if manager has sent clerk on break
		concessionClerkLineLock -> Acquire();			//Acquire LineLock to see if anybody is in line

		if(CCBreakState[myIndex] == ON_BREAK)	//check if manager has told you to go on break
		{
			concessionClerkLineLock -> Release();
			printf("\nConcessionClerk %d is going on break.\n",myIndex);
			concessionClerkManagerBreakCV[myIndex] -> Wait(concessionClerkManagerBreakLock);	//wait for manager to call you Off break
			concessionClerkLineLock -> Acquire();
		}
		else
		{
			managerCustomerCCFlag[myIndex] = 0;
		}

		concessionClerkState[myIndex] = CCNOT_BUSY;		//set state as available
		concessionClerkManagerBreakLock -> Release();	//release the break lock acquired with manager

		
		if (concessionClerkLineCount[myIndex] > 0)// Yes I have customers
		{
			//make yourself busy
			concessionClerkState[myIndex] = CCBUSY;
			printf("\nConcessionClerk %d has a line length %d and is signaling a customer.\n",myIndex, concessionClerkLineCount[myIndex]);

			concessionClerkLineCount[myIndex]--;// as I am going to service this customer, decrement line count by 1

			//wake up the customer to be serviced
			concessionClerkLineCV[myIndex] -> Signal(concessionClerkLineLock);
		}
		else
		{
			printf("\nConcessionClerk %d has no one in line. I am available for a customer.\n",myIndex);
			//no one in line so i am not busy, so make myself not busy
			concessionClerkState[myIndex] = CCNOT_BUSY;
		}
		
		//acquire a lock to start interaction with head customer TT game

		concessionClerkLock[myIndex] -> Acquire();
		// this lock is aacquired before releasing concessionClerkLineLock to aviod contex switching problems

		//as a customer to interact is found then we dont need this lock currently
		concessionClerkLineLock -> Release();

		//wait for customer to come to my counter and order number of popcorns
		concessionClerkCV[myIndex] -> Wait(concessionClerkLock[myIndex]);

		//calculate total amount to be paid by the customer, store it in monitor variable and share it with customer
		amt2CC[myIndex] = 5 * numberOfPopcornForCC[myIndex];

		//tell the customer to give the soda order
		concessionClerkCV[myIndex] -> Signal(concessionClerkLock[myIndex]);

		//wait for customer to come to my counter and order number of sodas
		concessionClerkCV[myIndex] -> Wait(concessionClerkLock[myIndex]);

		//calculate total amount to be paid by the customer, store it in monitor variable and share it with customer
		amt2CC[myIndex] += 4 * numberOfSodaForCC[myIndex];

		printf("\nConcessionClerk %d has an order for %d popcorn and %d soda. The cost is %d.\n",myIndex, numberOfPopcornForCC[myIndex], numberOfSodaForCC[myIndex], amt2CC[myIndex]);
		//give customer the bill to be paid and the food order(soda and popcorn)
		concessionClerkCV[myIndex] -> Signal(concessionClerkLock[myIndex]);

		//wait for the customer to make payment
		concessionClerkCV[myIndex] -> Wait(concessionClerkLock[myIndex]);

		concessionClerkManagerLock -> Acquire();
		// Customer made the payment, add it to the total food collection in mutually exclusive manner
		totalAmtByCC[myIndex] += amt2CC[myIndex];

		concessionClerkManagerLock -> Release();
		//added my amount to total food collection

		printf("\nConcessionClerk %d has been paid for the order.\n",myIndex);

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
	ticketTakerState[myIndex] = TTNOT_BUSY;		//make state as not busy so customers come and interact 
	ticketTakerLineLock -> Release();

	while(true)
	{
			mgrTTCustomerCommonLock -> Acquire();	//acquire common lock to see if manager has ask to start collecting tickets
			ttStateLock -> Acquire();		//acquire stateLock lock to see if seats are not full
			
			if(movieManagerState == NOT_STARTING || seatFullMessage == 1)	//if seats are alrady full or manager has not yet told to collect tickets
			{																//then wait for managers command
				ttStateLock -> Release();
				
				ticketTakerLineLock -> Acquire();
				ticketTakerLineCount[myIndex] = 0;			//initially clerking the line count
				lineTicketMessage[myIndex] = 1;				//make lineTicket message as not accepting
				ticketTakerLineCV[myIndex] -> Broadcast(ticketTakerLineLock);	//broadcast to everyone in line
				ticketTakerLineLock -> Release();
				
				mgrTTCustomerCommonCV -> Wait(mgrTTCustomerCommonLock);	//wait here fpr managers command
				ticketTakerLineLock -> Acquire();
					lineTicketMessage[myIndex] = 0;		//after getting command to start collecting tickets, set line Message as tickets are being accepted
				ticketTakerLineLock -> Release();
			}
			else
			{
				ttStateLock -> Release();	//release the lock as it is not required
			}
			mgrTTCustomerCommonLock -> Release();	//release interaction lock with manager

			ticketTakerLineLock -> Acquire();

			if (ticketTakerLineCount[myIndex] > 0)// Yes I have customers
			{
				ttStateLock -> Acquire();	//acquire lock to see if seats are full or not
				if(seatFullMessage == 1)	// check if seats are available in movie hall
				{
					lineTicketMessage[myIndex] = 1;	//if seats are no more available, then set line message as not accepting
					
					//recent change
					ticketTakerLineCount[myIndex] = 0;	//clear line count 
					
					ticketTakerLineCV[myIndex] -> Broadcast(ticketTakerLineLock);	//inform all customers in line to go back to lobby
					
					//recent change
					//ticketTakerState[myIndex] = TTNOT_BUSY;
					
					ticketTakerLineLock -> Release();	//release lock after infroming all customers
					
					ttStateLock -> Release();	//releasse lock acuired to see if seats are full or not
					
					continue;
				}
				else
				{	
					// tickets are being accepted at this point, so start interaction with the first customer in line
					//make myself busy
					ticketTakerState[myIndex] = TTBUSY;
					printf("\nTicketTaker %d has a line length %d and is signaling a customer.\n",myIndex, ticketTakerLineCount[myIndex]);

					ticketTakerLineCount[myIndex]--;// as I am going to service this customer, decrement line count by 1

					//wake up the customer to be serviced
					lineTicketMessage[myIndex] = 0;
					ticketTakerLineCV[myIndex] -> Signal(ticketTakerLineLock);		//call the first customer in your line
					ttStateLock -> Release();
				}
				
			}
			else
			{
				printf("\nTicketTaker %d has no one in line. I am available for a customer.\n",myIndex);
				//no one in line so i am not busy, so make myself not busy
				ticketTakerState[myIndex] = TTNOT_BUSY;
			}

			//acquire a lock to start interaction with head customer TT game

			ticketTakerLock[myIndex] -> Acquire();
			// this lock is aacquired before releasing ticketTakerLineLock to aviod contex switching problems

			//as a customer to interact is found then we dont need this lock currently
			ticketTakerLineLock -> Release();

			//wait for customer to come to my counter and order number of popcorns
			ticketTakerCV[myIndex] -> Wait(ticketTakerLock[myIndex]);
			printf("\nTicketTaker %d has received %d tickets.\n",myIndex, numberOfTicketsForTT[myIndex]);

			ttStateLock -> Acquire();	//again check if seats are being accepted or not

			if(seatFullMessage == 0)
			{
				if(totalAcceptedSeats + numberOfTicketsForTT[myIndex] <= 25)	//if this group can be accommodated then accept tickets or else reject it
				{
					totalAcceptedSeats += numberOfTicketsForTT[myIndex];
					printf("\nTicketTaker %d is allowing the group into the theater. The number of tickets taken is %d.\n",myIndex,numberOfTicketsForTT[myIndex] <= 25 );

					acceptMessage[myIndex] = 0;	//message set to 0 to inform customer that his tickets have been accepted
					ticketTakerCV[myIndex] -> Signal(ticketTakerLock[myIndex]);	//signal customer that the tickets have been accepted
					ticketTakerLock[myIndex] -> Release();	// release interaction lock with customer
					
					if(totalAcceptedSeats == 25)
					{
						seatFullMessage = 1; //if the theater is full after accepting this group then inform customer to go to lobby and wait 
						
						ticketTakerLineLock -> Acquire();
						ticketTakerLineCount[myIndex] = 0;		//clear line count
						lineTicketMessage[myIndex] = 1;			//set line message as ticket not being accepted
						ticketTakerLineCV[myIndex] -> Broadcast(ticketTakerLineLock);
						ticketTakerLineLock -> Release();		
						printf("\nTicketTaker %d has stopped taking tickets.\n",myIndex);
						ttStateLock -> Release();	//release statelock 
						continue;
					}
				}
				else
				{
					seatFullMessage = 1;	//if the theater is full after accepting this group then inform customer to go to lobby and wait 
					
					acceptMessage[myIndex] = 1;
					ticketTakerCV[myIndex] -> Signal(ticketTakerLock[myIndex]);	//inform customer that his tickets has been rejected
					ticketTakerLock[myIndex] -> Release();	//release interaction lock with customer

					ticketTakerLineLock -> Acquire();	//acquire line lock to inform others in line
					ticketTakerLineCount[myIndex] = 0;	//clear line count
					lineTicketMessage[myIndex] = 1;		//set line message as ticket not being accepted
					ticketTakerLineCV[myIndex] -> Broadcast(ticketTakerLineLock);
					ticketTakerLineLock -> Release();
					printf("\nTicketTaker %d is not allowing the group into the theater. The number of tickets taken is %d and the group size is %d.\n",myIndex, totalAcceptedSeats, numberOfTicketsForTT[myIndex]);
					printf("\nTicketTaker %d has stopped taking tickets.\n",myIndex);
					ttStateLock -> Release();		//release line lock
					
					continue;
				}
			}
			else
			{
				acceptMessage[myIndex] = 1;	//if the theater is full after accepting this group then inform customer to go to lobby and wait 
				
				ticketTakerCV[myIndex] -> Signal(ticketTakerLock[myIndex]);	//inform customer that his tickets has been rejected
				ticketTakerLock[myIndex] -> Release();	//release interaction lock with customer

				ticketTakerLineLock -> Acquire();		//acquire line lock to inform others in line
				lineTicketMessage[myIndex] = 1;			//set line message as ticket not being accepted
				ticketTakerLineCount[myIndex] = 0;		//clear line count
				ticketTakerLineCV[myIndex] -> Broadcast(ticketTakerLineLock);
				ticketTakerLineLock -> Release();
				printf("\nTicketTaker %d is not allowing the group into the theater. The number of tickets taken is %d and the group size is %d.\n",myIndex,totalAcceptedSeats, numberOfTicketsForTT[myIndex]);
				printf("\nTicketTaker %d has stopped taking tickets.\n",myIndex);
				ttStateLock -> Release();		//release line lock
				
				continue;
			}
			ttStateLock -> Release();
		}
}

//===========================================================
//	FUNCTION: Ticket Clerk Starts here
//===========================================================
void ticketClerk(int myIndex)//myIndex is used to indentify the TC to interact with
{
	int loopCount = 0;
	
	while(true)
	{
		ticketClerkManagerBreakLock -> Acquire();	//acquire manager tiecketclerk break lock
		ticketClerkLineLock -> Acquire();	// acquire line lock to see if managers has told to go on break

		if(TCBreakState[myIndex] == ON_BREAK)	//check if manager has asked you to go on break
		{
			ticketClerkLineLock -> Release();	//release line lock before going on break
			printf("\nTicketClerk %d is going on break.\n",myIndex);
			ticketClerkManagerBreakCV[myIndex] -> Wait(ticketClerkManagerBreakLock);	//wait for manager to call you off break
			ticketClerkLineLock -> Acquire();
		}
		else
		{
			managerCustomerTCFlag[myIndex] = 0;		
		}
		ticketClerkState[myIndex] = TCNOT_BUSY;	//	make state as available
		ticketClerkManagerBreakLock -> Release();

		if (ticketClerkLineCount[myIndex] > 0)// Yes I have customers
		{
			//make myself busy
			ticketClerkState[myIndex] = TCBUSY;	// make yourself busy beofre begining interaction
			printf("\nTicketClerk %d has a line length %d and is signaling a customer.\n",myIndex, ticketClerkLineCount[myIndex]);
			ticketClerkLineCount[myIndex]--;// as I am going to service this customer, decrement line count by 1

			//wake up the customer to be serviced
			ticketClerkLineCV[myIndex] -> Signal(ticketClerkLineLock);
		}
		else
		{
			printf("\nTicketClerk %d has no one in line. I am available for a customer.\n",myIndex);
			//no one in line so i am not busy, so make myself not busy
			ticketClerkState[myIndex] = TCNOT_BUSY;
		}

		//acquire a lock to start interaction with customer TT game

		ticketClerkLock[myIndex] -> Acquire();
		// this lock is aacquired before releasing ticketClerkLineLock to aviod contex switching problems

		//as a customer to interact is found then we dont need this lock currently
		ticketClerkLineLock -> Release();

		//wait for customer to come to my counter
		ticketClerkCV[myIndex] -> Wait(ticketClerkLock[myIndex]);

		//calculate total amount to be paid by the customer, store it in monitor variable and share it with customer
		amt2TC[myIndex] = 12 * numberOfTicketsForTC[myIndex];

		//tell the customer the amount to be paid
		printf("\nTicketClerk %d has an order for %d tickets and the cost is %d.\n",myIndex, numberOfTicketsForTC[myIndex], 12 * numberOfTicketsForTC[myIndex]);
		ticketClerkCV[myIndex] -> Signal(ticketClerkLock[myIndex]);

		//wait for the customer to make payment
		ticketClerkCV[myIndex] -> Wait(ticketClerkLock[myIndex]);

		ticketClerkManagerLock -> Acquire();
		//Customer made the payment, add it to the total ticket collection
		totalAmtByTC[myIndex] += amt2TC[myIndex];

		ticketClerkManagerLock -> Release();

		printf("\nTC[%d] gave tickets to Customer\n",myIndex);
		//provide customer the tickets
		ticketClerkCV[myIndex] -> Signal(ticketClerkLock[myIndex]);

		
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
void movieTechnician(int noUse)
{
	while(true)
	{
		movieTechManagerLock -> Acquire();	//movie tech acquires manager lock to see if he is asked to start a movie

		if(commandToStart == 0)//commandtostart must be 1 to start movie
		{
			printf("\nnMovieTechnician waits for signal from Manager to start movie\n");
			movieTechManagerCV -> Wait(movieTechManagerLock);		//wait here for manager
		}
		printf("\nMovieTechnician has started the movie.\n");

		int simulationTime = (rand() % (300 - 200 + 1) + 200);

		randomYield(simulationTime);
		printf("\nMovieTechnician has ended the movie.\n");
		
		movieTechLock -> Acquire();		//acquire lock to inform all customers waiting on this lock to inform them that movie is over
		movieFinishState = OVER;

		commandToStart = 0;		//resetting command to start

		printf("\nMovieTechnician has told all the customers to leave the theater room.\n");
		movieTechCV -> Broadcast(movieTechLock);		//inform all customers
		movieTechLock -> Release();		//release lock after broadcasting
		movieTechManagerLock -> Release();	//release manager lock
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
		randomYield(simulationTime);	
		randomYield(simulationTime);
		randomYield(simulationTime);

		//===============================
		//on break logic for Ticket Clerk
		int TCCounter = 0; 
		int TCIndex[5] = {-1};
		
		ticketClerkManagerBreakLock -> Acquire();
		// lock for checking the line count of ticket clerk

		for(int i = 0;i < MAX_TC; i++)
		{
			TCCounter = 0;
			ticketClerkLineLock -> Acquire();
			random = rand() % 10;
			if(((TCBreakState[i] == OFF_BREAK) && (ticketClerkLineCount[i] < 3 && ticketClerkBreakCount < 4)) && (random < 2))
			{						//send a clerk on break if he has less than 3 customers in line and some other clerk of same type is still working
				//determine whether TC should go on break randomly
				//set ONBREAK = TRUE
				managerCustomerTCFlag[i] = 1;	//set flag to 1 so that customer will know their clerk is going on break and they will join other line
				ticketClerkBreakCount++;		//increment breakcount
				TCBreakState[i] = ON_BREAK;		//set state as on break
				ticketClerkLineCount[i] = 0;
				printf("\nManager has told TicketClerk %d to go on break.\n", i);
				printf("\nManager Signals all the customers on TC[%d] line to restart line entry\n",i);
				ticketClerkLineCV[i] -> Broadcast(ticketClerkLineLock);	//Broadcast all customers waiting in his line
				ticketClerkLineLock -> Release();	//release line lock
				continue;
			}
			else if(TCBreakState[i] == ON_BREAK)
			{
				for(int j=0; j< MAX_TC; j++)
				{
					if((j != i) && (ticketClerkLineCount[j] > 5) && (ticketClerkBreakCount > 0))
					{								//find out if any clerk has more than 5 in line and some other clerk of same type is on break
						TCIndex[TCCounter] = j;
						TCCounter++;
					}
				}
				
				if(TCCounter > 0)		//found a line with more than 5 customers in line so calling one lerk off break
				{
					ticketClerkBreakCount--;
					TCBreakState[i] = OFF_BREAK;
					ticketClerkState[i] = TCNOT_BUSY;	// make state as available
					for(int k=0 ; k < TCCounter; k++)
					{
						managerCustomerTCFlag[TCIndex[k]] = 2;
						ticketClerkLineCount[TCIndex[k]] = 0;
						ticketClerkLineCV[TCIndex[k]] -> Broadcast(ticketClerkLineLock);	//inform customers in that line to go and join other lines
					}
					printf("\nManager has told TicketClerk %d to come Off Break.\n",i);
					ticketClerkManagerBreakCV[i] -> Signal(ticketClerkManagerBreakLock);	//signal clerk to come off break
				}
			}
			ticketClerkLineLock -> Release();	
		}
		
		ticketClerkManagerBreakLock -> Release();
		//=================================
		
		randomYield(simulationTime);
		randomYield(simulationTime);
		randomYield(simulationTime);
		
		//=================================
		movieTechLock -> Acquire();
		ttStateLock -> Acquire();
		movieTechManagerLock -> Acquire();
		movieSeatManagerLock -> Acquire();
		ticketTakerLineLock -> Acquire();
		mgrTTCustomerCommonLock -> Acquire();
		
		if(movieFinishState == FIRST_MOVIE)
		{
			//mgrTTCustomerCommonLock -> Acquire();
			printf("\nManager is telling Movie Technician to start the movie.\n");
			seatFullMessage = 0;
			movieManagerState = STARTING;	//starting the process to start first movie
			
			for(int k=0; k< numberOfTT ;k++)
			{
				lineTicketMessage[k] = 0;
			}
			mgrTTCustomerCommonCV -> Broadcast(mgrTTCustomerCommonLock);	//inform everyone waiting in that line to rejoin other line as new clerk will come up
			
		}
		else if(totalMovieRoomExitCount == totalAcceptedSeats && movieFinishState == OVER && totalAcceptedSeats!=0)
		{										//if this is not the first movie then check if customers have left after first show
			movieSeatLock -> Acquire();			//if they have, then make all seats as free
			for(int i=0;i < 5; i++)
			{
				for(int j =0; j <5 ;j++)
				{
					seats[i][j] = SEATS_FREE;
				}
				numberOfFreeSeats[i] = 5;
			}
			movieSeatLock -> Release();		// release seat lock
			//movieSeatManagerLock -> Acquire();
			totalOccupiedSeats = 0;

			remainingCount -= totalAcceptedSeats;	//decrement remaining count i.e customers yet to see the movie

			totalAcceptedSeats = 0;			// make total occupied seats as 0
			
			seatFullMessage = 0;			//make seatFullmessage as 0 so that other entities start working
			
			totalMovieRoomExitCount = 0;
					
			printf("\nManager is telling Movie Technician to start the movie.\n");
			movieManagerState = STARTING;		// set movie state as starting
			
			for(int k=0; k< numberOfTT ;k++)
			{
				lineTicketMessage[k] = 0;		// make Line ticket message of all TicketTakers as 0
			}
			
			mgrTTCustomerCommonCV -> Broadcast(mgrTTCustomerCommonLock);	//broadcast everyone waiting on manager that next movie is about to start
		}
		mgrTTCustomerCommonLock -> Release();
		ticketTakerLineLock -> Release();	//release locks
		movieSeatManagerLock -> Release();	//	...
		movieTechManagerLock -> Release();	//	...
		ttStateLock -> Release();
		
		movieTechLock -> Release();	//release movie tech interaction lock
	
		//=====================================
		
		randomYield(simulationTime);
		randomYield(simulationTime);
		randomYield(simulationTime);
		randomYield(simulationTime);

		//=====================================
		
		
		ticketClerkManagerLock -> Acquire();
		//Yohoo , customer made the payment, add it to the total ticket collection
		
		for(int i=0;i<MAX_TC;i++)
		{
			totalTicketClerkAmount += totalAmtByTC[i]; 	//find total amount collected by all TicketClerks
			printf("\nManager collected %d form TicketClerk %d.\n",totalAmtByTC[i] , i);
		}
		
		ticketClerkManagerLock -> Release();
		
		randomYield(simulationTime);
		randomYield(simulationTime);
		randomYield(simulationTime);
		randomYield(simulationTime);
		
		concessionClerkManagerLock -> Acquire();
		for(int i=0;i<numberOfCC;i++)
		{
			totalConcessionClerkAmount += totalAmtByCC[i];		//find total amount collected by all Concessionclerks
			printf("\nManager collected %d form ConcessionClerk %d.\n",totalAmtByCC[i], i );
		}
		

		concessionClerkManagerLock -> Release();

		printf("\nTotal money made by the office = %d.\n", totalTicketClerkAmount + totalConcessionClerkAmount);
		
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
		
		//===============================
			
		//on break logic for Ticket Clerk
		int CCCounter = 0; 
		int CCIndex[5] = {-1};
		
		concessionClerkManagerBreakLock -> Acquire();
				// lock for checking the line count of concession clerk

		for(int i = 0;i < numberOfCC; i++)
		{
			concessionClerkLineLock -> Acquire();
			random = rand() % 10;
			if(((CCBreakState[i] == OFF_BREAK) && (concessionClerkLineCount[i] < 3 && concessionClerkBreakCount < 4)) && (random < 2))
			{						//send a clerk on break if he has less than 3 customers in line and some other clerk of same type is still working
				//determine whether CC should go on break randomly
				//set ONBREAK = TRUE
				managerCustomerCCFlag[i] = 1;	//set flag to 1 so that customer will know their clerk is going on break and they will join other line
				concessionClerkBreakCount++;	//increment breakcount
				CCBreakState[i] = ON_BREAK;		//set state as on break
				concessionClerkLineCount[i] = 0;
				concessionClerkLineCV[i] -> Broadcast(concessionClerkLineLock);	//Broadcast all customers waiting in his line
				concessionClerkLineLock -> Release();	//release line lock
				continue;
			}
			else if(CCBreakState[i] == ON_BREAK)	//find out if any clerk has more than 5 in line and some other clerk of same type is on break
			{
				for(int j=0; j< numberOfCC; j++)
				{
					if((j != i) && (concessionClerkLineCount[j] > 2) && (concessionClerkBreakCount > 0))
					{
						CCIndex[CCCounter] = j;
						CCCounter++;
					}
				}
				
				if(CCCounter > 0)	//found a line with more than 5 customers in line so calling one lerk off break
				{
					concessionClerkBreakCount--;
					CCBreakState[i] = OFF_BREAK;	// make state as available
					concessionClerkState[i] = CCNOT_BUSY;
					for(int k=0 ; k < CCCounter; k++)
					{
						managerCustomerCCFlag[CCIndex[k]] = 2;
						concessionClerkLineCount[CCIndex[k]] = 0;	// make state as available
						concessionClerkLineCV[CCIndex[k]] -> Broadcast(concessionClerkLineLock);	//inform customers in that line to go and join other lines
					}
					concessionClerkManagerBreakCV[i] -> Signal(concessionClerkManagerBreakLock);	//signal clerk to come off break
				}
			}
			concessionClerkLineLock -> Release();
		}
		
				// releasing the lock to check the line count of concession clerk
		concessionClerkManagerBreakLock -> Release();
		//=================================
		movieTechManagerLock -> Acquire();
		movieSeatManagerLock -> Acquire();
		ttStateLock -> Acquire();		//acquire lock too check total accepted seats
		movieTechLock -> Acquire();		//acquire lock to interact with movie tech
		mgrTTCustomerCommonLock -> Acquire();	//acquire common lock 
		


		if(commandToStart == 0 && totalOccupiedSeats == totalAcceptedSeats && (movieFinishState == OVER || movieFinishState == FIRST_MOVIE) && (seatFullMessage == 1 || remainingCount == totalAcceptedSeats) && totalAcceptedSeats!=0)
		{				//determine if a new movie can begin or not
			movieFinishState = NOT_OVER;	
			movieTechManagerCV -> Signal(movieTechManagerLock);		//signal the movie tech to start a movie
			movieManagerState = NOT_STARTING;
		}
		mgrTTCustomerCommonLock -> Release();	//release common lock
		movieTechLock -> Release();		//release movie tech lock after informing them to start movie
		ttStateLock -> Release();		//release state lock
		
		
		movieSeatManagerLock -> Release();
		movieTechManagerLock -> Release();
		
		//=============================================
		
		randomYield(simulationTime);
		randomYield(simulationTime);
		randomYield(simulationTime);
		randomYield(simulationTime);
		
		//=============================================

		customerManagerExitLock -> Acquire();	//acquire lock to see if incount == exitcount
		
		if(allCustomerExitedCount == totalCustomerInCount)	//check if total customers have watched the movie and exited
		{
			customerManagerExitLock -> Release();	
			break;
		}
		
		customerManagerExitLock -> Release();		//release lock
		randomYield(simulationTime);
		randomYield(simulationTime);
		randomYield(simulationTime);
	}
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++
void movieTheater() {

	//int numberOfGroups = rand()%10 + 1;

	//int numberOfGroups = 20;

	//Create all Locks, Condition Variables and initialize monitor variables
	ticketClerkLineLock = new Lock("ticketClerkLineLock");
	ticketClerkManagerBreakLock = new Lock("ticketClerkManagerBreakLock");
	ticketClerkManagerLock = new Lock("ticketClerkManagerLock");
	ticketClerkManagerCV = new Condition("ticketClerkManagerCV"); // lock for updating the individual amount

	concessionClerkManagerLock = new Lock("concessionClerkManagerLock");
	concessionClerkLineLock = new Lock("concessionClerkLineLock");
	concessionClerkManagerBreakLock = new Lock("concessionClerkManagerBreakLock");
	concessionClerkManagerLock = new Lock("concessionClerkManagerLock"); // lock for updating the individual amount
	
	movieTechManagerLock = new Lock("movieTechManagerLock");
	movieTechManagerCV = new Condition("movieTechManagerCV");

	movieSeatManagerLock = new Lock("movieSeatManagerLock");
	
	movieSeatLock = new Lock("movieSeatLock");

	movieTechLock = new Lock("movieTechLock");

	movieTechCV = new Condition("movieTechCV");

	mgrTTCustomerCommonLock = new Lock("mgrTTCustomerCommonLock");
	mgrTTCustomerCommonCV = new Condition("mgrTTCustomerCommonCV");
	ttStateLock = new Lock("ttStateLock");

	ticketTakerLineLock = new Lock("ticketTakerLineLock");
	ttTicketAcceptState = NOT_ACCEPTING;
	
	customerManagerExitLock = new Lock("customerManagerExitLock");

	for (int i=0;i<MAX_TC;i++)
	{
		ticketClerkLineCV[i] = new Condition("ticketClerkLineCV");
		ticketClerkLock[i] = new Lock("ticketClerkLock");
		ticketClerkCV[i] = new Condition("ticketClerkCV");
		ticketClerkState[i] = TCBUSY;
		ticketClerkManagerBreakCV[i] = new Condition("ticketClerkManagerBreakCV");
		TCBreakState[i] = OFF_BREAK;
	}

	
	for (int i=0;i<MAX_CC;i++)
	{
		concessionClerkLineCV[i] = new Condition("concessionClerkLineCV");
		concessionClerkLock[i] = new Lock("concessionClerkLock");
		concessionClerkCV[i] = new Condition("concessionClerkCV");
		concessionClerkState[i] = CCBUSY;
		concessionClerkManagerBreakCV[i] = new Condition("concessionClerkManagerBreakCV");
		CCBreakState[i] = OFF_BREAK;
	}

	
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

	for ( int i=0; i<numberOfGroups; i++ ) {
		//Generate the size of the group, not counting head customer
		//int groupSize = (rand()%5) + 1;
		//int groupSize = 5;

		int nextCustomerNumber = 0;
		//Create and fork the head customer thread
		//The head customer of a group needs a group number, group size,
		//and a customer number. I will store all 3 values in a struct pointer and pass
		//it as the second argument to Fork
		CustomerData *current = new CustomerData;
		current->customerNumber = nextCustomerNumber;
		nextCustomerNumber++;//don't need a lock as only this main thread does this
		current->groupNumber = i;
		current->groupSize = groupMemCount[i];
		groupMemberCountForFood[i]=groupMemCount[i];

		customerManagerExitLock -> Acquire();
		totalCustomerInCount++;		//increment incount of all customers
		remainingCount++;			//increment remainingcount for all customers
		customerManagerExitLock -> Release();

		printf("\nCustomer %d in group %d has entered the movie theater.\n", current->customerNumber , current -> groupNumber);
		Thread *t = new Thread("Head Customer");
		t->Fork( headCustomer, (int) current );

	//	printf("\n### Created Group Leader %d ###\n",i);

		//now make all the regular Customers for this group
		for ( int j=1; j<groupMemCount[i]; j++ ) {
			CustomerData *currentGM = new CustomerData;
			currentGM->customerNumber = nextCustomerNumber;
			nextCustomerNumber++;
			currentGM->groupNumber = i;
			currentGM->groupSize = groupMemCount[i];
			customerManagerExitLock -> Acquire();
			totalCustomerInCount++;
			remainingCount++;
			customerManagerExitLock -> Release();
			printf("\nCustomer %d in group %d has entered the movie theater.\n", currentGM->customerNumber, currentGM -> groupNumber);
			Thread *tGM = new Thread("Group Member");
			tGM->Fork(customerGroupMember, (int) currentGM );
		}
	}

}




//----------------------------------- TEST CASES BEGIN-----------------------------------------------

//===================================================================================================
//concessionClerkTotalSale: Concession Clerk function to take order of popcorn and soda from customer
//===================================================================================================

void concessionClerkTotalSale(int myIndex)
{
	int loopCount = 0;

	while(loopCount < numberOfGroups)
	{
		concessionClerkLineLock -> Acquire();
		concessionClerkState[myIndex] = CCNOT_BUSY;
		if (concessionClerkLineCount[myIndex] > 0)// Yes I have customers
		{
			//make myself busy
			concessionClerkState[myIndex] = CCBUSY;
			concessionClerkLineCount[myIndex]--;// as I am going to service this customer, decrement line count by 1
			//wake up the customer to be serviced
			printf("\nConcessionClerk %d has a line length %d and is signaling a customer.\n",myIndex, concessionClerkLineCount[myIndex]);
			concessionClerkLineCV[myIndex] -> Signal(concessionClerkLineLock);
		}
		else
		{
			//no one in line so i am not busy, so make myself not busy
			printf("\nConcessionClerk %d has no one in line. I am available for a customer.\n",myIndex);
			concessionClerkState[myIndex] = CCNOT_BUSY;
		}
		concessionClerkLock[myIndex] -> Acquire();
		// this lock is aacquired before releasing concessionClerkLineLock to aviod contex switching problems
		//as a customer to interact is found then we dont need this lock currently
		concessionClerkLineLock -> Release();
		//wait for customer to come to my counter and order number of popcorns
		concessionClerkCV[myIndex] -> Wait(concessionClerkLock[myIndex]);
		//calculate total amount to be paid by the customer, store it in monitor variable and share it with customer
		amt2CCTotalSale[myIndex] = 5 * numberOfPopcornForCCTotalSale[myIndex];
		//tell the customer to give the soda order
		concessionClerkCV[myIndex] -> Signal(concessionClerkLock[myIndex]);
		//wait for customer to come to my counter and order number of sodas
		concessionClerkCV[myIndex] -> Wait(concessionClerkLock[myIndex]);
		//calculate total amount to be paid by the customer, store it in monitor variable and share it with customer
		amt2CCTotalSale[myIndex] += 4 * numberOfSodaForCCTotalSale[myIndex];
		//give customer the bill to be paid and the food order(soda and popcorn)
		printf("\nConcessionClerk %d has an order for %d popcorn and %d soda. The cost is %d.\n",myIndex, numberOfPopcornForCCTotalSale[myIndex], numberOfSodaForCCTotalSale[myIndex], amt2CCTotalSale[myIndex]);
		concessionClerkCV[myIndex] -> Signal(concessionClerkLock[myIndex]);
		//wait for the customer to make payment
		concessionClerkCV[myIndex] -> Wait(concessionClerkLock[myIndex]);
		concessionClerkManagerLock -> Acquire();
		totalAmtByCCTotalSale += amt2CCTotalSale[myIndex];
		concessionClerkManagerLock -> Release();
		loopCount++;
		//as interaction is over, release the lock
		concessionClerkLock[myIndex] -> Release();
		if(loopCount == numberOfGroups)
			break;
	}
}

//===================================================================================================
//ticketClerkTotalSale: Ticket Clerk function to take order of tickets from customer
//===================================================================================================

void ticketClerkTotalSale(int myIndex)
{
	int groupsForTotalSale = 0;
	while(true)
	{
		ticketClerkLineLock -> Acquire();
		ticketClerkState[myIndex] = TCNOT_BUSY;
		if (ticketClerkLineCount[myIndex] > 0)// Yes I have customers
		{
			//make myself busy
			ticketClerkState[myIndex] = TCBUSY;
			ticketClerkLineCount[myIndex]--;// as I am going to service this customer, decrement line count by 1
			//wake up the customer to be serviced
			printf("\nTicketClerk %d has a line length %d and is signaling a customer.\n",myIndex, ticketClerkLineCount[myIndex]);
			ticketClerkLineCV[myIndex] -> Signal(ticketClerkLineLock);
		}
		else
		{
			//no one in line so i am not busy, so make myself not busy
			printf("\nTicketClerk %d has no one in line. I am available for a customer.\n",myIndex);
			ticketClerkState[myIndex] = TCNOT_BUSY;
		}
		//acquire a lock to start interaction with customer TT game
		ticketClerkLock[myIndex] -> Acquire();
		// this lock is aacquired before releasing ticketClerkLineLock to aviod contex switching problems
		//as a customer to interact is found then we dont need this lock currently
		ticketClerkLineLock -> Release();
		//wait for customer to come to my counter
		ticketClerkCV[myIndex] -> Wait(ticketClerkLock[myIndex]);
		printf("\nTicketClerk %d has an order for %d tickets and the cost is %d.\n",myIndex, numberOfTicketsForTCTotalSale[myIndex], 12 * numberOfTicketsForTCTotalSale[myIndex]);
		//calculate total amount to be paid by the customer, store it in monitor variable and share it with customer
		amt2TCTotalSale[myIndex] = 12 * numberOfTicketsForTCTotalSale[myIndex];
		//tell the customer the amount to be paid
		ticketClerkCV[myIndex] -> Signal(ticketClerkLock[myIndex]);
		//wait for the customer to make payment
		ticketClerkCV[myIndex] -> Wait(ticketClerkLock[myIndex]);
		ticketClerkManagerLock -> Acquire();
		totalAmtByTCTotalSale += amt2TCTotalSale[myIndex];
		ticketClerkManagerLock -> Release();
		//provide customer the tickets
		ticketClerkCV[myIndex] -> Signal(ticketClerkLock[myIndex]);
		groupsForTotalSale++;
		//as interaction is over, release the lock
		ticketClerkLock[myIndex] -> Release();
		if(groupsForTotalSale == numberOfGroups)
		{
			break;
		}
	}
}

//========================================================================================================
//interactWithCCTotalSale: Head Customer function to place order of popcorn and soda with Concession Clerk
//========================================================================================================

void interactWithCCTotalSale(CustomerData *current)
{
	int myConcessionClerk = -1;
	int myGroupSize = current -> groupSize;
	while(myConcessionClerk == -1)
	{
		concessionClerkLineLock -> Acquire();
		for(int i = 0; i < numberOfCC; i++)
		{
			if(concessionClerkState[i] == CCNOT_BUSY)
			{
				//found a free CC
				myConcessionClerk = i;
				concessionClerkState[i] = CCBUSY;
				break;
			}
		}

		//If no free CC available then check for shortest line of CC
		if(myConcessionClerk == -1)
		{
			int shortestCCLine = 0; //by default the shortest line is the 1st one
			int shortestCCLineLength = concessionClerkLineCount[0];
			for(int i = 1; i < numberOfCC; i++)
			{
				if ((shortestCCLineLength > concessionClerkLineCount[i]) && (concessionClerkState[i] == CCBUSY))
				{
					shortestCCLineLength = concessionClerkLineCount[i];
					shortestCCLine = i;
				}
			}
				myConcessionClerk = shortestCCLine;
				//get into the line
				printf("\nCustomer %d in Group %d is getting in ConcessionClerk line %d \n", current->customerNumber, current->groupNumber, myConcessionClerk);
				concessionClerkLineCount[myConcessionClerk]++;
				concessionClerkLineCV[myConcessionClerk] -> Wait(concessionClerkLineLock);
		}

		concessionClerkLineLock ->Release();
	}
	concessionClerkLock[myConcessionClerk] -> Acquire();
	//pass no. of popcorn and wake up clerk
	printf("\nCustomer %d in Group %d is walking up to ConcessionClerk %d to buy %d popcorn and %d soda.\n", current->customerNumber, current->groupNumber, myConcessionClerk , numberOfPopcornForCCTotalSale[myConcessionClerk], numberOfSodaForCCTotalSale[myConcessionClerk]);
	numberOfPopcornForCCTotalSale[myConcessionClerk] = 5;
	//signal the CC regarding no. of popcorns
	concessionClerkCV[myConcessionClerk] -> Signal(concessionClerkLock[myConcessionClerk]);
	//wait for CC to give acknowledgement for popcorn order
	concessionClerkCV[myConcessionClerk] -> Wait(concessionClerkLock[myConcessionClerk]);
	//pass no. of sodas and wake up clerk
	numberOfSodaForCCTotalSale[myConcessionClerk] = 3;
	//signal the CC regarding no. of soda
	concessionClerkCV[myConcessionClerk] -> Signal(concessionClerkLock[myConcessionClerk]);
	concessionClerkCV[myConcessionClerk] -> Wait(concessionClerkLock[myConcessionClerk]);
	printf("\nCustomer %d in Group %d in CooncessionClerk line %d is paying %d for food.\n", current->customerNumber, current->groupNumber, myConcessionClerk, 5 * numberOfPopcornForCCTotalSale[myConcessionClerk] +  4 * numberOfSodaForCCTotalSale[myConcessionClerk]);
	concessionClerkCV[myConcessionClerk] -> Signal(concessionClerkLock[myConcessionClerk]);
	printf("\nCustomer %d in Group %d is leaving ConcessionClerk %d.\n", current->customerNumber, current->groupNumber, myConcessionClerk);
	concessionClerkLock[myConcessionClerk] -> Release();
}

//===================================================================================================
//interactWithTCTotalSale: Head Customer Function to buy tickets from Ticket Clerk
//===================================================================================================

void interactWithTCTotalSale(CustomerData *current)
{
	int myTicketClerk = -1;
	int myGroupSize = current -> groupSize;
	while(myTicketClerk == -1)
	{
		ticketClerkLineLock -> Acquire();
		//Check if any TicketClerk is not busy
		for(int i = 0; i < numberOfTC; i++)
		{
			if(ticketClerkState[i] == TCNOT_BUSY)
			{
				//found a free TC
				myTicketClerk = i;
				ticketClerkState[i] = TCBUSY;
				break;
			}
		}
		//If no free TC available then check for shortest line of TC
		if(myTicketClerk == -1)
		{
			int shortestTCLine = 0; //by default the shortest line is the 1st one
			int shortestTCLineLength = ticketClerkLineCount[0];
			for(int i = 1; i < numberOfTC; i++)
			{
				if ((shortestTCLineLength > ticketClerkLineCount[i]) && (ticketClerkState[i] == TCBUSY))
				{
					shortestTCLineLength = ticketClerkLineCount[i];
					shortestTCLine = i;
				}
			}
			myTicketClerk = shortestTCLine;
				//get into the line
			printf("\nCustomer %d in Group %d is getting in TicketClerk line %d\n",current->customerNumber, current->groupNumber, myTicketClerk);
			ticketClerkLineCount[myTicketClerk]++;
			ticketClerkLineCV[myTicketClerk] -> Wait(ticketClerkLineLock);
		}
		ticketClerkLineLock ->Release();
	}
	//acquire then new lock to start interaction with ticket clerk like TT game
	ticketClerkLock[myTicketClerk] -> Acquire();
	//pass no. of tickets and wake up clerk
	numberOfTicketsForTCTotalSale[myTicketClerk] = myGroupSize;
	//signal the TC regarding no. of tickets
	printf("\nCustomer %d in Group %d is walking up to TicketClerk %d to buy %d tickets\n",current->customerNumber, current->groupNumber, myTicketClerk, numberOfTicketsForTCTotalSale[myTicketClerk]);
	ticketClerkCV[myTicketClerk] -> Signal(ticketClerkLock[myTicketClerk]);
	//wait for response from TC
	ticketClerkCV[myTicketClerk] -> Wait(ticketClerkLock[myTicketClerk]);
	//pay the amount by signalling the TC
	printf("\nCustomer %d in Group %d in TicketClerk line %d is paying %d for tickets\n",current->customerNumber, current->groupNumber, myTicketClerk, numberOfTicketsForTCTotalSale[myTicketClerk]*12 );
	ticketClerkCV[myTicketClerk] -> Signal(ticketClerkLock[myTicketClerk]);
	ticketClerkCV[myTicketClerk] -> Wait(ticketClerkLock[myTicketClerk]);
	//release the TC lock acquired above for booking tickets
	printf("\nCustomer %d in Group %d is leaving TicketClerk %d\n", current->customerNumber, current->groupNumber, myTicketClerk);
	ticketClerkLock[myTicketClerk] -> Release();
}


void headCustomerTotalSale(int currentHC)
{
	CustomerData *current = (CustomerData *) currentHC;
	interactWithTCTotalSale(current);
	interactWithCCTotalSale(current);
	
}

//===================================================================================================
//etermineTotalSales: Function for Case 5 
//===================================================================================================

void determineTotalSales()
{
	for (int i=0;i<numberOfTC;i++)
	{
		ticketClerkLineCV[i] = new Condition("ticketClerkLineCV");
		ticketClerkLock[i] = new Lock("ticketClerkLock");
		ticketClerkCV[i] = new Condition("ticketClerkCV");
		ticketClerkState[i] = TCBUSY;
		ticketClerkManagerBreakCV[i] = new Condition("ticketClerkManagerBreakCV");
	}
	
	for (int i=0;i<numberOfCC;i++)
	{
		concessionClerkLineCV[i] = new Condition("concessionClerkLineCV");
		concessionClerkLock[i] = new Lock("concessionClerkLock");
		concessionClerkCV[i] = new Condition("concessionClerkCV");
		concessionClerkState[i] = CCBUSY;
		concessionClerkManagerBreakCV[i] = new Condition("concessionClerkManagerBreakCV");
	}
	
	for(int i=0; i<numberOfTC; i++) {
		Thread *tcThread = new Thread("Ticket Clerk");
		tcThread -> Fork (ticketClerkTotalSale, i);
	}
	
	for(int i=0; i<numberOfCC; i++) {
		Thread *ccThread = new Thread("Concession Clerk");
		ccThread -> Fork (concessionClerkTotalSale, i);
	}

	for ( int i=0; i <numberOfGroups; i++ ) 
	{
		int groupSize = 5;
		int nextCustomerNumber = 0;
		CustomerData *current = new CustomerData;
		current->customerNumber = nextCustomerNumber;
		nextCustomerNumber++;//don't need a lock as only this main thread does this
		current->groupNumber = i;
		current->groupSize = groupSize;
		Thread *t = new Thread("Head Customer");
		t->Fork(headCustomerTotalSale, (int) current );	
	}
	
	for(int i=0;i<300;i++)
	{
		currentThread -> Yield();
	}
	printf("\nTicket Clerks Collected Total amount = %d\n",totalAmtByTCTotalSale);
	printf("\nConcession Clerks Collected Total amount = %d\n",totalAmtByCCTotalSale);
}

void sendClerksONAndOFFBreak()
{
	return;
}

//===================================================================================================
//ticketClerkInteract: Ticket Clerk function to take order of tickets from customer
//===================================================================================================

void ticketClerkInteract(int myIndex)
{
	int groupsArrivedToInterAct = 0;
	while(true)
	{
		ticketClerkLineLock -> Acquire();
		ticketClerkState[myIndex] = TCNOT_BUSY;
		if (ticketClerkLineCount[myIndex] > 0)// Yes I have customers
		{
			//make myself busy
			printf("\nTicketClerk %d has a line length %d and is signaling a customer.\n",myIndex, ticketClerkLineCount[myIndex]);
			ticketClerkState[myIndex] = TCBUSY;
			ticketClerkLineCount[myIndex]--;// as I am going to service this customer, decrement line count by 1
			ticketClerkLineCV[myIndex] -> Signal(ticketClerkLineLock);
		}
		else
		{
			//no one in line so i am not busy, so make myself not busy
			printf("\nTicketClerk %d has no one in line. I am available for a customer.\n",myIndex);
			ticketClerkState[myIndex] = TCNOT_BUSY;
		}
		ticketClerkLock[myIndex] -> Acquire();
		ticketClerkLineLock -> Release();
		ticketClerkCV[myIndex] -> Wait(ticketClerkLock[myIndex]);
		printf("\nTicketClerk %d has an order for %d tickets and the cost is %d.\n",myIndex, numberOfTicketsForTCInteract[myIndex], 12 * numberOfTicketsForTCInteract[myIndex]);
		ticketClerkCV[myIndex] -> Signal(ticketClerkLock[myIndex]);
		ticketClerkCV[myIndex] -> Wait(ticketClerkLock[myIndex]);
		ticketClerkManagerLock -> Acquire();
		ticketClerkManagerLock -> Release();
		//provide customer the tickets
		ticketClerkCV[myIndex] -> Signal(ticketClerkLock[myIndex]);
		groupsArrivedToInterAct++;
		ticketClerkLock[myIndex] -> Release();
		if(groupsArrivedToInterAct == numberOfGroups)
			break;
	}
}

//===================================================================================================
//headCustomerInteract: Head Customer function to buy tickets from Ticket Clerk
//===================================================================================================

void headCustomerInteract(int currentHC)
{
	CustomerData *current = (CustomerData *) currentHC;
	int myTicketClerk = -1;

	int myGroupSize = current -> groupSize;

	while(myTicketClerk == -1)
	{
		ticketClerkLineLock -> Acquire();

		//Check if any TicketClerk is not busy
		for(int i = 0; i < numberOfTC; i++)
		{
			if(ticketClerkState[i] == TCNOT_BUSY)
			{
				//found a free TC
				myTicketClerk = i;
				ticketClerkState[i] = TCBUSY;
				break;
			}
		}

		//If no free TC available then check for shortest line of TC
		if(myTicketClerk == -1)
		{
			int shortestTCLine = 0; //by default the shortest line is the 1st one
			int shortestTCLineLength = ticketClerkLineCount[0];
			for(int i = 1; i < numberOfTC; i++)
			{
				if ((shortestTCLineLength > ticketClerkLineCount[i]) && (ticketClerkState[i] == TCBUSY))
				{
					shortestTCLineLength = ticketClerkLineCount[i];
					shortestTCLine = i;
				}
			}
			myTicketClerk = shortestTCLine;
				//get into the line
			printf("\nCustomer %d in Group %d is getting in TicketClerk line %d\n",current->customerNumber, current->groupNumber, myTicketClerk);
			ticketClerkLineCount[myTicketClerk]++;
			ticketClerkLineCV[myTicketClerk] -> Wait(ticketClerkLineLock);
		}

		ticketClerkLineLock ->Release();
	}
	ticketClerkLock[myTicketClerk] -> Acquire();
	numberOfTicketsForTCInteract[myTicketClerk] = current -> groupSize; 
	printf("\nCustomer %d in Group %d is walking up to TicketClerk %d to buy %d tickets\n",current->customerNumber, current->groupNumber, myTicketClerk, numberOfTicketsForTCInteract[myTicketClerk]);
	ticketClerkCV[myTicketClerk] -> Signal(ticketClerkLock[myTicketClerk]);
	ticketClerkCV[myTicketClerk] -> Wait(ticketClerkLock[myTicketClerk]);
	printf("\nCustomer %d in Group %d in TicketClerk line %d is paying %d for tickets\n",current->customerNumber, current->groupNumber, myTicketClerk, numberOfTicketsForTCInteract[myTicketClerk]*12 );
	ticketClerkCV[myTicketClerk] -> Signal(ticketClerkLock[myTicketClerk]);
	ticketClerkCV[myTicketClerk] -> Wait(ticketClerkLock[myTicketClerk]);
	printf("\nCustomer %d in Group %d is leaving TicketClerk %d\n", current->customerNumber, current->groupNumber, myTicketClerk);
	ticketClerkLock[myTicketClerk] -> Release();
}

//===================================================================================================
//interactWithAClerkOrTT: Function for Case 3
//===================================================================================================

void interactWithAClerkOrTT()
{
	for (int i=0;i<numberOfTC;i++)
	{
		ticketClerkLineCV[i] = new Condition("ticketClerkLineCV");
		ticketClerkLock[i] = new Lock("ticketClerkLock");
		ticketClerkCV[i] = new Condition("ticketClerkCV");
		ticketClerkState[i] = TCBUSY;
		ticketClerkManagerBreakCV[i] = new Condition("ticketClerkManagerBreakCV");
		//TCBreakState[i] = OFF_BREAK;
	}
	
	for(int i=0; i<numberOfTC; i++) {
		Thread *tcThread = new Thread("Ticket Clerk");
		tcThread -> Fork (ticketClerkInteract, i);
	}

	for ( int i=0; i < numberOfGroups; i++ ) 
	{
		//Generate the size of the group, not counting head customer
		int groupSize = 5;
		int nextCustomerNumber = 0;
		CustomerData *current = new CustomerData;
		current->customerNumber = nextCustomerNumber;
		nextCustomerNumber++;//don't need a lock as only this main thread does this
		current->groupNumber = i;
		current->groupSize = groupSize;
		Thread *t = new Thread("Head Customer");
		t->Fork(headCustomerInteract, (int) current );	
	}
}
//===============================================================================================================
//theaterManagerReadAmount: Theater Manager function to determine the total amount collected by each Ticket Clerk
//===============================================================================================================

void theaterManagerReadAmount(int myIndex)
{
		for(int i=0;i<numberOfTC;i++)
			printf("\nTotal amount collected by TC[%d] = %d\n",i,totalAmtByTCReadAmount[i]);
}

//===================================================================================================
//ticketClerkReadAmount: Ticket Clerk function to take order of tickets from customer
//===================================================================================================

void ticketClerkReadAmount(int myIndex)
{	
	int groupsArrived = 0;
	while(true)
	{
		ticketClerkLineLock -> Acquire();
		ticketClerkState[myIndex] = TCNOT_BUSY;
		if (ticketClerkLineCount[myIndex] > 0)// Yes I have customers
		{
			ticketClerkState[myIndex] = TCBUSY;
			printf("\nTicketClerk %d has a line length %d and is signaling a customer.\n",myIndex, ticketClerkLineCount[myIndex]);
			ticketClerkLineCount[myIndex]--;// as I am going to service this customer, decrement line count by 1
			ticketClerkLineCV[myIndex] -> Signal(ticketClerkLineLock);
		}
		else
		{
			ticketClerkState[myIndex] = TCNOT_BUSY;
		}
		ticketClerkLock[myIndex] -> Acquire();
		ticketClerkLineLock -> Release();
		ticketClerkCV[myIndex] -> Wait(ticketClerkLock[myIndex]);
		amt2TCReadAmount[myIndex] = 12 * numberOfTicketsForTCReadAmount[myIndex];
		printf("\nTicketClerk %d has an order for %d tickets and the cost is %d.\n",myIndex, numberOfTicketsForTCReadAmount[myIndex], amt2TCReadAmount[myIndex]);
		ticketClerkCV[myIndex] -> Signal(ticketClerkLock[myIndex]);
		ticketClerkCV[myIndex] -> Wait(ticketClerkLock[myIndex]);
		totalAmtByTCReadAmount[myIndex] += amt2TCReadAmount[myIndex];
		ticketClerkCV[myIndex] -> Signal(ticketClerkLock[myIndex]);
		groupsArrived++;
		ticketClerkLock[myIndex] -> Release();
		if(groupsArrived == numberOfGroups){
			break;
		}
	}
}

//===================================================================================================
//headCustomerBookTickets: Head Customer function to buy tickets from Ticket Clerk
//===================================================================================================

void headCustomerBookTickets(int currentHC)
{
	CustomerData *current = (CustomerData *) currentHC;
	int myTicketClerk = -1;
	int myGroupSize = current -> groupSize;
	while(myTicketClerk == -1)
	{
		ticketClerkLineLock -> Acquire();
		//Check if any TicketClerk is not busy
		for(int i = 0; i < numberOfTC; i++)
		{
			if(ticketClerkState[i] == TCNOT_BUSY)
			{
				//found a free TC
				myTicketClerk = i;
				ticketClerkState[i] = TCBUSY;
				break;
			}
		}
		//If no free TC available then check for shortest line of TC
		if(myTicketClerk == -1)
		{
			int shortestTCLine = 0; //by default the shortest line is the 1st one
			int shortestTCLineLength = ticketClerkLineCount[0];
			for(int i = 1; i <numberOfTC; i++)
			{
				if ((shortestTCLineLength > ticketClerkLineCount[i]) && (ticketClerkState[i] == TCBUSY))
				{
					shortestTCLineLength = ticketClerkLineCount[i];
					shortestTCLine = i;
				}
			}
			myTicketClerk = shortestTCLine;
				//get into the line
			printf("\nCustomer %d in Group %d is getting in TicketClerk line %d\n",current->groupNumber, myTicketClerk);
			ticketClerkLineCount[myTicketClerk]++;
			ticketClerkLineCV[myTicketClerk] -> Wait(ticketClerkLineLock);
		}
		ticketClerkLineLock ->Release();
	}
	ticketClerkLock[myTicketClerk] -> Acquire();
	numberOfTicketsForTCReadAmount[myTicketClerk] = myGroupSize;
	printf("\nCustomer %d in Group %d is walking up to TicketClerk %d to buy %d tickets\n",current->customerNumber, current->groupNumber, myTicketClerk, numberOfTicketsForTCReadAmount[myTicketClerk]);
	ticketClerkCV[myTicketClerk] -> Signal(ticketClerkLock[myTicketClerk]);
	ticketClerkCV[myTicketClerk] -> Wait(ticketClerkLock[myTicketClerk]);
	printf("\nCustomer %d in Group %d in TicketClerk line %d is paying %d for tickets\n",current->customerNumber, current->groupNumber, myTicketClerk, numberOfTicketsForTCReadAmount[myTicketClerk]*12 );
	ticketClerkCV[myTicketClerk] -> Signal(ticketClerkLock[myTicketClerk]);
	ticketClerkCV[myTicketClerk] -> Wait(ticketClerkLock[myTicketClerk]);
	printf("\nCustomer %d in Group %d is leaving TicketClerk %d\n", current->customerNumber, current->groupNumber, myTicketClerk);
	ticketClerkLock[myTicketClerk] -> Release();
}

//===================================================================================================
//readTotalAmountFromClerk: Function for Case 2 where manager check amount collected by Ticket Taker
//===================================================================================================

void readTotalAmountFromClerk()
{
	for (int i=0;i<numberOfTC;i++)
	{
		ticketClerkLineCV[i] = new Condition("ticketClerkLineCV");
		ticketClerkLock[i] = new Lock("ticketClerkLock");
		ticketClerkCV[i] = new Condition("ticketClerkCV");
		ticketClerkState[i] = TCBUSY;
		ticketClerkManagerBreakCV[i] = new Condition("ticketClerkManagerBreakCV");
		//TCBreakState[i] = OFF_BREAK;
	}
	for(int i=0; i<numberOfTC; i++) {
		Thread *tcThread = new Thread("Ticket Clerk");
		tcThread -> Fork (ticketClerkReadAmount, i);
	}
	for ( int i=0; i < numberOfGroups; i++ ) 
	{
		//Generate the size of the group, not counting head customer
		int groupSize = 5;
		int nextCustomerNumber = 0;
		CustomerData *current = new CustomerData;
		current->customerNumber = nextCustomerNumber;
		nextCustomerNumber++;//don't need a lock as only this main thread does this
		current->groupNumber = i;
		current->groupSize = groupSize;
		Thread *t = new Thread("Head Customer");
		t->Fork(headCustomerBookTickets, (int) current );	
	}
	for(int i =0;i< 200; i++)
		currentThread -> Yield();
	Thread *managerThread = new Thread("Manager");
	managerThread -> Fork (theaterManagerReadAmount,0);
}

//===================================================================================================
//ticketClerkGetIntLine: Ticket Clerk function for Case 1
//===================================================================================================

void ticketClerkGetIntLine(int myIndex)//myIndex is used to indentify the TC to interact with
{
	int loopCount = 0;
	ticketClerkLineLock -> Acquire();	
	ticketClerkState[myIndex] = TCNOT_BUSY;		
	ticketClerkLineLock -> Release();	
}

//===================================================================================================
//getInTCLine: Ticket Clerk function to get into the Ticket Taker Line
//===================================================================================================

void getInTCLine(CustomerData *current)
{
	int myTicketClerk = -1;
	int myGroupSize = current -> groupSize;
	while(myTicketClerk == -1)
	{
		ticketClerkLineLock -> Acquire();
		for(int i = 0; i < numberOfTC; i++)
		{
			if(ticketClerkState[i] == TCNOT_BUSY)
			{
				//found a free TC
				myTicketClerk = i;
				ticketClerkState[i] = TCBUSY;
				break;
			}
		}
		//If no free TC available then check for shortest line of TC
		if(myTicketClerk == -1)
		{
			int shortestTCLine = 0; //by default the shortest line is the 1st one
			int shortestTCLineLength = ticketClerkLineCount[0];
			for(int i = 1; i < numberOfTC; i++)
			{
				if ((shortestTCLineLength > ticketClerkLineCount[i]) && (ticketClerkState[i] == TCBUSY))
				{
					shortestTCLineLength = ticketClerkLineCount[i];
					shortestTCLine = i;
				}
			}
			myTicketClerk = shortestTCLine;
			//get into the line
			printf("\nCustomer %d in Group %d is getting in TicketClerk line %d\n",current->customerNumber, current->groupNumber, myTicketClerk);
			ticketClerkLineCount[myTicketClerk]++;
		}
		ticketClerkLineLock ->Release();
	}
}

void headCustomerGetInLine(int currentHC)
{
	CustomerData * current = (CustomerData *) currentHC;
	getInTCLine(current);
}

//===================================================================================================
//customerGetIntoLine: Function for Case 1 to get into the Ticket Taker Line
//===================================================================================================

void customerGetIntoLine()
{
	for(int i=0; i<numberOfTC; i++) {
		Thread *tcThread = new Thread("Ticket Clerk");
		tcThread -> Fork (ticketClerkGetIntLine, i);
	}
	
	for ( int i=0; i < numberOfGroups; i++ ) 
	{
		int groupSize = 5;
		int nextCustomerNumber = 0;
		CustomerData *current = new CustomerData;
		current->customerNumber = nextCustomerNumber;
		nextCustomerNumber++;
		current->groupNumber = i;
		current->groupSize = groupSize;
		Thread *t = new Thread("Head Customer");
		t->Fork(headCustomerGetInLine, (int) current );	
	}
}

//===================================================================================================
//createGroups: Function for Case 7 to create Random group numbers
//===================================================================================================

void createGroups(int numCust)
{
	int randomNum = 0;
	int remainingCust = numCust;
	int i =0;
	numberOfGroups = 0;
	while(true)
	{
		if(remainingCust <= 5)
		{
			groupMemCount[i] = remainingCust;
			numberOfGroups++;
			break;
		}		
		randomNum = rand()%5 + 1;
		groupMemCount[i] = randomNum;
		remainingCust -=randomNum;
		numberOfGroups++;
		i++;
	}
}

//===================================================================================================
//initializeVariables: Function to initialize the locks, condition variables and monitor variables
//===================================================================================================

void initializeVariables()
{
	ticketClerkLineLock = new Lock("ticketClerkLineLock");
	ticketClerkManagerBreakLock = new Lock("ticketClerkManagerBreakLock");
	ticketClerkManagerLock = new Lock("ticketClerkManagerLock");
	ticketClerkManagerCV = new Condition("ticketClerkManagerCV"); // lock for updating the individual amount

	concessionClerkManagerLock = new Lock("concessionClerkManagerLock");
	concessionClerkLineLock = new Lock("concessionClerkLineLock");
	concessionClerkManagerBreakLock = new Lock("concessionClerkManagerBreakLock");
	concessionClerkManagerLock = new Lock("concessionClerkManagerLock"); // lock for updating the individual amount
	
	movieTechManagerLock = new Lock("movieTechManagerLock");
	movieTechManagerCV = new Condition("movieTechManagerCV");

	movieSeatManagerLock = new Lock("movieSeatManagerLock");
	
	movieSeatLock = new Lock("movieSeatLock");

	movieTechLock = new Lock("movieTechLock");

	movieTechCV = new Condition("movieTechCV");

	mgrTTCustomerCommonLock = new Lock("mgrTTCustomerCommonLock");
	mgrTTCustomerCommonCV = new Condition("mgrTTCustomerCommonCV");
	ttStateLock = new Lock("ttStateLock");

	ticketTakerLineLock = new Lock("ticketTakerLineLock");
	ttTicketAcceptState = NOT_ACCEPTING;
	
	customerManagerExitLock = new Lock("customerManagerExitLock");

	for (int i=0;i<MAX_TC;i++)
	{
		ticketClerkLineCV[i] = new Condition("ticketClerkLineCV");
		ticketClerkLock[i] = new Lock("ticketClerkLock");
		ticketClerkCV[i] = new Condition("ticketClerkCV");
		ticketClerkState[i] = TCBUSY;
		ticketClerkManagerBreakCV[i] = new Condition("ticketClerkManagerBreakCV");
		//TCBreakState[i] = OFF_BREAK;
	}

	
	for (int i=0;i<MAX_CC;i++)
	{
		concessionClerkLineCV[i] = new Condition("concessionClerkLineCV");
		concessionClerkLock[i] = new Lock("concessionClerkLock");
		concessionClerkCV[i] = new Condition("concessionClerkCV");
		concessionClerkState[i] = CCBUSY;
		concessionClerkManagerBreakCV[i] = new Condition("concessionClerkManagerBreakCV");
		//CCBreakState[i] = OFF_BREAK;
	}

	
	for (int i=0;i<MAX_TT;i++)
	{
		ticketTakerLineCV[i] = new Condition("ticketTakerLineCV");
		ticketTakerLock[i] = new Lock("ticketTakerLock");
		ticketTakerCV[i] = new Condition("ticketTakerCV");
		ticketTakerState[i] = TTBUSY;

	}

	for(int i=0;i<MAX_GROUPS;i++)
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
}


//----------------------------------- TEST CASES END-------------------------------------------------


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//_____________________________________________________________________________________________________________________________________________________________________________
//_____________________________________________________________________________________________________________________________________________________________________________

void Problem2()
{
	int choice = 0;
	int numOfGroups = 0;
	int numberOfCustomers = 0;
	int numberOfTicketClerks = 0;
	int numberOfConcessionClerks = 0; 
	int numberOfTicketTakers = 0;
	int fixedNumOfGroups = 20;
	int fixedGroupSize = 5;
	int fixedNumOfTC = 5;
	int fixedNumOfCC = 5;
	int fixedNumOfTT = 3;
	
		printf("\n============ Movie Theater Application ===========\n");
		printf("\n1. Test Case 1: Customers always take the shortest line and no 2 customers ever choose the same shortest line at the same time\n");
		printf("\n2. Test Case 2: Managers only read one from one Clerk's total money received, at a time\n");
		printf("\n3. Test Case 3: Customers do not leave a Clerk, or TicketTaker, until they are told to do so\n\t Clerks and TicketTakers do not start with another Customer until they know the current Customer has left their area.\n");
		printf("\n4. Test Case 4: Total sales never suffers from a race condition\n");
		printf("\n5. Test Case 5: Customer groups always move together through the theater\n");
		printf("\n6. System Test: Tests the Entire System\n");
		printf("\n7. Exit\n");
		printf("\nPlease enter your choice: \n");
		scanf("%d",&choice);
		
		switch(choice)
			{
				case 1:
							printf("\nNumber of Customers = %d",(fixedNumOfGroups*fixedGroupSize));
							printf("\nNumber of Groups = %d",fixedNumOfGroups);
							printf("\nNumber of TicketClerks = %d",fixedNumOfTT);
							printf("\nNumber of ConcessionClerks = %d",fixedNumOfCC);
							printf("\nNumber of TicketTakers = %d\n",fixedNumOfTT);
							numberOfGroups = fixedNumOfGroups;
							numberOfTT = fixedNumOfTT;
							numberOfTC = fixedNumOfTC;
							numberOfCC = fixedNumOfCC;
							initializeVariables();
							customerGetIntoLine();
							break;
						
				case 2:
							printf("\nNumber of Customers = %d",(fixedNumOfGroups*fixedGroupSize));
							printf("\nNumber of Groups = %d",fixedNumOfGroups);
							printf("\nNumber of TicketClerks = %d",fixedNumOfTT);
							printf("\nNumber of ConcessionClerks = %d",fixedNumOfCC);
							printf("\nNumber of TicketTakers = %d\n",fixedNumOfTT);
							numberOfGroups = fixedNumOfGroups;
							numberOfTT = fixedNumOfTT;
							numberOfTC = fixedNumOfTC;
							numberOfCC = fixedNumOfCC;
							initializeVariables();
							readTotalAmountFromClerk();
							break;
						
				case 3: 
							printf("\nNumber of Customers = %d",(fixedNumOfGroups*fixedGroupSize));
							printf("\nNumber of Groups = %d",fixedNumOfGroups);
							printf("\nNumber of TicketClerks = %d",fixedNumOfTT);
							printf("\nNumber of ConcessionClerks = %d",fixedNumOfCC);
							printf("\nNumber of TicketTakers = %d\n",fixedNumOfTT);
							numberOfGroups = fixedNumOfGroups;
							numberOfTT = fixedNumOfTT;
							numberOfTC = fixedNumOfTC;
							numberOfCC = fixedNumOfCC;
							initializeVariables();
							interactWithAClerkOrTT();
							break;
						
				case 4:
							printf("\nNumber of Customers = %d",(fixedNumOfGroups*fixedGroupSize));
							printf("\nNumber of Groups = %d",fixedNumOfGroups);
							printf("\nNumber of TicketClerks = %d",fixedNumOfTT);
							printf("\nNumber of ConcessionClerks = %d",fixedNumOfCC);
							printf("\nNumber of TicketTakers = %d\n",fixedNumOfTT);
							numberOfGroups = fixedNumOfGroups;
							numberOfTT = fixedNumOfTT;
							numberOfTC = fixedNumOfTC;
							numberOfCC = fixedNumOfCC;
							initializeVariables();
							determineTotalSales();
							break; 
						
				case 5:
							for(int c=0;c<MAX_GROUPS;c++)
								groupMemCount[c]=fixedGroupSize;
							printf("\nNumber of Customers = %d",(fixedNumOfGroups*fixedGroupSize));
							printf("\nNumber of Groups = %d",fixedNumOfGroups);
							printf("\nNumber of TicketClerks = %d",fixedNumOfTC);
							printf("\nNumber of ConcessionClerks = %d",fixedNumOfCC);
							printf("\nNumber of TicketTakers = %d\n",fixedNumOfTT);
							initializeVariables();
							numberOfGroups = fixedNumOfGroups;
							numberOfTT = fixedNumOfTT;
							numberOfTC = fixedNumOfTC;
							numberOfCC = fixedNumOfCC;
							movieTheater();
							break;
						
				case 6:
							printf("\nEnter Number of Customers(<=50): \n");
							scanf("%d",&numberOfCustomers);
							createGroups(numberOfCustomers);
							printf("\nEnter Number of Ticket Clerks(<=5): \n");
							scanf("%d",&numberOfTicketClerks);
							printf("\nEnter Number of Concession Clerks(<=5): \n");
							scanf("%d",&numberOfConcessionClerks);
							printf("\nEnter Number of Ticket Takers(<=3): \n");
							scanf("%d",&numberOfTicketTakers);
							numberOfTT = numberOfTicketTakers;
							numberOfTC = numberOfTicketClerks;
							numberOfCC = numberOfConcessionClerks;
							printf("\nNumber of Customers = %d",numberOfCustomers);
							printf("\nNumber of Groups = %d",numberOfGroups);
							printf("\nNumber of TicketClerks = %d",numberOfTC);
							printf("\nNumber of ConcessionClerks = %d",numberOfCC);
							printf("\nNumber of TicketTakers = %d\n",numberOfTT);
							initializeVariables();
							movieTheater();
							break;
				default:
							break;
			}
	
}
//====================================================================================================================================================================================
//	Project Part - 1 Test Cases
//====================================================================================================================================================================================
// --------------------------------------------------
// Test Suite
// --------------------------------------------------


// --------------------------------------------------
// Test 1 - see TestSuite() for details
// --------------------------------------------------
Semaphore t1_s1("t1_s1",0);       // To make sure t1_t1 acquires the
                                  // lock before t1_t2
Semaphore t1_s2("t1_s2",0);       // To make sure t1_t2 Is waiting on the 
                                  // lock before t1_t3 releases it
Semaphore t1_s3("t1_s3",0);       // To make sure t1_t1 does not release the
                                  // lock before t1_t3 tries to acquire it
Semaphore t1_done("t1_done",0);   // So that TestSuite knows when Test 1 is
                                  // done
Lock t1_l1("t1_l1");		  // the lock tested in Test 1

// --------------------------------------------------
// t1_t1() -- test1 thread 1
//     This is the rightful lock owner
// --------------------------------------------------
void t1_t1() {
    t1_l1.Acquire();
    t1_s1.V();  // Allow t1_t2 to try to Acquire Lock
 
    printf ("%s: Acquired Lock %s, waiting for t3\n",currentThread->getName(),
	    t1_l1.getName());
    t1_s3.P();
    printf ("%s: working in CS\n",currentThread->getName());
    for (int i = 0; i < 1000000; i++) ;
    printf ("%s: Releasing Lock %s\n",currentThread->getName(),
	    t1_l1.getName());
    t1_l1.Release();
    t1_done.V();
}

// --------------------------------------------------
// t1_t2() -- test1 thread 2
//     This thread will wait on the held lock.
// --------------------------------------------------
void t1_t2() {

    t1_s1.P();	// Wait until t1 has the lock
    t1_s2.V();  // Let t3 try to acquire the lock

    printf("%s: trying to acquire lock %s\n",currentThread->getName(),
	    t1_l1.getName());
    t1_l1.Acquire();

    printf ("%s: Acquired Lock %s, working in CS\n",currentThread->getName(),
	    t1_l1.getName());
    for (int i = 0; i < 10; i++)
	;
    printf ("%s: Releasing Lock %s\n",currentThread->getName(),
	    t1_l1.getName());
    t1_l1.Release();
    t1_done.V();
}

// --------------------------------------------------
// t1_t3() -- test1 thread 3
//     This thread will try to release the lock illegally
// --------------------------------------------------
void t1_t3() {

    t1_s2.P();	// Wait until t2 is ready to try to acquire the lock

    t1_s3.V();	// Let t1 do it's stuff
    for ( int i = 0; i < 3; i++ ) {
	printf("%s: Trying to release Lock %s\n",currentThread->getName(),
	       t1_l1.getName());
	t1_l1.Release();
    }
}

// --------------------------------------------------
// Test 2 - see TestSuite() for details
// --------------------------------------------------
Lock t2_l1("t2_l1");		// For mutual exclusion
Condition t2_c1("t2_c1");	// The condition variable to test
Semaphore t2_s1("t2_s1",0);	// To ensure the Signal comes before the wait
Semaphore t2_done("t2_done",0);     // So that TestSuite knows when Test 2 is
                                  // done

// --------------------------------------------------
// t2_t1() -- test 2 thread 1
//     This thread will signal a variable with nothing waiting
// --------------------------------------------------
void t2_t1() {
    t2_l1.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
	   t2_l1.getName(), t2_c1.getName());
    t2_c1.Signal(&t2_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t2_l1.getName());
    t2_l1.Release();
    t2_s1.V();	// release t2_t2
    t2_done.V();
}

// --------------------------------------------------
// t2_t2() -- test 2 thread 2
//     This thread will wait on a pre-signalled variable
// --------------------------------------------------
void t2_t2() {
    t2_s1.P();	// Wait for t2_t1 to be done with the lock
    t2_l1.Acquire();
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
	   t2_l1.getName(), t2_c1.getName());
    t2_c1.Wait(&t2_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t2_l1.getName());
    t2_l1.Release();
}
// --------------------------------------------------
// Test 3 - see TestSuite() for details
// --------------------------------------------------
Lock t3_l1("t3_l1");		// For mutual exclusion
Condition t3_c1("t3_c1");	// The condition variable to test
Semaphore t3_s1("t3_s1",0);	// To ensure the Signal comes before the wait
Semaphore t3_done("t3_done",0); // So that TestSuite knows when Test 3 is
                                // done

// --------------------------------------------------
// t3_waiter()
//     These threads will wait on the t3_c1 condition variable.  Only
//     one t3_waiter will be released
// --------------------------------------------------
void t3_waiter() {
    t3_l1.Acquire();
    t3_s1.V();		// Let the signaller know we're ready to wait
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
	   t3_l1.getName(), t3_c1.getName());
    t3_c1.Wait(&t3_l1);
    printf("%s: freed from %s\n",currentThread->getName(), t3_c1.getName());
    t3_l1.Release();
    t3_done.V();
}


// --------------------------------------------------
// t3_signaller()
//     This threads will signal the t3_c1 condition variable.  Only
//     one t3_signaller will be released
// --------------------------------------------------
void t3_signaller() {

    // Don't signal until someone's waiting
    
    for ( int i = 0; i < 5 ; i++ ) 
	t3_s1.P();
    t3_l1.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
	   t3_l1.getName(), t3_c1.getName());
    t3_c1.Signal(&t3_l1);
    printf("%s: Releasing %s\n",currentThread->getName(), t3_l1.getName());
    t3_l1.Release();
    t3_done.V();
}
 
// --------------------------------------------------
// Test 4 - see TestSuite() for details
// --------------------------------------------------
Lock t4_l1("t4_l1");		// For mutual exclusion
Condition t4_c1("t4_c1");	// The condition variable to test
Semaphore t4_s1("t4_s1",0);	// To ensure the Signal comes before the wait
Semaphore t4_done("t4_done",0); // So that TestSuite knows when Test 4 is
                                // done

// --------------------------------------------------
// t4_waiter()
//     These threads will wait on the t4_c1 condition variable.  All
//     t4_waiters will be released
// --------------------------------------------------
void t4_waiter() {
    t4_l1.Acquire();
    t4_s1.V();		// Let the signaller know we're ready to wait
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
	   t4_l1.getName(), t4_c1.getName());
    t4_c1.Wait(&t4_l1);
    printf("%s: freed from %s\n",currentThread->getName(), t4_c1.getName());
    t4_l1.Release();
    t4_done.V();
}


// --------------------------------------------------
// t2_signaller()
//     This thread will broadcast to the t4_c1 condition variable.
//     All t4_waiters will be released
// --------------------------------------------------
void t4_signaller() {

    // Don't broadcast until someone's waiting
    
    for ( int i = 0; i < 5 ; i++ ) 
	t4_s1.P();
    t4_l1.Acquire();
    printf("%s: Lock %s acquired, broadcasting %s\n",currentThread->getName(),
	   t4_l1.getName(), t4_c1.getName());
    t4_c1.Broadcast(&t4_l1);
    printf("%s: Releasing %s\n",currentThread->getName(), t4_l1.getName());
    t4_l1.Release();
    t4_done.V();
}
// --------------------------------------------------
// Test 5 - see TestSuite() for details
// --------------------------------------------------
Lock t5_l1("t5_l1");		// For mutual exclusion
Lock t5_l2("t5_l2");		// Second lock for the bad behavior
Condition t5_c1("t5_c1");	// The condition variable to test
Semaphore t5_s1("t5_s1",0);	// To make sure t5_t2 acquires the lock after
                                // t5_t1

// --------------------------------------------------
// t5_t1() -- test 5 thread 1
//     This thread will wait on a condition under t5_l1
// --------------------------------------------------
void t5_t1() {
    t5_l1.Acquire();
    t5_s1.V();	// release t5_t2
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
	   t5_l1.getName(), t5_c1.getName());
    t5_c1.Wait(&t5_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t5_l1.getName());
    t5_l1.Release();
}

// --------------------------------------------------
// t5_t1() -- test 5 thread 1
//     This thread will wait on a t5_c1 condition under t5_l2, which is
//     a Fatal error
// --------------------------------------------------
void t5_t2() {
    t5_s1.P();	// Wait for t5_t1 to get into the monitor
    t5_l1.Acquire();
    t5_l2.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
	   t5_l2.getName(), t5_c1.getName());
    t5_c1.Signal(&t5_l2);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t5_l2.getName());
    t5_l2.Release();
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
	   t5_l1.getName());
    t5_l1.Release();
}

// --------------------------------------------------
// TestSuite()
//     This is the main thread of the test suite.  It runs the
//     following tests:
//
//       1.  Show that a thread trying to release a lock it does not
//       hold does not work
//
//       2.  Show that Signals are not stored -- a Signal with no
//       thread waiting is ignored
//
//       3.  Show that Signal only wakes 1 thread
//
//	 4.  Show that Broadcast wakes all waiting threads
//
//       5.  Show that Signalling a thread waiting under one lock
//       while holding another is a Fatal error
//
//     Fatal errors terminate the thread in question.
// --------------------------------------------------
void TestSuite() {
    Thread *t;
    char *name;
    int i;
    
    // Test 1

    printf("Starting Test 1\n");

    t = new Thread("t1_t1");
    t->Fork((VoidFunctionPtr)t1_t1,0);

    t = new Thread("t1_t2");
    t->Fork((VoidFunctionPtr)t1_t2,0);

    t = new Thread("t1_t3");
    t->Fork((VoidFunctionPtr)t1_t3,0);

    // Wait for Test 1 to complete
    for (  i = 0; i < 2; i++ )
	t1_done.P();

    // Test 2

    printf("Starting Test 2.  Note that it is an error if thread t2_t2\n");
    printf("completes\n");

    t = new Thread("t2_t1");
    t->Fork((VoidFunctionPtr)t2_t1,0);

    t = new Thread("t2_t2");
    t->Fork((VoidFunctionPtr)t2_t2,0);

    // Wait for Test 2 to complete
    t2_done.P();

    // Test 3

    printf("Starting Test 3\n");

    for (  i = 0 ; i < 5 ; i++ ) {
	name = new char [20];
	sprintf(name,"t3_waiter%d",i);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)t3_waiter,0);
    }
    t = new Thread("t3_signaller");
    t->Fork((VoidFunctionPtr)t3_signaller,0);

    // Wait for Test 3 to complete
    for (  i = 0; i < 2; i++ )
	t3_done.P();

    // Test 4

    printf("Starting Test 4\n");

    for (  i = 0 ; i < 5 ; i++ ) {
	name = new char [20];
	sprintf(name,"t4_waiter%d",i);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)t4_waiter,0);
    }
    t = new Thread("t4_signaller");
    t->Fork((VoidFunctionPtr)t4_signaller,0);

    // Wait for Test 4 to complete
    for (  i = 0; i < 6; i++ )
	t4_done.P();

    // Test 5

    printf("Starting Test 5.  Note that it is an error if thread t5_t1\n");
    printf("completes\n");

    t = new Thread("t5_t1");
    t->Fork((VoidFunctionPtr)t5_t1,0);

    t = new Thread("t5_t2");
    t->Fork((VoidFunctionPtr)t5_t2,0);

}
















//====================================================================================================================================================================================

#endif

