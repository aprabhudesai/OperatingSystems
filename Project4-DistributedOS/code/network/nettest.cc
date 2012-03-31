#include "copyright.h"

#include "system.h"
#include "network.h"
#include "post.h"
#include "interrupt.h"
#include "list.h"
#include "time.h"
#include "string.h"
#include <map>
#include <string>


#define MAX_SERVER_LOCKS 150
#define MAX_SERVER_CV 150
#define MAX_SERVER_MV 150

#define DYH_DEST_MAILBOX 1
int myID;
Timer *pingTimer;
time_t prevtime;

//For Mailbox 0
int farAddr = 0;
PacketHeader outPktHdr, inPktHdr;		
MailHeader outMailHdr, inMailHdr;
char *data;
char ack[45];					// Response message of the server
char buffer[MaxMailSize];
bool success;
int *toAddr;
char *message;
int *toMailBox;

// For mailbox 1
int farAddr1 = 0;
PacketHeader outPktHdr1, inPktHdr1;		
MailHeader outMailHdr1, inMailHdr1;
char *data1;
char ack1[45];					// Response message of the server
char buffer1[MaxMailSize];
bool success1;
int *toAddr1;
char *message1;
int *toMailBox1;


Lock *pingLock = new Lock("pingLock");
Condition *pingcv = new Condition("pingcv");
int pingflag = 0;
PacketHeader outPktHdr3, inPktHdr3;		
MailHeader outMailHdr3, inMailHdr3;

PacketHeader outPktHdr5, inPktHdr5;		
MailHeader outMailHdr5, inMailHdr5;

struct myInfo
{
	char *name;
	char type;
	int machineId[200];
	int mailbox[200];
	int mcIndex;
	int replyCount;
	char message[200][50];
	unsigned int timestamp;
	int reply;
	int recvmachineId[200];
	int recvmailbox[200];
	char recvtimestamp[200][20];
	int recvIndex;
	int posMachineId;
	int posMailbox;
	int resourceID;
	char waitLockState[1];
	int mcidInt;
	int mailbxInt;
};

myInfo *myInfoElement;

typedef std::map<string,myInfo*> MAPTYPE;

MAPTYPE pendReqMap;
MAPTYPE pendDHYMap; //not used
MAPTYPE::iterator it;
MAPTYPE::iterator itDYH; //not used

Lock *serverLockTableLock;
Lock *serverCVTableLock;
Lock *serverMVTableLock;
Lock *pendReqMapLock;
/*
*	Lock Structure and Table
*/
int serverLockIDCounter = 1;
struct Serverlock{
	char* lockName;
	bool isToBeDeleted;
	List* messageQueue;
	List* destMachineIDQueue;
	List* destMailBoxQueue;
	LockStatus lockStatus;
	int lockOwner;
	int lockOwnerMailBox;
	int useCount;
};
Serverlock serverLockTable[MAX_SERVER_LOCKS];

/*
*	Condition Variables Structure and Table
*/
int serverCVIDCounter = 1;
struct ServerCV{
	char *cvName;
	bool isToBeDeleted;
	List *messageWaitQueue;
	List *destMachineIDQueue;
	List* destMailBoxQueue;
	int waitLock;
	int waitQueueCount;
	int useCount;
	int cvOwnerMailBox;
};
ServerCV serverCVTable[MAX_SERVER_CV];

/*
*	Monitor Variables Structure and Table
*/
int serverMVIDCounter = 1;
struct ServerMV{
	char* mvName;
	int* mv;
	int size;
	int useCount;
};
ServerMV serverMVTable[MAX_SERVER_MV];

/*
*	Function to create TimeStamp
*	return type: char*
*	
*/

time_t now;
char the_date[20];
struct timeval tv;
struct timezone tz;
struct tm *tm1;
int timeStamp;

void getTimeStamp(){
	gettimeofday(&tv,&tz);
	tm1 = localtime(&tv.tv_sec);
	unsigned int myTimestamp = ((unsigned int)(tv.tv_usec + tv.tv_sec*1000000));
	sprintf(the_date,"%u",myTimestamp);
	return;
}


void setInfo(char *resourceName,char requestType,int requestorMachineId, int requestorMailBox,char *requestMessage,bool typ){	

	if(typ){
		myInfoElement->name = new char[strlen(resourceName)];
		strcpy(myInfoElement->name,resourceName);
		myInfoElement->type = requestType;
		//myInfoElement->message = new char[strlen(requestMessage)];
		strcpy(myInfoElement->message[0],requestMessage);
		myInfoElement -> reply = 0;
		myInfoElement -> recvIndex = 0;
		myInfoElement -> posMachineId = 0;
		myInfoElement -> posMailbox = 0;
		myInfoElement -> resourceID = 0;
		myInfoElement -> replyCount = noOfServers - 1;
	}
	myInfoElement->machineId[myInfoElement->mcIndex] = requestorMachineId;
	myInfoElement->mailbox[myInfoElement->mcIndex] = requestorMailBox;
	strcpy(myInfoElement->message[myInfoElement->mcIndex],requestMessage);
	myInfoElement->mcIndex++;
	return;
}

/*
*	Function: sendMessage - sends the Message to the Client
*/
void sendMessage(int to,int mailBox, char* toSendMessage){
			outPktHdr.to = to;
			outPktHdr.from = myID;
			outMailHdr.to = mailBox;
			outMailHdr.length = strlen(toSendMessage) + 1;
			success = postOffice->Send(outPktHdr, outMailHdr, toSendMessage);
			printf("Machind %d MailBox 0 sends message %s to Client Machine %d MailBox %d\n",myID,toSendMessage,to,mailBox);
			if ( !success ) {
			  printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
			  interrupt->Halt();
			}
}

/*
*	Function to send DYH Message to all servers
*	return boolean
*/
bool sendDYHMessage(char* DYHMessage){
	
	outPktHdr.from = myID;
	outMailHdr.to = DYH_DEST_MAILBOX;
	outMailHdr.length = strlen(DYHMessage) + 1;
	
	for(int i=0;i < noOfServers;i++){
		if(i != myID){
			outPktHdr.to = i;
			printf("Server Machine %d MailBox %d Sending DYH Message %s to machine id = %d\n",myID,currentThread -> myMailBox,DYHMessage,i);
			success = postOffice->Send(outPktHdr, outMailHdr, DYHMessage);
			if(!success){
				printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
				interrupt->Halt();
			}
		}
	}
	return true;
}

int isLockWithMe(int machineId,int mailBox,char *LockName,char reqType){
	serverLockTableLock -> Acquire();

	for(int i=1; i < serverLockIDCounter; i++){
		if(serverLockTable[i].lockName != NULL){
			if(strcmp(serverLockTable[i].lockName,LockName) == 0){
				if(reqType == 'a'){
					printf("Lock %s already Created, return LockID to machine %d and mailBox %d\n",LockName,machineId,mailBox);
					sprintf(ack,"%d",i);
					serverLockTable[i].useCount++;
				}
				else if(reqType == 'f' || reqType == 'g'){
					return i;				// Wait system call - so not releasing ServerLockTableLock
				}
				else if(reqType == 'b'){
					if(serverLockTable[i].lockOwner == machineId && serverLockTable[i].lockOwnerMailBox == mailBox){
						printf("Lock %s already owned by Machine ID %d and mailBox %d\n",LockName,machineId,mailBox);
						strcpy(ack,"1\0");
						serverLockTableLock -> Release();
						return 1;
					}
					if(serverLockTable[i].lockStatus == FREE){
						printf("Free Lock %s Found for Machine ID %d and mailBox %d\n",LockName,machineId,mailBox);
						serverLockTable[i].lockStatus = BUSY;
						serverLockTable[i].lockOwner = machineId;
						serverLockTable[i].lockOwnerMailBox = mailBox;
					}
					else{
						message = new char[50];
						strcpy(message,"1\0");
						toAddr = new int;
						toMailBox = new int;
						*toAddr = machineId;
						*toMailBox = mailBox;
						serverLockTable[i].messageQueue -> Append((void *)message);			// Add the message to the wait queue if the lock is not available
						serverLockTable[i].destMachineIDQueue -> Append((void *)toAddr);
						serverLockTable[i].destMailBoxQueue -> Append((void *)toMailBox);
						printf("Lock %s Busy so put Machine ID %d + mailBox %d to wait queue\n",LockName,machineId,mailBox);
						serverLockTableLock -> Release();
						return 2;
					}
					strcpy(ack,"1\0");
					serverLockTableLock -> Release();
					return 1;
				}
				else if(reqType == 'c'){
					if(serverLockTable[i].lockOwner != machineId && serverLockTable[i].lockOwnerMailBox != mailBox){
						printf("Lock %s not owned by Machine ID %d + mailBox %d\n",LockName,machineId,mailBox);
						strcpy(ack,"-1NotLockOwner\0");
						serverLockTableLock -> Release();
						return 1;
					}
					if(!(serverLockTable[i].messageQueue -> IsEmpty())){
						int *machIdPtr;
						int *machMailBoxPtr;
						char *messPtr;
						machIdPtr = (int *)serverLockTable[i].destMachineIDQueue -> Remove();		// Remove the machine id from the wait queue
						machMailBoxPtr = (int *)serverLockTable[i].destMailBoxQueue -> Remove();
						messPtr = (char *)serverLockTable[i].messageQueue -> Remove();			// Remove the message from the message wait queue
						serverLockTable[i].lockOwner = *machIdPtr;
						serverLockTable[i].lockOwnerMailBox = *machMailBoxPtr;
						printf("Lock %s Released by Machine ID %d + mailBox %d and setting machine %d and mailbox %d as lock owner\n",LockName,machineId,mailBox,*machIdPtr,*machMailBoxPtr);
						sendMessage(*machIdPtr,*machMailBoxPtr,messPtr);
					}
					else{
						serverLockTable[i].lockStatus = FREE;
						serverLockTable[i].lockOwner = -1;
						serverLockTable[i].lockOwnerMailBox = -1;
						printf("Lock %s Released by Machine ID %d + mailBox %d and setting lock Free as Lock wait queue empty\n",LockName,machineId,mailBox);
					}
					strcpy(ack,"1\0");
					serverLockTableLock -> Release();
					return 1;
				}
				else if(reqType == 'd'){
					
					if(!(serverLockTable[i].messageQueue -> IsEmpty())){
						printf("\nToBeDeleted Set by Machine ID %d and mailBox %d for Lock %s\n",machineId,mailBox,LockName);
						serverLockTable[i].isToBeDeleted = true;
						strcpy(ack,"1\0");
						return 1;
					}
					serverLockTable[i].useCount--;

					if(serverLockTable[i].useCount == 0){
						serverLockTable[i].lockName = NULL;
						serverLockTable[i].lockOwner = -1;
						serverLockTable[i].lockOwnerMailBox = -1;
						delete serverLockTable[i].messageQueue;
						delete serverLockTable[i].destMachineIDQueue;
						delete serverLockTable[i].destMailBoxQueue;
						printf("Lock %s Deleted by Machine ID %d and mailBox %d\n",LockName,machineId,mailBox);
					}
					strcpy(ack,"1\0");
					serverLockTableLock -> Release();
					return 1;
				}
				serverLockTableLock -> Release();
				return i;
			}
		}
	}
	serverLockTableLock -> Release();
	strcpy(ack,"LockResourceNotFound\0");
	return 0;
}


int CreateLockResource(int machineId,int mailBox,char *LockName){

	serverLockTableLock -> Acquire();

	if(serverLockIDCounter >= MAX_SERVER_LOCKS){
		printf("Maximum Locks created at server\n");
		serverLockTableLock -> Release();
		return -1;
	}
	serverLockTable[serverLockIDCounter].lockName = new char[31];
	strcpy(serverLockTable[serverLockIDCounter].lockName,LockName);
	serverLockTable[serverLockIDCounter].isToBeDeleted = false;
	serverLockTable[serverLockIDCounter].lockStatus = FREE;
	serverLockTable[serverLockIDCounter].lockOwner = -1;
	serverLockTable[serverLockIDCounter].lockOwnerMailBox = -1;
	serverLockTable[serverLockIDCounter].useCount = 1;
	serverLockTable[serverLockIDCounter].messageQueue = new List;
	serverLockTable[serverLockIDCounter].destMachineIDQueue = new List;
	serverLockTable[serverLockIDCounter].destMailBoxQueue = new List;
	printf("Lock %s created for Machine ID %d and mailBox %d\n",LockName,machineId,mailBox);
	int lockid = serverLockIDCounter;
	serverLockIDCounter++;
	serverLockTableLock -> Release();
	return lockid;
}

/*
*	Function: Create Lock Syscall - Creates the requested lock after checking for valid arguments
*/
int CreateServerLock(int machineId,int mailBox,char *LockName,char* requestMessage){

	int length;
	length = strlen(LockName);

	if(length >= 48 || length <= 0){
		printf("Invalid Lock Name %s Length by Machine ID %d and mailBox %d\n",LockName,machineId,mailBox);
		strcpy(ack,"-1Invalid Lock Name\0");
		return 1;
	}
	char *temp;
	temp = LockName;

	while(*temp != '\0'){
		if(*temp =='\n' ||*temp == '\t' || !((*temp >= 97 && *temp <= 122) || (*temp >= 65 && *temp <=90) || (*temp >= 48 && *temp <= 57))){
			printf("Invalid Lock Name %s by Machine ID %d and mailBox %d\n",LockName,machineId,mailBox);
			strcpy(ack,"-1Invalid Lock Name\0");
			return 1;
		}
		temp++;
	}
	if(noOfServers > 1){
			pendReqMapLock -> Acquire();
			int retVal = isLockWithMe(machineId,mailBox,LockName,'a');
			if(retVal== 0){

				//Sad, we don't have the lock, need to ask other servers
				char char_key[45] = "a\0";
				strcat(char_key,LockName);
				string key = string(char_key);

				//Check if we have the DYH request for the lock and Create Request

					it = pendReqMap.find(MAPTYPE::key_type(key));

					if(it == pendReqMap.end()){
						// Not Found in Map, so create new entry and add it to the Pending Queue Map

						myInfoElement = (myInfo*)malloc(sizeof(myInfo));
						myInfoElement -> mcIndex = 0;
						// Add all details to the structure of resource
						setInfo(LockName,'a',machineId,mailBox,requestMessage,true);
		
						char DYH_Message[100];
						strcpy(DYH_Message,"#a\0");
						strcat(DYH_Message,LockName);
						strcat(DYH_Message,",");
						getTimeStamp();
						strcat(DYH_Message,the_date);
						timeStamp = atoi(the_date);
						myInfoElement -> timestamp = timeStamp;
						pendReqMap.insert(MAPTYPE::value_type(key,myInfoElement));
						if(!sendDYHMessage(DYH_Message)){
							printf("DYH Send: Error\n - Halt");
							pendReqMapLock -> Release();
							Exit(0);
						}
					}
					else{ // add to the existing entry in the Pending Queue Structure received in Find
						myInfoElement = it->second;
						setInfo(LockName,'a',machineId,mailBox,requestMessage,false);
					}
				pendReqMapLock -> Release();
				return 0;
			}
			else{
				pendReqMapLock -> Release();
				return 1;
			}
	}
	else{
		int retVal = isLockWithMe(machineId,mailBox,LockName,'a');
		if(retVal == 0)
			retVal = CreateLockResource(machineId,mailBox,LockName);
		if(retVal == -1){
			strcpy(ack,"-1Lock Full\0");
		}
		sprintf(ack,"%d",retVal);
		return 1;
	}
}

/*********************************************************************************************************/
/*
*	Lock Syscalls Section
*/
/*********************************************************************************************************/

/*
*	Function: Acquire Lock Syscall - Acquires the lock after checking for valid arguments
*/
int AcquireLock(int machineId,int mailBox,char *LockName,char* requestMessage){
	int length;
	length = strlen(LockName);

	if(length >= 48 || length <= 0){
		printf("Invalid Lock Name %s Length by Machine ID %d and mailBox %d\n",LockName,machineId,mailBox);
		strcpy(ack,"-1InvalidLockName\0");
		return 1;
	}
	char *temp;
	temp = LockName;

	while(*temp != '\0'){
		if(*temp =='\n' ||*temp == '\t' || !((*temp >= 97 && *temp <= 122) || (*temp >= 65 && *temp <=90) || (*temp >= 48 && *temp <= 57))){
			printf("Invalid Lock Name %s by Machine ID %d and mailBox %d\n",LockName,machineId,mailBox);
			strcpy(ack,"-1InvalidLockName\0");
			return 1;
		}
		temp++;
	}
	if(noOfServers > 1){
			pendReqMapLock -> Acquire();
			int retVal = isLockWithMe(machineId,mailBox,LockName,'b');
			if(retVal == 0){

				//Sad, we don't have the lock, need to ask other servers
				char char_key[45] = "b\0";
				strcat(char_key,LockName);
				string key = string(char_key);

				//Check if we have the DYH request for the lock and Create Request

					it = pendReqMap.find(MAPTYPE::key_type(key));

					if(it == pendReqMap.end()){
						// Not Found in Map, so create new entry and add it to the Pending Queue Map

						myInfoElement = (myInfo*)malloc(sizeof(myInfo));
						myInfoElement -> mcIndex = 0;
						// Add all details to the structure of resource
						setInfo(LockName,'b',machineId,mailBox,requestMessage,true);
		
						char DYH_Message[100];
						strcpy(DYH_Message,"#b\0");
						strcat(DYH_Message,LockName);

						pendReqMap.insert(MAPTYPE::value_type(key,myInfoElement));
						if(!sendDYHMessage(DYH_Message)){
							printf("DYH Send: Error\n - Halt");
							pendReqMapLock -> Release();
							Exit(0);
						}
					}
					else{ // add to the existing entry in the Pending Queue Structure received in Find
						myInfoElement = it->second;
						setInfo(LockName,'b',machineId,mailBox,requestMessage,false);
					}
				pendReqMapLock -> Release();
				return 0;
			}
			else if(retVal == 1){
				pendReqMapLock -> Release();
				return 1;  // return 1 as lock was free
			}
			else if(retVal == 2){
				pendReqMapLock -> Release();
				return 0; // return 0 as put to wait Queue
			}
			return 0;
	}
	else{
		int retVal = isLockWithMe(machineId,mailBox,LockName,'b');
		if(retVal==2)
			return 0;
		else return 1;
	}
}

/*
*	Function: Release Lock Syscall - Releases the lock after checking for valid arguments
*/
int ReleaseLock(int machineId, int mailBox,char* LockName,char* requestMessage){

	int length;
	length = strlen(LockName);

	if(length >= 48 || length <= 0){
		printf("Invalid Lock Name %s Length by Machine ID %d and mailBox %d\n",LockName,machineId,mailBox);
		strcpy(ack,"-1InvalidLockName\0");
		return 1;
	}
	char *temp;
	temp = LockName;

	while(*temp != '\0'){
		if(*temp =='\n' ||*temp == '\t' || !((*temp >= 97 && *temp <= 122) || (*temp >= 65 && *temp <=90) || (*temp >= 48 && *temp <= 57))){
			printf("Invalid Lock Name %s by Machine ID %d and mailBox %d\n",LockName,machineId,mailBox);
			strcpy(ack,"-1InvalidLockName\0");
			return 1;
		}
		temp++;
	}
	if(noOfServers > 1){
			pendReqMapLock -> Acquire();
			int retVal = isLockWithMe(machineId,mailBox,LockName,'c');
			if(retVal == 0){
				cout << "\nI dnt have the lock so forward message\n";
				//Sad, we don't have the lock, need to ask other servers
				char char_key[45] = "c\0";
				strcat(char_key,LockName);
				string key = string(char_key);
				cout<<"\n Release - Key = "<<key<<"\n";
				//Check if we have the DYH request for the lock and Create Request
				
				cout<<"\nAcquired pend req lock\n";
					it = pendReqMap.find(MAPTYPE::key_type(key));

					if(it == pendReqMap.end()){
						// Not Found in Map, so create new entry and add it to the Pending Queue Map

						myInfoElement = (myInfo*)malloc(sizeof(myInfo));
						myInfoElement -> mcIndex = 0;
						// Add all details to the structure of resource
						setInfo(LockName,'c',machineId,mailBox,requestMessage,true);

						char DYH_Message[100];
						strcpy(DYH_Message,"#c\0");
						strcat(DYH_Message,LockName);

						pendReqMap.insert(MAPTYPE::value_type(key,myInfoElement));
						cout << "\nI dnt have the lock so forward message - 2\n";
						if(!sendDYHMessage(DYH_Message)){
							printf("DYH Send: Error\n - Halt");
							pendReqMapLock -> Release();
							Exit(0);
						}
					}
					else{ // add to the existing entry in the Pending Queue Structure received in Find
						myInfoElement = it->second;
						setInfo(LockName,'c',machineId,mailBox,requestMessage,false);
					}
				pendReqMapLock -> Release();
				return 0;
			}
			else{
				pendReqMapLock -> Release();
				return 1;
			}
	}
	else{
		int retVal = isLockWithMe(machineId,mailBox,LockName,'c');
		return 1;
	}
}

/*
*  Function: Delete Lock Syscall - Deletes the requested lock after checking for valid arguments
*/
int DeleteServerLock(int machineId,int mailBox,char* LockName,char* requestMessage)
{
	int length;
	length = strlen(LockName);

	if(length >= 48 || length <= 0){
		printf("Invalid Lock Name %s Length by Machine ID %d and mailBox %d\n",LockName,machineId,mailBox);
		strcpy(ack,"-1InvalidLockName\0");
		return 1;
	}
	char *temp;
	temp = LockName;

	while(*temp != '\0'){
		if(*temp =='\n' ||*temp == '\t' || !((*temp >= 97 && *temp <= 122) || (*temp >= 65 && *temp <=90) || (*temp >= 48 && *temp <= 57))){
			printf("Invalid Lock Name %s by Machine ID %d and mailBox %d\n",LockName,machineId,mailBox);
			strcpy(ack,"-1InvalidLockName\0");
			return 1;
		}
		temp++;
	}
	if(noOfServers > 1){
			pendReqMapLock -> Acquire();
			int retVal = isLockWithMe(machineId,mailBox,LockName,'d');
			if(retVal == 0){
				cout << "\nI dnt have the lock so forward message\n";
				//Sad, we don't have the lock, need to ask other servers
				char char_key[45] = "d\0";
				strcat(char_key,LockName);
				string key = string(char_key);
				cout<<"\n Release - Key = "<<key<<"\n";
				//Check if we have the DYH request for the lock and Create Request
				
				cout<<"\nAcquired pend req lock\n";
					it = pendReqMap.find(MAPTYPE::key_type(key));

					if(it == pendReqMap.end()){
						// Not Found in Map, so create new entry and add it to the Pending Queue Map

						myInfoElement = (myInfo*)malloc(sizeof(myInfo));
						myInfoElement -> mcIndex = 0;
						// Add all details to the structure of resource
						setInfo(LockName,'d',machineId,mailBox,requestMessage,true);

						char DYH_Message[100];
						strcpy(DYH_Message,"#d\0");
						strcat(DYH_Message,LockName);

						pendReqMap.insert(MAPTYPE::value_type(key,myInfoElement));
						cout << "\nI dnt have the lock so forward message - 2\n";
						if(!sendDYHMessage(DYH_Message)){
							printf("DYH Send: Error\n - Halt");
							pendReqMapLock -> Release();
							Exit(0);
						}
					}
					else{ // add to the existing entry in the Pending Queue Structure received in Find
						myInfoElement = it->second;
						setInfo(LockName,'d',machineId,mailBox,requestMessage,false);
					}
				pendReqMapLock -> Release();
				return 0;
			}
			else{
				pendReqMapLock -> Release();
				return 1;
			}
	}
	else{
		int retVal = isLockWithMe(machineId,mailBox,LockName,'d');
		return 1;
	}
}

/*********************************************************************************************************/
/*
*	Condition Variable Section
*/
/*********************************************************************************************************/

int isCVWithMe(int machineId,int mailBox,char *CVName,char reqType){
	serverCVTableLock -> Acquire();

	for(int i=1; i < serverCVIDCounter; i++){
		if(serverCVTable[i].cvName != NULL){
			if(strcmp(serverCVTable[i].cvName,CVName) == 0){
				if(reqType == 'e'){
					printf("CV %s already Created, return MVID to machine %d and mailBox %d\n",CVName,machineId,mailBox);
					sprintf(ack,"%d",i);
					serverCVTable[i].useCount++;
				}
				else if(reqType == 'f' || reqType == 'g'){
					return i;					// NOTE: IMP 'serverCVTableLock' not released in case of 'wait' req Type
											// to make it atomic activity, so if CV Found serverCVTableLock is not released
				}
				else if(reqType == 'i'){
					if(!(serverCVTable[i].messageWaitQueue -> IsEmpty())){
						printf("Machine %d and mailBox %d set CV %s toBeDeleted to true\n",machineId,mailBox,CVName);
						serverCVTable[i].isToBeDeleted = true;
						strcpy(ack,"1\0");
						return 1;
					}
					serverCVTable[i].useCount--;

					if(serverCVTable[i].useCount == 0){
						serverCVTable[i].cvName = NULL;
						serverCVTable[i].waitLock = -1;
						serverCVTable[i].waitQueueCount = -1;
						delete serverCVTable[i].messageWaitQueue;
						delete serverCVTable[i].destMailBoxQueue;
						delete serverCVTable[i].destMachineIDQueue;
						printf("Machine %d and mailBox %d deleted CV %s\n",machineId,mailBox,CVName);
					}
					strcpy(ack,"1\0");
					serverCVTableLock -> Release();
					return 1;
				}
				serverCVTableLock -> Release();
				return i;
			}
		}
	}
	strcpy(ack,"ResourceNotFound\0");
	serverCVTableLock -> Release();
	return 0;
}


int CreateCVResource(int machineId,int mailBox,char *CVName){

	serverCVTableLock -> Acquire();
	if(serverCVIDCounter >= MAX_SERVER_CV){
		printf("Max CV Created at Server\n");
		//strcpy(ack,"-1CV Full\0");
		serverCVTableLock -> Release();
		return -1;
	}
	serverCVTable[serverCVIDCounter].cvName = new char[31];
	strcpy(serverCVTable[serverCVIDCounter].cvName,CVName);
	serverCVTable[serverCVIDCounter].isToBeDeleted = false;
	serverCVTable[serverCVIDCounter].messageWaitQueue = new List;
	serverCVTable[serverCVIDCounter].destMachineIDQueue = new List;
	serverCVTable[serverCVIDCounter].destMailBoxQueue = new List;
	serverCVTable[serverCVIDCounter].waitLock = 0;
	serverCVTable[serverCVIDCounter].useCount = 1;
	serverCVTable[serverCVIDCounter].waitQueueCount = 0;
	printf("CV Created at server, return CVID: %d to machineID: %d and mailBox %d\n",serverCVIDCounter,machineId,mailBox);
	int cvid = serverCVIDCounter;
	serverCVIDCounter++;
	serverCVTableLock -> Release();
	return cvid;
}

/*
*	Function: Create CV Syscall - creates a condition variable after checking for valid arguments
*/
int CreateServerCV(int machineId,int mailBox,char *CVName,char* requestMessage)
{
	int length;
	length = strlen(CVName);
	if(length >= 48 || length <= 0){
		printf("Invalid CV Name issued by machineID %d and mailBox %d\n",machineId,mailBox);
		strcpy(ack,"-1Invalid CV Name\0");
		return 1;
	}
	char *temp;
	temp = CVName;
	while(*temp != '\0'){
		if(*temp =='\n' || *temp == '\t' || !((*temp >= 97 && *temp <= 122) || (*temp >= 65 && *temp <=90) || (*temp >= 48 && *temp <= 57))){
			printf("Invalid CV Name issued by machineID %d and mailBox %d\n",machineId,mailBox);
			strcpy(ack,"-1InvalidCVName\0");
			return 1;
		}
		temp++;
	}
	if(noOfServers > 1){
			pendReqMapLock -> Acquire();
			if(isCVWithMe(machineId,mailBox,CVName,'e') == 0){

				//Sad, we don't have the lock, need to ask other servers
				char char_key[45] = "e\0";
				strcat(char_key,CVName);
				string key = string(char_key);

				//Check if we have the DYH request for the lock and Create Request

					it = pendReqMap.find(MAPTYPE::key_type(key));

					if(it == pendReqMap.end()){
						// Not Found in Map, so create new entry and add it to the Pending Queue Map

						myInfoElement = (myInfo*)malloc(sizeof(myInfo));
						myInfoElement -> mcIndex = 0;
						// Add all details to the structure of resource
						setInfo(CVName,'e',machineId,mailBox,requestMessage,true);
		
						char DYH_Message[100];
						strcpy(DYH_Message,"#e\0");
						strcat(DYH_Message,CVName);
						strcat(DYH_Message,",");
						getTimeStamp();
						strcat(DYH_Message,the_date);
						timeStamp = atoi(the_date);
						myInfoElement -> timestamp = timeStamp;
						pendReqMap.insert(MAPTYPE::value_type(key,myInfoElement));
						if(!sendDYHMessage(DYH_Message)){
							printf("DYH Send: Error\n - Halt");
							pendReqMapLock -> Release();
							Exit(0);
						}
					}
					else{ // add to the existing entry in the Pending Queue Structure received in Find
						myInfoElement = it->second;
						setInfo(CVName,'e',machineId,mailBox,requestMessage,false);
					}
				pendReqMapLock -> Release();
				return 0;
			}
			else{
				pendReqMapLock -> Release();
				return 1;
			}
	}
	else{
		int retVal = isCVWithMe(machineId,mailBox,CVName,'e');
		if(retVal == 0)
			retVal = CreateCVResource(machineId,mailBox,CVName);
		if(retVal == -1){
			strcpy(ack,"-1CV Full\0");
			return 1;
		}
		sprintf(ack,"%d",retVal);
		return 1;
	}
}

/*
*	Function:  isHeldByCurrentThread - checks if the machine id requesting the lock for a CV is the owner or not
*/
bool isHeldByCurrentThread(int machineId, int mailBox,int lockid)
{
	if(serverLockTable[lockid].lockOwner == machineId && serverLockTable[lockid].lockOwnerMailBox == mailBox)
		return true;
	else 
		return false;
}

/*
*	Function:  WaitServerCV - Calls a wait on the condition variable after checking for valid arguments
*/
int WaitServerCV(int machineId,int mailBox, char *msg, char *requestMessage)
{
	char LockName[40];
	char CVName[40];
	int lockid = -1,cvid = -1;
	int index = 0;
	while(*msg != ',')
	{
		*(CVName + index) = *msg;				// Get the cvid from the arguments passed
		msg++;
		index++;
	}
	*(CVName + index) = '\0';
	msg++;
	index = 0;
	while(*msg != '\0')
	{
		*(LockName + index) = *msg;				// Get the lock id from the parameters passed
		msg++;
		index++;
	}
	*(LockName + index) = '\0';
	//lockid = atoi(lock);
	if(noOfServers > 1){
			pendReqMapLock -> Acquire();
			cvid = isCVWithMe(machineId,mailBox,CVName,'f');
			if( cvid == 0){
				//Sad, we don't have the CV, need to ask other servers
				char char_key[45] = "f\0";
				strcat(char_key,CVName);
				string key = string(char_key);

				//Check if we have the DYH request for the lock and Create Request

					it = pendReqMap.find(MAPTYPE::key_type(key));

					if(it == pendReqMap.end()){
						// Not Found in Map, so create new entry and add it to the Pending Queue Map

						myInfoElement = (myInfo*)malloc(sizeof(myInfo));
						myInfoElement -> mcIndex = 0;
						// Add all details to the structure of resource
						setInfo(CVName,'f',machineId,mailBox,requestMessage,true);
		
						char DYH_Message[100];
						strcpy(DYH_Message,"#f\0");
						strcat(DYH_Message,CVName);

						pendReqMap.insert(MAPTYPE::value_type(key,myInfoElement));
						if(!sendDYHMessage(DYH_Message)){
							printf("DYH Send: Error\n - Halt");
							pendReqMapLock -> Release();
							Exit(0);
						}
					}
					else{ // add to the existing entry in the Pending Queue Structure received in Find
						myInfoElement = it->second;
						setInfo(CVName,'f',machineId,mailBox,requestMessage,false);
					}
				pendReqMapLock -> Release();
				return 0;
			}
			else{ // CV Found, now need to check Lock
				lockid = isLockWithMe(machineId,mailBox,LockName,'f');
				if( lockid == 0){
					serverCVTableLock -> Release();  // Release the serverCVTableLock which was not released in isCVWithMe Function
		
					//Sad, we don't have the lock, need to ask other servers
					char char_key[45] = "x\0";
					strcat(char_key,LockName);
					string key = string(char_key);
				
					//Check if we have the DYH request for the lock and Create Request
					//pendReqMapLock -> Acquire();
						it = pendReqMap.find(MAPTYPE::key_type(key));
						char temp_resName[40];
						sprintf(temp_resName,"%d",cvid);
						if(it == pendReqMap.end()){
							// Not Found in Map, so create new entry and add it to the Pending Queue Map

							myInfoElement = (myInfo*)malloc(sizeof(myInfo));
							myInfoElement -> mcIndex = 0;
							// Add all details to the structure of resource
							//Stored the CV ID as a string for reference to CV
							setInfo(temp_resName,'x',machineId,mailBox,requestMessage,true);

							char DYH_Message[100];
							strcpy(DYH_Message,"#x\0");
							strcat(DYH_Message,LockName);
							strcat(DYH_Message,",");

							char temp_char[20];
							sprintf(temp_char,"%d,%d",machineId,mailBox);
							strcat(DYH_Message,temp_char);
							pendReqMap.insert(MAPTYPE::value_type(key,myInfoElement));
							if(!sendDYHMessage(DYH_Message)){
								printf("DYH Send: Error\n - Halt");
								pendReqMapLock -> Release();
								Exit(0);
							}
						}
						else{ // add to the existing entry in the Pending Queue Structure received in Find
							myInfoElement = it->second;
							setInfo(temp_resName,'x',machineId,mailBox,requestMessage,false);
						}
					pendReqMapLock -> Release();
					return 0;
				}
				else{ // Lock Found and CV also Found
					//Check if current thread has acquired the lock
					if(!(isHeldByCurrentThread(machineId,mailBox,lockid)))
					{
						printf("machindId %d and mailBox %d does not own the lock %s\n", machineId,mailBox,LockName);
						strcpy(ack,"-1NotLockOwner\0");
						serverCVTableLock -> Release();
						serverLockTableLock -> Release();
						pendReqMapLock -> Release();
						return 1;
					}
					serverLockTableLock -> Release();
					//First Thread with conditionLock calling wait
					if(serverCVTable[cvid].waitLock == 0)
						serverCVTable[cvid].waitLock = lockid;
		
					//waitLock and conditionLock do not match
					if(serverCVTable[cvid].waitLock != lockid){
						printf("MachineId %d and mailBox %d cannot wait for condition as condition Lock is not same as waitLock\n",machineId,mailBox);
						strcpy(ack,"-1InvalidLock\0");
						serverCVTableLock -> Release();
						pendReqMapLock -> Release();
						return 1;
					}
					//exiting the monitor before going to sleep
					ReleaseLock(machineId,mailBox,LockName,"NULL");

					// add the current thread to condition variable's wait queue
					message = new char[50];
					strcpy(message,"1\0");
					toAddr = new int;
					*toAddr = machineId;
					toMailBox = new int;
					*toMailBox = mailBox;
					//put the thread to sleep

					serverCVTable[cvid].messageWaitQueue -> Append((void *)message);				// Add the message to the wait queue
					serverCVTable[cvid].destMachineIDQueue -> Append((void *)toAddr);				// Add the machine id to the wait queue
					serverCVTable[cvid].destMailBoxQueue -> Append((void*)toMailBox);
					printf("Machine Id %d and mailBox %d put to wait queue of CV %s and Lock %s\n", machineId,mailBox,CVName,LockName);
					serverCVTableLock -> Release();
					pendReqMapLock -> Release();
					return 0;
				}
			}
	}
	else{
			cvid = isCVWithMe(machineId,mailBox,CVName,'f');
			if(cvid == 0)
				return 1;
			else serverCVTableLock -> Release();
			lockid = isLockWithMe(machineId,mailBox,LockName,'f');
			if(lockid == 0)
				return 1;
			else serverLockTableLock -> Release();

			if(!(isHeldByCurrentThread(machineId,mailBox,lockid))){
				printf("MachindId %d and mailBox %d does not own the lock %s\n", machineId,mailBox,LockName);
				strcpy(ack,"-1NotLockOwner\0");
				return 1;
			}

			//First Thread with conditionLock calling wait
			if(serverCVTable[cvid].waitLock == 0)
				serverCVTable[cvid].waitLock = lockid;

			//waitLock and conditionLock do not match
			if(serverCVTable[cvid].waitLock != lockid){
				printf("MachineId %d and mailBox %d cannot wait for condition as condition Lock is not same as waitLock \n",machineId,mailBox);
				strcpy(ack,"-1InvalidLock\0");
				return 1;
			}
			//exiting the monitor before going to sleep
			ReleaseLock(machineId,mailBox,LockName,"NULL");
			// add the current thread to condition variable's wait queue
			message = new char[50];
			strcpy(message,"1\0");
			toAddr = new int;
			*toAddr = machineId;
			toMailBox = new int;
			*toMailBox = mailBox;
			//put the thread to sleep
			serverCVTable[cvid].messageWaitQueue -> Append((void *)message);				// Add the message to the wait queue
			serverCVTable[cvid].destMachineIDQueue -> Append((void *)toAddr);				// Add the machine id to the wait queue
			serverCVTable[cvid].destMailBoxQueue -> Append((void*)toMailBox);
			printf("Machine Id %d and mailBox %d put to wait queue of CV %s and Lock %s\n", machineId,mailBox,CVName,LockName);
			return 0;
	}
}


/*
*	Function:  SignalServerCV - Signals the waiting thread after checking for valid arguments
*/
int SignalServerCV(int machineId, int mailBox,char *msg, char* requestMessage)
{
	char LockName[40];
	char CVName[40];
	int lockid = -1,cvid = -1;
	int index = 0;
	while(*msg != ',')
	{
		*(CVName + index) = *msg;				// Get the cvid from the arguments passed
		msg++;
		index++;
	}
	*(CVName + index) = '\0';
	msg++;
	index = 0;
	while(*msg != '\0')
	{
		*(LockName + index) = *msg;				// Get the lock id from the parameters passed
		msg++;
		index++;
	}
	*(LockName + index) = '\0';

	if(noOfServers > 1){
			pendReqMapLock -> Acquire();
			cvid = isCVWithMe(machineId,mailBox,CVName,'g');
			if( cvid == 0){
				//Sad, we don't have the lock, need to ask other servers
				char char_key[45] = "g\0";
				strcat(char_key,CVName);
				string key = string(char_key);
				
				//Check if we have the DYH request for the lock and Create Request
		
					it = pendReqMap.find(MAPTYPE::key_type(key));

					if(it == pendReqMap.end()){
						// Not Found in Map, so create new entry and add it to the Pending Queue Map

						myInfoElement = (myInfo*)malloc(sizeof(myInfo));
						myInfoElement -> mcIndex = 0;
						// Add all details to the structure of resource
						setInfo(CVName,'g',machineId,mailBox,requestMessage,true);

						char DYH_Message[100];
						strcpy(DYH_Message,"#g\0");
						strcat(DYH_Message,CVName);

						pendReqMap.insert(MAPTYPE::value_type(key,myInfoElement));
						if(!sendDYHMessage(DYH_Message)){
							printf("DYH Send: Error\n - Halt");
							pendReqMapLock -> Release();
							Exit(0);
						}
					}
					else{ // add to the existing entry in the Pending Queue Structure received in Find
						myInfoElement = it->second;
						setInfo(CVName,'g',machineId,mailBox,requestMessage,false);
					}
				pendReqMapLock -> Release();
				return 0;
			}
			else{ // CV Found, now need to check Lock
				lockid = isLockWithMe(machineId,mailBox,LockName,'g');
				if( lockid == 0){
					serverCVTableLock -> Release();  // Release the serverCVTableLock which was not released in isCVWithMe Function

					//Sad, we don't have the lock, need to ask other servers
					char char_key[45] = "y\0";
					strcat(char_key,LockName);
					string key = string(char_key);
				
					//Check if we have the DYH request for the lock and Create Request
						it = pendReqMap.find(MAPTYPE::key_type(key));
						char temp_resName[40];
						sprintf(temp_resName,"%d",cvid);
						if(it == pendReqMap.end()){
							// Not Found in Map, so create new entry and add it to the Pending Queue Map

							myInfoElement = (myInfo*)malloc(sizeof(myInfo));
							myInfoElement -> mcIndex = 0;
							// Add all details to the structure of resource
							setInfo(temp_resName,'y',machineId,mailBox,requestMessage,true);

							char DYH_Message[100];
							strcpy(DYH_Message,"#y\0");
							strcat(DYH_Message,LockName);
							strcat(DYH_Message,",");

							char temp_char[20];
							sprintf(temp_char,"%d,%d",machineId,mailBox);
							strcat(DYH_Message,temp_char);
							pendReqMap.insert(MAPTYPE::value_type(key,myInfoElement));
							if(!sendDYHMessage(DYH_Message)){
								printf("DYH Send: Error\n - Halt");
								pendReqMapLock -> Release();
								Exit(0);
							}
						}
						else{ // add to the existing entry in the Pending Queue Structure received in Find
							myInfoElement = it->second;
							setInfo(temp_resName,'y',machineId,mailBox,requestMessage,false);
						}
					pendReqMapLock -> Release();
					return 0;
				}
				else{ // Lock Found and CV also Found
					//Check if current thread has acquired the lock
					if(!(isHeldByCurrentThread(machineId,mailBox,lockid)))
					{
						printf("machindId %d and mailBox %d does not own the lock %s\n", machineId,mailBox,LockName);
						strcpy(ack,"-1NotLockOwner\0");
						serverCVTableLock -> Release();
						serverLockTableLock -> Release();
						pendReqMapLock -> Release();
						return 1;
					}
					serverLockTableLock -> Release();

					if(serverCVTable[cvid].messageWaitQueue -> IsEmpty()){
						printf("Machine Id %d and mailBox %d signalled using Lock %s and CV %s, but no client waiting\n", machineId,mailBox,LockName,CVName);
						strcpy(ack,"-1 No Client Waiting");
						serverCVTableLock -> Release();
						pendReqMapLock -> Release();
						return 1;
					}
					//waitLock and conditionLock do not match
					if(serverCVTable[cvid].waitLock != lockid){
						printf("Machine Id: %d and mailBox %d cannot signal for condition as condition Lock %s passed is not same as WaitLock\n",machineId,mailBox,LockName);
						strcpy(ack,"-1InvalidCVLock\0");
						serverCVTableLock -> Release();
						pendReqMapLock -> Release();
						return 1;
					}

					serverCVTable[cvid].messageWaitQueue -> Remove();										// Remove a message from the wait queue

					if(serverCVTable[cvid].messageWaitQueue -> IsEmpty()){
						serverCVTable[cvid].waitLock = 0;
					}

					AcquireLock(*((int *)serverCVTable[cvid].destMachineIDQueue -> Remove()),*((int *)serverCVTable[cvid].destMailBoxQueue -> Remove()),LockName,NULL);				// Try to acquire the lock which is used for the signalling
					printf("Machine Id %d and mailBox %d signalled using CV: %s and Lock %s\n", machineId,mailBox,CVName,LockName);
					strcpy(ack,"1\0");
					serverCVTableLock -> Release();
					pendReqMapLock -> Release();
					return 1;
				}
			}
	}
	else{
		cvid = isCVWithMe(machineId,mailBox,CVName,'g');
		if(cvid == 0)
			return 1;
		else serverCVTableLock -> Release();
		lockid = isLockWithMe(machineId,mailBox,LockName,'g');
		if(lockid == 0)
			return 1;
		else serverLockTableLock -> Release();

		if(!(isHeldByCurrentThread(machineId,mailBox,lockid))){
			printf("machindId %d and mailBox %d does not own the lock %s\n", machineId,mailBox,LockName);
			strcpy(ack,"-1NotLockOwner\0");
			return 1;
		}
		if(serverCVTable[cvid].messageWaitQueue -> IsEmpty()){
			printf("Machine Id %d and mailBox %d signalled using Lock %s and CV %s, but no client waiting\n", machineId,mailBox,LockName,CVName);
			strcpy(ack,"-1 No Client Waiting");
			return 1;
		}
		//waitLock and conditionLock do not match
		if(serverCVTable[cvid].waitLock != lockid){
			printf("Machine Id: %d and mailBox %d cannot signal for condition as condition Lock %s passed is not same as WaitLock %s\n",machineId,mailBox,LockName,serverLockTable[lockid].lockName);
			strcpy(ack,"-1InvalidCVLock\0");
			return 1;
		}
		serverCVTable[cvid].messageWaitQueue -> Remove();										// Remove a message from the wait queue
		if(serverCVTable[cvid].messageWaitQueue -> IsEmpty()){
			serverCVTable[cvid].waitLock = 0;
		}

		AcquireLock(*((int *)serverCVTable[cvid].destMachineIDQueue -> Remove()),*((int *)serverCVTable[cvid].destMailBoxQueue -> Remove()),LockName,NULL);				// Try to acquire the lock which is used for the signalling
		printf("Machine Id %d and mailBox %d signalled using CV: %s and Lock %s\n", machineId,mailBox,CVName,LockName);
		strcpy(ack,"1\0");
		return 1;
	}
}

/*
*	Function:  BroadCastServerCV - Broadcasts a condition variable after checking for valid arguments
*/
int BroadcastServerCV(int machineId, int mailBox,char *msg, char* requestMessage)
{
	char LockName[40];
	char CVName[40];
	int lockid = -1,cvid = -1;
	int index = 0;
	while(*msg != ',')
	{
		*(CVName + index) = *msg;				// Get the cvid from the arguments passed
		msg++;
		index++;
	}
	*(CVName + index) = '\0';
	//cvid = atoi(cv);
	msg++;
	index = 0;
	while(*msg != '\0')
	{
		*(LockName + index) = *msg;				// Get the lock id from the parameters passed
		msg++;
		index++;
	}
	*(LockName + index) = '\0';
	//lockid = atoi(lock);
	if(noOfServers > 1){
			pendReqMapLock -> Acquire();
			cvid = isCVWithMe(machineId,mailBox,CVName,'g');
			if( cvid == 0){
				//Sad, we don't have the lock, need to ask other servers
				char char_key[45] = "h\0";
				strcat(char_key,CVName);
				string key = string(char_key);
				//Check if we have the DYH request for the lock and Create Request
					it = pendReqMap.find(MAPTYPE::key_type(key));
					if(it == pendReqMap.end()){
						// Not Found in Map, so create new entry and add it to the Pending Queue Map

						myInfoElement = (myInfo*)malloc(sizeof(myInfo));
						myInfoElement -> mcIndex = 0;
						// Add all details to the structure of resource
						setInfo(CVName,'h',machineId,mailBox,requestMessage,true);
		
						char DYH_Message[100];
						strcpy(DYH_Message,"#h\0");
						strcat(DYH_Message,CVName);

						pendReqMap.insert(MAPTYPE::value_type(key,myInfoElement));
						if(!sendDYHMessage(DYH_Message)){
							printf("DYH Send: Error\n - Halt");
							pendReqMapLock -> Release();
							Exit(0);
						}
					}
					else{ // add to the existing entry in the Pending Queue Structure received in Find
						myInfoElement = it->second;
						setInfo(CVName,'h',machineId,mailBox,requestMessage,false);
					}
				pendReqMapLock -> Release();
				return 0;
			}
			else{ // CV Found, now need to check Lock
				lockid = isLockWithMe(machineId,mailBox,LockName,'g');
				if( lockid == 0){
					serverCVTableLock -> Release();  // Release the serverCVTableLock which was not released in isCVWithMe Function
		
					//Sad, we don't have the lock, need to ask other servers
					char char_key[45] = "w\0";
					strcat(char_key,LockName);
					string key = string(char_key);
				
					//Check if we have the DYH request for the lock and Create Request
					//pendReqMapLock -> Acquire();
						it = pendReqMap.find(MAPTYPE::key_type(key));
						char temp_resName[40];
						sprintf(temp_resName,"%d",cvid);
						if(it == pendReqMap.end()){
							// Not Found in Map, so create new entry and add it to the Pending Queue Map

							myInfoElement = (myInfo*)malloc(sizeof(myInfo));
							myInfoElement -> mcIndex = 0;
							// Add all details to the structure of resource
							setInfo(temp_resName,'w',machineId,mailBox,requestMessage,true);

							char DYH_Message[100];
							strcpy(DYH_Message,"#w\0");
							strcat(DYH_Message,LockName);
							strcat(DYH_Message,",");

							char temp_char[20];
							sprintf(temp_char,"%d,%d",machineId,mailBox);
							strcat(DYH_Message,temp_char);
							pendReqMap.insert(MAPTYPE::value_type(key,myInfoElement));
							if(!sendDYHMessage(DYH_Message)){
								printf("DYH Send: Error\n - Halt");
								pendReqMapLock -> Release();
								Exit(0);
							}
						}
						else{ // add to the existing entry in the Pending Queue Structure received in Find
							myInfoElement = it->second;
							setInfo(temp_resName,'w',machineId,mailBox,requestMessage,false);
						}
					pendReqMapLock -> Release();
					return 0;
				}
				else{ // Lock Found and CV also Found
					//Check if current thread has acquired the lock
					if(!(isHeldByCurrentThread(machineId,mailBox,lockid)))
					{
						printf("machindId %d and mailBox %d does not own the lock %s\n", machineId,mailBox,LockName);
						strcpy(ack,"-1NotLockOwner\0");
						serverCVTableLock -> Release();
						serverLockTableLock -> Release();
						pendReqMapLock -> Release();
						return 1;
					}
					serverLockTableLock -> Release();

					//waitLock and conditionLock do not match
					if(serverCVTable[cvid].messageWaitQueue -> IsEmpty()){
						printf("Machine Id %d and mailBox %d signalled using Lock %s and CV %s, but no client waiting\n", machineId,mailBox,LockName,CVName);
						strcpy(ack,"-1 No Client Waiting");
						serverCVTableLock -> Release();
						pendReqMapLock -> Release();
						return 1;
					}

					if(serverCVTable[cvid].waitLock != lockid){
						printf("Machine Id: %d and mailBox %d cannot signal for condition as condition Lock %s passed is not same as WaitLock\n",machineId,mailBox,LockName);
						strcpy(ack,"-1InvalidCVLock\0");
						serverCVTableLock -> Release();
						pendReqMapLock -> Release();
						return 1;
					}
		
					while(!serverCVTable[cvid].messageWaitQueue -> IsEmpty()){
						serverCVTable[cvid].messageWaitQueue -> Remove();										// Remove a message from the wait queue
						AcquireLock(*((int *)serverCVTable[cvid].destMachineIDQueue -> Remove()),*((int *)serverCVTable[cvid].destMailBoxQueue -> Remove()),LockName,NULL);				// Try to acquire the lock which is used for the signalling
						printf("Machine Id %d and mailBox %d broadcasted using CV: %s and Lock %s\n", machineId,mailBox,CVName,LockName);
					}
					serverCVTable[cvid].waitLock = 0;

					strcpy(ack,"1\0");
					serverCVTableLock -> Release();
					pendReqMapLock -> Release();
					return 1;
				}
			}
	}
	else{
		cvid = isCVWithMe(machineId,mailBox,CVName,'g');
		if(cvid == 0)
			return 1;
		else serverCVTableLock -> Release();
		lockid = isLockWithMe(machineId,mailBox,LockName,'g');
		if(lockid == 0)
			return 1;
		else serverLockTableLock -> Release();
		if(!(isHeldByCurrentThread(machineId,mailBox,lockid))){
			printf("machindId %d and mailBox %d does not own the lock %s\n", machineId,mailBox,LockName);
			strcpy(ack,"-1NotLockOwner\0");
			return 1;
		}

		//waitLock and conditionLock do not match
		if(serverCVTable[cvid].messageWaitQueue -> IsEmpty()){
			printf("Machine Id %d and mailBox %d signalled using Lock %s and CV %s, but no client waiting\n", machineId,mailBox,LockName,CVName);
			strcpy(ack,"-1 No Client Waiting");
			return 1;
		}
		if(serverCVTable[cvid].waitLock != lockid){
			printf("Machine Id: %d and mailBox %d cannot signal for condition as condition Lock %s passed is not same as WaitLock %s\n",machineId,mailBox,LockName,serverLockTable[lockid].lockName);
			strcpy(ack,"-1InvalidCVLock\0");
			return 1;
		}
		while(!serverCVTable[cvid].messageWaitQueue -> IsEmpty()){
			serverCVTable[cvid].messageWaitQueue -> Remove();										// Remove a message from the wait queue
			AcquireLock(*((int *)serverCVTable[cvid].destMachineIDQueue -> Remove()),*((int *)serverCVTable[cvid].destMailBoxQueue -> Remove()),LockName,NULL);				// Try to acquire the lock which is used for the signalling
			printf("Machine Id %d and mailBox %d broadcasted using CV: %s and Lock %s\n", machineId,mailBox,CVName,LockName);
		}
		serverCVTable[cvid].waitLock = 0;
		strcpy(ack,"1\0");
		return 1;
	}
}


/*
*	Function:  DeleteServerCV - Deletes a condition variable after checking for valid arguments
*/
int DeleteServerCV(int machineId,int mailBox,char *CVName,char * requestMessage)
{
	int length;
	length = strlen(CVName);
	if(length >= 48 || length <= 0){
		printf("Invalid CV Name issued by machineID %d and mailBox %d\n",machineId,mailBox);
		strcpy(ack,"-1Invalid CV Name\0");
		return 1;
	}
	char *temp;
	temp = CVName;
	while(*temp != '\0'){
		if(*temp =='\n' || *temp == '\t' || !((*temp >= 97 && *temp <= 122) || (*temp >= 65 && *temp <=90) || (*temp >= 48 && *temp <= 57))){
			printf("Invalid CV Name issued by machineID %d and mailBox %d\n",machineId,mailBox);
			strcpy(ack,"-1InvalidCVName\0");
			return 1;
		}
		temp++;
	}
	if(noOfServers > 1){
		pendReqMapLock -> Acquire();
		if(isCVWithMe(machineId,mailBox,CVName,'i') == 0){

			//Sad, we don't have the lock, need to ask other servers
			char char_key[45] = "i\0";
			strcat(char_key,CVName);
			string key = string(char_key);

			//Check if we have the DYH request for the lock and Create Request
			
				it = pendReqMap.find(MAPTYPE::key_type(key));
				
				if(it == pendReqMap.end()){
					// Not Found in Map, so create new entry and add it to the Pending Queue Map

					myInfoElement = (myInfo*)malloc(sizeof(myInfo));
					myInfoElement -> mcIndex = 0;
					// Add all details to the structure of resource
					setInfo(CVName,'i',machineId,mailBox,requestMessage,true);

					char DYH_Message[100];
					strcpy(DYH_Message,"#i\0");
					strcat(DYH_Message,CVName);

					pendReqMap.insert(MAPTYPE::value_type(key,myInfoElement));
					if(!sendDYHMessage(DYH_Message)){
						printf("DYH Send: Error\n - Halt");
						pendReqMapLock -> Release();
						Exit(0);
					}
				}
				else{ // add to the existing entry in the Pending Queue Structure received in Find
					myInfoElement = it->second;
					setInfo(CVName,'i',machineId,mailBox,requestMessage,false);
				}
			pendReqMapLock -> Release();
			return 0;
		}
		else{
			pendReqMapLock -> Release();
			return 1;
		}
	}
	else{
		int id = isCVWithMe(machineId,mailBox,CVName,'i');
		return 1;
	}
}

/*********************************************************************************************************/
/*
*	Monitor Variables Syscalls
*/
/*********************************************************************************************************/

int isMVWithMe(int machineId,int mailBox,char *MVName,char reqType,int index, int val){
	serverMVTableLock -> Acquire();

	for(int i=1; i < serverMVIDCounter; i++){
		if(serverMVTable[i].mvName != NULL){
			if(strcmp(serverMVTable[i].mvName,MVName) == 0){
				printf("MV %s already Created, return MV to machine %d and mailBox %d\n",MVName,machineId,mailBox);
				if(reqType == 'j'){
					printf("MV %s already Created, return MVID to machine %d and mailBox %d\n",MVName,machineId,mailBox);
					sprintf(ack,"%d",i);
					serverCVTable[i].useCount++;
				}
				if(reqType == 'l' || reqType == 'k'){
					if(index >= serverMVTable[i].size){
						strcpy(ack,"-1Invalid size\0");
						serverMVTableLock -> Release();
						return i;
					}
					if(reqType == 'l'){
						int *ptr= serverMVTable[i].mv;
						*(ptr + index) = val;
						printf("MV[%d] MVName %s set to Value %d by Machine Id %d and mailBox %d\n",index,MVName,val,machineId,mailBox);
						strcpy(ack,"1\0");
					}
					if(reqType == 'k'){
						int *ptr= serverMVTable[i].mv;
						int mvval = *(ptr + index);
						printf("MV[%d] MVName %s get Value %d by Machine Id %d and mailBox %d\n",index,MVName,mvval,machineId,mailBox);
						sprintf(ack,"%d",mvval);
					}
					else if(reqType == 'm'){
						serverMVTable[i].useCount--;
						if(serverMVTable[i].useCount == 0){
							serverMVTable[i].mvName = NULL;
							serverMVTable[i].mv = NULL;
							serverMVTable[i].size = -1;
						}
						strcpy(ack,"1\0");
						serverMVTableLock -> Release();
						return 1;
					}
				}
				serverMVTableLock -> Release();
				return i;
			}
		}
	}
	serverMVTableLock -> Release();
	sprintf(ack,"%s","ResourceNotFound");
	return 0;
}

int CreateMVResource(int machineId,int mailBox,char *mess){
	char str1[40], str2[10];
	int index = 0;
	while(*mess != ','){
		*(str1 + index) = *mess;
		index++;
		mess++;
	}
	*(str1 + index)='\0';
	mess++;
	index =0;
	while(*mess != '\0'){
		*(str2 + index) = *mess;
		index++;
		mess++;
	}
	*(str2 + index)='\0';
	serverMVTableLock -> Acquire();

	if(serverMVIDCounter >= MAX_SERVER_MV){
		strcpy(ack,"-1MV Full\0");
		return -1;
	}

	serverMVTable[serverMVIDCounter].mvName = new char[31];
	strcpy(serverMVTable[serverMVIDCounter].mvName,str1);
	serverMVTable[serverMVIDCounter].size = atoi(str2);
	serverMVTable[serverMVIDCounter].mv = new int[atoi(str2)];
	serverMVTable[serverMVIDCounter].useCount = 1;
	printf("MV %s created for Machine Id %d and mailBox %d\n",str1,machineId,mailBox);
	int mvid = serverMVIDCounter;
	serverMVIDCounter++;
	serverMVTableLock -> Release();
	return mvid;
}

/*
*	Function:  CreateMV - Creates a monitor variable after checking for valid arguments
*/
int CreateMV(int machineId,int mailBox,char *mess,char* requestMessage){
	char str1[40], str2[10];
	int index = 0;
	char *messPtr;
	messPtr = mess;
	while(*mess != ','){
		if(*mess == '\0' || *mess == '\n' || *mess == '\t'){
			printf("Illegal MV Name provided by Machine Id %d and mailBox %d\n", machineId,mailBox);
			strcpy(ack,"-1InvalidMessage\0");					//Check the validity of the argument # 1 - monitor variable name
			return 1;
		}
		*(str1 + index) = *mess;
		index++;
		mess++;
	}
	*(str1 + index)='\0';
	mess++;
	index =0;
	int length = strlen(str1);
	if(length >= 48 || length <= 0){
		printf("Illegal MV Name %s provided by Machine Id %d and mailBox %d\n",str1,machineId, mailBox);
		strcpy(ack,"-1Invalid MV Name\0");
		return 1;
	}
	
	while(*mess != '\0'){
		if(*mess == '\n' || *mess == '\t' || index == 3 || (!(*mess >= 48 && *mess <= 57))){
			printf("Illegal MV Index Size for MVName %s provided by Machine Id %d and mailBox %d\n",str1,machineId,mailBox);
			strcpy(ack,"-1MessageFormatInvalid\0");		//Check the validity of the argument # 2 - size of the monitor varialble
			return 1;
		}
		*(str2 + index) = *mess;
		index++;
		mess++;
	}
	*(str2 + index)='\0';
	if(atoi(str2) > 100){
		strcpy(ack,"-1MVSizeTooLarge\0");					//Check the validity of the argument # 1 - monitor variable name
		printf("MV Index for MV Name %s too large, provided by Machine Id %d and mailBox %d\n", str1,machineId,mailBox);
		return 1;
	}
	if(noOfServers > 1){
		pendReqMapLock -> Acquire();
		if(isMVWithMe(machineId,mailBox,str1,'j',-1,-1) == 0){

			//Sad, we don't have the lock, need to ask other servers
			char char_key[45] = "j\0";
			strcat(char_key,str1);
			string key = string(char_key);

			//Check if we have the DYH request for the lock and Create Request
			
				it = pendReqMap.find(MAPTYPE::key_type(key));
				
				if(it == pendReqMap.end()){
					// Not Found in Map, so create new entry and add it to the Pending Queue Map

					myInfoElement = (myInfo*)malloc(sizeof(myInfo));
					myInfoElement -> mcIndex = 0;
					// Add all details to the structure of resource
					setInfo(str1,'j',machineId,mailBox,requestMessage,true);

					char DYH_Message[100];
					strcpy(DYH_Message,"#j\0");
					strcat(DYH_Message,str1);
					strcat(DYH_Message,",");
					strcat(DYH_Message,str2);
					strcat(DYH_Message,",");
					getTimeStamp();
					strcat(DYH_Message,the_date);
					timeStamp = atoi(the_date);
					myInfoElement -> timestamp = timeStamp;
					pendReqMap.insert(MAPTYPE::value_type(key,myInfoElement));
					if(!sendDYHMessage(DYH_Message)){
						printf("DYH Send: Error\n - Halt");
						pendReqMapLock -> Release();
						Exit(0);
					}
				}
				else{ // add to the existing entry in the Pending Queue Structure received in Find
					myInfoElement = it->second;
					setInfo(str1,'j',machineId,mailBox,requestMessage,false);
				}
			pendReqMapLock -> Release();
			return 0;
		}
		else{
			pendReqMapLock -> Release();
			return 1;
		}
	}
	else{
		int id = isMVWithMe(machineId,mailBox,str1,'j',-1,-1);
		if(id == 0)
			id = CreateMVResource(machineId,mailBox,messPtr);
		if(id == -1){
			strcpy(ack,"-1MV Full\0");
			return 1;
		}
		sprintf(ack,"%d",id);
		return 1;
	}
}


/*
*	Function:  SetValue - Sets the value for a monitor variable after checking for valid arguments
*/
//Message Format:-> MVid + , + index + , + value + \0
int SetValue(int machineId,int mailBox,char *mess,char* requestMessage){

	char str1[100],mvName[100],str2[100],str3[100];
	int index = 0;
	int mvID = -1, ind = -1, val= -1;
	while(*mess != ','){
		if(*mess == '\0' || *mess == '\n' || *mess == '\t'){
			printf("Set Value: Illegal MV Name provided by Machine Id %d and mailBox %d\n", machineId,mailBox);
			strcpy(ack,"-1MessageFormatInvalid\0");		//Check the validity of the argument # 1 - name of the monitor variable
			return 1;
		}
		*(str1 + index) = *mess;
		index++;
		mess++;
	}
	*(str1 + index)='\0';
	mess++;
	index =0;

	strcpy(mvName,str1);

	while(*mess != ','){
		if(*mess == '\0' || *mess == '\n' || *mess == '\t' || (!(*mess >= 48 && *mess <= 57))){
			printf("SetValue: Illegal MV Index position for MV Name %s provided by Machine Id %d and mailBox %d\n",str1, machineId,mailBox);
			strcpy(ack,"-1MessageFormatInvalid\0");		//Check the validity of the argument # 2 - index position
			return 1;
		}
		*(str1 + index) = *mess;
		index++;
		mess++;
	}
	*(str1 + index)='\0';
	mess++;
	index =0;
	strcpy(str2,str1);
	ind = atoi(str1);   // Index to be set

	while(*mess != '\0'){
		if(*mess == '\n' || *mess == '\t' || index == 10 || (!(*mess >= 48 && *mess <= 57))){
			printf("SetValue: Illegal Value to be set for MV Name %s provided by Machine Id %d and mailBox %d\n",str1,machineId,mailBox);
			strcpy(ack,"-1MessageFormatInvalid\0");								//Check the validity of the argument # 3 - value to set
			return 1;
		}
		*(str1 + index) = *mess;
		index++;
		mess++;
	}
	*(str1 + index)='\0';
	mess++;
	index =0;
	strcpy(str3,str1);
	val = atoi(str1);  // value to be set

	if(noOfServers > 1){
			pendReqMapLock -> Acquire();
			if(isMVWithMe(machineId,mailBox,mvName,'l',ind,val) == 0){

				//Sad, we don't have the lock, need to ask other servers
				char char_key[45] = "l\0";
				strcat(char_key,mvName);
				string key = string(char_key);

				//Check if we have the DYH request for the lock and Create Request

					it = pendReqMap.find(MAPTYPE::key_type(key));

					if(it == pendReqMap.end()){
						// Not Found in Map, so create new entry and add it to the Pending Queue Map

						myInfoElement = (myInfo*)malloc(sizeof(myInfo));
						myInfoElement -> mcIndex = 0;
						// Add all details to the structure of resource
						setInfo(mvName,'l',machineId,mailBox,requestMessage,true);
		
						char DYH_Message[100];
						strcpy(DYH_Message,"#l\0");
						strcat(DYH_Message,mvName);
						strcat(DYH_Message,",");
						strcat(DYH_Message,str2);
						strcat(DYH_Message,",");
						strcat(DYH_Message,str3);
						pendReqMap.insert(MAPTYPE::value_type(key,myInfoElement));
						if(!sendDYHMessage(DYH_Message)){
							printf("DYH Send: Error\n - Halt");
							pendReqMapLock -> Release();
							Exit(0);
						}
					}
					else{ // add to the existing entry in the Pending Queue Structure received in Find
						myInfoElement = it->second;
						setInfo(mvName,'l',machineId,mailBox,requestMessage,false);
					}
				pendReqMapLock -> Release();
				return 0;
			}
			else{
				pendReqMapLock -> Release();
				return 1;
			}
	}
	else{
		int id = isMVWithMe(machineId,mailBox,mvName,'l',ind,val);
		return 1;
	}
}

/*
*	Function:  GetValue - Gets the value for a monitor variable after checking for valid arguments and returns it to the client
*/
//Message Format:-> MVid + , + index + \0
int GetValue(int machineId,int mailBox,char *mess,char* requestMessage){
	
	char str1[100],mvName[100],str2[100],str3[100];
	int index = 0;
	int mvID = -1, ind = -1, val= -1;

	while(*mess != ','){
		if(*mess == '\0' || *mess == '\n' || *mess == '\t'){
			printf("Get Value: Illegal MV Name provided by Machine Id %d and mailBox %d\n", machineId,mailBox);
			strcpy(ack,"-1MessageFormatInvalid\0");		//Check the validity of the argument # 1 - name of the monitor variable
			return 1;
		}
		*(str1 + index) = *mess;
		index++;
		mess++;
	}
	*(str1 + index)='\0';
	mess++;
	index =0;


	strcpy(mvName,str1);

	while(*mess != '\0'){
		if(*mess == '\0' || *mess == '\n' || *mess == '\t' || (!(*mess >= 48 && *mess <= 57))){
			printf("GetValue: Illegal MV Index position for MV Name %s provided by Machine Id %d and mailBox %d\n",str1, machineId,mailBox);
			strcpy(ack,"-1MessageFormatInvalid\0");		//Check the validity of the argument # 2 - index position
			return 1;
		}
		*(str1 + index) = *mess;
		index++;
		mess++;
	}
	*(str1 + index)='\0';
	mess++;
	index =0;
	strcpy(str2,str1);
	ind = atoi(str1);   // Index to be set

	if(noOfServers > 1){
			pendReqMapLock -> Acquire();
			if(isMVWithMe(machineId,mailBox,mvName,'k',ind,-1) == 0){

				//Sad, we don't have the lock, need to ask other servers
				char char_key[45] = "k\0";
				strcat(char_key,mvName);
				string key = string(char_key);

				//Check if we have the DYH request for the lock and Create Request

					it = pendReqMap.find(MAPTYPE::key_type(key));

					if(it == pendReqMap.end()){
						// Not Found in Map, so create new entry and add it to the Pending Queue Map

						myInfoElement = (myInfo*)malloc(sizeof(myInfo));
						myInfoElement -> mcIndex = 0;
						// Add all details to the structure of resource
						setInfo(mvName,'k',machineId,mailBox,requestMessage,true);
		
						char DYH_Message[100];
						strcpy(DYH_Message,"#k\0");
						strcat(DYH_Message,mvName);
						strcat(DYH_Message,",");
						strcat(DYH_Message,str2);
						pendReqMap.insert(MAPTYPE::value_type(key,myInfoElement));
						if(!sendDYHMessage(DYH_Message)){
							printf("DYH Send: Error\n - Halt");
							pendReqMapLock -> Release();
							Exit(0);
						}
					}
					else{ // add to the existing entry in the Pending Queue Structure received in Find
						myInfoElement = it->second;
						setInfo(mvName,'k',machineId,mailBox,requestMessage,false);
					}
				pendReqMapLock -> Release();
				return 0;
			}
			else{
				pendReqMapLock -> Release();
				return 1;
			}
	}
	else{
		int id = isMVWithMe(machineId,mailBox,mvName,'k',ind,-1);
		return 1;
	}
}

/*
*	Function:  DeleteServerMV - Deletes the requested monitor variable after checking for valid arguments
*/
int DeleteServerMV(int machineId, int mailBox,char* str1,char* requestMessage)
{
	if(noOfServers > 1){
		pendReqMapLock -> Acquire();
		if(isMVWithMe(machineId,mailBox,str1,'m',-1,-1) == 0){

			//Sad, we don't have the lock, need to ask other servers
			char char_key[45] = "m\0";
			strcat(char_key,str1);
			string key = string(char_key);

			//Check if we have the DYH request for the lock and Create Request
			
				it = pendReqMap.find(MAPTYPE::key_type(key));
				
				if(it == pendReqMap.end()){
					// Not Found in Map, so create new entry and add it to the Pending Queue Map

					myInfoElement = (myInfo*)malloc(sizeof(myInfo));
					myInfoElement -> mcIndex = 0;
					// Add all details to the structure of resource
					setInfo(str1,'m',machineId,mailBox,requestMessage,true);
					char DYH_Message[100];
					strcpy(DYH_Message,"#m\0");
					strcat(DYH_Message,str1);
					pendReqMap.insert(MAPTYPE::value_type(key,myInfoElement));
					if(!sendDYHMessage(DYH_Message)){
						printf("DYH Send: Error\n - Halt");
						pendReqMapLock -> Release();
						Exit(0);
					}
				}
				else{ // add to the existing entry in the Pending Queue Structure received in Find
					myInfoElement = it->second;
					setInfo(str1,'m',machineId,mailBox,requestMessage,false);
				}
			pendReqMapLock -> Release();
			return 0;
		}
		else{
			pendReqMapLock -> Release();
			return 1;
		}
	}
	else{
		int id=isMVWithMe(machineId,mailBox,str1,'m',-1,-1);
		return 1;
	}
}

/*
*	Function:  messageParser - Used by the server to find the syscall requested by the server
*/
int messageParser(int machineId,int mailBox, char* mess1){
	int returnVal;
	char *requestMessage;
	requestMessage = mess1;
	char ch = *mess1;
	mess1++;

	/* New Additions For request forwarded by server
	
	*/
	
	switch(ch){
		case 'a':	
					returnVal = CreateServerLock(machineId,mailBox,mess1,requestMessage);
					return returnVal;
					break;

		case 'b':	
					returnVal = AcquireLock(machineId,mailBox,mess1,requestMessage);
					return returnVal;
					break;


		case 'c':	
					char *myptr;
					if((myptr = strchr(requestMessage,'*')) != NULL){
							char lockName[40];
							int i=1;
							while(*(requestMessage + i)!= ','){
								*(lockName + i -1) = *(requestMessage + i);
								i++;
							}
							*(lockName + i -1) = '\0';
							returnVal = ReleaseLock(machineId,mailBox,lockName,requestMessage);
							return 0;
					}
					else{
						returnVal = ReleaseLock(machineId,mailBox,mess1,requestMessage);
						return returnVal;
					}
					break;

		case 'd':
					returnVal = DeleteServerLock(machineId,mailBox,mess1,requestMessage);
					return returnVal;
					break;

		case 'e':	
					returnVal = CreateServerCV(machineId,mailBox,mess1,requestMessage);
					return returnVal;

					break;

		case 'f':	
					returnVal = WaitServerCV(machineId,mailBox,mess1,requestMessage);
					return returnVal;
					break;

		case 'g':	
					returnVal = SignalServerCV(machineId,mailBox,mess1,requestMessage);
					return returnVal;
					break;

		case 'h':	
					returnVal = BroadcastServerCV(machineId,mailBox,mess1,requestMessage);
					return returnVal;
					break;

		case 'i':	
					returnVal = DeleteServerCV(machineId,mailBox,mess1,requestMessage);
					return returnVal;

					break;
		case 'j':	
					returnVal = CreateMV(machineId,mailBox,mess1,requestMessage);
					return returnVal;
					break;

		case 'k':	
					returnVal = GetValue(machineId,mailBox,mess1,requestMessage);
					return returnVal; 
					break;

		case 'l':	
					returnVal = SetValue(machineId,mailBox,mess1,requestMessage);
					return returnVal; 
					break;

		case 'm':	
					returnVal = DeleteServerMV(machineId,mailBox,mess1,requestMessage);
					return returnVal;
					break;

		default:	
					*ack = ch;
					printf("Invalid Syscall by machine %d mailBox %d",machineId,mailBox);
					strcpy(ack + 1,":SysCallNotFound\0");
					return 1;
	}

	strcpy(ack,"-2");
	return -1;
}

int handleDYHRequest(char *reqType,int machineid, int mailboxno, char *msg)
{
	char resName[40]; 
	int i=0,j=0;
	char tsStr[30];
	char sizeChar[10];
	char indexChar[10];
	char valueChar[10];
	unsigned int tsInt;
	int size,indexInt,valueInt,mcidInt,mailboxInt;
	char *msgForMV;
	char mcidChar[10];
	char mailboxChar[10];
	bool success2;
	if(reqType[0] == 'x' || reqType[0] == 'y' || reqType[0] == 'w')
	{
		//the request is for a wait lock Wait,Signal,Broadcast
		while(*(msg+i) != ',')
		{
			resName[j] = *(msg+i);
			i++;
			j++;
		}
		resName[j] = '\0';
		i++;
		j=0;
		while(*(msg+i) != ',')
		{
			mcidChar[j] = *(msg+i);
			i++;
			j++;
		}
		mcidChar[j] = '\0';
		mcidInt = atoi(mcidChar);
		i++;
		j=0;
		while(*(msg+i) != '\0')
		{
			mailboxChar[j] = *(msg+i);
			i++;
			j++;
		}
		mailboxChar[j] = '\0';
		mailboxInt = atoi(mailboxChar);
	}
	else if(reqType[0] == 'a' || reqType[0] == 'e')
	{
		while(*(msg+i) != ',')
		{
			resName[j] = *(msg+i);
			i++;
			j++;
		}
		resName[j] = '\0';
		i++;
		j=0;
		while(*(msg+i) != '\0')
		{
			tsStr[j] = *(msg+i);
			i++;
			j++;
		}
		tsStr[j] = '\0';
		tsInt = atoi(tsStr);
	}
	else if(reqType[0] == 'j')
	{
		while(*(msg+i) != ',')
		{
			resName[j] = *(msg+i);
			i++;
			j++;
		}
		resName[j] = '\0';
		i++;
		j=0;
		while(*(msg+i) != ',')
		{
			sizeChar[j] = *(msg+i);
			j++;
			i++;
		}
		i++;
		sizeChar[j] = '\0';
		size = atoi(sizeChar);
		j=0;
		while(*(msg+i) != '\0')
		{
			tsStr[j] = *(msg+i);
			i++;
			j++;
		}
		tsStr[j] = '\0';
		tsInt = atoi(tsStr);
	}
	else if(reqType[0] == 'l' || reqType[0] == 'k')
	{
		while(*(msg+i) != ',')
		{
			resName[j] = *(msg+i);
			i++;
			j++;
		}
		resName[j] = '\0';
		i++;
		if(reqType[0] == 'k')
		{
			j=0;
			while(*(msg+i) != '\0')
			{
				indexChar[j] = *(msg+i);
				j++;
				i++;
			}
			i++;
			indexChar[j] = '\0';
			indexInt = atoi(indexChar);
		}
		if(reqType[0] == 'l')
		{
			j=0;
			while(*(msg+i) != ',')
			{
				indexChar[j] = *(msg+i);
				j++;
				i++;
			}
			i++;
			indexChar[j] = '\0';
			indexInt = atoi(indexChar);
			j=0;
			while(*(msg+i) != '\0')
			{
				valueChar[j] = *(msg+i);
				j++;
				i++;
			}
			i++;
			valueChar[j] = '\0';
			valueInt = atoi(valueChar);
		}
	}
	else
	{
		while(*(msg+i) != '\0')
		{
			resName[j] = *(msg+i);
			i++;
			j++;
		}
		resName[j] = '\0';
	}
	char myKeyChar[50];
	strcpy(myKeyChar,reqType);
	strcat(myKeyChar,resName);
	string mykey;
	mykey = string(myKeyChar);
	pendReqMapLock -> Acquire();
	it = pendReqMap.find(MAPTYPE::key_type(mykey));
	if(it == pendReqMap.end())		// I have not sent a DYH for the same resource so I only check if I have it or not
	{
		// The DYH request is for a lock I have not sent a DYH for
		if(reqType[0] == 'a' || reqType[0] == 'b' || reqType[0] == 'c' || reqType[0] == 'd')
		{
			serverLockTableLock -> Acquire();
			for(int k = 1; k < serverLockIDCounter; k++)
			{
				if(serverLockTable[k].lockName != NULL)
				{
					if(strcmp(serverLockTable[k].lockName,resName) == 0)
					{
						printf("\nI have the Lock %s, return LockID to machine %d and mailBox %d\n",resName,machineid,mailboxno);
						sprintf(ack1,"$%c%s,%d",reqType[0],resName,k);
						outPktHdr1.to = machineid;										// Send the response to the client
						outMailHdr1.to = 1;
						outMailHdr1.from = 1;
						outPktHdr1.from = myID;
						outMailHdr1.length = strlen(ack1) + 1;
						serverLockTable[k].useCount++;
						success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
						serverLockTableLock -> Release();
						pendReqMapLock -> Release();
						return 0;
					}
				}
			}
			printf("\nI don't have the Lock %s, return a No to machine %d and mailbox %d\n",resName,machineid,mailboxno);
			outPktHdr1.to = machineid;										// Send the response to the client
			outMailHdr1.to = 1;
			outMailHdr1.from = 1;
			outPktHdr1.from = myID;
			if(reqType[0] == 'a')
			{
				sprintf(ack1,"!%c%s,%s",reqType[0],resName,tsStr);
			}
			else
			{
				strcpy(ack1,"!");
				strcat(ack1,reqType);
				strcat(ack1,resName);
			}
			cout << "\n ack1: " <<ack1 << "\n";
			outMailHdr1.length = strlen(ack1) + 1;
			success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
			serverLockTableLock -> Release();
			pendReqMapLock -> Release();
			return 0;
		}
		//The DYH request is for a CV I have not sent a DYH for
		if(reqType[0] == 'e' || reqType[0] == 'f' || reqType[0] == 'g' || reqType[0] == 'h' || reqType[0] == 'i')
		{
			serverCVTableLock -> Acquire();
			for(int k=1; k < serverCVIDCounter; k++)
			{
				if(serverCVTable[k].cvName != NULL)
				{
					if(strcmp(serverCVTable[k].cvName,resName) == 0)
					{
						printf("\nI have the CV %s, so return ID to machineID %d and mailBox %d\n",resName,machineid,mailboxno);
						sprintf(ack1,"$%c%s,%d",reqType[0],resName,k);
						outPktHdr1.to = machineid;										// Send the response to the client
						outMailHdr1.to = 1;
						outMailHdr1.from = 1;
						outPktHdr1.from = myID;
						outMailHdr1.length = strlen(ack1) + 1;
						serverCVTable[k].useCount++;
						success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
						serverCVTableLock -> Release();
						pendReqMapLock -> Release();
						return 0;
					}
				}	
			}
			printf("\nI don't have the CV %s, return a No to machine %d and mailbox %d\n",resName,machineid,mailboxno);
			if(reqType[0] == 'e')
			{
				sprintf(ack1,"!%c%s,%s",reqType[0],resName,tsStr);
			}
			else
			{
				sprintf(ack1,"!%c%s",reqType[0],resName);
			}
			outPktHdr1.to = machineid;										// Send the response to the client
			outMailHdr1.to = 1;
			outMailHdr1.from = 1;
			outPktHdr1.from = myID;
			outMailHdr1.length = strlen(ack1) + 1;
			success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
			serverCVTableLock -> Release();
			pendReqMapLock -> Release();
			return 0;
		}
		//The DYH request is for a MV I have not sent a DYH for
		if(reqType[0] == 'j' || reqType[0] == 'l' || reqType[0] == 'k' || reqType[0] == 'm')
		{
			serverMVTableLock -> Acquire();
			for(int k=1; k < serverMVIDCounter; k++)
			{
				if(serverMVTable[k].mvName != NULL)
				{
					if(strcmp(serverMVTable[k].mvName,resName) == 0)
					{
						if(reqType[0] == 'l' || reqType[0] == 'k')
						{
							//The DYH is for a Set / Get MV
							//I have to determine if the index passed is correct
							if(reqType[0] == 'l')
							{
								//I will set the MV value and return a response to the server
								sprintf(ack1,"$%c%s,%d,%d",reqType[0],resName,indexInt,valueInt);
							}
							else if(reqType[0] == 'k')
							{
								// I will get the value of the MV and return the value to the server
								sprintf(ack1,"$%c%s,%d",reqType[0],resName,indexInt);
							}
						}
						else
						{
							sprintf(ack1,"$%c%s,%d",reqType[0],resName,k);
						}
						printf("\nI have the MV %s, returning MVInfo to Machine Id %d and mailBox %d\n",resName,machineid,mailboxno);
						outPktHdr1.to = machineid;										// Send the response to the client
						outMailHdr1.to = 1;
						outMailHdr1.from = 1;
						outPktHdr1.from = myID;
						outMailHdr1.length = strlen(ack1) + 1;
						serverMVTable[k].useCount++;
						success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
						serverMVTableLock -> Release();
						pendReqMapLock -> Release();
						return 0;
					}
				}
			}
			printf("\nI don't have the MV %s, return a No to machine %d and mailbox %d\n",resName,machineid,mailboxno);
			if(reqType[0] == 'j')
			{
				sprintf(ack1,"!%c%s,%s",reqType[0],resName,tsStr);
			}
			else
			{
				sprintf(ack1,"!%c%s",reqType[0],resName);
			}
			outPktHdr1.to = machineid;										// Send the response to the client
			outMailHdr1.to = 1;
			outMailHdr1.from = 1;
			outPktHdr1.from = myID;
			outMailHdr1.length = strlen(ack1) + 1;
			success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
			serverMVTableLock -> Release();
			pendReqMapLock -> Release();
			return 0;
		}
		if(reqType[0] == 'x')
		{
			//The DYH is for a wait lock so I need to check in the lock table
			serverLockTableLock -> Acquire();
			for(int k=1; k < serverLockIDCounter; k++)
			{
				if(serverLockTable[k].lockName != NULL)
				{
					if(strcmp(serverLockTable[k].lockName,resName) == 0)
					{
						if(!isHeldByCurrentThread(mcidInt,mailboxInt,k))
						{
							//the lock found but the owner of the WAIT LOCK is different
							printf("\nI have the WAIT LOCK %s, but the WAIT LOCK owner is different\n",resName);
							sprintf(ack1,"$%c%s,%d,+",reqType[0],resName,k);
							outPktHdr1.to = machineid;										// Send the response to the client
							outMailHdr1.to = 1;
							outMailHdr1.from = 1;
							outPktHdr1.from = myID;
							outMailHdr1.length = strlen(ack1) + 1;
							success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
							serverLockTableLock -> Release();
							pendReqMapLock -> Release();
							return 0;
						}
						else
						{
							printf("\nI have the Lock %s, return LockID to machine %d and mailBox %d\n",resName,machineid,mailboxno);
							sprintf(ack1,"$%c%s,%d,*,%d,%d",reqType[0],resName,k,mcidInt,mailboxInt);
							outPktHdr1.to = machineid;										// Send the response to the client
							outMailHdr1.to = 1;
							outMailHdr1.from = 1;
							outPktHdr1.from = myID;
							outMailHdr1.length = strlen(ack1) + 1;
							success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
							serverLockTableLock -> Release();
							pendReqMapLock -> Release();
							return 0;
						}
					}
				}
			}
			printf("\nI don't have the Lock %s, return a No to machine %d and mailbox %d\n",resName,machineid,mailboxno);
			outPktHdr1.to = machineid;										// Send the response to the client
			outMailHdr1.to = 1;
			outMailHdr1.from = 1;
			outPktHdr1.from = myID;
			sprintf(ack1,"!%c%s",reqType[0],resName);
			outMailHdr1.length = strlen(ack1) + 1;
			success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
			serverLockTableLock -> Release();
			pendReqMapLock -> Release();
			return 0;		
		}
		if(reqType[0] == 'y')
		{
			//The DYH is for a wait lock so I need to check in the lock table
			serverLockTableLock -> Acquire();
			for(int k=1; k < serverLockIDCounter; k++)
			{
				if(serverLockTable[k].lockName != NULL)
				{
					if(strcmp(serverLockTable[k].lockName,resName) == 0)
					{
						if(!isHeldByCurrentThread(mcidInt,mailboxInt,k))
						{
							//the lock found but the owner of the WAIT LOCK is different
							printf("\nI have the WAIT LOCK %s, but the WAIT LOCK owner is different\n",resName);
							sprintf(ack1,"$%c%s,%d,+",reqType[0],resName,k);
							outPktHdr1.to = machineid;										// Send the response to the client
							outMailHdr1.to = 1;
							outMailHdr1.from = 1;
							outPktHdr1.from = myID;
							outMailHdr1.length = strlen(ack1) + 1;
							success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
							serverLockTableLock -> Release();
							pendReqMapLock -> Release();
							return 0;
						}
						else
						{
							printf("\nI have the Lock %s, return LockID to machine %d and mailBox %d\n",resName,machineid,mailboxno);
							sprintf(ack1,"$%c%s,%d,*,%d,%d",reqType[0],resName,k,mcidInt,mailboxInt);
							outPktHdr1.to = machineid;										// Send the response to the client
							outMailHdr1.to = 1;
							outMailHdr1.from = 1;
							outPktHdr1.from = myID;
							outMailHdr1.length = strlen(ack1) + 1;
							success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
							serverLockTableLock -> Release();
							pendReqMapLock -> Release();
							return 0;
						}
					}
				}
			}
			printf("\nI don't have the Lock %s, return a No to machine %d and mailbox %d\n",resName,machineid,mailboxno);
			outPktHdr1.to = machineid;										// Send the response to the client
			outMailHdr1.to = 1;
			outMailHdr1.from = 1;
			outPktHdr1.from = myID;
			strcpy(ack1,"!");
			strcat(ack1,reqType);
			strcat(ack1,resName);
			outMailHdr1.length = strlen(ack1) + 1;
			success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
			serverLockTableLock -> Release();
			pendReqMapLock -> Release();
			return 0;		
		}
		if(reqType[0] == 'w')
		{
			//The DYH is for a wait lock so I need to check in the lock table
			serverLockTableLock -> Acquire();
			for(int k=1; k < serverLockIDCounter; k++)
			{
				if(serverLockTable[k].lockName != NULL)
				{
					if(strcmp(serverLockTable[k].lockName,resName) == 0)
					{
						if(!isHeldByCurrentThread(mcidInt,mailboxInt,k))
						{
							//the lock found but the owner of the WAIT LOCK is different
							printf("\nI have the WAIT LOCK %s, but the WAIT LOCK owner is different\n",resName);
							sprintf(ack1,"$%c%s,%d,+",reqType[0],resName,k);
							outPktHdr1.to = machineid;										// Send the response to the client
							outMailHdr1.to = 1;
							outMailHdr1.from = 1;
							outPktHdr1.from = myID;
							outMailHdr1.length = strlen(ack1) + 1;
							success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
							serverLockTableLock -> Release();
							pendReqMapLock -> Release();
							return 0;
						}
						else
						{
							printf("\nI have the Lock %s, return LockID to machine %d and mailBox %d\n",resName,machineid,mailboxno);
							sprintf(ack1,"$%c%s,%d,*,%d,%d",reqType[0],resName,k,mcidInt,mailboxInt);
							outPktHdr1.to = machineid;										// Send the response to the client
							outMailHdr1.to = 1;
							outMailHdr1.from = 1;
							outPktHdr1.from = myID;
							outMailHdr1.length = strlen(ack1) + 1;
							success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
							serverLockTableLock -> Release();
							pendReqMapLock -> Release();
							return 0;
						}
					}
				}
			}
			printf("\nI don't have the Lock %s, return a No to machine %d and mailbox %d\n",resName,machineid,mailboxno);
			outPktHdr1.to = machineid;										// Send the response to the client
			outMailHdr1.to = 1;
			outMailHdr1.from = 1;
			outPktHdr1.from = myID;
			strcpy(ack1,"!");
			strcat(ack1,reqType);
			strcat(ack1,resName);
			outMailHdr1.length = strlen(ack1) + 1;
			success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
			serverLockTableLock -> Release();
			pendReqMapLock -> Release();
			return 0;		
		}
	}
	else			// I have sent a DYH for the same resource so I will store the DYH info of the server in my map 
	{
		if(reqType[0] == 'a' || reqType[0] == 'e' || reqType[0] == 'j')  // I have a DYH for "CreateX" type pf request
		{
			unsigned int ts = it->second->timestamp;
			printf("\nComparing Timestamps: Received Timestamp is %d Stored Timestamp is %d\n",tsInt,ts);
			if(ts > tsInt)			//check for timestamp of the incoming request to do total ordering
			{
				printf("\nI don't have the Resource %s, return a No to machine %d and mailbox %d\n",resName,machineid,mailboxno);
				sprintf(ack1,"!%c%s,%s",reqType[0],resName,tsStr);
				outPktHdr1.to = machineid;										// Send the response to the client
				outMailHdr1.to = 1;
				outMailHdr1.from = 1;
				outPktHdr1.from = myID;
				outMailHdr1.length = strlen(ack1) + 1;
				success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
				pendReqMapLock -> Release();
				return 0;
			}
			else if(ts == tsInt)
			{
				if(myID < machineid)
				{
					printf("\nI don't have the Resource %s, return a No to machine %d and mailbox %d\n",resName,machineid,mailboxno);
					sprintf(ack1,"!%c%s,%s",reqType[0],resName,tsStr);
					outPktHdr1.to = machineid;										// Send the response to the client
					outMailHdr1.to = 1;
					outMailHdr1.from = 1;
					outPktHdr1.from = myID;
					outMailHdr1.length = strlen(ack1) + 1;
					success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
					pendReqMapLock -> Release();
					return 0;
				}
				else
				{
					strcpy(it->second->recvtimestamp[it->second->recvIndex],tsStr);
					it->second->recvmachineId[it->second->recvIndex] = machineid;
					it->second->recvmailbox[it->second->recvIndex] = 1;
					it->second->recvIndex = it->second->recvIndex + 1;
					pendReqMap.insert(MAPTYPE::value_type(mykey,it->second));
					pendReqMapLock -> Release();
					return 0;
				}
			}
			else
			{
				strcpy(it->second->recvtimestamp[it->second->recvIndex],tsStr);
				it->second->recvmachineId[it->second->recvIndex] = machineid;
				it->second->recvmailbox[it->second->recvIndex] = 1;
				it->second->recvIndex = it->second->recvIndex + 1;
				pendReqMap.insert(MAPTYPE::value_type(mykey,it->second));
				pendReqMapLock -> Release();
				return 0;
			}
		}
		else
		{
			printf("\nI don't have the Resource %s, return a No to machine %d and mailbox %d\n",resName,machineid,mailboxno);
			sprintf(ack1,"!%c%s",reqType[0],resName);
			outPktHdr1.to = machineid;										// Send the response to the client
			outMailHdr1.to = 1;
			outMailHdr1.from = 1;
			outPktHdr1.from = myID;
			outMailHdr1.length = strlen(ack1) + 1;
			success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
			pendReqMapLock -> Release();
			return 0;
		}
	}
	return 1;
}

int handleYesResponse(char *reqType,int machineid, int mailboxno, char *msg)
{
	char resName[40];
	int i=0,j=0;
	char id[20],indexChar[20],valueChar[20],waitLockState[1],mcidChar[3],mailbxChar[3];
	int idInt,indexInt,valueInt,mcidInt,mailbxInt;
	bool success2;
	char *check, *check1;
	while(*(msg+i) != ',')
	{
		resName[j] = *(msg+i);
		i++;
		j++;
	}
	resName[j] = '\0';
	j=0;
	i++;
	if(reqType[0] == 'l' || reqType[0] == 'k')
	{
		if(reqType[0] == 'k')
		{
			j=0;
			while(*(msg+i) != '\0')
			{
				indexChar[j] = *(msg+i);
				j++;
				i++;
			}
			i++;
			indexChar[j] = '\0';
			indexInt = atoi(indexChar);
		}
		if(reqType[0] == 'l')
		{
			j=0;
			while(*(msg+i) != ',')
			{
				indexChar[j] = *(msg+i);
				j++;
				i++;
			}
			i++;
			indexChar[j] = '\0';
			indexInt = atoi(indexChar);
			j=0;
			while(*(msg+i) != '\0')
			{
				valueChar[j] = *(msg+i);
				j++;
				i++;
			}
			valueChar[j] = '\0';
			valueInt = atoi(valueChar);
		}
	}
	else if(reqType[0] == 'x' || reqType[0] == 'y' || reqType[0] == 'w')
	{
		check = strchr(msg,'*');
		check1 = strchr(msg,'+');
		//the response is for a wait lock
		while(*(msg+i) != ',')
		{	
			id[j] = *(msg+i);
			i++;
			j++;
		}
		id[j] = '\0';
		idInt = atoi(id);
		i++;
		waitLockState[0] = *(msg+i);
		i++;
		i++;
		if(check != NULL)
		{
			j=0;
			while(*(msg+i) != ',')
			{
				mcidChar[j] = *(msg+i);
				i++;
				j++;
			}
			mcidChar[j] = '\0';
			mcidInt = atoi(mcidChar);
			i++;
			j=0;
			while(*(msg+i) != '\0')
			{
				mailbxChar[j] = *(msg+i);
				i++;
				j++;
			}
			mailbxChar[j] = '\0';
			mailbxInt = atoi(mailbxChar);
		}
	}
	else
	{
		j = 0;
		while(*(msg+i) != '\0')
		{
			id[j] = *(msg+i);
			i++;
			j++;
		}
		id[j] = '\0';
		idInt = atoi(id);
	}
	char myKeyChar[50];
	strcpy(myKeyChar,reqType);
	strcat(myKeyChar,resName);
	string mykey;
	mykey = string(myKeyChar);
	pendReqMapLock -> Acquire();
	it = pendReqMap.find(MAPTYPE::key_type(mykey));
	if(it == pendReqMap.end())
	{
		pendReqMapLock -> Release();
		return 0;
	}
	it->second->posMachineId = machineid;
	it->second->posMailbox = mailboxno;
	it->second->resourceID = idInt;
	if(reqType[0] == 'x' || reqType[0] == 'y' || reqType[0] == 'w')
	{
		it->second->waitLockState[0] = waitLockState[0];
		it->second->mcidInt = mcidInt;
		it->second->mailbxInt = mailbxInt;
	}
	it->second->reply = 1;
	it->second->replyCount = it->second->replyCount - 1;
	if(it->second->replyCount == 0)
	{
		//I received all the replies to my DYH
		if(reqType[0] == 'a' || reqType[0] == 'e' || reqType[0] == 'j')
		{
			//respond back with the id to all the clients in my queue
			for(int k = 0 ; k < it->second->mcIndex; k++)
			{
				sprintf(ack1,"%d",idInt);                              
				outPktHdr1.to = it->second->machineId[k];										// Send the response to the client
				outPktHdr1.from = myID;
				outMailHdr1.to = it->second->mailbox[k];
				outMailHdr1.from = 0;
				outMailHdr1.length = strlen(ack1) + 1;
				//printf("Sent \"%s\" to outPktHdr.to: %d,outMailHdr.to: %d \n",ack1,outPktHdr1.to,outMailHdr1.to);
				success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
			}
			for(int k = 0; k < it->second->recvIndex; k++)
			{
				sprintf(ack1,"!%c%s,%s",reqType[0],resName,it->second->recvtimestamp[k]);
				outPktHdr1.to = it->second->recvmachineId[k];										// Send the response to the client
				outPktHdr1.from = myID;
				outMailHdr1.to = 1;
				outMailHdr1.from = 1;
				outMailHdr1.length = strlen(ack1) + 1;
				//printf("Sent \"%s\" to outPktHdr.to: %d,outMailHdr.to: %d \n",ack1,outPktHdr1.to,outMailHdr1.to);
				success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
			}
			pendReqMap.erase(mykey);
			pendReqMapLock -> Release();
			return 0;
		}
		else
		{
			//Send all the queued requests to the server who owns the resource
			if(reqType[0] == 'l')
			{
				for(int k = 0 ; k < it->second->mcIndex ; k++)
				{
					sprintf(ack1,"%s",it->second->message[k]);
					outPktHdr1.to = it->second->posMachineId;										// Send the response to the client
					outPktHdr1.from = it->second->machineId[k];
					outMailHdr1.to = 0;
					outMailHdr1.from = it->second->mailbox[k];
					outMailHdr1.length = strlen(ack1) + 1;
					//printf("Sent \"%s\" to outPktHdr.to: %d,outMailHdr.to: %d \n",ack1,outPktHdr1.to,outMailHdr1.to);
					success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
				}
				pendReqMap.erase(mykey);
				pendReqMapLock -> Release();
				return 0;
			}
			else if(reqType[0] == 'k')
			{
				for(int k = 0 ; k < it->second->mcIndex ; k++)
				{
					sprintf(ack1,"%s",it->second->message[k]);
					outPktHdr1.to = it->second->posMachineId;										// Send the response to the client
					outPktHdr1.from = it->second->machineId[k];
					outMailHdr1.to = 0;
					outMailHdr1.from = it->second->mailbox[k];
					outMailHdr1.length = strlen(ack1) + 1;
					//printf("Sent \"%s\" to outPktHdr.to: %d,outMailHdr.to: %d \n",ack1,outPktHdr1.to,outMailHdr1.to);
					success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
				}
				pendReqMap.erase(mykey);
				pendReqMapLock -> Release();
				return 0;
			}
			else if(reqType[0] == 'x')
			{
				//The yes response is for a wait lock I had asked for
				if(waitLockState[0] == '*')
				{
					serverCVTableLock -> Acquire();
					int idOfCV = atoi(it->second->name);
					if(serverCVTable[idOfCV].waitLock == 0)
						serverCVTable[idOfCV].waitLock = idInt;

					//waitLock and conditionLock do not match
					if(serverCVTable[idOfCV].waitLock != idInt){
						printf("MachineId %d and mailBox %d cannot wait for condition as condition Lock is not same as waitLock \n",mcidInt,mailbxInt);
						strcpy(ack1,"-1InvalidLock\0");
						outPktHdr1.to = mcidInt;										// Send the response to the client
						outPktHdr1.from = myID;
						outMailHdr1.to = mailbxInt;
						outMailHdr1.from = 1;
						outMailHdr1.length = strlen(ack1) + 1;
						success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
						pendReqMap.erase(mykey);
						serverCVTableLock -> Release();
						pendReqMapLock -> Release();
						return 0;
					}
					//the server has the lock and the owner is valid
					//Send a release lock request to the server
					sprintf(ack1,"c%s,*",resName);
					outPktHdr1.to = it->second->posMachineId;										// Send the response to the client
					outPktHdr1.from = mcidInt;
					outMailHdr1.to = 0;
					outMailHdr1.from = mailbxInt;
					outMailHdr1.length = strlen(ack1) + 1;
					printf("Sent \"%s\" to outPktHdr.to: %d,outMailHdr.to: %d \n",ack1,outPktHdr1.to,outMailHdr1.to);
					success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
					message = new char[50];
					strcpy(message,"1\0");
					toAddr1 = new int;
					*toAddr1 = mcidInt;
					toMailBox1 = new int;
					*toMailBox1 = mailbxInt;
					//put the thread to sleep
					serverCVTable[idOfCV].messageWaitQueue -> Append((void *)message);				// Add the message to the wait queue
					serverCVTable[idOfCV].destMachineIDQueue -> Append((void *)toAddr1);				// Add the machine id to the wait queue
					serverCVTable[idOfCV].destMailBoxQueue -> Append((void*)toMailBox1);
					printf("Machine Id %d and mailBox %d put to wait queue of CV and Lock %s\n", mcidInt,mailbxInt,resName);
					pendReqMap.erase(mykey);
					serverCVTableLock -> Release();
					pendReqMapLock -> Release();
					return 0;
				}
				else if(waitLockState[0] == '+')
				{
					//the server has the lock but the owner is different
					//thus I will reply error to the client
					sprintf(ack1,"-1Wait called using a worng lock");
					for(int k = 0 ; k < it->second->mcIndex ; k++)
					{
						outPktHdr1.to = it->second->machineId[k];										// Send the response to the client
						outPktHdr1.from = myID;
						outMailHdr1.to = it->second->mailbox[k];
						outMailHdr1.from = 1;
						outMailHdr1.length = strlen(ack1) + 1;
						printf("Sent \"%s\" to outPktHdr.to: %d,outMailHdr.to: %d \n",ack1,outPktHdr1.to,outMailHdr1.to);
						success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
					}
					pendReqMap.erase(mykey);
					pendReqMapLock -> Release();
					return 0;
				}
			}
			else if(reqType[0] == 'y')
			{
				//The yes response is for a wait lock I had asked for
				if(waitLockState[0] == '*')
				{
					serverCVTableLock -> Acquire();
					int idOfCV = atoi(it->second->name);
					//the server has the lock and the owner is valid
					//Send a release lock request to the server

					if(serverCVTable[idOfCV].messageWaitQueue -> IsEmpty()){
						printf("Machine Id %d and mailBox %d signalled using Lock %s and CV %s, but no client waiting\n", mcidInt,mailbxInt,resName,serverCVTable[idOfCV].cvName);
						strcpy(ack1,"-1 No Client Waiting");
						outPktHdr1.to = mcidInt;										// Send the response to the client
						outPktHdr1.from = myID;
						outMailHdr1.to = mailbxInt;
						outMailHdr1.from = 0;
						outMailHdr1.length = strlen(ack1) + 1;
						success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
						pendReqMap.erase(mykey);
						serverCVTableLock -> Release();
						pendReqMapLock -> Release();
						return 0;
					}
					if(serverCVTable[idOfCV].waitLock != idInt){
						printf("Machine Id: %d and mailBox %d cannot signal for condition as condition Lock %s passed is not same as WaitLock\n",mcidInt,mailbxInt,resName);
						strcpy(ack1,"-1InvalidCVLock\0");
						outPktHdr1.to = mcidInt;										// Send the response to the client
						outPktHdr1.from = myID;
						outMailHdr1.to = mailbxInt;
						outMailHdr1.from = 0;
						outMailHdr1.length = strlen(ack1) + 1;
						success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
						pendReqMap.erase(mykey);
						serverCVTableLock -> Release();
						pendReqMapLock -> Release();
						return 0;
					}
					serverCVTable[idOfCV].messageWaitQueue -> Remove();										// Remove a message from the wait queue
					if(serverCVTable[idOfCV].messageWaitQueue -> IsEmpty()){
						serverCVTable[idOfCV].waitLock = 0;
					}
					printf("Machine Id %d and mailBox %d signalled using CV: %s and Lock %s\n", mcidInt,mailbxInt,serverCVTable[idOfCV].cvName,resName);
					pendReqMap.erase(mykey);
					sprintf(ack1,"1");
					outPktHdr1.to = mcidInt;										// Send the response to the client
					outPktHdr1.from = myID;
					outMailHdr1.to = mailbxInt;
					outMailHdr1.from = 0;
					outMailHdr1.length = strlen(ack1) + 1;
					success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
					
					for(int p = 0 ; p < 10000; p++)
					{
						//Delay
					}
					sprintf(ack1,"b%s",resName);
					
					outPktHdr1.to = it->second->posMachineId;										// Send the response to the client
					outPktHdr1.from = *((int *)serverCVTable[idOfCV].destMachineIDQueue -> Remove());
					outMailHdr1.to = 0;
					outMailHdr1.from = *((int *)serverCVTable[idOfCV].destMailBoxQueue -> Remove());
					outMailHdr1.length = strlen(ack1) + 1;
					printf("Sent \"%s\" to outPktHdr.to: %d,outMailHdr.to: %d \n",ack1,outPktHdr1.to,outMailHdr1.to);
					success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
					pendReqMap.erase(mykey);
					serverCVTableLock -> Release();
					pendReqMapLock -> Release();
					return 0;
					//waitLock and conditionLock do not match
				}
				else if(waitLockState[0] == '+')
				{
					//the server has the lock but the owner is different
					//thus I will reply error to the client
					sprintf(ack1,"-1Signal called using a worng lock");
					for(int k = 0 ; k < it->second->mcIndex ; k++)
					{
						outPktHdr1.to = it->second->machineId[k];										// Send the response to the client
						outPktHdr1.from = myID;
						outMailHdr1.to = it->second->mailbox[k];
						outMailHdr1.from = 1;
						outMailHdr1.length = strlen(ack1) + 1;
						printf("Sent \"%s\" to outPktHdr.to: %d,outMailHdr.to: %d \n",ack1,outPktHdr1.to,outMailHdr1.to);
						success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
					}
					pendReqMap.erase(mykey);
					pendReqMapLock -> Release();
					return 0;
				}
			}
			else if(reqType[0] == 'w')
			{
				//The yes response is for a wait lock I had asked for
				if(waitLockState[0] == '*')
				{
					serverCVTableLock -> Acquire();
					int idOfCV = atoi(it->second->name);
					//the server has the lock and the owner is valid
					//Send a release lock request to the server
					if(serverCVTable[idOfCV].messageWaitQueue -> IsEmpty()){
						printf("Machine Id %d and mailBox %d signalled using Lock %s and CV %s, but no client waiting\n", mcidInt,mailbxInt,resName,serverCVTable[idOfCV].cvName);
						strcpy(ack1,"-1 No Client Waiting");
						outPktHdr1.to = mcidInt;										// Send the response to the client
						outPktHdr1.from = myID;
						outMailHdr1.to = mailbxInt;
						outMailHdr1.from = 0;
						outMailHdr1.length = strlen(ack1) + 1;
						success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
						pendReqMap.erase(mykey);
						serverCVTableLock -> Release();
						pendReqMapLock -> Release();
						return 0;
					}
					if(serverCVTable[idOfCV].waitLock != idInt){
						printf("Machine Id: %d and mailBox %d cannot signal for condition as condition Lock %s passed is not same as WaitLock\n",mcidInt,mailbxInt,resName);
						strcpy(ack1,"-1InvalidCVLock\0");
						outPktHdr1.to = mcidInt;										// Send the response to the client
						outPktHdr1.from = myID;
						outMailHdr1.to = mailbxInt;
						outMailHdr1.from = 0;
						outMailHdr1.length = strlen(ack1) + 1;
						success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
						pendReqMap.erase(mykey);
						serverCVTableLock -> Release();
						pendReqMapLock -> Release();
						return 0;
					}
					printf("Machine Id %d and mailBox %d signalled using CV: %s and Lock %s\n", mcidInt,mailbxInt,serverCVTable[idOfCV].cvName,resName);
					sprintf(ack1,"1");
					outPktHdr1.to = mcidInt;										// Send the response to the client
					outPktHdr1.from = myID;
					outMailHdr1.to = mailbxInt;
					outMailHdr1.from = 0;
					outMailHdr1.length = strlen(ack1) + 1;
					success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
						
					while(!(serverCVTable[idOfCV].messageWaitQueue -> IsEmpty()))
					{
						serverCVTable[idOfCV].messageWaitQueue -> Remove();										// Remove a message from the wait queue
						if(serverCVTable[idOfCV].messageWaitQueue -> IsEmpty()){
							serverCVTable[idOfCV].waitLock = 0;
						}
						for(int p = 0 ; p < 10000; p++)
						{
							//Delay
						}
						sprintf(ack1,"b%s",resName);
						outPktHdr1.to = it->second->posMachineId;										// Send the response to the client
						outPktHdr1.from = *((int *)serverCVTable[idOfCV].destMachineIDQueue -> Remove());
						outMailHdr1.to = 0;
						outMailHdr1.from = *((int *)serverCVTable[idOfCV].destMailBoxQueue -> Remove());
						outMailHdr1.length = strlen(ack1) + 1;
						printf("Sent \"%s\" to outPktHdr.to: %d,outMailHdr.to: %d \n",ack1,outPktHdr1.to,outMailHdr1.to);
						success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
					}
					pendReqMap.erase(mykey);
					serverCVTableLock -> Release();
					pendReqMapLock -> Release();
					return 0;
					//waitLock and conditionLock do not match
				}
				else if(waitLockState[0] == '+')
				{
					//the server has the lock but the owner is different
					//thus I will reply error to the client
					sprintf(ack1,"-1Broadcast called using a worng lock");
					for(int k = 0 ; k < it->second->mcIndex ; k++)
					{
						outPktHdr1.to = it->second->machineId[k];										// Send the response to the client
						outPktHdr1.from = myID;
						outMailHdr1.to = it->second->mailbox[k];
						outMailHdr1.from = 1;
						outMailHdr1.length = strlen(ack1) + 1;
						printf("Sent \"%s\" to outPktHdr.to: %d,outMailHdr.to: %d \n",ack1,outPktHdr1.to,outMailHdr1.to);
						success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
					}
					pendReqMap.erase(mykey);
					pendReqMapLock -> Release();
					return 0;
				}
			}
			else if(reqType[0] == 'f' || reqType[0] == 'g' || reqType[0] == 'h')
			{
				//I have found the owner of the CV to whom I will fwd my wait request
				sprintf(ack1,it->second->message[0]);
			}
			else
			{
				sprintf(ack1,"%c%s",reqType[0],resName);
			}
			for(int k = 0 ; k < it->second->mcIndex ; k++)
			{
				outPktHdr1.to = it->second->posMachineId;										// Send the response to the client
				outPktHdr1.from = it->second->machineId[k];
				outMailHdr1.to = 0;
				outMailHdr1.from = it->second->mailbox[k];
				outMailHdr1.length = strlen(ack1) + 1;
				printf("Sent \"%s\" to outPktHdr.to: %d,outMailHdr.to: %d \n",ack1,outPktHdr1.to,outMailHdr1.to);
				success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
			}
			pendReqMap.erase(mykey);
			pendReqMapLock -> Release();
			return 0;
		}
	}
	else
	{
		pendReqMapLock -> Release();
		return 0;
	}
	return 1;
}

int handleNoResponse(char *reqType,int machineid, int mailboxno, char *msg)
{
	char resName[40],tsStr[30];
	int i=0,j=0,handleFlag = 1;
	unsigned int tsInt;
	char id[10];
	int idInt;
	bool success2;
	printf("\nIn handle no response for msg %s\n",msg);
	if(reqType[0] == 'a' || reqType[0] == 'e' || reqType[0] == 'j')
	{
		while(*(msg+i) != ',')
		{
			resName[j] = *(msg+i);
			i++;
			j++;
		}
		resName[j] = '\0';
		i++;
		j=0;
		while(*(msg+i) != '\0')
		{
			tsStr[j] = *(msg+i);
			i++;
			j++;
		}
		tsStr[j] = '\0';
		tsInt = atoi(tsStr);
	}
	else
	{
		j=0;
		while(*(msg+i) != '\0')
		{
			resName[j] = *(msg+i);
			i++;
			j++;
		}
		resName[j] = '\0';
	}
	char myKeyChar[50];
	strcpy(myKeyChar,reqType);
	strcat(myKeyChar,resName);
	string mykey;
	mykey = string(myKeyChar);
	pendReqMapLock -> Acquire();
	it = pendReqMap.find(MAPTYPE::key_type(mykey));
	if(it == pendReqMap.end())
	{
		pendReqMapLock -> Release();
		return 0;
	}
	if(reqType[0] == 'a' || reqType[0] == 'e' || reqType[0] == 'j')
	{
		//Compare Timestamps and reject requests that are not of the current request's timestamp
		if(tsInt == it->second->timestamp)
		{
			handleFlag = 1;
		}	
		else
		{
			handleFlag = 0;
		}
	}
	if(handleFlag == 1)
	{
		it->second->replyCount = it->second->replyCount - 1;
		if(it->second->replyCount == 0)
		{
			//I received the all replies to my DYH
			if(it->second->reply == 0)
			{
				if(reqType[0] == 'a' || reqType[0] == 'e' || reqType[0] == 'j')
				{
					int newLockId = -1;
					if(reqType[0] == 'a')
					{	
						//I have all no's for my DYH createX request
						//I will create the requested resource
						newLockId = isLockWithMe(myID,1,resName,'f');
						if(newLockId == 0)
						{
							cout <<"\nCreating Lock "<<resName<<"\n";
							newLockId = CreateLockResource(myID,0,resName);
						}
					}
					else if(reqType[0] == 'e' )
					{
						newLockId = isCVWithMe(myID,1,resName,'f');
						if(newLockId == 0)
						{
							cout<<"\nCreating CV "<<resName<<"\n";
							newLockId = CreateCVResource(myID,0,resName);
						}
					}
					else if(reqType[0] == 'j')
					{
						newLockId = isMVWithMe(myID,1,resName,'z',0,0);
						if(newLockId == 0)
						{
							cout<<"\nCreating MV "<<resName<<"\n";
							newLockId = CreateMVResource(myID,0,it->second->message[0]+1);
						}
					}
					for(int k = 0 ; k < it->second->mcIndex; k++)
					{
						sprintf(ack1,"%d",newLockId);
						outPktHdr1.to = it->second->machineId[k];										// Send the response to the client
						outPktHdr1.from = myID;
						outMailHdr1.to = it->second->mailbox[k];
						outMailHdr1.from = 1;
						outMailHdr1.length = strlen(ack1) + 1;
						printf("Sent \"%s\" to outPktHdr.to: %d,outMailHdr.to: %d \n",ack1,outPktHdr1.to,outMailHdr1.to);
						success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
					}
					for(int k = 0 ; k < it->second->recvIndex; k++)
					{
						sprintf(ack1,"$%c%s,%d",reqType[0],it->second->name,newLockId);
						outPktHdr1.to = it->second->recvmachineId[k];										// Send the response to the client
						outPktHdr1.from = myID;
						outMailHdr1.to = 1;
						outMailHdr1.from = 1;
						outMailHdr1.length = strlen(ack1) + 1;
						printf("Sent \"%s\" to outPktHdr.to: %d,outMailHdr.to: %d \n",ack1,outPktHdr1.to,outMailHdr1.to);
						success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
					}
				}
				else
				{
					//I will reply all the clients "Error" as they have requested some resource which no server has
					for(int k = 0 ; k < it->second->mcIndex; k++)
					{
						sprintf(ack1,"-1WrongResourceName");
						outPktHdr1.to = it->second->machineId[k];										// Send the response to the client
						outPktHdr1.from = myID;
						outMailHdr1.to = it->second->mailbox[k];
						outMailHdr1.from = 1;
						outMailHdr1.length = strlen(ack1) + 1;
						printf("Sent \"%s\" to outPktHdr.to: %d,outMailHdr.to: %d \n",ack1,outPktHdr1.to,outMailHdr1.to);
						success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
					}
				}
				pendReqMap.erase(mykey);
				pendReqMapLock -> Release();
				return 0;
			}
			else
			{
				//one of the servers has sent me a "yes"
				//I will send all the requests in the queue to that server
				//I received all the replies to my DYH
				if(reqType[0] == 'a' || reqType[0] == 'e' || reqType[0] == 'j')
				{
					//respond back with the id to all the clients in my queue
					for(int k = 0 ; k < it->second->mcIndex; k++)
					{
						sprintf(ack1,"%d",it->second->resourceID);
						outPktHdr1.to = it->second->machineId[k];										// Send the response to the client
						outPktHdr1.from = myID;
						outMailHdr1.to = it->second->mailbox[k];
						outMailHdr1.from = 0;
						outMailHdr1.length = strlen(ack1) + 1;
						printf("Sent \"%s\" to outPktHdr.to: %d,outMailHdr.to: %d \n",ack1,outPktHdr1.to,outMailHdr1.to);
						success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
					}
					for(int k = 0; k < it->second->recvIndex; k++)
					{
						sprintf(ack1,"!%c%s,%s",reqType[0],resName,it->second->recvtimestamp[k]);
						outPktHdr1.to = it->second->recvmachineId[k];										// Send the response to the client
						outPktHdr1.from = myID;
						outMailHdr1.to = 1;
						outMailHdr1.from = 1;
						outMailHdr1.length = strlen(ack1) + 1;
						printf("Sent \"%s\" to outPktHdr.to: %d,outMailHdr.to: %d \n",ack1,outPktHdr1.to,outMailHdr1.to);
						success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
					}
					pendReqMap.erase(mykey);
					pendReqMapLock -> Release();
					return 0;
				}
				else
				{
					//Send all the queued requests to the server who owns the resource
					if(reqType[0] == 'l')
					{
						for(int k = 0 ; k < it->second->mcIndex ; k++)
						{
							sprintf(ack1,"%s",it->second->message[k]);
							outPktHdr1.to = it->second->posMachineId;										// Send the response to the client
							outPktHdr1.from = it->second->machineId[k];
							outMailHdr1.to = 0;
							outMailHdr1.from = it->second->mailbox[k];
							outMailHdr1.length = strlen(ack1) + 1;
							printf("Sent \"%s\" to outPktHdr.to: %d,outMailHdr.to: %d \n",ack1,outPktHdr1.to,outMailHdr1.to);
							success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
						}
						pendReqMap.erase(mykey);
						pendReqMapLock -> Release();
						return 0;
					}
					else if(reqType[0] == 'k')
					{
						for(int k = 0 ; k < it->second->mcIndex ; k++)
						{
							sprintf(ack1,"%s",it->second->message[k]);
							outPktHdr1.to = it->second->posMachineId;										// Send the response to the client
							outPktHdr1.from = it->second->machineId[k];
							outMailHdr1.to = 0;
							outMailHdr1.from = it->second->mailbox[k];
							outMailHdr1.length = strlen(ack1) + 1;
							printf("Sent \"%s\" to outPktHdr.to: %d,outMailHdr.to: %d \n",ack1,outPktHdr1.to,outMailHdr1.to);
							success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
						}
						pendReqMap.erase(mykey);
						pendReqMapLock -> Release();
						return 0;
					}
					else if(reqType[0] == 'x')
					{
						//The yes response is for a wait lock I had asked for
						if(it->second->waitLockState[0] == '*')
						{
							serverCVTableLock -> Acquire();
							int idOfCV = atoi(it->second->name);
							if(serverCVTable[idOfCV].waitLock == 0)
								serverCVTable[idOfCV].waitLock = it->second->resourceID;

							//waitLock and conditionLock do not match
							if(serverCVTable[idOfCV].waitLock != it->second->resourceID){
								printf("MachineId %d and mailBox %d cannot wait for condition as condition Lock is not same as waitLock \n",it->second->mcidInt,it->second->mailbxInt);
								strcpy(ack1,"-1InvalidLock\0");
								outPktHdr1.to = it->second->mcidInt;										// Send the response to the client
								outPktHdr1.from = myID;
								outMailHdr1.to = it->second->mailbxInt;
								outMailHdr1.from = 1;
								outMailHdr1.length = strlen(ack1) + 1;
								success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
								pendReqMap.erase(mykey);
								serverCVTableLock -> Release();
								pendReqMapLock -> Release();
								return 0;
							}
							//the server has the lock and the owner is valid
							//Send a release lock request to the server
							sprintf(ack1,"c%s,*",resName);
							outPktHdr1.to = it->second->posMachineId;										// Send the response to the client
							outPktHdr1.from = it->second->mcidInt;
							outMailHdr1.to = 0;
							outMailHdr1.from = it->second->mailbxInt;
							outMailHdr1.length = strlen(ack1) + 1;
							printf("Sent \"%s\" to outPktHdr.to: %d,outMailHdr.to: %d \n",ack1,outPktHdr1.to,outMailHdr1.to);
							success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
							message = new char[50];
							strcpy(message,"1\0");
							toAddr1 = new int;
							*toAddr1 = it->second->mcidInt;
							toMailBox1 = new int;
							*toMailBox1 = it->second->mailbxInt;
							//put the thread to sleep
							serverCVTable[idOfCV].messageWaitQueue -> Append((void *)message);				// Add the message to the wait queue
							serverCVTable[idOfCV].destMachineIDQueue -> Append((void *)toAddr1);				// Add the machine id to the wait queue
							serverCVTable[idOfCV].destMailBoxQueue -> Append((void*)toMailBox1);
							printf("Machine Id %d and mailBox %d put to wait queue of CV and Lock %s\n", it->second->mcidInt,it->second->mailbxInt,resName);
							pendReqMap.erase(mykey);
							serverCVTableLock -> Release();
							pendReqMapLock -> Release();
							return 0;
						}
						else if(it->second->waitLockState[0] == '+')
						{
							//the server has the lock but the owner is different
							//thus I will reply error to the client
							sprintf(ack1,"-1Wait called using a wrong lock");
							for(int k = 0 ; k < it->second->mcIndex ; k++)
							{
								outPktHdr1.to = it->second->machineId[k];										// Send the response to the client
								outPktHdr1.from = myID;
								outMailHdr1.to = it->second->mailbox[k];
								outMailHdr1.from = 1;
								outMailHdr1.length = strlen(ack1) + 1;
								printf("Sent \"%s\" to outPktHdr.to: %d,outMailHdr.to: %d \n",ack1,outPktHdr1.to,outMailHdr1.to);
								success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
							}
							pendReqMap.erase(mykey);
							pendReqMapLock -> Release();
							return 0;
						}
					}
					else if(reqType[0] == 'y')
					{
						//The yes response is for a wait lock I had asked for
						if(it->second->waitLockState[0] == '*')
						{
							serverCVTableLock -> Acquire();
							int idOfCV = atoi(it->second->name);
							//the server has the lock and the owner is valid
							//Send a release lock request to the server

							if(serverCVTable[idOfCV].messageWaitQueue -> IsEmpty()){
								printf("Machine Id %d and mailBox %d signalled using Lock %s and CV %s, but no client waiting\n",it->second->mcidInt,it->second->mailbxInt,resName,serverCVTable[idOfCV].cvName);
								strcpy(ack1,"-1NoClientWaiting");
								outPktHdr1.to = it->second->mcidInt;										// Send the response to the client
								outPktHdr1.from = myID;
								outMailHdr1.to = it->second->mailbxInt;
								outMailHdr1.from = 0;
								outMailHdr1.length = strlen(ack1) + 1;
								success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
								pendReqMap.erase(mykey);
								serverCVTableLock -> Release();
								pendReqMapLock -> Release();
								return 0;
							}
							if(serverCVTable[idOfCV].waitLock != it->second->resourceID){
								printf("Machine Id: %d and mailBox %d cannot signal for condition as condition Lock %s passed is not same as WaitLock\n",it->second->mcidInt,it->second->mailbxInt,resName);
								strcpy(ack1,"-1InvalidCVLock");
								outPktHdr1.to = it->second->mcidInt;										// Send the response to the client
								outPktHdr1.from = myID;
								outMailHdr1.to = it->second->mailbxInt;
								outMailHdr1.from = 0;
								outMailHdr1.length = strlen(ack1) + 1;
								success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
								pendReqMap.erase(mykey);
								serverCVTableLock -> Release();
								pendReqMapLock -> Release();
								return 0;
							}
							serverCVTable[idOfCV].messageWaitQueue -> Remove();										// Remove a message from the wait queue
							if(serverCVTable[idOfCV].messageWaitQueue -> IsEmpty()){
								serverCVTable[idOfCV].waitLock = 0;
							}
							printf("Machine Id %d and mailBox %d signalled using CV: %s and Lock %s\n", it->second->mcidInt,it->second->mailbxInt,serverCVTable[idOfCV].cvName,resName);
							pendReqMap.erase(mykey);
							sprintf(ack1,"1");
							outPktHdr1.to = it->second->mcidInt;										// Send the response to the client
							outPktHdr1.from = myID;
							outMailHdr1.to = it->second->mailbxInt;
							outMailHdr1.from = 0;
							outMailHdr1.length = strlen(ack1) + 1;
							success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);							
							for(int p = 0 ; p < 10000; p++)
							{
								//Delay
							}
							sprintf(ack1,"b%s",resName);
							outPktHdr1.to = it->second->posMachineId;										// Send the response to the client
							outPktHdr1.from = *((int *)serverCVTable[idOfCV].destMachineIDQueue -> Remove());
							outMailHdr1.to = 0;
							outMailHdr1.from = *((int *)serverCVTable[idOfCV].destMailBoxQueue -> Remove());
							outMailHdr1.length = strlen(ack1) + 1;
							printf("Sent \"%s\" to outPktHdr.to: %d,outMailHdr.to: %d \n",ack1,outPktHdr1.to,outMailHdr1.to);
							success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
							pendReqMap.erase(mykey);
							serverCVTableLock -> Release();
							pendReqMapLock -> Release();
							return 0;
							//waitLock and conditionLock do not match
						}
						else if(it->second->waitLockState[0] == '+')
						{
							//the server has the lock but the owner is different
							//thus I will reply error to the client
							sprintf(ack1,"-1Signal called using a worng lock");
							for(int k = 0 ; k < it->second->mcIndex ; k++)
							{
								outPktHdr1.to = it->second->machineId[k];										// Send the response to the client
								outPktHdr1.from = myID;
								outMailHdr1.to = it->second->mailbox[k];
								outMailHdr1.from = 1;
								outMailHdr1.length = strlen(ack1) + 1;
								printf("Sent \"%s\" to outPktHdr.to: %d,outMailHdr.to: %d \n",ack1,outPktHdr1.to,outMailHdr1.to);
								success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
							}
							pendReqMap.erase(mykey);
							pendReqMapLock -> Release();
							return 0;
						}
					}
					else if(reqType[0] == 'w')
					{
						//The yes response is for a wait lock I had asked for
						if(it->second->waitLockState[0] == '*')
						{
							serverCVTableLock -> Acquire();
							int idOfCV = atoi(it->second->name);
							//the server has the lock and the owner is valid
							//Send a release lock request to the server
							if(serverCVTable[idOfCV].messageWaitQueue -> IsEmpty()){
								printf("Machine Id %d and mailBox %d signalled using Lock %s and CV %s, but no client waiting\n", it->second->mcidInt,it->second->mailbxInt,resName,serverCVTable[idOfCV].cvName);
								strcpy(ack1,"-1 No Client Waiting");
								outPktHdr1.to = it->second->mcidInt;										// Send the response to the client
								outPktHdr1.from = myID;
								outMailHdr1.to = it->second->mailbxInt;
								outMailHdr1.from = 0;
								outMailHdr1.length = strlen(ack1) + 1;
								success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
								pendReqMap.erase(mykey);
								serverCVTableLock -> Release();
								pendReqMapLock -> Release();
								return 0;
							}
							if(serverCVTable[idOfCV].waitLock != it->second->resourceID){
								printf("Machine Id: %d and mailBox %d cannot signal for condition as condition Lock %s passed is not same as WaitLock\n",it->second->mcidInt,it->second->mailbxInt,resName);
								strcpy(ack1,"-1InvalidCVLock");
								outPktHdr1.to = it->second->mcidInt;										// Send the response to the client
								outPktHdr1.from = myID;
								outMailHdr1.to = it->second->mailbxInt;
								outMailHdr1.from = 0;
								outMailHdr1.length = strlen(ack1) + 1;
								success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
								pendReqMap.erase(mykey);
								serverCVTableLock -> Release();
								pendReqMapLock -> Release();
								return 0;
							}
							printf("Machine Id %d and mailBox %d signalled using CV: %s and Lock %s\n", it->second->mcidInt,it->second->mailbxInt,serverCVTable[idOfCV].cvName,resName);
							sprintf(ack1,"1");
							outPktHdr1.to = it->second->mcidInt;										// Send the response to the client
							outPktHdr1.from = myID;
							outMailHdr1.to = it->second->mailbxInt;
							outMailHdr1.from = 0;
							outMailHdr1.length = strlen(ack1) + 1;
							success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);	
							while(!(serverCVTable[idOfCV].messageWaitQueue -> IsEmpty()))
							{
								serverCVTable[idOfCV].messageWaitQueue -> Remove();										// Remove a message from the wait queue
								
								if(serverCVTable[idOfCV].messageWaitQueue -> IsEmpty()){
									serverCVTable[idOfCV].waitLock = 0;
								}
								
								for(int p = 0 ; p < 10000; p++)
								{
									//Delay
								}
								sprintf(ack1,"b%s",resName);
								outPktHdr1.to = it->second->posMachineId;										// Send the response to the client
								outPktHdr1.from = *((int *)serverCVTable[idOfCV].destMachineIDQueue -> Remove());
								outMailHdr1.to = 0;
								outMailHdr1.from = *((int *)serverCVTable[idOfCV].destMailBoxQueue -> Remove());
								outMailHdr1.length = strlen(ack1) + 1;
								printf("Sent \"%s\" to outPktHdr.to: %d,outMailHdr.to: %d \n",ack1,outPktHdr1.to,outMailHdr1.to);
								success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
							}
							pendReqMap.erase(mykey);
							serverCVTableLock -> Release();
							pendReqMapLock -> Release();
							return 0;
							//waitLock and conditionLock do not match
						}
						else if(it->second->waitLockState[0] == '+')
						{
							//the server has the lock but the owner is different
							//thus I will reply error to the client
							sprintf(ack1,"-1Broadcast called using a worng lock");
							for(int k = 0 ; k < it->second->mcIndex ; k++)
							{
								outPktHdr1.to = it->second->machineId[k];										// Send the response to the client
								outPktHdr1.from = myID;
								outMailHdr1.to = it->second->mailbox[k];
								outMailHdr1.from = 1;
								outMailHdr1.length = strlen(ack1) + 1;
								printf("Sent \"%s\" to outPktHdr.to: %d,outMailHdr.to: %d \n",ack1,outPktHdr1.to,outMailHdr1.to);
								success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
							}
							pendReqMap.erase(mykey);
							pendReqMapLock -> Release();
							return 0;
						}
					}
					else if(reqType[0] == 'f' || reqType[0] == 'g' || reqType[0] == 'h')
					{
						//I have found the owner of the CV to whom I will fwd my wait request
						sprintf(ack1,it->second->message[0]);
					}
					else
					{
						sprintf(ack1,"%c%s",reqType[0],resName);
					}
					for(int k = 0 ; k < it->second->mcIndex ; k++)
					{
						outPktHdr1.to = it->second->posMachineId;										// Send the response to the client
						outPktHdr1.from = it->second->machineId[k];
						outMailHdr1.to = 0;
						outMailHdr1.from = it->second->mailbox[k];
						outMailHdr1.length = strlen(ack1) + 1;
						printf("Sent \"%s\" to outPktHdr.to: %d,outMailHdr.to: %d \n",ack1,outPktHdr1.to,outMailHdr1.to);
						success2 = postOffice->Send(outPktHdr1, outMailHdr1, ack1);
					}
					pendReqMap.erase(mykey);
					pendReqMapLock -> Release();
					return 0;
				}
			}
		}
		else
		{
			pendReqMapLock -> Release();
			return 0;
		}
	}
	else
	{
		pendReqMapLock -> Release();
		return 0;
	}
	return 1;
}

void MailBox1(int nouse)
{
	int returnVal1;
	char typeOfReq[2];
	char messType;
	printf("\nServer Thread for MailBox # 1 Started\n");
	while(true)
	{
		// Wait for the first message from the other machine
		typeOfReq[1]= '\0';
		printf("Server Waiting to receive\n");
		postOffice->Receive(1, &inPktHdr1, &inMailHdr1, buffer1);							// Receive Message from the client
		printf("Got \"%s\" from %d, box %d\n",buffer1,inPktHdr1.from,inMailHdr1.from);
		fflush(stdout);
		char *buff;
		buff = buffer1;
		messType = *buff;
		buff++;
		typeOfReq[0] = *buff;
		buff++;
		if(messType == '#')
		{
			returnVal1 = handleDYHRequest(typeOfReq,inPktHdr1.from,inMailHdr1.from,buff);
		}
		if(messType == '$')
		{
			returnVal1 = handleYesResponse(typeOfReq,inPktHdr1.from,inMailHdr1.from,buff);
		}
		if(messType == '!')
		{
			returnVal1 = handleNoResponse(typeOfReq,inPktHdr1.from,inMailHdr1.from,buff);
		}
	}
    // Then we're done!
    interrupt->Halt();
}

void pingServer(int nouse)
{
	char reply[50];
	char ack2[35];
	time_t nowtime;
	int k = 0;
	nowtime = time(NULL);
	if((nowtime - prevtime) > 5)
	{
		prevtime = nowtime;
		serverLockTableLock -> Acquire();
		for(int i = 1 ;i < serverLockIDCounter; i++)
		{
			if(serverLockTable[i].lockOwner >= 0)
			{
				sprintf(ack2,"ping");
				outPktHdr3.to = serverLockTable[i].lockOwner;										// Send the response to the client
				outPktHdr3.from = myID;
				outMailHdr3.to = 39;
				outMailHdr3.from = 3;
				outMailHdr3.length = strlen(ack2) + 1;
				printf("Sent \"%s\" to outPktHdr.to: %d,outMailHdr.to: %d \n",ack2,outPktHdr3.to,outMailHdr3.to);
				success = postOffice->Send(outPktHdr3, outMailHdr3, ack2);
				if(!success)
				{
					printf("\nClient with Machine Id [ %d ] has died\n",serverLockTable[i].lockOwner);
					printf("\nThe client who died owned a lock\n");
					if(!(serverLockTable[i].messageQueue -> IsEmpty()))
					{
						printf("\nThere is a server waiting to acquire this lock\n");
						sprintf(reply,"%d",i);
						int *destmid  = (int *)serverLockTable[i].destMachineIDQueue -> Remove();		// Remove the machine id from the wait queue
						int *destmail = (int *)serverLockTable[i].destMailBoxQueue -> Remove();
						char *messPtr = (char *)serverLockTable[i].messageQueue -> Remove();
						printf("\nMaking the machine [ %d ] as the new Lock Owner\n",*destmid);
						serverLockTable[i].lockOwner = *destmid;
						outPktHdr5.to = *destmid;										// Send the response to the client
						outPktHdr5.from = myID;
						outMailHdr5.to = *destmail;
						outMailHdr5.from = 3;
						outMailHdr5.length = strlen(reply) + 1;
						printf("Sent \"%s\" to outPktHdr.to: %d,outMailHdr.to: %d \n",reply,outPktHdr5.to,outMailHdr5.to);
						success = postOffice->Send(outPktHdr5, outMailHdr5, reply);
					}
					else
					{
						printf("\nNo Client(s) are waiting to acquire Lock %d\n",i);
						printf("\nThus Making the lock status as FREE\n");
						serverLockTable[i].lockStatus = FREE;
						serverLockTable[i].lockOwner = -1;
					}
				}
			}
		}
		serverLockTableLock -> Release();
	}
}

void serverPingerCreator(int nu)
{
	prevtime = time(NULL);
	pingTimer = new Timer(pingServer,0,0);
}


/*
*	Function:  MailTest - Server Main Function
*/
void
MailBox_0(int nouse)
{
	int returnVal;
	int machIDReqFrom,machMailReqFrom;
	printf("\nServer Started\n");
	while(true)
	{
		// Wait for the first message from the other machine
		printf("Server Waiting to receive\n");
		postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);							// Receive Message from the client
		printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
		machIDReqFrom = inPktHdr.from;
		machMailReqFrom = inMailHdr.from;
		fflush(stdout);
		if(testcase == 2)
		{
			if(pingflag == 0)
			{
				printf("\nForking The Server Ping Thread\n");
				Thread *serverPingThread = new Thread("serverPingThread");
				serverPingThread -> Fork((VoidFunctionPtr)serverPingerCreator,0);
				pingflag = 1;
			}
		}
		returnVal = messageParser(inPktHdr.from, inMailHdr.from,buffer);	// Check the request and create the appropriate response
		// This is done by the message parser which calls the appropriate functions
		// Send acknowledgement to the other machine (using "reply to" mailbox
		// in the message that just arrived
		if(returnVal == 1){
			outPktHdr.to = machIDReqFrom;		// Send the response to the client
			outMailHdr.to = machMailReqFrom;
			outMailHdr.from = 0;
			outPktHdr.from = myID;
			outMailHdr.length = strlen(ack) + 1;
			printf("Sent \"%s\" to outPktHdr.to: %d,outMailHdr.to: %d \n",ack,outPktHdr.to,outMailHdr.to);
			success = postOffice->Send(outPktHdr, outMailHdr, ack);
			if ( !success ) {
			  printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
			  interrupt->Halt();
			}
		}
	}
    // Then we're done!
    interrupt->Halt();
}

void
MailTest(){

	myID = netname; // get the machine ID which is set in system.cc
				// this is the 'netname' variable in system.cc initialzed to -m parameter passed in commandLine

	serverLockTableLock = new Lock("serverLockTable");
	serverCVTableLock = new Lock("serverCVTable");
	serverMVTableLock = new Lock("serverMVTable");
	pendReqMapLock = new Lock("pendReqMap");
	printf("\nForking MailBox 1 thread\n");
	Thread *mailboxOneThread = new Thread("mailboxOneThread");
	Thread *mailboxZeroThread = new Thread("mailboxZeroThread");
	mailboxOneThread -> Fork((VoidFunctionPtr)MailBox1,0);
	printf("\nForking MailBox 0 thread\n");
	mailboxZeroThread -> Fork((VoidFunctionPtr)MailBox_0,0);
}

