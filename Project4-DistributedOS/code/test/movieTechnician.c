#include "syscall.h"

enum MOVIESTATE {OVER, NOT_OVER, FIRST_MOVIE};
enum CUSTOMERMOVIEWAITINGSTATE {NOT_WAITING, WAITING};
int commandToStartMVId = 0;
int movieFinishStateMVId;
int movieTechStateMVId;
int mgrMovieTechExitMVId;

int movieTechManagerLockId;
int movieTechManagerCVId;	/*lock and CV between manager and movie technician for synchronization*/
int movieTechLockId;
int movieTechCVId;

int simulationTime;

void randomYield(int waitCount)
{
	while(waitCount-- > 0)
	{
		Yield();
	}
}

void CreateMVs()
{
	commandToStartMVId = CreateMV("commandToStart", 14, 1);
	movieFinishStateMVId = CreateMV("movieFinishState", 16, 1);
	movieTechStateMVId = CreateMV("movieTechState", 14, 1);
	mgrMovieTechExitMVId = CreateMV("mgrMovieTechExit", 16, 1);

}

void CreateLockAndCvs()
{
	movieTechManagerLockId = CreateLock("movieTechMgrLock", 16);
	movieTechManagerCVId = CreateCV("movieTechMgrCV", 14);
	movieTechLockId = CreateLock("movieTechLock", 13);
	movieTechCVId = CreateCV("movieTechCV", 11);

}
int main()
{
	/*Random(-11);*/
	CreateMVs();
	CreateLockAndCvs();
	while(1)
	{
		if(GetMV("mgrMovieTechExit", 16, 0) == 1)
		{
			Exit(0);
		}
		Acquire("movieTechMgrLock", 16);	/*movie tech acquires manager lock to see if he is asked to start a movie*/
		if(GetMV("commandToStart", 14, 0) == 0)	/*commandtostart must be 1 to start movie*/
		{
			WriteToConsole("\nMovieTechnician waits for signal from Manager to start movie\n\0", -1, -1, -1);
			Wait("movieTechMgrCV", 14, "movieTechMgrLock", 16);		/*wait here for manager*/
			if(GetMV("mgrMovieTechExit", 16, 0) == 1)
			{
				Exit(0);
			}
		}
		/*Release("movieTechMgrLock", 16);*/
		WriteToConsole("\nThe MovieTechnician has started the movie.\n\0", -1, -1, -1);

		simulationTime = Random((300 - 200 + 1) + 200);
			randomYield(simulationTime);
		WriteToConsole("\nThe MovieTechnician has ended the movie.\n\0", -1, -1, -1);

		/*SetMV("commandToStart", 14, 0, 0);
		Release("movieTechMgrLock", 16);*/

		Acquire("movieTechLock", 13);		/*acquire lock to inform all customers waiting on this lock to inform them that movie is over*/

		SetMV("movieFinishState", 16, 0, OVER);
		SetMV("commandToStart", 14, 0, 0);

		WriteToConsole("\nThe MovieTechnician has told all customers to leave the theater room.\n\0", -1, -1, -1);
		Broadcast("movieTechCV", 11, "movieTechLock", 13);		/*inform all customers*/
		/*Acquire("movieTechMgrLock", 16);*/
		Release("movieTechLock", 13);		/*release lock after broadcasting*/
		/*resetting command to start*/

		Release("movieTechMgrLock", 16);	/*release manager lock*/
	}
}
