#include "syscall.h"

/* Define the maximum count of each entity in the application*/
#define MAX_TC 5 /*Maximum number of TicketClerks*/
#define MAX_CC 5 /*Maximum number of ConcessionClerks*/
#define MAX_TT 3 /*Maximum number of TicketTakers*/
#define MAX_GROUPS 8	/* Maximum number of Customer Groups*/
/*#define NUM_OF_GROUPS 10 Maximum number of Customer Groups*/
#define MAX_GROUP_SIZE 5

int random_wait=100;
void theaterManager();

int ticketClerkManagerBreakLockId;
int ticketClerkManagerBreakCVId[MAX_TC];	/*lock between manager and ticket clerk to send him on breaks*/
/*int ticketClerkBreakCount = 0;*/



int ticketClerkLineLockId;	/* For mutual exclusion to enter Ticket Taker Line*/
int ticketClerkLineCVId[MAX_TC];	/*To wait on the line lock to start interaction with customer*/
/*int ticketClerkLineCount[MAX_TC] = {0}; */ /*To maintain the count of customer in each clerk's line*/
enum TCSTATE {TCBUSY,TCNOT_BUSY};
/*ticketClerkState[MAX_TC] = {TCBUSY};*/		/*To check the state of TicketClerk if is busy or available*/
	/*Initialize the state to busy so that if customers comes up before TicketClerk they will have to wait*/

/*int managerCustomerTCFlag[MAX_TC] = {0};	/*Used to inform customers in line to join a new line*/

enum BREAKSTATE {ON_BREAK,OFF_BREAK};
/*TCBreakState[MAX_TC] = {OFF_BREAK,OFF_BREAK,OFF_BREAK,OFF_BREAK,OFF_BREAK},
CCBreakState[MAX_TC] = {OFF_BREAK,OFF_BREAK,OFF_BREAK,OFF_BREAK,OFF_BREAK};*/
	/*To check the Break status of TicketClerk if a TicketClerk is working or not*/
	/*Initializing break state to OFF_BREAK*/


int movieTechLockId;
int movieTechCVId;
enum MOVIESTATE {OVER, NOT_OVER, FIRST_MOVIE};
/*movieFinishState = FIRST_MOVIE;*/



int ttStateLockId;		/*lock between tickettakers, customers and manager to see if tickets are being accepted or not */
enum TTSTATE {NOT_ACCEPTING, ACCEPTING};
/*ttTicketAcceptState = NOT_ACCEPTING;
int totalAcceptedSeats = 0;
int seatFullMessage = 0;*/



int movieTechManagerLockId;
int movieTechManagerCVId;	/*lock and CV between manager and movie technician for synchronization*/
/*int commandToStart = 0;*/



int movieSeatManagerLockId;
/*int totalOccupiedSeats = 0;	/*theater seats lock for manager*/
/*int totalMovieRoomExitCount = 0;*/




int ticketTakerLineLockId;	/*lock and CV to enter ticket taker's line*/
int ticketTakerLineCVId[MAX_TT];
/*int ticketTakerLineCount[MAX_TT] = {0};*/
enum TTWORKSTATE {TTBUSY , TTNOT_BUSY};
/*ticketTakerState[MAX_TT] = {TTBUSY, TTBUSY, TTBUSY};*/

/*int lineTicketMessage[MAX_TT]={0};*/

int mgrTTCustomerCommonLockId;
int mgrTTCustomerCommonCVId;	/*lock and CV between manager and ticket taker and customers to wait for manager to say when a movie is starting*/
enum MOVIEMGRSTATE {STARTING, NOT_STARTING};
/*movieManagerState = {NOT_STARTING};*/



int movieSeatLockId;

enum SEATSTATE {SEATS_FREE, SEATS_OCCUPIED};
/*seats[5][5]= {SEATS_FREE};*/

/*int myOccupiedSeats[MAX_GROUPS][5]= {-1};
int numberOfFreeSeats[5] = {5,5,5,5,5}; /* counter for the free seats per row*/


int ticketClerkManagerLockId;
int ticketClerkManagerCVId;	/*lock and CV between manager and ticket clerk going and coming back from breaks*/


int concessionClerkManagerLockId;

int concessionClerkManagerBreakLockId;
int concessionClerkManagerBreakCVId[MAX_TC];	/*lock between manager and concession clerk to send him on breaks*/
/*int concessionClerkBreakCount = 0;*/

int concessionClerkLineLockId;		/*lock and CV to enter into concession clerk line*/
int concessionClerkLineCVId[MAX_CC];
/*int concessionClerkLineCount[MAX_CC] = {0,0,0,0,0};*/
enum CCSTATE {CCBUSY , CCNOT_BUSY};
/*concessionClerkState[MAX_CC] =  {CCBUSY, CCBUSY,CCBUSY,CCBUSY,CCBUSY};*/

/*int managerCustomerCCFlag[MAX_CC] = {0,0,0,0,0};*/

int customerManagerExitLockId;
/*int allCustomerExitedCount = 0;		/* lock for manager to check if all customers who entered have exited the theater , if yes then stop the simulation*/
/*int totalCustomerInCount = 0;*/

/*
int totalTicketClerkAmount = 0;
int totalConcessionClerkAmount = 0;*/	/*Total Amount collected by all clerks*/
/*int remainingCount = 0;
int totalAmtByTC[MAX_TC] = {0};
int totalAmtByCC[MAX_CC] = {0,0,0,0,0};*/

/*
*
*	Monitor Variables Declaration
*
*/

int movieFinishStateMVId;
int ttTicketAcceptStateMVId;
int totalAcceptedSeatsMVId;
int seatFullMessageMVId;
int totalOccupiedSeatsMVId;	/*theater seats lock for manager*/
int totalMovieRoomExitCountMVId;
int commandToStartMVId;
int movieManagerStateMVId;
int concessionClerkBreakCountMVId;
int allCustomerExitedCountMVId;
int totalCustomerInCountMVId;
int totalTicketClerkAmountMVId;
int totalConcessionClerkAmountMVId;
int remainingCountMVId;

int mgrMovieTechExitMVId;
/*------------------------------------------------------------------------------*/
int ticketClerkLineCountMVId;
int ticketClerkStateMVId;
int managerCustomerTCFlagMVId;
int TCBreakStateMVId;
int totalAmtByTCMVId;
/*------------------------------------------------------------------------------*/
int ticketTakerLineCountMVId;
int ticketTakerStateMVId;
int lineTicketMessageMVId;
/*-----------------------------------------------------------------------------*/
int concessionClerkLineCountMVId;
int CCBreakStateMVId;
int concessionClerkStateMVId;
int managerCustomerCCFlagMVId;
int totalAmtByCCMVId;

/*-------------------------------------------*/
int seatsMVId;  /* MV size needs to be 5*/
int numberOfFreeSeatsMVId;


char buff[1];
char *name = "";
char *name1 = "";

/*---------------------------------------------------------------------*/
char ticketClerkLineCVName[5][20] = {"ticketClerkLineCV0",
								"ticketClerkLineCV1",
								"ticketClerkLineCV2",
								"ticketClerkLineCV3",
								"ticketClerkLineCV4",
								};


char ticketClerkManagerBreakCVName[5][20] = {
											"tcManagerBreakCV0",
											"tcManagerBreakCV1",
											"tcManagerBreakCV2",
											"tcManagerBreakCV3",
											"tcManagerBreakCV4",
											};


char concessionClerkLineCVName[5][12] = {
									"ccLineCV0",	
									"ccLineCV1",
									"ccLineCV2",	
									"ccLineCV3",	
									"ccLineCV4",	
									};


char concessionClerkManagerBreakCVName[5][20] = {
											"ccManagerBreakCV0",
											"ccManagerBreakCV1",
											"ccManagerBreakCV2",
											"ccManagerBreakCV3",
											"ccManagerBreakCV4",
										};


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
/*	FUNCTION: Manager Starts here*/
/*===========================================================*/
void theaterManager() 
{
	int random=0, TCCounter = 0, TCIndex[5], CCCounter = 0, CCIndex[5], simulationTime, i, j, k;
	int commandToStart;
	int totalOccupiedSeats;
	int totalAcceptedSeats;
	int movieFinishState;
	int seatFullMessage;
	int remainingCount;

	int temp1, temp2, temp3;
	while(1)
	{
		
		simulationTime = Random((600 - 200 + 1) + 400);
		randomYield(simulationTime);
		random_wait=100;
				while(random_wait--!=0);
				random_wait=100;
		/*===============================*/
		/*on break logic for Ticket Clerk*/
		
		TCCounter = 0; 
		for(i = 0; i < 5 ; i++)
		{
			TCIndex[i] = -1;
		}
		
		/*=================================*/
		
		randomYield(simulationTime);

		/*=================================*/
		Random(-1);

		Acquire("movieTechMgrLock", 16);
		Acquire("movieTechLock", 13);
		Acquire("mgrTTCustLock", 13);
		Acquire("ticketTakerLineLock", 19);
		Acquire("ttStateLock", 11);
		Acquire("movieSeatManagerLock", 20);

		if(GetMV("movieFinishState",16,0) == FIRST_MOVIE && GetMV("movieManagerState",17,0) != STARTING)
		{
			WriteToConsole("\nManager is telling the MovieTechnician to start the movie.\n\0", -1, -1, -1);
			SetMV("seatFullMessage",15,0,0);
			SetMV("movieManagerState",17,0,STARTING);	/*starting the process to start first movie*/
			
			for(k=0; k< MAX_TT ;k++)
			{
				SetMV("lineTicketMessage",17,k,0);
			}
			Broadcast("mgrTTCustCV", 11, "mgrTTCustLock", 13);	/*broadcast everyone waiting on manager that next movie is about to start*/

		}
		else if((GetMV("totalMovieRoomExitCount",23,0) == GetMV("totalOccupiedSeats",18,0)) && GetMV("movieFinishState",16,0) == OVER && GetMV("totalOccupiedSeats",18,0)!=0 && GetMV("remainingCount",14,0) != 0)
		{										/*if this is not the first movie then check if customers have left after first show*/
			Acquire("movieSeatLock", 13);			/*if they have, then make all seats as free*/
			
			for(i=0;i<25;i++){
				SetMV("seats",5,i,SEATS_FREE);
			}
			for(i=0;i<5;i++){
				SetMV("numberOfFreeSeats",17,i,5);
			}
			Release("movieSeatLock", 13);		/* release seat lock*/

			temp1 = GetMV("totalOccupiedSeats",18,0);
			temp2 = GetMV("remainingCount",14,0);
			SetMV("remainingCount",14,0,temp2-temp1);	/*decrement remaining count i.e customers yet to see the movie*/


			SetMV("totalOccupiedSeats",18,0,0);
			SetMV("seatFullMessage",15,0,0);			/*make seatFullmessage as 0 so that other entities start working*/
			
			SetMV("totalMovieRoomExitCount",23,0,0);

			/*SetMV("commandToStart", 14, 0, 0);*/
					
			WriteToConsole("\nManager is telling the MovieTechnician to start the movie.\n\0", -1, -1, -1);
			SetMV("movieManagerState",17,0,STARTING);		/* set movie state as starting*/
			
			for(k=0; k< MAX_TT ;k++)
			{
				SetMV("lineTicketMessage",17,k,0);		/* make Line ticket message of all TicketTakers as 0*/
			}
			
			Broadcast("mgrTTCustCV", 11, "mgrTTCustLock", 13);	/*broadcast everyone waiting on manager that next movie is about to start*/
		}


		Release("movieSeatManagerLock", 20);
		Release("ttStateLock", 11);
		Release("ticketTakerLineLock", 19);
		Release("mgrTTCustLock", 13);
		Release("movieTechLock", 13);
		Release("movieTechMgrLock", 16);

		/*=====================================*/
		
		randomYield(simulationTime);

		/*=====================================*/
		
		
		Acquire("tcManagerLock", 13);
		/*Customer made the payment, add it to the total ticket collection*/
		
		for(i=0;i<MAX_TC;i++)
		{
			temp1 = GetMV("totalTicketClerkAmount",22,0);
			SetMV("totalTicketClerkAmount",22,0,temp1 + GetMV("totalAmtByTC",12,i));	/*find total amount collected by all TicketClerks*/
			WriteToConsole("\nManager collected %d form TicketClerk %d.\n\0",GetMV("totalAmtByTC",12,i), i, -1);
			SetMV("totalAmtByTC",12,i,0);	/*reset amount collected by this clerk as it is counted by manager already*/
		}
		
		Release("tcManagerLock", 13);

		/*randomYield(simulationTime);*/
		
		Acquire("ccManagerLock", 13);
		for(i=0;i<MAX_CC;i++)
		{
			temp1 = GetMV("totalCCAmount",13,0);
			SetMV("totalCCAmount",13,0, temp1 + GetMV("totalAmtByCC",12,i));		/*find total amount collected by all Concessionclerks*/
			WriteToConsole("\nManager collected %d form ConcessionClerk %d.\n\0",GetMV("totalAmtByCC",12,i), i, -1);
			SetMV("totalAmtByCC",12,i,0);	/*reset amount collected by this clerk as it is counted by manager already*/
		}
		

		Release("ccManagerLock", 13);
		WriteToConsole("\nTotal money made by office = %d.\n\0", GetMV("totalTicketClerkAmount",22,0) + GetMV("totalCCAmount",13,0), -1, -1);
		
		randomYield(simulationTime);

		/*======================================*/
		
		/*on break logic for Concession Clerk*/
		CCCounter = 0; 
		
		for(i = 0; i < 5 ; i++)
		{
			CCIndex[i] = -1;
		}
		
		
		
		/*=================================*/
		randomYield(simulationTime);

		/*=================================*/
		Random(-1);
		
		Acquire("movieTechMgrLock", 16);
				Acquire("movieTechLock", 13);
				Acquire("mgrTTCustLock", 23);
				Acquire("ttStateLock", 11);
				Acquire("movieSeatManagerLock", 20);

		commandToStart = GetMV("commandToStart",14,0);
		totalOccupiedSeats = GetMV("totalOccupiedSeats",18,0);
		totalAcceptedSeats = GetMV("totalAcceptedSeats",18,0);
		movieFinishState = GetMV("movieFinishState",16,0);
		seatFullMessage = GetMV("seatFullMessage",15,0);
		remainingCount = GetMV("remainingCount",14,0);

		if(commandToStart == 0 && totalOccupiedSeats == totalAcceptedSeats && (movieFinishState == OVER || movieFinishState == FIRST_MOVIE) && (seatFullMessage == 1 || (remainingCount == totalOccupiedSeats && totalAcceptedSeats!=0)))
		{				/*determine if a new movie can begin or not*/
			SetMV("commandToStart", 14, 0, 1);
			SetMV("movieFinishState",16,0,NOT_OVER);	
			Signal("movieTechMgrCV", 14, "movieTechMgrLock", 16);		/*signal the movie tech to start a movie*/
			SetMV("movieManagerState",17,0,NOT_STARTING);
			SetMV("totalAcceptedSeats",18,0,0);			/* make total occupied seats as 0*/

		}
		Release("movieSeatManagerLock", 20);
		Release("ttStateLock", 11);
		Release("mgrTTCustLock", 23);
		Release("movieTechLock", 13);
		Release("movieTechMgrLock", 16);

		/*=============================================*/
		
		randomYield(simulationTime);

		/*=============================================*/

		Acquire("customerManagerExitLock", 23);	/*acquire lock to see if incount == exitcount*/
		if(GetMV("allCustomerExitedCount",22,0) == GetMV("totalCustomerInCount",20,0) && GetMV("movieFinishState",16,0) != FIRST_MOVIE)	/*check if total customers have watched the movie and exited*/
		{
			SetMV("mgrMovieTechExit", 16, 0, 1);

			Acquire("movieTechMgrLock", 16);
				Signal("movieTechMgrCV", 14, "movieTechMgrLock", 16);
			Release("movieTechMgrLock", 16);

			for(i = 0;i < MAX_TT; i++)
			{
				itoa(buff,1,i);
				name = "ticketTakerLock";
				*(name + 15) =  buff[0];
				Acquire(name,16);

				name1 = "ticketTakerCV";
				*(name1 + 13) =  buff[0];

				Signal(name1,14, name,16);
				Release(name,16);
			}

			for(i = 0;i < MAX_TC; i++)
			{
				itoa(buff,1,i);
				name = "ticketClerkLock";
				*(name + 15) =  buff[0];
				Acquire(name,16);

				name1 = "ticketClerkCV";
				*(name1 + 13) =  buff[0];

				Signal(name1,14, name,16);
				Release(name,16);
			}

			for(i = 0;i < MAX_CC; i++)
			{
				itoa(buff,1,i);
				name = "ccLock";
				*(name + 6) =  buff[0];
				Acquire(name,7);

				name1 = "ccCV";
				*(name1 + 4) =  buff[0];

				Signal(name1,5, name,7);
				Release(name,7);
			}

			Release("customerManagerExitLock", 23);	
			random_wait = 512;
			while(random_wait--!=0);
			random_wait = 112;
			Exit(0);
		}
		
		Release("customerManagerExitLock", 23);		/*release lock*/


	}
}

void CreateMonitors(){
	int i = 0;

	movieFinishStateMVId = CreateMV("movieFinishState",16,1);
	ttTicketAcceptStateMVId = CreateMV("ttTicketAcceptState",19,1);
	totalAcceptedSeatsMVId = CreateMV("totalAcceptedSeats",18,1);
	seatFullMessageMVId = CreateMV("seatFullMessage",15,1);
	totalOccupiedSeatsMVId = CreateMV("totalOccupiedSeats",18,1);	/*theater seats lock for manager*/
	totalMovieRoomExitCountMVId = CreateMV("totalMovieRoomExitCount",23,1);
	commandToStartMVId = CreateMV("commandToStart",14,1);
	movieManagerStateMVId = CreateMV("movieManagerState",17,1);
	concessionClerkBreakCountMVId = CreateMV("ccBreakCount",12,1);
	allCustomerExitedCountMVId = CreateMV("allCustomerExitedCount",22,1);
	totalCustomerInCountMVId = CreateMV("totalCustomerInCount",20,1);
	totalTicketClerkAmountMVId = CreateMV("totalTicketClerkAmount",22,1);
	totalConcessionClerkAmountMVId = CreateMV("totalCCAmount",13,1);
	remainingCountMVId = CreateMV("remainingCount",14,1);
	mgrMovieTechExitMVId = CreateMV("mgrMovieTechExit", 16, 1);
	/*------------------------------------------------------------------------------*/
	ticketClerkLineCountMVId = CreateMV("ticketClerkLineCount",20,MAX_TC);
	ticketClerkStateMVId = CreateMV("ticketClerkState",16,MAX_TC);
	managerCustomerTCFlagMVId = CreateMV("managerCustomerTCFlag",21,MAX_TC);
	TCBreakStateMVId = CreateMV("TCBreakState",12,MAX_TC);
	totalAmtByTCMVId = CreateMV("totalAmtByTC",12,MAX_TC);
	/*------------------------------------------------------------------------------*/
	ticketTakerLineCountMVId = CreateMV("ticketTakerLineCount",20,MAX_TT);
	ticketTakerStateMVId = CreateMV("ticketTakerState",16,MAX_TT);
	lineTicketMessageMVId = CreateMV("lineTicketMessage",17,MAX_TT);
	/*-----------------------------------------------------------------------------*/
	concessionClerkLineCountMVId = CreateMV("ccLineCount",11,MAX_CC);
	CCBreakStateMVId = CreateMV("CCBreakState",12,MAX_CC);
	concessionClerkStateMVId = CreateMV("ccState",7,MAX_CC);
	managerCustomerCCFlagMVId = CreateMV("managerCustomerCCFlag",21,MAX_CC);
	totalAmtByCCMVId = CreateMV("totalAmtByCC",12,MAX_CC);

	/*-------------------------------------------*/
	seatsMVId = CreateMV("seats",5,25);
	
	for(i=0;i<25;i++){
		SetMV("seats",5,i,SEATS_FREE);
	}
	
	numberOfFreeSeatsMVId = CreateMV("numberOfFreeSeats",17,5);
	for(i=0;i<5;i++){
		SetMV("numberOfFreeSeats",17,i,5);
	}
}



int main(){
	int i;	
	/*Random(-15);*/
	CreateMonitors();
	/*Create all Locks, Condition Variables and initialize monitor variables*/
	ticketClerkLineLockId = CreateLock("tcLineLock", 10);
	ticketClerkManagerBreakLockId = CreateLock("tcManagerBreakLock", 18);
	ticketClerkManagerLockId = CreateLock("tcManagerLock", 13);
	movieTechLockId = CreateLock("movieTechLock", 13);
	ttStateLockId = CreateLock("ttStateLock", 11);
	movieTechManagerLockId = CreateLock("movieTechMgrLock", 16);
	movieSeatManagerLockId = CreateLock("movieSeatManagerLock", 20);
	ticketTakerLineLockId = CreateLock("ticketTakerLineLock", 19);
	mgrTTCustomerCommonLockId = CreateLock("mgrTTCustLock", 13);
	customerManagerExitLockId = CreateLock("customerManagerExitLock", 23);
	concessionClerkLineLockId = CreateLock("ccLineLock", 10);
	concessionClerkManagerBreakLockId = CreateLock("ccManagerBreakLock", 18);
	concessionClerkManagerLockId = CreateLock("ccManagerLock", 13);
	movieSeatLockId = CreateLock("movieSeatLock", 13);
	
	mgrTTCustomerCommonCVId = CreateCV("mgrTTCustCV", 11);
	movieTechManagerCVId = CreateCV("movieTechMgrCV", 14);
	
/*
	for (i = 0; i < MAX_TC; i++)
	{
			ticketClerkLineCVId[i] = CreateCV(ticketClerkLineCVName[i],18);
			ticketClerkManagerBreakCVId[i] = CreateCV(ticketClerkManagerBreakCVName[i],17);
	}
	
	for (i = 0; i < MAX_CC; i++)
	{
			concessionClerkLineCVId[i] = CreateCV(concessionClerkLineCVName[i],9);
			concessionClerkManagerBreakCVId[i] = CreateCV(concessionClerkManagerBreakCVName[i],17);
	}
*/
	theaterManager();
}


