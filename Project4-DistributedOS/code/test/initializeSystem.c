#include "syscall.h"

/* Define the maximum count of each entity in the application*/
#define MAX_TC 5
#define MAX_CC 5
#define MAX_TT 3
#define MAX_GROUPS 8
#define MAX_GROUP_SIZE 5

enum TCSTATE {TCBUSY,TCNOT_BUSY};
enum BREAKSTATE {ON_BREAK,OFF_BREAK};
enum TICKETSTATUS {BOOKED, NOT_BOOKED};
enum TTSTATE {NOT_ACCEPTING, ACCEPTING};
enum ORDERSTATE {ORDER_NOTRECEIVED,ORDER_RECEIVED};
enum CCSTATE {CCBUSY , CCNOT_BUSY};
enum TTWORKSTATE {TTBUSY , TTNOT_BUSY};
enum SUBMITSTATE {TICKETS_ACCEPTED,TICKETS_NOTACCEPTED};
enum SEATSTATE {SEATS_FREE, SEATS_OCCUPIED};
enum MOVIESTATE {OVER, NOT_OVER, FIRST_MOVIE};
enum MOVIEMGRSTATE {STARTING, NOT_STARTING};
enum CUSTOMERMOVIEWAITINGSTATE {NOT_WAITING, WAITING};
enum GROUPSTATE {REGROUPED, NOT_REGROUPED};


int movieSeatManagerLockId;
int movieSeatLockId;

char buff[1];
char *name = "";

int customerManagerExitLockId;

int groupTicketLockId[MAX_GROUPS];	/*lock for interaction with group members after receiving tickets*/
int groupTicketLockCVId[MAX_GROUPS];	/*CV for group members to wait for Head customer*/

int groupFoodLockId[MAX_GROUPS], groupFoodLockCVId[MAX_GROUPS];

int groupTicketLockId[MAX_GROUPS];	/*lock for interaction with group members after receiving tickets*/
int groupTicketLockCVId[MAX_GROUPS];	/*CV for group members to wait for Head customer*/

/*For mutual exclusion so that only one member customer can specify food order to Head Customer*/
int intraGroupFoodLockId[MAX_GROUPS];

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


int nextHCIndexLockId;





/* Single Entities */
int ticketClerkLineLockId;
int nextTCIndexMVId;
int nextCCIndexMVId;
int nextTTIndexMVId;
int nextHCIndexMVId;
int ttTicketAcceptStateMVId;
int totalAcceptedSeatsMVId;
int seatFullMessageMVId;
int allCustomerExitedCountMVId;
int totalCustomerInCountMVId;
int totalTicketClerkAmountMVId;
int totalConcessionClerkAmountMVId;	/*Total Amount collected by all clerks*/
int remainingCountMVId;
int ticketClerkBreakCountMVId;
int concessionClerkBreakCountMVId;
int commandToStartMVId;
int movieManagerStateMVId;
int movieFinishStateMVId;
int movieTechStateMVId;
int totalOccupiedSeatsMVId;
int totalMovieRoomExitCountMVId;
int numberOfGroupsMVId;
int customerNumberIndexMVId;
int customerNumberIndexLockId;
int mgrMovieTechExitMVId;

/* TC Entries */
int ticketClerkStateMVId;
int managerCustomerTCFlagMVId;
int TCBreakStateMVId;
int numberOfTicketsForTCMVId;
int amt2TCMVId;
int totalAmtByTCMVId;
int ticketClerkLineCountMVId;

/*MAX_GROUPS Entities */
int hasTicketsMVId;
int numberOfSodaMVId; 
int numberOfPopcornMVId;
int groupMemberCountForFoodMVId;
int hasOrderedMVId;
int groupFoodWaitFlagMVId;
int hasSubmittedMVId;
int groupTicketSubmitFlagMVId;
int groupExitCountMVId;
int groupRegroupStateMVId;
int groupRegroupFlagMVId;
int groupTheaterExitCountMVId;
int groupMemCountMVId;
int groupMembersCountMVId;
int memberCountForGroupMembersMVId;
int groupMemberCountForSeatsMVId;

/*	MAC_CC */
int concessionClerkLineCountMVId;
int concessionClerkStateMVId;
int managerCustomerCCFlagMVId;
int numberOfPopcornForCCMVId;
int numberOfSodaForCCMVId;
int amt2CCMVId;
int totalAmtByCCMVId;
int CCBreakStateMVId;
int concessionClerkLineLockId;	/* For mutual exclusion to enter Ticket Taker Line*/


/* MAX_TT */
int ticketTakerLineCountMVId;
int ticketTakerStateMVId;
int lineTicketMessageMVId;
int numberOfTicketsForTTMVId;
int acceptMessageMVId;
int ticketTakerLineLockId;	/* For mutual exclusion to enter Ticket Taker Line*/

/* ------------------------------------- */

int seatsMVId;  /* MV size needs to be 1*/
int numberOfFreeSeatsMVId;

/*------------------------------------------*/




int ticketClerkLineCVId[MAX_TC];
int ticketClerkLockId[MAX_TC];
int ticketClerkCVId[MAX_TC];

int concessionClerkLineCVId[MAX_CC];
int concessionClerkLockId[MAX_CC];
int concessionClerkCVId[MAX_CC];

int ticketTakerLineCVId[MAX_TT];
int ticketTakerLockId[MAX_TT];
int ticketTakerCVId[MAX_TT];

void createTCLocksCV()
{
	ticketClerkLineLockId = CreateLock("tcLineLock", 10);

	ticketClerkLineCVId[0] = CreateCV("ticketClerkLineCV0", 18);
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
	ticketClerkCVId[4] = CreateCV("ticketClerkCV4", 14);
}

void createCCLocksCV()
{
	concessionClerkLineLockId = CreateLock("ccLineLock", 10);
	concessionClerkLineCVId[0] = CreateCV("ccLineCV0", 9);
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
	concessionClerkCVId[4] = CreateCV("ccCV4", 5);
}

void createTTLocksCV()
{
	ticketTakerLineLockId = CreateLock("ticketTakerLineLock", 19);
	ticketTakerLineCVId[0] = CreateCV("ticketTakerLineCV0", 18);
	ticketTakerLockId[0] = CreateLock("ticketTakerLock0", 16);
	ticketTakerCVId[0] = CreateCV("ticketTakerCV0", 14);
	ticketTakerLineCVId[1] = CreateCV("ticketTakerLineCV1", 18);
	ticketTakerLockId[1] = CreateLock("ticketTakerLock1", 16);
	ticketTakerCVId[1] = CreateCV("ticketTakerCV1", 14);
	ticketTakerLineCVId[2] = CreateCV("ticketTakerLineCV2", 18);
	ticketTakerLockId[2] = CreateLock("ticketTakerLock2", 16);
	ticketTakerCVId[2] = CreateCV("ticketTakerCV2", 14);
}

void CreateMonitors(){
	
	int i;
	nextTCIndexMVId	= CreateMV("nextTCIndex",11,1);
	nextCCIndexMVId	= CreateMV("nextCCIndex",11,1);
	nextTTIndexMVId	= CreateMV("nextTTIndex",11,1);
	nextHCIndexMVId	= CreateMV("nextHCIndex",11,1);
	ttTicketAcceptStateMVId = CreateMV("ttTicketAcceptState",19,1);
	totalAcceptedSeatsMVId = CreateMV("totalAcceptedSeats",18,1);
	seatFullMessageMVId = CreateMV("seatFullMessage",15,1);
	allCustomerExitedCountMVId = CreateMV("allCustomerExitedCount",22,1);	
	totalCustomerInCountMVId = CreateMV("totalCustomerInCount",20,1);
	totalTicketClerkAmountMVId = CreateMV("totalTicketClerkAmount",22,1);
	totalConcessionClerkAmountMVId = CreateMV("totalCCAmount",13,1);
	remainingCountMVId = CreateMV("remainingCount",14,1);
	ticketClerkBreakCountMVId = CreateMV("ticketClerkBreakCount",21,1);
	commandToStartMVId = CreateMV("commandToStart",14,1);
	movieManagerStateMVId = CreateMV("movieManagerState",17,1);
	movieFinishStateMVId = CreateMV("movieFinishState",16,1);
	movieTechStateMVId = CreateMV("movieTechState",14,1);
	totalOccupiedSeatsMVId = CreateMV("totalOccupiedSeats",18,1);
	totalMovieRoomExitCountMVId = CreateMV("totalMovieRoomExitCount",23,1);
	concessionClerkBreakCountMVId = CreateMV("ccBreakCount",12,1);
	mgrMovieTechExitMVId = CreateMV("mgrMovieTechExit", 16, 1);
	
	/*	initialize */
	SetMV("nextTCIndex",11,0,0);
	SetMV("nextCCIndex",11,0,0);
	SetMV("nextTTIndex",11,0,0);
	SetMV("nextHCIndex",11,0,0);
	SetMV("totalAcceptedSeats",18,0,0);
	SetMV("seatFullMessage",15,0,0);
	SetMV("ccBreakCount",12,0,0);
	SetMV("ticketClerkBreakCount",21,0,0);
	SetMV("remainingCount",14,0,0);
	SetMV("totalCCAmount",13,0,0);
	SetMV("totalTicketClerkAmount",22,0,0);
	SetMV("totalCustomerInCount",20,0,0);
	SetMV("allCustomerExitedCount",22,0,0);
	SetMV("movieManagerState",17,0,NOT_STARTING);
	SetMV("commandToStart",14,0,0);
	SetMV("movieTechState",14,0,NOT_WAITING);
	SetMV("movieFinishState",16,0,FIRST_MOVIE);
	SetMV("totalMovieRoomExitCount",23,0,0);
	SetMV("totalOccupiedSeats",18,0,0);
	SetMV("ttTicketAcceptState",19,0,NOT_ACCEPTING);
	SetMV("ccBreakCount",12,0,0);
	SetMV("mgrMovieTechExit", 16, 0, 0);
	
	ticketClerkStateMVId = CreateMV("ticketClerkState",16,MAX_TC);
	managerCustomerTCFlagMVId = CreateMV("managerCustomerTCFlag",21,MAX_TC);
	TCBreakStateMVId = CreateMV("TCBreakState",12,MAX_TC);
	numberOfTicketsForTCMVId = CreateMV("numberOfTicketsForTC",20,MAX_TC);
	amt2TCMVId = CreateMV("amt2TC",6,MAX_TC);
	totalAmtByTCMVId = CreateMV("totalAmtByTC",12,MAX_TC);
	ticketClerkLineCountMVId = CreateMV("ticketClerkLineCount", 20, MAX_TC);
	
	for(i=0;i<MAX_TC;i++)
	{
		SetMV("ticketClerkState",16, i, TCNOT_BUSY);
		SetMV("ticketClerkLineCount",20, i, 0);
		SetMV("managerCustomerTCFlag",21,i,0);
		SetMV("TCBreakState",12,i,OFF_BREAK);
		SetMV("numberOfTicketsForTC",20,i,0);
		SetMV("amt2TC",6,i,0);
		SetMV("totalAmtByTC",12,i,0);
	}
	
	numberOfGroupsMVId = CreateMV("numberOfGroups",14,1);
		SetMV("numberOfGroups",14,0,10);
	customerNumberIndexMVId = CreateMV("customerNumberIndex",19,MAX_GROUPS);
	customerNumberIndexLockId = CreateLock("customerNumberIndexLock",23);
	hasTicketsMVId = CreateMV("hasTickets",10,MAX_GROUPS);
	numberOfSodaMVId = CreateMV("numberOfSoda",12,MAX_GROUPS);
	numberOfPopcornMVId = CreateMV("numberOfPopcorn",15,MAX_GROUPS);
	groupMemberCountForFoodMVId = CreateMV("groupMemberCountForFood",23,MAX_GROUPS);
	hasOrderedMVId = CreateMV("hasOrdered",10,MAX_GROUPS);
	groupFoodWaitFlagMVId = CreateMV("groupFoodWaitFlag",17,MAX_GROUPS);
	hasSubmittedMVId = CreateMV("hasSubmitted",12,MAX_GROUPS);
	groupTicketSubmitFlagMVId = CreateMV("groupTicketSubmitFlag",21,MAX_GROUPS);
	groupExitCountMVId = CreateMV("groupExitCount",14,MAX_GROUPS);
	groupRegroupStateMVId = CreateMV("groupRegroupState",17,MAX_GROUPS);
	groupRegroupFlagMVId = CreateMV("groupRegroupFlag",16,MAX_GROUPS);
	groupTheaterExitCountMVId = CreateMV("groupTheaterExitCount",21,MAX_GROUPS);
	groupMemCountMVId = CreateMV("groupMemCount",13,MAX_GROUPS);
	groupMembersCountMVId = CreateMV("groupMembersCount",17,MAX_GROUPS);
	memberCountForGroupMembersMVId = CreateMV("memCountForGrpMembers", 21,MAX_GROUPS);
	groupMemberCountForSeatsMVId = CreateMV("GMCountForSeats",15,MAX_GROUPS);
	nextHCIndexLockId = CreateLock("nextHCIndexLock",15);
	
	for(i=0;i<MAX_GROUPS;i++)
	{
		SetMV("hasTickets",10,i,NOT_BOOKED);
		SetMV("numberOfSoda",12,i,0);
		SetMV("numberOfPopcorn",15,i,0);
		SetMV("groupMemberCountForFood",23,i,MAX_GROUP_SIZE);
		/*SetMV("groupMemberCountForFood",23,i,2);*/
		SetMV("hasOrdered",10,i,ORDER_NOTRECEIVED);
		SetMV("groupFoodWaitFlag",17,i,0);
		SetMV("hasSubmitted",12,i,TICKETS_NOTACCEPTED);
		SetMV("groupTicketSubmitFlag",21,i,0);
		SetMV("groupExitCount",14,i,MAX_GROUP_SIZE);
		SetMV("groupRegroupState",17,i,NOT_REGROUPED);
		SetMV("groupRegroupFlag",16,i,0);
		SetMV("groupTheaterExitCount",21,i,0);
		SetMV("memCountForGrpMembers", 21,i,MAX_GROUP_SIZE);
		SetMV("groupMembersCount",17,i,MAX_GROUP_SIZE);
		/*SetMV("groupMembersCount",17,i,2);*/
		SetMV("GMCountForSeats",15,i,MAX_GROUP_SIZE);
	}
	concessionClerkLineCountMVId = CreateMV("ccLineCount",11,MAX_CC);
	concessionClerkStateMVId = CreateMV("ccState",7,MAX_CC);
	managerCustomerCCFlagMVId = CreateMV("managerCustomerCCFlag",21,MAX_CC);
	numberOfPopcornForCCMVId = CreateMV("numberOfPopcornForCC",20,MAX_CC);
	numberOfSodaForCCMVId = CreateMV("numberOfSodaForCC",17,MAX_CC);
	amt2CCMVId = CreateMV("amt2CC",6,MAX_CC);
	totalAmtByCCMVId = CreateMV("totalAmtByCC",12,MAX_CC);
	CCBreakStateMVId = CreateMV("CCBreakState",12,MAX_CC);
	
	for(i=0;i<MAX_CC;i++)
	{
		SetMV("ccLineCount",11,i,0);
		SetMV("ccState",7,i,CCNOT_BUSY);
		SetMV("managerCustomerCCFlag",21,i,0);
		SetMV("numberOfPopcornForCC",20,i,0);
		SetMV("numberOfSodaForCC",17,i,0);
		SetMV("amt2CC",6,i,0);
		SetMV("totalAmtByCC",12,i,0);
		SetMV("CCBreakState",12,i,OFF_BREAK);
	}
	
	ticketTakerLineCountMVId = CreateMV("ticketTakerLineCount",20,MAX_TT);
	ticketTakerStateMVId = CreateMV("ticketTakerState",16,MAX_TT);
	lineTicketMessageMVId = CreateMV("lineTicketMessage",17,MAX_TT);
	numberOfTicketsForTTMVId = CreateMV("numberOfTicketsForTT",20,MAX_TT);
	acceptMessageMVId = CreateMV("acceptMessage",13,MAX_TT);
	
	for(i=0;i<MAX_TT;i++)
	{
		SetMV("ticketTakerLineCount",20,i,0);
		/*SetMV("ticketTakerState",16,i,TTNOT_BUSY);*/
		SetMV("ticketTakerState",16,i,TTBUSY);
		SetMV("lineTicketMessage",17,i,0);
		SetMV("numberOfTicketsForTT",20,i,0);
		SetMV("acceptMessage",13,i,0);
	}
	/* Need to think about this
		int myOccupiedSeats[MAX_GROUPS][5]= {-1};
	*/

	/*seatsMVId[0] = CreateMV("SeatRow1",8,5);
	seatsMVId[1] = CreateMV("SeatRow2",8,5);
	seatsMVId[2] = CreateMV("SeatRow3",8,5);
	seatsMVId[3] = CreateMV("SeatRow4",8,5);
	seatsMVId[4] = CreateMV("SeatRow5",8,5);
	
	for(i=0;i<5;i++){
		SetMV("SeatRow1",8,i,SEATS_FREE);
		SetMV("SeatRow2",8,i,SEATS_FREE);
		SetMV("SeatRow3",8,i,SEATS_FREE);
		SetMV("SeatRow4",8,i,SEATS_FREE);
		SetMV("SeatRow5",8,i,SEATS_FREE);
	}*/
	
	seatsMVId = CreateMV("seats",5,25);
	
	for(i=0;i<25;i++){
		SetMV("seats",5,i,SEATS_FREE);
	}
	
	numberOfFreeSeatsMVId = CreateMV("numberOfFreeSeats",17,5);
	for(i=0;i<5;i++){
		SetMV("numberOfFreeSeats",17,i,5);
	}
}


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

void createGroupLocksCV()
{
	int myGroupNumber, i;
	customerManagerExitLockId = CreateLock("customerManagerExitLock", 23);

	for(i = 0;i < MAX_GROUPS; i++)
	{
		myGroupNumber = i;
		itoa(buff,1,myGroupNumber);
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
	}
}


int main(){

	movieTechLockId = CreateLock("movieTechLock",13);
	movieSeatManagerLockId = CreateLock("movieSeatManagerLock",20);

	movieSeatLockId = CreateLock("movieSeatLock",13);

	createTCLocksCV();
	createCCLocksCV();
	createTTLocksCV();
	CreateMonitors();
	createGroupLocksCV();

}
