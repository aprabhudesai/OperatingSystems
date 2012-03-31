//	Simple test cases for the threads assignment.
//

#include "copyright.h"
#include "system.h"
#ifdef CHANGED
#include "synch.h"
#endif

#ifdef CHANGED

// ----------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------

//Global Variables for Movie Theater
struct CustomerData
{
	int customerNumber;
	int groupNumber;
	int groupSize; //This value is used by the group head
};

#define MAX_TC 5
#define MAX_CC 5
#define MAX_TT 3
#define MAX_GROUPS 10

// Interaction: Customer with Ticket Clerk

//Lock tcLock("tcLock");

Lock *ticketClerkLineLock;// Deciding the line to get into

Condition *ticketClerkLineCV[MAX_TC];//To wait on the line lock to start interaction with customer

int ticketClerkLineCount[MAX_TC] = {0};

enum TCSTATE {TCBUSY , TCNOT_BUSY};

TCSTATE ticketClerkState[MAX_TC] = {TCNOT_BUSY};

int numberOfTicketsForTC[MAX_TC] = {0};

Lock *ticketClerkLock[MAX_TC];

Condition *ticketClerkCV[MAX_TC];

int amt2TC[MAX_TC] = {0}, totalAmtByTC = 0;

Lock *groupTicketLock[MAX_GROUPS];
Condition *groupTicketLockCV[MAX_GROUPS];
enum TICKETSTATUS {BOOKED, NOT_BOOKED};
TICKETSTATUS hasTickets[MAX_GROUPS] = {NOT_BOOKED};

Lock *groupFoodLock[MAX_GROUPS];
Condition *groupFoodLockCV[MAX_GROUPS];
int numberOfSoda[MAX_GROUPS], numberOfPopcorn[MAX_GROUPS];
int groupMemberCountForFood[MAX_GROUPS];

void interactionForTicketsWithTC(CustomerData * current)
{
	int myTicketClerk = -1;

	int myGroupSize = current -> groupSize;

	printf("\nHeadCustomer[%d] acquired line lock\n",current->groupNumber);
	ticketClerkLineLock -> Acquire();

	printf("\nHeadCustomer[%d] Checks if any TC is Free \n",current->groupNumber);
	//Check if any TicketClerk is not busy
	for(int i = 0; i < MAX_TC; i++)
	{
		if(ticketClerkState[i] == TCNOT_BUSY)
		{
			//found a free TC
			printf("\nHeadCustomer [%d] found Free TC[%d]\n",current->groupNumber,i);
			myTicketClerk = i;
			ticketClerkState[i] = TCBUSY;
			break;
		}
	}

	//If all are on break - condition has to be used

	//If no free TC available then check for shortest line of TC
	if(myTicketClerk == -1)
	{
		printf("\nHeadCustomer [%d] did not find free TC\n",current->groupNumber);
		int shortestTCLine = 0; //by default the shortest line is the 1st one
		int shortestTCLineLength = ticketClerkLineCount[0];
		for(int i = 1; i < MAX_TC; i++)
		{
			if ((shortestTCLineLength > ticketClerkLineCount[i]) && (ticketClerkState[i] == TCBUSY))
			{
				printf("\nHeadCustomer [%d] found shortest tc line TC[%d]\n",current->groupNumber,i);
				shortestTCLineLength = ticketClerkLineCount[i];
				shortestTCLine = i;
			}
		}

		// I have the shortest line
		myTicketClerk = shortestTCLine;

		printf("\nHeadCustomer [%d] entered TC[%d]\n",current->groupNumber,myTicketClerk);
		//get into the line
		ticketClerkLineCount[myTicketClerk]++;
		printf("\nHeadCustomer [%d] waiting in TC[%d] for its turn \n",current->groupNumber,myTicketClerk);
		ticketClerkLineCV[myTicketClerk] -> Wait(ticketClerkLineLock);
	}
	printf("\nHeadCustomer [%d] got signal from TC[%d] for its turn \n",current->groupNumber,myTicketClerk);

	printf("\nHeadCustomer [%d] releases LINE LOCK as it is serviced by TC[%d] for its turn \n",current->groupNumber,myTicketClerk);
	//release lock as I have come into the shortest line
	ticketClerkLineLock ->Release();

	printf("\nHeadCustomer [%d] acquired TC[%d] interaction Lock\n",current->groupNumber,myTicketClerk);
	//acquire then new lock to start interaction with ticket clerk like TT game
	ticketClerkLock[myTicketClerk] -> Acquire();

	printf("\nHeadCustomer [%d] asks and signals for tickets to TC[%d]\n",current->groupNumber,myTicketClerk);
	//pass no. of tickets and wake up clerk
	numberOfTicketsForTC[myTicketClerk] = myGroupSize;

	//signal the TC regarding no. of tickets
	ticketClerkCV[myTicketClerk] -> Signal(ticketClerkLock[myTicketClerk]);

	printf("\nHeadCustomer [%d] waiting for amount for Tickets by TC[%d]\n",current->groupNumber,myTicketClerk);

	printf("\nHeadCustomer [%d] ticket Status %d\n",current->groupNumber,hasTickets[current -> groupNumber]);
	//wait for response from TC
	ticketClerkCV[myTicketClerk] -> Wait(ticketClerkLock[myTicketClerk]);

	printf("\nHeadCustomer [%d] pays ticket amount to TC[%d]\n",current->groupNumber,myTicketClerk);
	//pay the amount by signalling the TC
	ticketClerkCV[myTicketClerk] -> Signal(ticketClerkLock[myTicketClerk]);

	printf("\nHeadCustomer [%d] waits for ticket from TC[%d]\n",current->groupNumber,myTicketClerk);
	ticketClerkCV[myTicketClerk] -> Wait(ticketClerkLock[myTicketClerk]);

	//hasTickets[MAX_GROUPS] is a monitor variable
	hasTickets[current -> groupNumber] = BOOKED;

	printf("\nHeadCustomer [%d] received tickets from TC[%d]\n",current->groupNumber,myTicketClerk);
	//-------end of booking of tickets section---------//

	/*
	 Lock* groupTicketLock{MAX_GROUPS]
	 Condition* groupTicketLockCV[MAX_GROUPS]
	 enum ticketStatus {BOOKED, NOT_BOOKED}
	 ticketStatus hasTickets[MAX_GROUPS]
	 */

//Interaction: Head Customer with group members
	//Acquire group ticket lock to start with interaction
	printf("\nHeadCustomer [%d] acquired interaction lock with members of Group\n",current->groupNumber);
	groupTicketLock[current -> groupNumber] -> Acquire();

	printf("\nHeadCustomer [%d] released interaction lock with TC[%d]\n",current->groupNumber,myTicketClerk);
	//release the TC lock acquired above for booking tickets
	ticketClerkLock[myTicketClerk] -> Release();

	if(current -> groupSize > 1)
	{
		//signal waiting group members about the booked tickets by broadcasting
		printf("\nHeadCustomer [%d] broadcasts all members of Group about booked tickets\n",current->groupNumber);
		groupTicketLockCV[current -> groupNumber] -> Broadcast(groupTicketLock[current -> groupNumber]);
	}
	//For Later Logic
	printf("\nHeadCustomer [%d] acquires food interaction lock with members of Group\n",current->groupNumber);
	//Acquire Food Lock for Head Customer to interact with group members
	groupFoodLock[current -> groupNumber] -> Acquire();

	//release the TC lock as the tickets are bought
	printf("\nHeadCustomer [%d] releases interaction lock with members of Group\n",current->groupNumber);
	groupTicketLock[current -> groupNumber] -> Release();

}

void interactWithGroupMembersForFoodOrder(CustomerData *myData)
{
	//int groupMemberCount = myData -> groupSize;
	int myGroupNumber = myData -> groupNumber;

	while(groupMemberCountForFood[myGroupNumber] > 1)
	{
		//waiting for order from every group member
		printf("\nHC[%d] is waiting for his Member Count before wait[%d] to order food\n",myGroupNumber,groupMemberCountForFood[myGroupNumber]);
		groupFoodLockCV[myGroupNumber] -> Wait(groupFoodLock[myGroupNumber]);

		//group member placed order so decrement count and wait for next group member to tell the food order
	}

	//order soda and popcorn [handle condition if I want to order or not]

	//head customer's soda and popcorn order
	numberOfSoda[myGroupNumber] += 1;
	numberOfPopcorn[myGroupNumber] += 1;

	if(myData -> groupSize > 1)
	{
		//inform all group members about order collected from them
		printf("\nHC[%d] broadcasts to all group members that food order collected\n",myGroupNumber);
		groupFoodLockCV[myGroupNumber] -> Broadcast(groupFoodLock[myGroupNumber]);
	}

	//release food lock as the order taken from group members
	printf("\nHC[%d] releases food lock\n", myGroupNumber);
	printf("\nTotal Soda for %d = %d\n",myGroupNumber,numberOfSoda[myGroupNumber]);
	printf("\nTotal Soda for %d = %d\n",myGroupNumber,numberOfSoda[myGroupNumber]);
	groupFoodLock[myGroupNumber] -> Release();
}

void customerGroupMember(int currentGM)
{
	CustomerData *myData = (CustomerData *) currentGM;
	int myGroupNumber = myData -> groupNumber;

	//Acquire group ticket lock to check if the head customer bought the tickets
	groupTicketLock[myGroupNumber] -> Acquire();
	printf("\nGM[%d] of Group %d waiting acquired lock for interaction with HC[%d]\n",myData->customerNumber,myGroupNumber,myGroupNumber);

	printf("Group Number %d has Ticket status = %d",myGroupNumber,hasTickets[myGroupNumber]);
	//check if group head has bought tickets
	if (hasTickets[myGroupNumber] != BOOKED)
	{
		printf("\nGM[%d] of Group %d waiting for tickets to be booked by HC[%d]\n",myData->customerNumber,myGroupNumber,myGroupNumber);
		//if tickets are not booked wait for the head customer
		groupTicketLockCV[myGroupNumber] -> Wait(groupTicketLock[myGroupNumber]);
	}
	printf("\nGM[%d] of Group %d released lock as tickets booked by HC[%d]\n",myData->customerNumber,myGroupNumber,myGroupNumber);

	//tickets are booked as release the lock, if food needed then go the food counter after this
	groupTicketLock[myGroupNumber] -> Release();


	//acquire the food lock for group before placing food order with the head of group
	groupFoodLock[myGroupNumber] -> Acquire();
	printf("\nGM[%d] of Group %d acquired lock to convey food order to HC[%d]\n",myData->customerNumber,myGroupNumber,myGroupNumber);

	//logic to check if customer wants to order soda (75% probability)
	//if (check if customer wants soda)

	numberOfSoda[myGroupNumber] += 1;

	//logic to check if customer wants to order popcorn (75% probability)
	//if (check if customer wants popcorn)
	numberOfPopcorn[myGroupNumber] += 1;

	//Order is given to the Head Customer, so decrement the count as a Member of Group
	groupMemberCountForFood[myGroupNumber]--;

	//inform head customer about the choice of soda an popcorn
	groupFoodLockCV[myGroupNumber] -> Signal(groupFoodLock[myGroupNumber]);
	printf("\nGM[%d] of Group %d Ordered Food and waiting for ack from HC[%d]\n",myData->customerNumber,myGroupNumber,myGroupNumber);

	//group member is waiting for the head to place order and get the order from the concession clerk
	groupFoodLockCV[myGroupNumber] -> Wait(groupFoodLock[myGroupNumber]);

	//release the food lock as the food order is obtained
	groupFoodLock[myGroupNumber] -> Release();
}

void headCustomer(int currentHC)
{
	CustomerData * current = (CustomerData *) currentHC;
	printf("\nHeadCustomer[%d] ENTERING interactionForTicketsWithTC to decide and interact with TC\n",current->groupNumber);
	interactionForTicketsWithTC(current);
	printf("\nHeadCustomer[%d] EXITING interactionForTicketsWithTC to decide and interact with TC\n",current->groupNumber);

	printf("\nHeadCustomer[%d] ENTERING interactWithGroupMembersForFoodOrder to take food order\n",current->groupNumber);
	interactWithGroupMembersForFoodOrder(current);
	printf("\nHeadCustomer[%d] EXITING interactWithGroupMembersForFoodOrder as food order taken\n",current->groupNumber);
}

//============================================================================

void ticketClerk(int myIndex)//myIndex is used to indentify the TC to interact with
{
	int loopCount = 0;
	while( loopCount < 5)
	{
		loopCount++;

		//acquire lock to check if anyone is waiting in line
		printf("\nTC[%d] acquires line lock\n",myIndex);
		ticketClerkLineLock -> Acquire();

		printf("\nTC[%d] Checks if customers waiting in line\n",myIndex);
		if (ticketClerkLineCount[myIndex] > 0)// Yes I have customers
		{
			//make myself busy
			printf("\nTC[%d] finds %d customers waiting in line\n",myIndex,ticketClerkLineCount[myIndex]);
			ticketClerkState[myIndex] = TCBUSY;

			ticketClerkLineCount[myIndex]--;// as I am going to service this customer, decrement line count by 1

			printf("\nTC[%d] processes first customer in line by signaling it\n",myIndex);
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

		printf("\nTC[%d] acquires interaction lock\n",myIndex);
		ticketClerkLock[myIndex] -> Acquire();
		// this lock is aacquired before releasing ticketClerkLineLock to aviod contex switching problems

		printf("\nTC[%d] releases line lock\n",myIndex);
		//as a customer to interact is found then we dont need this lock currently
		ticketClerkLineLock -> Release();

		printf("\nTC[%d] waiting for customer to come for interaction on interaction lock\n",myIndex);
		//wait for customer to come to my counter
		ticketClerkCV[myIndex] -> Wait(ticketClerkLock[myIndex]);
		printf("\nTC[%d] got signal from Customer regarding number of tickets\n",myIndex);

		//calculate total amount to be paid by the customer, store it in monitor variable and share it with customer
		amt2TC[myIndex] = 12 * numberOfTicketsForTC[myIndex];

		printf("\nTC[%d] asks customer to pay the amount %d for %d tickets\n",myIndex,amt2TC[myIndex],numberOfTicketsForTC[myIndex]);
		//tell the customer the amount to be paid
		ticketClerkCV[myIndex] -> Signal(ticketClerkLock[myIndex]);

		//wait for the customer to make payment
		ticketClerkCV[myIndex] -> Wait(ticketClerkLock[myIndex]);

		//Yohoo , customer made the payment, add it to the total ticket collection
		totalAmtByTC += amt2TC[myIndex];

		printf("\nTC[%d] gave tickets to Customer\n",myIndex);
		//provide customer the tickets
		ticketClerkCV[myIndex] -> Signal(ticketClerkLock[myIndex]);

		printf("\nTC[%d] releases interaction lock\n",myIndex);
		//as interaction is over, release the lock
		ticketClerkLock[myIndex] -> Release();
	}
}

void TestSuite() {

	//int numberOfGroups = rand()%10 + 1;

	int numberOfGroups = 5;

	ticketClerkLineLock = new Lock("ticketClerkLineLock");

	for (int i=0;i<MAX_TC;i++)
	{
		ticketClerkLineCV[i] = new Condition("ticketClerkLineCV");
		ticketClerkLock[i] = new Lock("ticketClerkLock");
		ticketClerkCV[i] = new Condition("ticketClerkCV");
	}

	for(int i=0;i<numberOfGroups;i++)
	{
		groupTicketLock[i] = new Lock("groupTicketLock");
		groupTicketLockCV[i] = new Condition("groupTicketLockCV");
		hasTickets[i] = NOT_BOOKED;

		groupFoodLock[i] = new Lock("groupFoodLock");
		groupFoodLockCV[i] = new Condition("groupFoodLockCV");
		numberOfSoda[i] = 0;
		numberOfPopcorn[i] = 0;
	}
	for(int i=0; i<MAX_TC; i++) {
		Thread *tcThread = new Thread("Ticket Clerk");
		tcThread -> Fork (ticketClerk, i);
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

		Thread *t = new Thread("Head Customer");
		t->Fork( headCustomer, (int) current );

		printf("\n### Created Group Leader %d ###\n",i);

		//now make all the regular Customers for this group
		for ( int j=1; j<groupSize; j++ ) {
			CustomerData *currentGM = new CustomerData;
			currentGM->customerNumber = nextCustomerNumber;
			nextCustomerNumber++;
			currentGM->groupNumber = i;

			Thread *tGM = new Thread("Group Member");
			tGM->Fork(customerGroupMember, (int) currentGM );
		}
	}

}
#endif
