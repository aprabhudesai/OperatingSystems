#include "syscall.h"

#define MAX_GROUPS 8	/* Maximum number of Customer Groups*/
struct CustomerData	/*Customer Data Structure*/
{
	int customerNumber;
	int groupNumber;
	int groupSize; /*This value is used by the group head*/
	int mySeat;
};

enum TICKETSTATUS {BOOKED, NOT_BOOKED};
enum ORDERSTATE {ORDER_NOTRECEIVED,ORDER_RECEIVED};
enum SUBMITSTATE {TICKETS_ACCEPTED,TICKETS_NOTACCEPTED};
enum MOVIESTATE {OVER, NOT_OVER, FIRST_MOVIE};
enum CUSTOMERMOVIEWAITINGSTATE {NOT_WAITING, WAITING};
enum GROUPSTATE {REGROUPED, NOT_REGROUPED};

/*	All MV's	*/
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
int groupMembersCountMVId;
int memberCountForGroupMembersMVId;
int groupRegroupFlagMVId;

int myGroupNumber;
int groupSize;
int customerIndex;

int myOccupiedSeats[MAX_GROUPS][5]= {-1};

int nextHCIndexLockId;	/*lock between Head customers to take their index after they are forked*/

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
char *name = "";
char *name1 = "";
char *name2 = "";
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


void randomYield(int waitCount)
{
	while(waitCount-- > 0)
	{
		Yield();
	}
}

void customerGroupMember()
{
	int temp_popcorn, temp_soda, random_popcorn_order, random_soda_order, i;
	int wait_count, temp;
	struct CustomerData myData; /*to add customer group number logic*/

	myData.groupNumber = myGroupNumber;		/*group members create their data structure after sorting group number and group size*/
	myData.customerNumber =  customerIndex;
	myData.groupSize = groupSize;

	Acquire("customerManagerExitLock", 23);
			/*increment in count of all customers*/
		SetMV("totalCustomerInCount", 20, 0, GetMV("totalCustomerInCount", 20, 0) + 1);
			/*increment remaining count for all customers*/
		SetMV("remainingCount", 14, 0, GetMV("remainingCount", 14, 0) + 1);
	Release("customerManagerExitLock", 23);

	itoa(buff,1,myGroupNumber);
	name = "grpTLock";
	*(name+8) = buff[0];

	WriteToConsole("\nGroupMember %d in group %d has entered the movie theater.\n\0", myData.customerNumber , myData.groupNumber, -1);
	/*Acquire group ticket lock to check if the head customer bought the tickets*/
	Acquire(name, 9);

	/*check if group head has bought tickets*/

	name1 = "grpTCV";
	*(name1+6) = buff[0];


	if(GetMV("hasTickets", 10, myGroupNumber) != BOOKED)
	{
		WriteToConsole("\nCustomer %d of group %d is waiting for the HeadCustomer.\n\0", myData.customerNumber,myGroupNumber, -1);
		/*if tickets are not booked wait for the head customer*/
		Wait(name1, 7, name, 9);

	}
	WriteToConsole("\nCustomer %d of group %d has been told by the HeadCustomer to proceed.\n\0", myData.customerNumber,myGroupNumber, -1);
	/*tickets are booked as release the lock, if food needed then go the food counter after this*/
	Release(name, 9);

	name = "intraGrpFoodLock";
	*(name+16) = buff[0];

	/*Acquire Intra group Lock to have mutual exclusion in food order*/
	Acquire(name, 17);

	name1 = "groupFoodLock";
	*(name1+13) = buff[0];


	/*Acquire this lock to place order to Head Customer*/
	Acquire(name1, 14);

	WriteToConsole("\nCustomer %d in Group %d is in the lobby.\n\0", myData.customerNumber, myData.groupNumber, -1);
	temp_popcorn = 0;
	temp_soda = 0;
	random_popcorn_order = Random(100);		/*random value between 0-99*/
	if(random_popcorn_order > 25)					/*75 percent chance of an order*/
	{
		temp_popcorn = 1;
	}
		else
	{
		temp_popcorn = 0;
	}
	random_soda_order = Random(100);			/*random value between 0-99*/
	if(random_soda_order > 25)						/*75 percent chance of an order*/
	{
		temp_soda = 1;
	}
	else
	{
		temp_soda = 0;
	}
	WriteToConsole("\nCustomer %d in Group %d wants %d popcorn and %d soda.\n\0",
	(((myData.customerNumber) * 1000000) + ((myGroupNumber) * 10000) +  ((temp_popcorn) * 100) + temp_soda), -5, 4);	/* 4  or more arguments so need to pass arguments using IInd method*/

	SetMV("numberOfPopcorn", 15, myGroupNumber, GetMV("numberOfPopcorn", 15, myGroupNumber) + temp_popcorn);
	SetMV("numberOfSoda", 12, myGroupNumber, GetMV("numberOfSoda", 12, myGroupNumber) + temp_soda);

	/*Order is placed so decrement the count and signal head customer*/

	SetMV("groupMemberCountForFood", 23, myGroupNumber, GetMV("groupMemberCountForFood", 23, myGroupNumber) - 1);

	name2 = "groupFoodLockCV";
	*(name2+15) = buff[0];

	Signal(name2, 16, name1, 14);  /*signal head customer every time*/

	Release(name1, 14);

	/*release intragroup food lock so that next waiting customer can order to head customer*/
	Release(name, 19);

	name2 = "groupFoodWaitLock";
	*(name2+17) = buff[0];

	/*Acquire lock to check if Head Customer has received the Order*/
	Acquire(name2, 18);

	/* wait if the head customer is interacting with concession clerk*/

	name = "groupFoodWaitCV";
	*(name+15) = buff[0];

	if(GetMV("hasOrdered", 10, myGroupNumber) == ORDER_NOTRECEIVED)
	{
		SetMV("groupFoodWaitFlag", 17, myGroupNumber, 1);	/*this is for Head customer's Reference that atleast one group member is waiting for Order Confirmation*/
		Wait(name, 16, name2, 18);
	}


	WriteToConsole("\nCustomer %d in Group %d is leaving the lobby.\n\0",myData.customerNumber, myData.groupNumber, -1);
	Release(name2, 18);

	name = "groupTSLock";
	*(name+11) = buff[0];

	name1 = "groupTSCV";
	*(name1+9) = buff[0];

	/*Acquire lock to check if Head Customer has given tickets to Ticket Taker*/
	Acquire(name, 12);

	/*if(hasSubmitted[myGroupNumber]== TICKETS_NOTACCEPTED)*/ /* wait if the head customer is interacting with TicketTaker to submit tickets*/
	if(GetMV("hasSubmitted", 12, myGroupNumber) == TICKETS_NOTACCEPTED)
	{
		/*groupTicketSubmitFlag[myGroupNumber] = 1;*/ /*this is for Head customer's Reference that atleast one group member is waiting for Order Confirmation*/
		SetMV("groupTicketSubmitFlag", 21, myGroupNumber, 1);
		Wait(name1, 10, name, 12);
	}
	Release(name, 12);



	name = "groupSeatWaitLock";
	*(name+17) = buff[0];

	/*Acquire this lock to receive the seat number allocated by Head Customer*/
	Acquire(name, 18);

	/*The seat allocated is received so decrement the counter of group members needed to communicate with HC*/
	SetMV("GMCountForSeats",15, myGroupNumber, GetMV("GMCountForSeats",15, myGroupNumber) - 1);

	WriteToConsole("\nCustomer %d in group %d is sitting in a theater room seat\n\0", myData.customerNumber, myGroupNumber, -1);

	name1 = "groupSeatWaitCV";
	*(name1+15) = buff[0];
	Signal(name1, 16, name, 18);

	Release(name, 18);



	/*take movie start-stop lock and wait on that*/
	/*-----------------------------------------At this point every body is seated in movie hall--------------------------------------------*/

	Acquire("movieTechLock", 13);	/*Acquire movieTech lock to see if movie is over*/

	/*if(movieFinishState == NOT_OVER || movieFinishState == FIRST_MOVIE)*/
	temp = GetMV("movieFinishState", 16, 0);
	if( temp == NOT_OVER  || temp == FIRST_MOVIE)
	{
		Wait("movieTechCV", 11, "movieTechLock", 13);		/* wait for movie to over*/
	}

	Release("movieTechLock", 13);	/* release movietech lock*/



	myData.mySeat = 0;
	name = "groupExitLock";
	*(name+13) = buff[0];

	/*Group Member acquires movieexitLock to interact with HeadCustomer for Exit*/
	Acquire(name, 14);

	/*groupExitCount[myGroupNumber]--;*/
	SetMV("groupExitCount", 14,myGroupNumber, GetMV("groupExitCount", 14,myGroupNumber) - 1);

	name1 = "groupExitCV";
	*(name1+11) = buff[0];
	Signal(name1, 12, name, 14);
	WriteToConsole("\nCustomer %d in group %d is getting out of a theater room seat\n\0", myData.customerNumber, myGroupNumber, -1);

	Release(name, 14);
	/*GroupMember releases movieexitLock after interacting with HeadCustomer*/



	name = "groupRegroupLock";
	*(name+16) = buff[0];

	/*Acquire regroup lock and wait for members of group went to bathroom*/
	wait_count = Random(100);
	wait_count = wait_count * 25;
	while(wait_count-- !=0);

	Acquire(name, 17);

	/*if(groupRegroupState[myGroupNumber] == NOT_REGROUPED)*/
	if(GetMV("groupRegroupState", 17, myGroupNumber) == NOT_REGROUPED)
	{
		WriteToConsole("\nCustomer %d of group %d is waiting for the group to form.\n\0", myData.customerNumber,myGroupNumber, -1);
		/*groupRegroupFlag[myGroupNumber] = 1;*/			/*Wait for all the group memner before you can exit the movie hall*/
		SetMV("groupRegroupFlag", 16, myGroupNumber, 1);

		name1 = "groupRegroupCV";
		*(name1+14) = buff[0];
		Wait(name1, 15, name, 17);
	}

	Release(name, 17);		/*release grgroup lock before proceeding*/



	/*===============================================================*/
	/*	FUNCTION: All customers may go to bathroom and then Exit here*/
	/*===============================================================*/

	if(Random(100) < 25)
	{		/*25 percent chance of a person to go to bathroom*/
		WriteToConsole("\nCustomer %d in Group %d is going to the bathroom.\n\0",  myData.customerNumber, myData.groupNumber, -1);
		randomYield(200);
		WriteToConsole("\nCustomer %d in Group %d is leaving the bathroom.\n\0",  myData.customerNumber, myData.groupNumber, -1);
	}

	name = "groupTELock";
	*(name+11) = buff[0];

	name1 = "groupTECV";
	*(name1+9) = buff[0];

	wait_count = Random(100);
	wait_count = wait_count * 25;
	while(wait_count-- !=0);
	Acquire(name, 12);

	temp = GetMV("groupTheaterExitCount", 21, myGroupNumber) + 1;
	SetMV("groupTheaterExitCount", 21, myGroupNumber, temp);

	/* All group members will wait, the last one will come and broadcast*/
	if(temp == myData.groupSize)
	{
		Broadcast(name1, 10, name, 12);
	}
	else
	{
		Wait(name1, 10, name, 12);
	}
	Release(name, 12);
	Acquire("customerManagerExitLock", 23);
		/*increment the total exit count with the group size*/

	SetMV("allCustomerExitedCount", 22, 0, GetMV("allCustomerExitedCount", 22, 0) + 1);
	Release("customerManagerExitLock", 23);

	WriteToConsole("\nCustomer %d in group %d has left the movie theater.\n\0", myData.customerNumber, myGroupNumber, -1);

	Exit(0);
}



void CreateMVs()
{
	/*memberCountForGroupMembersMVId = CreateMV("memCountForGrpMembers", 21, MAX_GROUPS);
	remainingCountMVId = CreateMV("remainingCount", 14, 1);
	numberOfSodaMVId = CreateMV("numberOfSoda", 12, MAX_GROUPS);
	numberOfPopcornMVId = CreateMV("numberOfPopcorn", 15, MAX_GROUPS);
	groupMemberCountForFoodMVId = CreateMV("groupMemberCountForFood", 23, MAX_GROUPS);
	hasTicketsMVId = CreateMV("hasTickets", 10, MAX_GROUPS);
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
	groupMembersCountMVId = CreateMV("groupMembersCount", 17, MAX_GROUPS);
	movieFinishStateMVId = CreateMV("movieFinishState", 16, 1);*/
}

void CreateLockAndCV()
{

	customerManagerExitLockId = CreateLock("customerManagerExitLock", 23);
	movieTechLockId = CreateLock("movieTechLock", 13);
	movieTechCVId = CreateCV("movieTechCV", 11);

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
	int temp;
	temp=3200;
	for(i = 0 ; i < temp ; i++){
	}
	/*CreateMVs();*/
	/*nextHCIndexLockId = CreateLock("nextHCIndexLock",15);*/
	
	temp = Acquire("nextHCIndexLock",15);
	for(i = 0; i < MAX_GROUPS; i++)		/*All group members will take the lock and find out which group they go in*/
	{
		/*if(GetMV("memCountForGrpMembers", 21, i) != -1)
		{*/
			temp = GetMV("memCountForGrpMembers", 21, i);

			if(temp > 1)	/*greater than 1 because one will be the Head customer*/
			{
				temp = temp -1;
				SetMV("memCountForGrpMembers", 21, i, temp);
				/*customerIndex = GetMV("groupMembersCount", 17, i) - temp;*/
				customerIndex = 5 - temp;
				myGroupNumber = i;
				groupSize = 5;
				/*groupSize = GetMV("groupMembersCount", 17, myGroupNumber);*/
				break;
			}
		/*}*/
	}

	Release("nextHCIndexLock",15);

	CreateLockAndCV();

	customerGroupMember();
}
