#include "copyright.h"

#include "system.h"
#include "network.h"
#include "post.h"
#include "interrupt.h"
#include "list.h"

#define MAX_SERVER_LOCKS 51
#define MAX_SERVER_CV 51
#define MAX_SERVER_MV 51

int farAddr = 0;
PacketHeader outPktHdr, inPktHdr;		
MailHeader outMailHdr, inMailHdr;
char *data;
char ack[25];					// Response message of the server
char buffer[MaxMailSize];
bool success;
int *toAddr;
char *message;

/*
*	Lock Structure and Table
*/
int serverLockIDCounter = 1;
struct Serverlock{
	char* lockName;
	bool isToBeDeleted;
	List* messageQueue;
	List* destMachineIDQueue;
	LockStatus lockStatus;
	int lockOwner;
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
	int waitLock;
	int waitQueueCount;
	int useCount;
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
*	Function: sendMessage - sends the Message to the Client
*/
void sendMessage(int to, char* toSendMessage){
			outPktHdr.to = to;
			outPktHdr.from = 0;
			outMailHdr.to = inMailHdr.from;
			outMailHdr.length = strlen(toSendMessage) + 1;
			success = postOffice->Send(outPktHdr, outMailHdr, toSendMessage);
			if ( !success ) {
			  printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
			  interrupt->Halt();
			}
}

/*********************************************************************************************************/
/*
*	Lock Syscalls Section
*/
/*********************************************************************************************************/

/*
*	Function: Create Lock Syscall - Creates the requested lock after checking for valid arguments
*/
void CreateServerLock(int machineId,char *LockName){

	int length;
	length = strlen(LockName);
	if(length >= 31 || length <= 0){
		printf("Invalid Lock Name Length by Machine ID %d\n",machineId);
		strcpy(ack,"-1Invalid Lock Name\0");
		return;
	}
	char *temp;
	temp = LockName;

	while(*temp != '\0'){
		if(*temp =='\n' ||*temp == '\t' || !((*temp >= 97 && *temp <= 122) || (*temp >= 65 && *temp <=90) || (*temp >= 48 && *temp <= 57))){
			printf("Invalid Lock Name by Machine ID %d\n",machineId);
			strcpy(ack,"-1Invalid Lock Name\0");
			return;
		}
		temp++;
	}
	for(int i=1; i < serverLockIDCounter; i++){
		if(serverLockTable[i].lockName != NULL){
			if(strcmp(serverLockTable[i].lockName,LockName) == 0){
				printf("Lock already Created, return LockID to machine %d\n",machineId);
				sprintf(ack,"%d",i);
				serverLockTable[i].useCount++;
				return;
			}
		}
	}
	if(serverLockIDCounter >= MAX_SERVER_LOCKS){
		printf("Max Lock created at server\n");
		strcpy(ack,"-1Lock Full\0");
		return;
	}
	serverLockTable[serverLockIDCounter].lockName = new char[31];
	strcpy(serverLockTable[serverLockIDCounter].lockName,LockName);
	serverLockTable[serverLockIDCounter].isToBeDeleted = false;
	serverLockTable[serverLockIDCounter].lockStatus = FREE;
	serverLockTable[serverLockIDCounter].lockOwner = 0;
	serverLockTable[serverLockIDCounter].useCount = 1;
	serverLockTable[serverLockIDCounter].messageQueue = new List;
	serverLockTable[serverLockIDCounter].destMachineIDQueue = new List;
	printf("Lock %d created for Machine ID %d\n",serverLockIDCounter,machineId);
	sprintf(ack,"%d",serverLockIDCounter);
	serverLockIDCounter++;
	return;
}

/*
*	Function: Acquire Lock Syscall - Acquires the lock after checking for valid arguments
*/
int AcquireLock(int machineId, int lock_ID){
	if(lock_ID <= 0 || lock_ID >= serverLockIDCounter){					// Check for the validity of the parameters passed
		printf("Invalid LockID by Machine ID %d\n",machineId);
		strcpy(ack,"-1WrongLockID\0");
		return 1;
	}
	if(machineId < 0){
		printf("Invalid Machine ID %d\n",machineId);
		strcpy(ack,"-1Wrong MachineID\0");
		return 1;
	}
	
	if(serverLockTable[lock_ID].lockOwner == machineId){
		printf("Lock already owned by Machine ID %d\n",machineId);
		strcpy(ack,"1\0");
		return 1;
	}
	if(serverLockTable[lock_ID].lockStatus == FREE){
		printf("Free Lock Found for Machine ID %d\n",machineId);
		serverLockTable[lock_ID].lockStatus = BUSY;
		serverLockTable[lock_ID].lockOwner = machineId;
	}
	else{
		message = new char[50];
		strcpy(message,"1\0");
		toAddr = new int;
		*toAddr = machineId;
		serverLockTable[lock_ID].messageQueue -> Append((void *)message);			// Add the message to the wait queue if the lock is not available
		serverLockTable[lock_ID].destMachineIDQueue -> Append((void *)toAddr);
		printf("Lock Busy so put Machine ID %d to wait queue\n",machineId);
		return 0;
	}
	strcpy(ack,"1\0");
	return 1;
}

/*
*	Function: Release Lock Syscall - Releases the lock after checking for valid arguments
*/
int ReleaseLock(int machineId, int lock_ID){
	if(lock_ID <= 0 || lock_ID >= serverLockIDCounter){					// Check the validity of the parameters
		printf("Invalid LockID by Machine ID %d\n",machineId);
		strcpy(ack,"-1WrongLockID\0");
		return 1;
	}
	if(machineId < 0){
		strcpy(ack,"-1WrongMachineID\0");
		return 1;
	}
	if(serverLockTable[lock_ID].lockOwner != machineId){
		printf("Lock already owned by Machine ID %d\n",machineId);
		strcpy(ack,"-1NotLockOwner\0");
		return 1;
	}
	if(!(serverLockTable[lock_ID].messageQueue -> IsEmpty())){
		int *machIdPtr;
		char *messPtr;
		machIdPtr = (int *)serverLockTable[lock_ID].destMachineIDQueue -> Remove();		// Remove the machine id from the wait queue
		messPtr = (char *)serverLockTable[lock_ID].messageQueue -> Remove();			// Remove the message from the message wait queue
		serverLockTable[lock_ID].lockOwner = *machIdPtr;
		sendMessage(*machIdPtr,messPtr);
	}
	else{
		serverLockTable[lock_ID].lockStatus = FREE;
		serverLockTable[lock_ID].lockOwner = 0;
	}
	strcpy(ack,"1\0");
	return 1;
}

/*
*  Function: Delete Lock Syscall - Deletes the requested lock after checking for valid arguments
*/
int DeleteServerLock(int machineId,int lockId)
{
	if(lockId < 1 || lockId >= serverLockIDCounter){
		printf("Invalid Lock by Machine ID %d\n",machineId);
		strcpy(ack,"-1WrongLockID\0");
		return 1;
	}
	if(serverLockTable[lockId].lockName == NULL){
		printf("Lock not found requested to be deleted by Machine ID %d\n",machineId);
		strcpy(ack,"-1LockNotExists\0");
		return 1;
	}
	if(!(serverLockTable[lockId].messageQueue -> IsEmpty())){
		serverLockTable[lockId].isToBeDeleted = true;
		strcpy(ack,"1\0");
		return 1;
	}

	serverLockTable[lockId].useCount--;
	if(serverLockTable[lockId].useCount == 0){
		serverLockTable[lockId].lockName = NULL;
		serverLockTable[lockId].lockOwner = 0;
		delete serverLockTable[lockId].messageQueue;
		delete serverLockTable[lockId].destMachineIDQueue;
	}
	strcpy(ack,"1\0");
	return 1;
}

/*********************************************************************************************************/
/*
*	Condition Variable Section
*/
/*********************************************************************************************************/

/*
*	Function: Create CV Syscall - creates a condition variable after checking for valid arguments
*/
void CreateServerCV(int machineId,char *CVName)
{
	int length;
	length = strlen(CVName);
	if(length >= 31 || length <= 0){
		printf("Invalid CV Name issued by machineID %d\n",machineId);
		strcpy(ack,"-1Invalid CV Name\0");
		return;
	}
	char *temp;
	temp = CVName;
	while(*temp != '\0'){
		if(*temp =='\n' || *temp == '\t' || !((*temp >= 97 && *temp <= 122) || (*temp >= 65 && *temp <=90) || (*temp >= 48 && *temp <= 57))){
			printf("Invalid CV Name issued by machineID %d\n",machineId);
			strcpy(ack,"-1Invalid CV Name\0");
			return;
		}
		temp++;
	}
	for(int i=1; i < serverCVIDCounter; i++){
		if(serverCVTable[i].cvName != NULL){
			if(strcmp(serverCVTable[i].cvName,CVName) == 0){
				printf("CV already created, so return ID to machineID %d\n",machineId);
				sprintf(ack,"%d",i);
				serverCVTable[i].useCount++;
				return;
			}
		}
	}
	if(serverCVIDCounter >= MAX_SERVER_CV){
		printf("Max CV Created at Server\n");
		strcpy(ack,"-1CV Full\0");
		return;
	}

	serverCVTable[serverCVIDCounter].cvName = new char[31];
	strcpy(serverCVTable[serverCVIDCounter].cvName,CVName);
	serverCVTable[serverCVIDCounter].isToBeDeleted = false;
	serverCVTable[serverCVIDCounter].messageWaitQueue = new List;
	serverCVTable[serverCVIDCounter].destMachineIDQueue = new List;
	serverCVTable[serverCVIDCounter].waitLock = 0;
	serverCVTable[serverCVIDCounter].useCount = 1;
	serverCVTable[serverCVIDCounter].waitQueueCount = 0;
	printf("CV Created at server, return CVID: %d to machineID: %d\n",serverCVIDCounter,machineId);
	sprintf(ack,"%d",serverCVIDCounter);
	serverCVIDCounter++;
	return;
}

/*
*	Function:  isHeldByCurrentThread - checks if the machine id requesting the lock for a CV is the owner or not
*/
bool isHeldByCurrentThread(int machineId, int lockid)
{
	if(serverLockTable[lockid].lockOwner == machineId)
		return true;
	else return false;
}

/*
*	Function:  WaitServerCV - Calls a wait on the condition variable after checking for valid arguments
*/
int WaitServerCV(int machineId, char *msg)
{
	char lock[20];
	char cv[20];
	int lockid,cvid;
	int index = 0;
	while(*msg != ',')
	{
		*(cv + index) = *msg;				// Get the cvid from the arguments passed
		msg++;
		index++;
	}
	*(cv + index) = '\0';
	cvid = atoi(cv);
	msg++;
	index = 0;
	while(*msg != '\0')
	{
		*(lock + index) = *msg;				// Get the lock id from the parameters passed
		msg++;
		index++;
	}
	*(lock + index) = '\0';
	lockid = atoi(lock);

	if(lockid <= 0){
		printf("MachineId: %d cannot wait for condition as condition lock is negative %d\n",machineId,lockid);
		strcpy(ack,"-1InvalidLockID\0");
		return 1;
	}
	//Check if current thread has acquired the lock
	if(!(isHeldByCurrentThread(machineId,lockid)))
	{
		printf("machindId %d does not owns the lock %d\n", machineId,lockid);
		strcpy(ack,"-1NotLockOwner\0");
		return 1;
	}
	//First Thread with conditionLock calling wait
	if(serverCVTable[cvid].waitLock == 0)
		serverCVTable[cvid].waitLock = lockid;

	//waitLock and conditionLock do not match
	if(serverCVTable[cvid].waitLock != lockid){
		printf("MachineId %d cannot wait for condition as condition Lock is not same as waitLock \n",machineId);
		strcpy(ack,"-1InvalidLock\0");
		return 1;
	}
	//exiting the monitor before going to sleep
	ReleaseLock(machineId,lockid);

	// add the current thread to condition variable's wait queue
	message = new char[50];
	strcpy(message,"1\0");
	toAddr = new int;
	*toAddr = machineId;
	//put the thread to sleep
	
	serverCVTable[cvid].messageWaitQueue -> Append((void *)message);				// Add the message to the wait queue
	serverCVTable[cvid].destMachineIDQueue -> Append((void *)toAddr);				// Add the machine id to the wait queue
	printf("Machine Id %d put to wait queue of CV %d and Lock %d\n", machineId,cvid,lockid);
	return 0; 																		// Return 0 - which means the server will not send response
}

/*
*	Function:  SignalServerCV - Signals the waiting thread after checking for valid arguments
*/
int SignalServerCV(int machineId, char *msg)
{
	char lock[20];
	char cv[20];
	int lockid,cvid;
	int index = 0;
	while(*msg != ',')
	{
		*(cv + index) = *msg;				// Get the cvid from the parameter passed
		msg++;
		index++;
	}
	*(cv + index) = '\0';
	cvid = atoi(cv);
	msg++;
	index = 0;
	while(*msg != '\0')
	{
		*(lock + index) = *msg;				// Get the lock id from the parameter passed
		msg++;
		index++;
	}
	*(lock + index) = '\0';
	lockid = atoi(lock);

	if(lockid <= 0){
		printf("Machine %d cannot signal for condition as condition lock is negative %d\n",machineId,lockid);
		strcpy(ack,"-1InvalidLockID\0");
		return 1;
	}
	//Check if current thread has acquired the lock
	if(!(isHeldByCurrentThread(machineId,lockid)))
	{
		printf("MachineId: %d does not owns the lock %d\n", machineId,lockid);
		strcpy(ack,"-1NotLockOwner\0");
		return 1;
	}
	if(serverCVTable[cvid].messageWaitQueue -> 	IsEmpty()){
		printf("Machine Id %d signalled using Lock %d, but no client waiting\n", machineId,lockid);
		strcpy(ack,"-1No client waiting");
		return 1;
	}

	//waitLock and conditionLock do not match
	if(serverCVTable[cvid].waitLock != lockid){
		printf("Machine Id: %d cannot wait for condition as condition Lock is not same as WaitLock\n",machineId);
		strcpy(ack,"-1InvalidCVLock\0");
		return 1;
	}

	serverCVTable[cvid].messageWaitQueue -> Remove();										// Remove a message from the wait queue
	
	if(serverCVTable[cvid].messageWaitQueue -> 	IsEmpty()){
		serverCVTable[cvid].waitLock = 0;
	}

	AcquireLock(*((int *)serverCVTable[cvid].destMachineIDQueue -> Remove()),lockid);				// Try to acquire the lock which is used for the signalling
	printf("Machine Id %d signalled using CVId: %d and Lock %d\n", machineId,cvid,lockid);
	strcpy(ack,"1\0");
	return 1;
}

/*
*	Function:  BroadCastServerCV - Broadcasts a condition variable after checking for valid arguments
*/
int BroadcastServerCV(int machineId, char *msg)
{
	char lock[20];
	char cv[20];
	int lockid,cvid;
	int index = 0;
	char *mess;
	mess = msg;
	while(*msg != ',')
	{
		*(cv + index) = *msg;				// Get the cvid from the parameter passed
		msg++;
		index++;
	}
	*(cv + index) = '\0';
	cvid = atoi(cv);
	msg++;
	index = 0;
	while(*msg != '\0')
	{
		*(lock + index) = *msg;				// Get the lock id from the parameter passed
		msg++;
		index++;
	}
	*(lock + index) = '\0';
	lockid = atoi(lock);


	if(lockid <= 0){
		printf("Machine Id %d cannot wait for condition as condition lock is negative %d\n",machineId,lockid);
		strcpy(ack,"-1InvalidLockID\0");
		return 1;
	}
	//Check if current thread has acquired the lock
	if(!(isHeldByCurrentThread(machineId,lockid)))
	{
		printf("MachineID %d does not owns the lock %d\n", machineId,lockid);
		strcpy(ack,"-1NotLockOwner\0");
		return 1;
	}
	//waitLock and conditionLock do not match
	if(serverCVTable[cvid].waitLock != lockid){
		printf("MachineId: %d cannot wait for condition as condition Lock is not same as waitLock \n",machineId);
		strcpy(ack,"-1InvalidCVLock\0");
		return 1;
	}

	while(!(serverCVTable[cvid].messageWaitQueue -> IsEmpty())){
		SignalServerCV(machineId,mess);
	}
	return 1;
}


/*
*	Function:  DeleteServerCV - Deletes a condition variable after checking for valid arguments
*/
int DeleteServerCV(int machineId,int cvId)
{
	if(cvId < 1 || cvId > serverCVIDCounter)												// Check for the validity of the arguments passed to the system call
	{
		strcpy(ack,"-1Wrong CV ID - deleteCV\0");
		return 1;
	}
	if(serverCVTable[cvId].cvName == NULL)
	{
		strcpy(ack,"-1CVdoesNotExist\0");
		return 1;
	}
	if(!(serverCVTable[cvId].messageWaitQueue -> IsEmpty()))
	{
		serverCVTable[cvId].isToBeDeleted = true;
		strcpy(ack,"1\0");
		return 1;
	}
	serverCVTable[cvId].useCount--;

	if(serverCVTable[cvId].useCount == 0)
	{
		serverCVTable[cvId].cvName = NULL;
		serverCVTable[cvId].waitLock = -1;
		serverCVTable[cvId].waitQueueCount = -1;
		delete serverCVTable[cvId].messageWaitQueue;
	}
	strcpy(ack,"1\0");
	return 1;
}

/*********************************************************************************************************/
/*
*	Monitor Variables Syscalls
*/
/*********************************************************************************************************/

/*
*	Function:  CreateMV - Creates a monitor variable after checking for valid arguments
*/
void CreateMV(int machineId,char *mess){
	char str1[20], str2[4];
	int index = 0;
	while(*mess != ','){
		if(*mess == '\0' || *mess == '\n' || *mess == '\t'){
			printf("Illegal MV Name provided by Machine Id %d\n", machineId);
			strcpy(ack,"-1InvalidMessage\0");					//Check the validity of the argument # 1 - monitor variable name
			return;
		}
		*(str1 + index) = *mess;
		index++;
		mess++;
	}
	*(str1 + index)='\0';
	mess++;
	index =0;
	int length = strlen(str1);
	if(length >= 31 || length <= 0){
		printf("Illegal MV Name provided by Machine Id %d\n", machineId);
		strcpy(ack,"-1Invalid MV Name\0");
		return;
	}
	for(int i=1; i < serverMVIDCounter; i++){
		if(serverMVTable[i].mvName != NULL){
			if(strcmp(serverMVTable[i].mvName,str1) == 0){
				printf("MV Name already created, returning MVId to Machine Id %d\n", machineId);
				sprintf(ack,"%d",i);
				serverMVTable[i].useCount++;
				return;
			}
		}
	}
	if(serverMVIDCounter >= MAX_SERVER_MV){
		strcpy(ack,"-1MV Full\0");
		return;
	}

	while(*mess != '\0'){
		if(*mess == '\n' || *mess == '\t' || index == 3 || (!(*mess >= 48 && *mess <= 57))){
			printf("Illegal MV Index Size provided by Machine Id %d\n", machineId);
			strcpy(ack,"-1MessageFormatInvalid\0");		//Check the validity of the argument # 2 - size of the monitor varialble
			return;
		}
		*(str2 + index) = *mess;
		index++;
		mess++;
	}
	*(str2 + index)='\0';
	if(atoi(str2) > 100){
		strcpy(ack,"-1MVSizeTooLarge\0");					//Check the validity of the argument # 1 - monitor variable name
		printf("MV Index too large, provided by Machine Id %d\n", machineId);
		return;
	}
	serverMVTable[serverMVIDCounter].mvName = new char[31];
	strcpy(serverMVTable[serverMVIDCounter].mvName,str1);
	serverMVTable[serverMVIDCounter].size = atoi(str2);
	serverMVTable[serverMVIDCounter].mv = new int[atoi(str2)];
	serverMVTable[serverMVIDCounter].useCount = 1;
	printf("MV Created, MVId %d returned to Machine Id %d\n",serverMVIDCounter,machineId);
	sprintf(ack,"%d",serverMVIDCounter);
	serverMVIDCounter++;
	return;
}

/*
*	Function:  SetValue - Sets the value for a monitor variable after checking for valid arguments
*/
//Message Format:-> MVid + , + index + , + value + \0
void SetValue(int machineId,char *mess){
	printf("\nMessage Received: %s\n",mess);
	char str1[10];
	int index = 0;
	int mvID, ind, val;
	while(*mess != ','){
		if(*mess == '\0' || *mess == '\n' || *mess == '\t' || index == 2 || (!(*mess >= 48 && *mess <= 57))){
			printf("Illegal MV ID provided by Machine Id %d\n", machineId);
			strcpy(ack,"-1MessageFormatInvalid\0");		//Check the validity of the argument # 1 - name of the monitor variable
			return;
		}
		*(str1 + index) = *mess;
		index++;
		mess++;
	}
	*(str1 + index)='\0';
	mess++;
	index =0;
	mvID = atoi(str1);
	
	if(mvID >= serverMVIDCounter || mvID <= 0){
		printf("Illegal MV ID provided by Machine Id %d\n", machineId);
		strcpy(ack,"-1Invalid MachineID\0");
		return;
	}
	
	while(*mess != ','){
		if(*mess == '\0' || *mess == '\n' || *mess == '\t' || (!(*mess >= 48 && *mess <= 57))){
			printf("Illegal MV Index position provided by Machine Id %d\n", machineId);
			strcpy(ack,"-1MessageFormatInvalid\0");		//Check the validity of the argument # 2 - index position
			return;
		}
		*(str1 + index) = *mess;
		index++;
		mess++;
	}
	*(str1 + index)='\0';
	mess++;
	index =0;
	ind = atoi(str1);
	
	if(ind >= serverMVTable[mvID].size){
		strcpy(ack,"-1Invalid size\0");
		return;
	}
	
	while(*mess != '\0'){
		if(*mess == '\n' || *mess == '\t' || index == 10 || (!(*mess >= 48 && *mess <= 57))){
			printf("Illegal MV to be set Value provided by Machine Id %d\n", machineId);
			strcpy(ack,"-1MessageFormatInvalid\0");									//Check the validity of the argument # 3 - value to set
			return;
		}
		*(str1 + index) = *mess;
		index++;
		mess++;
	}
	*(str1 + index)='\0';
	mess++;
	index =0;
	val = atoi(str1);
	
	int *ptr= serverMVTable[mvID].mv;
	
	*(ptr + ind) = val;
	printf("MV[%d] set to Value %d by Machine Id %d\n",val,ind,machineId);
	printf("\nIndex: %d,value to be set: %d\n",ind,*(ptr + ind));
	strcpy(ack,"1\0");
	return;
}

/*
*	Function:  GetValue - Gets the value for a monitor variable after checking for valid arguments and returns it to the client
*/
//Message Format:-> MVid + , + index + \0
void GetValue(int machineId,char *mess){
	
	char str1[10];
	int index = 0;
	int mvID, ind, val;
	while(*mess != ','){															
		if(*mess == '\0' || *mess == '\n' || *mess == '\t' || index == 2 || (!(*mess >= 48 && *mess <= 57))){		
			strcpy(ack,"-1MessageFormatInvalid\0");														//Check the validity of the argument # 1 - name of the monitor variable
			return;
		}
		*(str1 + index) = *mess;
		index++;
		mess++;
	}
	*(str1 + index)='\0';
	mess++;
	index =0;
	mvID = atoi(str1);
	
	if(mvID >= serverMVIDCounter || mvID <= 0){
		strcpy(ack,"-1Invalid MV ID\0");
		return;
	}
	
	while(*mess != '\0'){
		if(*mess == '\n' || *mess == '\t' || (!(*mess >= 48 && *mess <= 57))){
			printf("Illegal MV Index position provided by Machine Id %d\n", machineId);
			strcpy(ack,"-1MessageFormatInvalid\0");													//Check the validity of the argument # 2 - value to set
			return;
		}
		*(str1 + index) = *mess;
		index++;
		mess++;
	}
	*(str1 + index)='\0';
	mess++;
	index =0;
	ind = atoi(str1);
	
	if(ind >= serverMVTable[mvID].size){
		strcpy(ack,"-1Invalid size\0");
		return;
	}
	
	int *ptr= serverMVTable[mvID].mv;								// Get the monitor variable value
	val = *(ptr + ind);
	printf("MV[%d] Value %d returned to Machine Id %d\n",val,ind,machineId);
	sprintf(ack,"%d",val);
	return;
}

/*
*	Function:  DeleteServerMV - Deletes the requested monitor variable after checking for valid arguments
*/
int DeleteServerMV(int machineId,int mvId)
{
	if(mvId < 1 || mvId > serverMVIDCounter)
	{
		strcpy(ack,"-1Wrong MV ID\0");
		return 1;
	}
	if(serverMVTable[mvId].mv == NULL)
	{
		strcpy(ack,"-1MVNotExists\0");
		return 1;
	}
	serverMVTable[mvId].useCount--;
	if(serverMVTable[mvId].useCount == 0){
		serverMVTable[mvId].mvName = NULL;
		serverMVTable[mvId].mv = NULL;
		serverMVTable[mvId].size = -1;
	}
	strcpy(ack,"1\0");
	return 1;
}

/*
*	Function:  messageParser - Used by the server to find the syscall requested by the server
*/
int messageParser(int machineId, char* mess1){
	int returnVal;
	char ch = *mess1;
	mess1++;
	switch(ch){
		case 'a':	
					CreateServerLock(machineId,mess1);
					return 1;
					break;
		case 'b':	
					returnVal = AcquireLock(machineId,atoi(mess1));
					return returnVal;
					break;
		case 'c':	
					returnVal = ReleaseLock(machineId,atoi(mess1));
					return returnVal;
					break;
		case 'd':
					DeleteServerLock(machineId,atoi(mess1));
					return 1;
					break;
		case 'e':
					CreateServerCV(machineId,mess1);
					return 1;
					break;
		case 'f':
					returnVal = WaitServerCV(machineId,mess1);
					return returnVal;
					break;
		case 'g':
					returnVal = SignalServerCV(machineId,mess1);
					return returnVal;
					break;
		case 'h':
					returnVal = BroadcastServerCV(machineId,mess1);
					return returnVal;
					break;

		case 'i':	
					DeleteServerCV(machineId,atoi(mess1));
					return 1;
					break;
		case 'j': 
					CreateMV(machineId,mess1);
					return 1;
					break;
		case 'l':
					SetValue(machineId,mess1);
					return 1; 
					break;
		case 'k':
					GetValue(machineId,mess1);
					return 1; 
					break;
		case 'm':
					DeleteServerMV(machineId,atoi(mess1));
					return 1;
					break;
		default:	
					*ack = ch;
					strcpy(ack + 1,":SysCallNotFound\0");
					return 1;
	}
	strcpy(ack,"-2");
	return -1;
}

/*
*	Function:  MailTest - Server Main Function
*/
void
MailTest()
{
	int returnVal;
	printf("\nServer Started\n");
	while(true)
	{
		// Wait for the first message from the other machine
		printf("Server Waiting to receive\n");
		postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);							// Receive Message from the client
		printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
		fflush(stdout);
	
		returnVal = messageParser(inPktHdr.from,buffer);	// Check the request and create the appropriate response
		// This is done by the message parser which calls the appropriate functions
		// Send acknowledgement to the other machine (using "reply to" mailbox
		// in the message that just arrived
		if(returnVal == 1){
			outPktHdr.to = inPktHdr.from;										// Send the response to the client
			outMailHdr.to = 0;
			outMailHdr.from = 0;
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
