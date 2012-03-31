// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include <stdio.h>
#include <iostream>
#include "synch.h"
#include "thread.h"
//#include "bitmap.h"

using namespace std;

#ifdef NETWORK
#include "network.h"
#include "post.h"
// Global Declarations for RPC's
PacketHeader outPktHdr1, inPktHdr1;	
MailHeader outMailHdr1, inMailHdr1;
char *request;
char response[MaxMailSize];

//-------------------------------------------------
//	Data structure to hold the lock information
//-------------------------------------------------
struct kernelLock
{
	Lock *systemCallLock;
	AddrSpace* addrSpace;
	bool isToBeDeleted;
};

//Table of Locks
kernelLock kernelLockTable[MAX_LOCKS];

//location in KernelLockTable for next Lock entry
static int nextLockLocation = 0;

//Lock for the kernel lock table to ensure mutually exclusive access
Lock *kernelLockTableLock = new Lock("kernelLockTableLock");

//-------------------------------------------------
//	Data structure to hold the Condition information
//-------------------------------------------------
/*  Kernel Table to hold Condition Variables used by Threads
and Address Space of the User Program to which the thread belongs */
struct kernelCV{
	Condition *systemCallCV;
	AddrSpace *addrspace;
	bool toBeDeleted;
};

/* Define Kernel CV Table for the struct declaration */
kernelCV kernelCVTable[MAX_CV];

/* location to store the next Condition Variable accessed in KernelCVTable	*/
static int nextCVLocation = 0;

/* Lock needed to provide mutual exclusive access to KernelCVTable	*/
Lock *kernelCVTableLock = new Lock("kernelCVTableLock");


//-------------------------------------------------
//	Data structure to hold the Monitor Variable information
//-------------------------------------------------
/*  Kernel Table to hold Monitor Variables used by Threads
and Address Space of the User Program to which the thread belongs */
struct kernelMV{
	int *systemCallMV;
	AddrSpace *addrSpace;
	bool toBeDeleted;
};

/* Define Kernel CV Table for the struct declaration */
kernelMV kernelMVTable[MAX_MV];

/* location to store the next Condition Variable accessed in KernelCVTable	*/
static int nextMVLocation = 0;

/* Lock needed to provide mutual exclusive access to KernelCVTable	*/
Lock *kernelMVTableLock = new Lock("kernelMVTableLock");

#endif

//-------------------------------------------------
//	Data structure to hold the semaphore information
//-------------------------------------------------
//Kernel Semaphore Entry Structure
struct kernelSemaphore
{
	Semaphore *systemCallSemaphore;
	AddrSpace* addrSpace;
	bool isToBeDeleted;
};

//Table of Semaphore Entries
kernelSemaphore kernelSemaphoreTable[MAX_SEM];

//This is the next location of the Semaphore in the lock table
static int nextSemaphoreLocation = 0;

//Lock for the kernel lock table to ensure mutually exclusive access
Lock *kernelSemaphoreTableLock = new Lock("kernelSemaphoreTableLock");

//--------------------------------------------------------------------


//----------------------------------------------------------------------
// Function addFIFOPage
//  Function to add the page currently in use to the back of the FIFO queue
//----------------------------------------------------------------------
int *valTemp;
void addFIFOPage(int pageToAdd)
{
	valTemp = new int;
	FIFOLock -> Acquire();
	*valTemp = pageToAdd;
	int *ptrToAdd = valTemp;
	FIFOQueue -> Append((void *) ptrToAdd);			// Add the selected page to the FIFO Queue - This happens only for the first 32 pages of the physical memory
	FIFOLock -> Release();
}

//----------------------------------------------------------------------
// Function getFIFOPage
//  Function find the page that can be replaced from the main memory - using FIFO
//----------------------------------------------------------------------
int getFIFOPage()
{
	int *ptr; 
	int pageToReplace = -1;
	FIFOLock -> Acquire();
	while(pageToReplace == -1)
	{
		ptr = (int *) FIFOQueue -> Remove();			// Select the page for eviction to be the first one in the queue
		pageToReplace = *ptr; 
		if(ipt[pageToReplace].use == true)
		{
			FIFOQueue -> Append((void *) ptr);
			pageToReplace = -1;
			continue;
		}
		else
			break;
	}
	FIFOLock -> Release();
	return pageToReplace;
}
//----------------------------------------------------------------------
// Function handleMemoryFull
//  Function find the page that can be replaced from the main memory - Can be using FIFO or RANDOM
//----------------------------------------------------------------------
int handleMemoryFull(int vp)
{	
	int pageToEvict = -1;
	//iptLock -> Acquire();
	currentThread -> space -> pageTableLock -> Acquire();
	if(currentThread -> space -> pageTable[vp].valid == true)		// This is to check if the page for which I got an IPT Miss has been loaded into the memory by some other thread
	{																		// If it is loaded then I have to search the IPT again for my page
		currentThread -> space -> pageTableLock -> Release();
		return -1;
	}
	else
	{
		currentThread -> space -> pageTableLock -> Release();
	}
	if(pageReplacementPolicy == 1)				// Check if the page replacement policy is RANDOM
	{
		iptLock -> Acquire();
		while(pageToEvict == -1)
		{
			pageToEvict = rand() % 32;			// Select a random page between 0 - 31
			if(ipt[pageToEvict].use == true || ipt[pageToEvict].valid == false)
			{
				pageToEvict = -1;				// If in use then so the page selection again
				continue;
			}
			else
			{
				ipt[pageToEvict].use = true;	// If not in use then set use bit to true to reserve it for use
				break;
			}
		}
		//iptLock -> Release();
	}
	else 			// By default the policy is FIFO
	{
		iptLock -> Acquire();
		pageToEvict = getFIFOPage();						// Get the page from the FIFO Queue
		ipt[pageToEvict].use = true;						// reserve it for use
		addFIFOPage(pageToEvict);							
	}
	IntStatus oldIntStatus = interrupt -> SetLevel(IntOff); 	// Setting the interupts off		
	for(int i=0; i < TLBSize; i++)					
	{
		if(machine -> tlb[i].physicalPage == pageToEvict && machine -> tlb[i].valid == true)		// Check if the page selected for eviction is in TLB and has dirty bit set to true
		{
			//Propagate the dirty bit to the IPT
			ipt[pageToEvict].dirty = machine -> tlb[i].dirty;			//propagate the dirty bit to IPT
			machine -> tlb[i].valid = false;							// Invalidate the TLB entry
			break;
		}
	}
	interrupt -> SetLevel(oldIntStatus);		
	int dirtyFlag = 0, myvpn, flag =0;
	AddrSpace *tempSpace;
	if(ipt[pageToEvict].dirty == true)
		dirtyFlag = 1;
	myvpn = ipt[pageToEvict].virtualPage;
	int pid = ipt[pageToEvict].processId;						// Get the process id of the process to which the page belongs to
		tempSpace = processTable[pid].space;					// Obtain the address space pointer to access the other processe's page table
		flag = 1;
	tempSpace -> pageTableLock -> Acquire();	
	iptLock -> Release();
	swapFileLock -> Acquire();
	if(dirtyFlag == 1)
	{
		int mySwapFileLocation = swapFileBitMap -> Find();
		swapFile -> WriteAt(&(machine->mainMemory[((pageToEvict) * PageSize)]), PageSize, mySwapFileLocation * PageSize);		// Write the page to swap file as it is dirty
		tempSpace -> pageTable[myvpn].swapByteOffset = mySwapFileLocation * PageSize;		// Update the byte offset for the page in the page table of the current process
		tempSpace -> pageTable[myvpn].diskLocation = 3;				// To indicate that the location of the page is in the swap file
	}
	tempSpace -> pageTable[myvpn].valid = false;
	swapFileLock -> Release();
	tempSpace -> pageTableLock -> Release();	
	return pageToEvict;
}

//----------------------------------------------------------------------
// Function handleIPTMiss
//  Function to handle the page faults for IPT - Finds a physical page from 
//	main memory and if found a free physical page then copies the data from executable / swap file into the page
//----------------------------------------------------------------------

int handleIPTMiss(int virtualpn)
{	
	currentThread -> space -> pageTableLock -> Acquire();
	if(currentThread -> space -> pageTable[virtualpn].valid == true)		// This is to check if the page for which I got an IPT Miss has been loaded into the memory by some other thread
	{																		// If it is loaded then I have to search the IPT again for my page
		currentThread -> space -> pageTableLock -> Release();
		return -1;
	}
	else
	{
		currentThread -> space -> pageTableLock -> Release();
	}
	iptLock -> Acquire();
	physicalPageBitMapLock -> Acquire();
	int freePhysicalPage = -1;
	freePhysicalPage = physicalPageBitMap -> Find();		// Find a free physical page
	if(freePhysicalPage != -1)
		ipt[freePhysicalPage].use = true;
	physicalPageBitMapLock -> Release();
	iptLock -> Release();				
	if(freePhysicalPage == -1)
	{
		freePhysicalPage = handleMemoryFull(virtualpn);				// Memory is full, so find a page for eviction
		if(freePhysicalPage == -1)
			return -1;
	}
	else if(pageReplacementPolicy == 0)
	{
		ipt[freePhysicalPage].use = true;
		addFIFOPage(freePhysicalPage);		// Add the page to FIFO queue
	}
	iptLock -> Acquire();				
	currentThread -> space -> pageTableLock -> Acquire();	
	if(currentThread -> space -> pageTable[virtualpn].diskLocation == 1)		// Check if the page is in the executable
	{
		currentThread -> space -> executable->ReadAt(&(machine->mainMemory[freePhysicalPage * PageSize]),PageSize , currentThread -> space -> pageTable[virtualpn].byteOffset);
	}
	if(currentThread -> space -> pageTable[virtualpn].diskLocation == 3)		// Check if the page is in the swap file
	{
		swapFile->ReadAt(&(machine->mainMemory[freePhysicalPage * PageSize]),PageSize , currentThread -> space -> pageTable[virtualpn].swapByteOffset);
		ipt[freePhysicalPage].dirty = false;
	}
	if(currentThread -> space -> pageTable[virtualpn].diskLocation == 2)		// Check if the page is not on disk (stack or uninit)
		ipt[freePhysicalPage].dirty = false;
	currentThread -> space -> pageTable[virtualpn].valid = true;					// Update the page table entry for the physical page
	currentThread -> space -> pageTable[virtualpn].physicalPage = freePhysicalPage;	
	ipt[freePhysicalPage].virtualPage = virtualpn;									// Update the IPT with the selected physical page
	ipt[freePhysicalPage].physicalPage = freePhysicalPage;							// along with the other details
	ipt[freePhysicalPage].valid = true;
	ipt[freePhysicalPage].readOnly = false;
	ipt[freePhysicalPage].processId = currentThread -> pID;							// Store the process ID to which the page has been given to
	currentThread -> space -> pageTableLock -> Release();
	iptLock -> Release();
	return freePhysicalPage;
}

//----------------------------------------------------------------------
// Function HandlePageFault
//  Function to handle the page faults - searches the IPT and if found loads the page into the TLB
//----------------------------------------------------------------------
void HandlePageFault()
{
	int physicalPage = -1;
	unsigned int badVaddr = machine->ReadRegister(BadVAddrReg);		// Find out the bad virtual address
	int vpn = badVaddr/ PageSize;
	while(physicalPage == -1)
	{
		iptLock -> Acquire();
		for(unsigned int i = 0; i < NumPhysPages ; i++)
		{
			if(ipt[i].valid == true && ipt[i].virtualPage == vpn && ipt[i].processId == currentThread -> pID && ipt[i].use == false)
			{
				physicalPage = i;				// Found the page in the IPT
				ipt[physicalPage].use = true;
				break;
			}
		}
	
		iptLock ->  Release();
		if(physicalPage == -1)
		{
			physicalPage = handleIPTMiss(vpn);		// Didnt find the page in the IPT
		}
	}
	IntStatus oldIntStatus = interrupt -> SetLevel(IntOff); 	// Setting the interupts off
	if(machine -> tlb[currentTLBLocation].valid == true && machine -> tlb[currentTLBLocation].dirty == true)
		ipt[machine -> tlb[currentTLBLocation].physicalPage].dirty = machine -> tlb[currentTLBLocation].dirty;	// Propagate the dirty bit to IPT
	machine -> tlb[currentTLBLocation].virtualPage = ipt[physicalPage].virtualPage;		
	machine -> tlb[currentTLBLocation].physicalPage = physicalPage;						// Update the TLB with the entries from the IPT
	machine -> tlb[currentTLBLocation].valid = true;
	machine -> tlb[currentTLBLocation].use = ipt[physicalPage].use;
	machine -> tlb[currentTLBLocation].readOnly = ipt[physicalPage].readOnly; 
	currentTLBLocation = (currentTLBLocation +1) % TLBSize;
	interrupt -> SetLevel(oldIntStatus);		// Restore Interrupts
	iptLock -> Acquire();
	ipt[physicalPage].use = false;		// set the use bit to false so that some other thread / process can use this
	iptLock -> Release();
}

int copyin(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes from the current thread's virtual address vaddr.
    // Return the number of bytes so read, or -1 if an error occors.
    // Errors can generally mean a bad virtual address was passed in.
    bool result;
    int n=0;			// The number of bytes copied in
    int *paddr = new int;

    while ( n >= 0 && n < len) {
      result = machine->ReadMem( vaddr, 1, paddr );
      while(!result) // FALL 09 CHANGES
	  {
   			result = machine->ReadMem( vaddr, 1, paddr ); // FALL 09 CHANGES: TO HANDLE PAGE FAULT IN THE ReadMem SYS CALL
	  }	
      
      buf[n++] = *paddr;
     
      if ( !result ) {
	//translation failed
	return -1;
      }

      vaddr++;
    }

    delete paddr;
    return len;
}

int copyout(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes to the current thread's virtual address vaddr.
    // Return the number of bytes so written, or -1 if an error
    // occors.  Errors can generally mean a bad virtual address was
    // passed in.
    bool result;
    int n=0;			// The number of bytes copied in

    while ( n >= 0 && n < len) {
      // Note that we check every byte's address
      result = machine->WriteMem( vaddr, 1, (int)(buf[n++]) );

      if ( !result ) {
	//translation failed
	return -1;
      }

      vaddr++;
    }

    return n;
}

void Create_Syscall(unsigned int vaddr, int len) {
    // Create the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  No
    // way to return errors, though...
    char *buf = new char[len+1];	// Kernel buffer to put the name in

    if (!buf) return;

    if( copyin(vaddr,len,buf) == -1 ) {
	printf("%s","Bad pointer passed to Create\n");
	delete buf;
	return;
    }

    buf[len]='\0';

    fileSystem->Create(buf,0);
    delete[] buf;
    return;
}

int Open_Syscall(unsigned int vaddr, int len) {
    // Open the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  If
    // the file is opened successfully, it is put in the address
    // space's file table and an id returned that can find the file
    // later.  If there are any errors, -1 is returned.
    char *buf = new char[len+1];	// Kernel buffer to put the name in
    OpenFile *f;			// The new open file
    int id;				// The openfile id

    if (!buf) {
	printf("%s","Can't allocate kernel buffer in Open\n");
	return -1;
    }

    if( copyin(vaddr,len,buf) == -1 ) {
	printf("%s","Bad pointer passed to Open\n");
	delete[] buf;
	return -1;
    }

    buf[len]='\0';

    f = fileSystem->Open(buf);
    delete[] buf;

    if ( f ) {
	if ((id = currentThread->space->fileTable.Put(f)) == -1 )
	    delete f;
	return id;
    }
    else
	return -1;
}

void Write_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one. For disk files, the file is looked
    // up in the current address space's open file table and used as
    // the target of the write.
    
    char *buf;		// Kernel buffer for output
    OpenFile *f;	// Open file for output

    if ( id == ConsoleInput) return;
    
    if ( !(buf = new char[len]) ) {
	printf("%s","Error allocating kernel buffer for write!\n");
	return;
    } else {
        if ( copyin(vaddr,len,buf) == -1 ) {
	    printf("%s","Bad pointer passed to to write: data not written\n");
	    delete[] buf;
	    return;
	}
    }

    if ( id == ConsoleOutput) {
      for (int ii=0; ii<len; ii++) {
	printf("%c",buf[ii]);
      }

    } else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	    f->Write(buf, len);
	} else {
	    printf("%s","Bad OpenFileId passed to Write\n");
	    len = -1;
	}
    }

    delete[] buf;
}

int Read_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one.    We reuse len as the number of bytes
    // read, which is an unnessecary savings of space.
    char *buf;		// Kernel buffer for input
    OpenFile *f;	// Open file for output

    if ( id == ConsoleOutput) return -1;
    
    if ( !(buf = new char[len]) ) {
	printf("%s","Error allocating kernel buffer in Read\n");
	return -1;
    }

    if ( id == ConsoleInput) {
      //Reading from the keyboard
      scanf("%s", buf);

      if ( copyout(vaddr, len, buf) == -1 ) {
	printf("%s","Bad pointer passed to Read: data not copied\n");
      }
    } else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	    len = f->Read(buf, len);
	    if ( len > 0 ) {
	        //Read something from the file. Put into user's address space
  	        if ( copyout(vaddr, len, buf) == -1 ) {
		    printf("%s","Bad pointer passed to Read: data not copied\n");
		}
	    }
	} else {
	    printf("%s","Bad OpenFileId passed to Read\n");
	    len = -1;
	}
    }

    delete[] buf;
    return len;
}

void Close_Syscall(int fd) {
    // Close the file associated with id fd.  No error reporting.
    OpenFile *f = (OpenFile *) currentThread->space->fileTable.Remove(fd);

    if ( f ) {
      delete f;
    } else {
      printf("%s","Tried to close an unopen file\n");
    }
}

#ifdef NETWORK

int SendMessageToServer()
{
	outPktHdr1.to = 0; //faraddr
	outPktHdr1.from = 1;
	outMailHdr1.to = 0;
    outMailHdr1.from = 0;
    outMailHdr1.length = strlen(request) + 1;
	bool success = postOffice -> Send(outPktHdr1, outMailHdr1, request);
	//printf("\nSending message to the server...\n");
	
    if ( !success ) 
	{
		printf("The postOfficeClient Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
		interrupt -> Halt();
		return -1;
    }
	else
	{
		return 0;
	}
}

void RecieveMessageFromServer()
{
	printf("\nIn Receive Message\n");
	//printf("\ninPktHdr1 = %d \n inPktHdr1 = %d\n", outPktHdr1, outMailHdr1);
	postOffice->Receive(0, &inPktHdr1, &inMailHdr1, response);
    printf("Got \"%s\" from %d, box %d\n",response,inPktHdr1.from,inMailHdr1.from);
    fflush(stdout);    
}

/*
*	Create Monitor System Call:
*   Creates a Monitor (interger Array) in the kernel monitor variable table
*/
int CreateMV_Syscall(unsigned int vaddr, int monitorLen, int monitorSize)
{
	char buffer[50];
	char *mvName;
	if(monitorLen < 0)
	{
		printf("\nYou have passed a negative value for the monitor variable length - Create Monitor Syscall\n");
		return -1;
	}
	
	//Create mvName character array to store the monitor variable name
	if ( !(mvName = new char[monitorLen+1]) ) {
		printf("\nError allocating kernel buffer for write - Create Monitor Variable Syscall\n");
		kernelMVTableLock -> Release();
		return -1;
	}
	else 
	{
		//get the name of the monitor variable name using the copyin function
		//in mvName character array
		mvName ="\0";
		if ( copyin(vaddr,monitorLen,mvName) == -1 )
		{
			printf("\nBad pointer passed to to write: data not written - Create Monitor Variable Syscall\n");
			delete[] mvName;
			kernelMVTableLock -> Release();
			return -1;
		}
	}

	//generate the message to be sent to the server
	sprintf(buffer,"j%s,%d",mvName,monitorSize);
	int mySize = strlen(buffer);
	request = new char[mySize];
	strcpy(request,buffer);
	
	int success = SendMessageToServer();	//send create lock request to server
	if(success == -1)
	{
		printf("Unable to send Create monitor variable request to server...\n");	
		return -1;
	}
	//printf("Sent Create monitor variable request to server...\n");
	
	//request sent successfully...Now wait for response
	
	RecieveMessageFromServer();
	
	int status = atoi (response);
	//if response is -1 then request failed
	if(status == -1)
	{
		printf("\nServer failed to create monitor variable\n");
		return -1;
	}
	return status;
}

/* 
*	Delete Condition Variable System Call : Deletes the condition variable corresponding to the id passed by the user
*/
int DeleteMV_Syscall(int mvID)
{
	char buffer[20];
	//check validity of mvId
	if(mvID < 1 || mvID > MAX_MV)
	{
		printf("\nYou have passed a negative value for the monitor variable Id - Delete Monitor Syscall\n");
		return -1;
	}
	
	//generate message to be sent to the server
	sprintf(buffer,"m%d",mvID);
	int mySize = strlen(buffer);
	request = new char[mySize];
	strcpy(request,buffer);
	
	int success = SendMessageToServer();	//send create lock request to server
	if(success == -1)
	{
		printf("Unable to send Delete monitor variable request to server...\n");	
		return -1;
	}
	//printf("Sent Delete monitor variable request to server...\n");
	
	//request sent successfully...Now wait for response
	
	RecieveMessageFromServer();

	int status = atoi (response);
	//if response is -1 then request failed
	if(status == -1)
	{
		printf("\nFailed to send Delete monitor variable request to server\n");
		return -1;
	}
	return status;	
}

/* 
*	Returns the value in Monitor variable at at given index
*/
//Message Format:-> MVid + , + index + \0
int GetMV_Syscall(int mvID, int index)
{
	char buffer[20];
	//check validity of mvId
	if(mvID < 1 || mvID > MAX_MV)
	{
		printf("\nInvalid Monitor Variable Id\n");
		return -1;
	}

	//generate message to be sent to the server
	sprintf(buffer,"k%d,%d",mvID,index);
	int mySize = strlen(buffer);
	request = new char[mySize];
	strcpy(request,buffer);

	int success = SendMessageToServer();	//send create lock request to server
	if(success == -1)
	{
		printf("Unable to send Get monitor variable request to server...\n");	
		return -1;
	}
	//printf("Sent Get monitor variable request to server...\n");
	
	//request sent successfully...Now wait for response
	
	RecieveMessageFromServer();
	
	int status = atoi (response);
	//if response is -1 then request failed
	if(status == -1)
	{
		printf("\nFailed to Get monitor variable value from server\n");
		return -1;
	}

	return status;
}

/* 
*	Returns the value in Monitor variable at at given index
*/
//Message Format:-> MVid + , + index + , + value + \0
int SetMV_Syscall(int mvID, int index, int value)
{
	char buffer[20];
	//check validity of mvId
	if(mvID < 1 || mvID > MAX_MV)
	{
		printf("\nInvalid Monitor Variable Id\n");
		return -1;
	}
	//generate message to be sent to the server
	sprintf(buffer,"l%d,%d,%d",mvID,index, value);
	int mySize = strlen(buffer);
	request = new char[mySize];
	strcpy(request,buffer);

	int success = SendMessageToServer();	//send create lock request to server
	if(success == -1)
	{
		printf("Unable to send Set monitor variable request to server...\n");	
		return -1;
	}
	//printf("Sent Set monitor variable request to server...\n");
	
	//request sent successfully...Now wait for response
	
	RecieveMessageFromServer();
	
	int status = atoi (response);
	//if response is -1 then request failed
	if(status == -1)
	{
		printf("\nFailed to Set monitor variable value\n");
		return -1;
	}

	return status;
}

/*
*	Create Lock System Call:
*   Creates a lock in the kernel lock table
*/
int CreateLock_Syscall(unsigned int vaddr, int lockLen)
{
	char *lockName;
	char buffer[50];
	//check the validity of lock length
	if(lockLen < 1 || lockLen > 40)
	{
		printf("\nYou have passed a invalid value for the Lock length - Create Lock Syscall\n");
		return -1;
	}

	//Create lockName character array to store the lock name
	if ( !(lockName = new char[lockLen+1]) ) {
		printf("\nError allocating kernel buffer for write - Create Lock Syscall\n");
		return -1;
	}
	else 
	{
		//get the name of the lock name using the copyin function
		//in lockName character array
		lockName = "\0";
		if ( copyin(vaddr,lockLen,lockName) == -1 ) 
		{
			printf("\nBad pointer passed to to write: data not written - Create Lock Syscall\n");
			delete[] lockName;
			return -1;
		}
	}
	//generate message to be sent to the server
	sprintf(buffer, "a%s", lockName);
	int mySize = strlen(buffer);
	request = new char[mySize];
	strcpy(request,buffer);
	
	int success = SendMessageToServer();	//send create lock request to server
	if(success == -1)
	{
		printf("Unable to send Create lock request to server...\n");	
		return -1;
	}
	//printf("Sent Create lock request to server...\n");
	
	//request sent successfully...Now wait for response
	
	RecieveMessageFromServer();
	
	int status = atoi (response);
	
	//if response is -1 then request failed
	if(status == -1)
	{
		printf("\nServer failed to create lock\n");
		return -1;
	}

	return status;
}

/* 
*	Create Condition Variable System Call: 
*	Creates a condition variable in the kernel CV table	
*/
int CreateCV_Syscall(unsigned int vaddr, int CVLen)
{
	char *CVName;
	char buffer[50];
	//Validate the CV Name length provided by used in function call
	if(CVLen < 1){
		printf("\nYou have passed a negative value for the Condition Variable length - Create CV Syscall\n");
		return -1;
	}
	//Create character array to store the CV name
	if ( !(CVName = new char[CVLen+1]) ) {
		printf("\nError allocating kernel buffer for CreateCV System Call\n");
		return -1;
	}
	else{
		//get the name of CV in the newly created array
		CVName = "\0";
		if ( copyin(vaddr,CVLen,CVName) == -1 ) {
			printf("\nBad pointer passed to to CreateCV: data not written - Create CV Syscall\n");
			delete[] CVName;
			return -1;
		}
	}
	//generate message to be sent to the server
	sprintf(buffer, "e%s",CVName);
	int mySize = strlen(buffer);
	request = new char[mySize];
	strcpy(request,buffer);
	
	int success = SendMessageToServer();	//send create lock request to server
	if(success == -1)
	{
		printf("Unable to send Create condition request to server...\n");	
		return -1;
	}
	//printf("Sent Create condition request to server...\n");
	
	//request sent successfully...Now wait for response
	
	RecieveMessageFromServer();
	//if response is -1 then request failed
	int status = atoi (response);
	
	if(status == -1)
	{
		printf("\nServer failed to create condition\n");
		return -1;
	}

	return status;
}

/*
*	Acquire Lock System Call: 
*	Acquires the lock using the lock id passed by the user
*/
int Acquire_Syscall(int lockID)
{
	char buffer[20];
	//check the validity of lockId
	if(lockID < 1 || lockID > MAX_LOCKS)
	{
		printf("\nYou have passed a invalid value for the Lock Id - Acquire Lock Syscall\n");
		return -1;
	}
	//generate message to be sent to the server
	sprintf(buffer,"b%d",lockID);
	int mySize = strlen(buffer);
	request = new char[mySize];
	strcpy(request,buffer);

	int success = SendMessageToServer();	//send create lock request to server
	if(success == -1)
	{
		printf("Unable to send Acquire lock request to server...\n");	
		return -1;
	}
	//printf("Sent Acquire lock request to server...\n");
	
	//request sent successfully...Now wait for response
	
	RecieveMessageFromServer();
	
	int status = atoi (response);
	
	//if response is -1 then request failed
	if(status == -1)
	{
		printf("\nServer failed to acquire lock\n");
		return -1;
	}

	return status;
}

/*
*	Release Lock System Call: 
*	Releases the lock using the lock id passed by the user
*/
int Release_Syscall(int lockID)
{
	char buffer[20];
	//check the validity of lockId
	if(lockID < 1 || lockID > MAX_LOCKS)
	{
		printf("\nYou have passed a invalid value for the Lock Id - Release Lock Syscall\n");
		return -1;
	}
	//generate message to be sent to the server
	sprintf(buffer,"c%d",lockID);
	int mySize = strlen(buffer);
	request = new char[mySize];
	strcpy(request,buffer);

	
	int success = SendMessageToServer();	//send create lock request to server
	if(success == -1)
	{
		printf("Unable to send Release lock message to server...\n");	
		return -1;
	}
	//printf("Sent Release lock message to server...\n");
	
	//request sent successfully...Now wait for response
	
	RecieveMessageFromServer();
	
	int status = atoi (response);
	//if response is -1 then request failed
	if(status == -1)
	{
		printf("\nServer failed to release lock\n");
		return -1;
	}

	return status;
}

/* 
*	Wait System Call: 
*	Calls a wait on the condition variable id passed by the user 
*/
//Format : SyscallId(f) cvId, lockId
int Wait_Syscall(int cvID, int lockID)
{
	char buffer[20];
	//check the validity of lockId and cvId
	if(cvID < 1 || cvID > MAX_CV)
	{
		printf("\nYou have passed a negative value for the Condition Id - Wait Syscall\n");
		return -1;
	}
	if(lockID < 1 || lockID > MAX_LOCKS)
	{
		printf("\nYou have passed a negative value for the Lock Id - Wait Syscall\n");
		return -1;
	}

	//generate message to be sent to the server
	sprintf(buffer, "f%d,%d", cvID, lockID);
	int mySize = strlen(buffer);
	request = new char[mySize];
	strcpy(request,buffer);

	
	int success = SendMessageToServer();	//send create lock request to server
	if(success == -1)
	{
		printf("Unable to send Wait request to server...\n");	
		return -1;
	}
	//printf("Sent Wait request to server...\n");
	
	//request sent successfully...Now wait for response
	
	RecieveMessageFromServer();
	
	int status = atoi (response);
	//if response is -1 then request failed
	if(status == -1)
	{
		printf("\nServer failed to Wait for the given condition\n");
		return -1;
	}

	return status;
}

/* 
*	Signal System Call : 
*	Calls a signal on the condition variable id passed by the user
*/

//Format : SyscallId(g) cvId, lockId
int Signal_Syscall(int cvID, int lockID)
{
	char buffer[20];
	//check the validity of lockId and cvId
	if(cvID < 1 || cvID > MAX_CV)
	{
		printf("\nYou have passed a negative value for the Condition Id - Signal Syscall\n");
		return -1;
	}
	if(lockID < 1 || lockID > MAX_LOCKS)
	{
		printf("\nYou have passed a negative value for the Lock Id - Signal Syscall\n");
		return -1;
	}

	//generate message to be sent to the server
	sprintf(buffer, "g%d,%d", cvID, lockID);
	int mySize = strlen(buffer);
	request = new char[mySize];
	strcpy(request,buffer);
	
	int success = SendMessageToServer();	//send create lock request to server
	if(success == -1)
	{
		printf("Unable to send Signal request to server...\n");	
		return -1;
	}
	//printf("Sent Signal request to server...\n");
	
	//request sent successfully...Now wait for response
	
	RecieveMessageFromServer();
	
	int status = atoi (response);
	
	//if response is -1 then request failed
	if(status == -1)
	{
		printf("\nServer failed to Signal for the given condition\n");
		return -1;
	}
	return status;
}


/* 
*	Broadcast System Call : 
*	Calls a broadcast on the condition variable id passed by the user
*/
//Format : SyscallId(h) cvId, lockId
int Broadcast_Syscall(int cvID, int lockID)
{
	char buffer[20];
	//check the validity of lockId and cvId
	if(cvID < 1 || cvID > MAX_CV)
	{
		printf("\nYou have passed a negative value for the Condition Id - Broadcast Syscall\n");
		return -1;
	}
	if(lockID < 1 || lockID > MAX_LOCKS)
	{
		printf("\nYou have passed a negative value for the Lock Id - Broadcast Syscall\n");
		return -1;
	}

	//generate message to be sent to the server
	sprintf(buffer, "h%d,%d", cvID, lockID);
	int mySize = strlen(buffer);
	request = new char[mySize];
	strcpy(request,buffer);

	
	int success = SendMessageToServer();	//send create lock request to server
	if(success == -1)
	{
		printf("Unable to send Broadcast request to server...\n");	
		return -1;
	}
	//printf("Sent Broadcast request to server...\n");
	
	//request sent successfully...Now wait for response
	
	RecieveMessageFromServer();
	
	int status = atoi (response);
	
	//if response is -1 then request failed
	if(status == -1)
	{
		printf("\nServer failed to Broadcast for the given condition\n");
		return -1;
	}
	return status;
}

/*
*	Delete Lock Syscall: 
*	Deletes the lock corresponding to the id passed by the user
*/
int DeleteLock_Syscall(int lockID)
{
	char buffer[20];
	//check the validity of lockId
	if(lockID < 1 || lockID > MAX_LOCKS )
	{
		printf("\nYou have entered Invalid Lock Id\n");
		return -1;
	}
	//generate message to be sent to the server
	sprintf(buffer,"d%d",lockID);
	int mySize = strlen(buffer);
	request = new char[mySize];
	strcpy(request,buffer);

	int success = SendMessageToServer();	//send create lock request to server
	if(success == -1)
	{
		printf("Unable to send Delete Lock request to server...\n");	
		return -1;
	}
	//printf("Sent Delete Lock request to server...\n");
	
	RecieveMessageFromServer();
	
	int status = atoi (response);
	//if response is -1 then request failed
	if(status == -1)
	{
		printf("\nServer failed to Delete Lock\n");
		return -1;
	}

	return status;
}

/* 
*	Delete Condition Variable System Call : Deletes the condition variable corresponding to the id passed by the user
*/

int DeleteCV_Syscall(int cvID)
{
	char buffer[20];
	//check the validity of cvId
	if(cvID < 1 || cvID > MAX_CV)
	{
		printf("\nYou have passed a negative value for the Condition Id - Delete Condition Syscall\n");
		return -1;
	}
	
	//generate message to be sent to the server
	sprintf(buffer, "i%d", cvID);
	int mySize = strlen(buffer);
	request = new char[mySize];
	strcpy(request,buffer);
	
	int success = SendMessageToServer();	//send create lock request to server
	if(success == -1)
	{
		printf("Unable to send Delete condition request to server...\n");	
		return -1;
	}
	//printf("Sent Delete condition request to server...\n");
	
	//request sent successfully...Now wait for response
	
	RecieveMessageFromServer();
	
	int status = atoi (response);
	//if response is -1 then request failed
	if(status == -1)
	{
		printf("\nServer failed to delete condition\n");
		return -1;
	}
	return status;
}
#endif

/*
*	kernel_thread: This function is called by Fork Syscall to actually create a child thread
*/
void kernel_thread(unsigned int vaddr)
{	
	machine->WriteRegister(PCReg, vaddr);
	machine->WriteRegister(NextPCReg, vaddr + 4);
	(currentThread->space) -> RestoreState();
	machine->WriteRegister(StackReg, (processTable[currentThread -> pID].childStackPageLocation[currentThread -> cID] + 1) * PageSize - 16);	// load stack reg with the stack location
    machine->Run();			// jump to the user progam
}

/*
*	Fork System Call: This system call creates a new thread (child) in the same address space as that of the process
*					  and allocates a new stack for the thread
*/
int Fork_Syscall(unsigned int vaddr)
{
	//Acquire processtable lock for mutual exclusion
	processTableLock -> Acquire();	
	int cID = -1,pID =-1;
	//check if the virtual address passed is valid or not
	if(vaddr < 0 || (vaddr > currentThread -> space -> CodeSize))
	{
		printf("\nThe Virtual Address Passed for the function is invalid\n");
		processTableLock -> Release();
		return -1;
	}
	pID = currentThread -> pID;
	//check if more threads can be created in this process
	if(processTable[pID].noOfThreads >= MAX_CHILD)
	{
		printf("\nThe maximum number of threads per process is reached\n");
		processTableLock -> Release();
		return -1;
	}
	//create new thread
	Thread *childThread;
	if( !(childThread= new Thread("NewChildThead")))
	{
		printf("\nError creating thread in Fork System Call\n");
		processTableLock -> Release();
		return -1;
	}
	//assign parent thread address space to child thread
	childThread -> space = currentThread -> space;
	int myStackTopVirtualPage = (childThread -> space) -> stackPageAllocation(); // allocate stack for the new thread
	if(myStackTopVirtualPage == -1)
	{
		printf("\nError creating stack for the thread\n");
		delete childThread;
		processTableLock -> Release();
		return -1;
	}
	processTable[pID].noOfThreads++;	//increase the no of threads in the process
	cID = processTable[pID].childIdCounter;		// assign id to the child
	processTable[pID].childIdCounter++;		// increase the child id counter for the next child (when its created)
	childThread -> cID = cID;	// Give the child its id
	childThread -> pID = pID;	// Give the child the id for accessing process table
	processTable[childThread -> pID].childStackPageLocation[childThread -> cID] = myStackTopVirtualPage - 1;	// store the location of the first stack page
	processTableLock -> Release();
	childThread -> Fork((VoidFunctionPtr)kernel_thread, vaddr);	// call a kernel function which calls nachos Fork system call
	return 1;
}

/*
*	exec_thread: This is function which is used by Exec Syscall to actually fork a process
*/
void exec_thread(int Nouse)
{
	(currentThread -> space) -> InitRegisters();		// set the initial register values
    (currentThread -> space) -> RestoreState();		// load page table register	
	machine -> Run();			// jump to the user progam
}

/*
*	Exec System Call: This system call creates a new process and allocated it a new address space
*/
int Exec_Syscall(unsigned int vaddr)
{
	//Acquire processtable lock for mutual exclusion
	processTableLock -> Acquire();
	char *fileName;
	int outLength = 100;
	//create new address space for this process
	AddrSpace *space;
	//check if space is available
	if ( !(fileName = new char[outLength]) ) {
		printf("%s","Error allocating kernel buffer in Exec\n");
		processTableLock -> Release();
		return -1;
    }
	bool result;
    int n=0, index1 = -1;			// The number of bytes copied in
	do{
		index1++;
		copyin(vaddr + index1, 1, fileName + index1);
	}while(*(fileName + index1) != '\0' && index1 < outLength);
	OpenFile *executable = fileSystem -> Open(fileName);
	//open the executable file return error if it is NULL
	if (executable == NULL) 
	{
		printf("Unable to open file %s\n", fileName);
		processTableLock -> Release();
		return -1;
    }
    space = new AddrSpace(executable);	
	processCounterLock -> Acquire();	//acuire processcounterlock for mutual exclusion
	//check if a new process can be created or not
	if(processIdCounter >= MAX_PROCESSES)
	{
		printf("\nMax process limit reached\n");
		space -> DeleteStackForThread((space -> numPages) - 1);
		space -> DeleteProcessPhysicalPages();
		delete space;
		processCounterLock -> Release();
		processTableLock -> Release();
		return -1;
	}
	//create new thread 
	Thread *processThread;
	if(!(processThread= new Thread("ProcessThread")))
	{
		printf("\nError creating process thread\n");
		space -> DeleteStackForThread((space -> numPages) - 1);
		space -> DeleteProcessPhysicalPages();
		delete space;
		processCounterLock -> Release();
		processTableLock -> Release();
		return -1;
	}
	//allocated new address space to the process thread
	processThread -> space = space;
    //delete executable;			// close file
	int pid = processIdCounter;	// assign process id to the newly created process
	int cid;
	processIdCounter++;	// increment value for the next process
	totalNumOfProcesses++;
	processTable[pid].pid = pid;	// store the process id in the process table
	processTable[pid].isAlive = true;
	processTable[pid].space = space;	// store the address space in the process table
	processTable[pid].noOfThreads=1;	// increment the no of threads (this will be one as this is the first thread of the process)
	processTable[pid].childStackPageLocation[processTable[pid].childIdCounter] = (space -> numPages) - 1;
	cid = processTable[pid].childIdCounter;
	processTable[pid].childIdCounter++;	// make the child id counter for the process as 0
	processThread -> pID = pid;			// assign process id to the newly created thread
	processThread -> cID = cid;
	processCounterLock -> Release();
	processTableLock -> Release();
	processThread -> Fork(exec_thread,10);	// call the exec thread function which will actually exec the new process
	return pid;
}

/*
*	Exit System Call: This system call clears the address space and the stack allocated to a process
*					  If the current processs is the last process then it deletes all the system resources
					  and halts the machine
*/
void Exit_Syscall(unsigned int status)
{
	//acquire processtable lcok for mutual exclusion
	processTableLock -> Acquire();
	int cid = currentThread -> cID;
	int pid = currentThread -> pID;
	int myStackLocation;
	processTable[pid].noOfThreads--;
	// check if this is the last thread of the process
	//printf("\nResult = %d\n",status);
	if(processTable[pid].noOfThreads == 0 || (processTable[pid].noOfWaitingThreads - processTable[pid].noOfThreads == 0))
	{
		//printf("\n\t** cid %d of pid %d Delete Physical Pages\n",cid,pid);
		currentThread -> space -> DeleteProcessPhysicalPages();
		delete currentThread -> space -> executable;
		processTable[pid].pid = 0;
		processTable[pid].space = NULL;
		processTable[pid].childIdCounter = 0;
		processTable[pid].isAlive = false;
		processCounterLock -> Acquire();
		totalNumOfProcesses--;
		if(totalNumOfProcesses == 0)
		{	
			//printf("\nNo of Waiting Threads %d\n",processTable[pid].noOfWaitingThreads);
			//printf("\n\t** cid %d pid %d Delete Last Process Stack Pages\n",cid,pid);
			myStackLocation = processTable[pid].childStackPageLocation[cid];	// for deleting the stack
			currentThread -> space -> DeleteStackForThread(myStackLocation);		// of the current thread
			processCounterLock -> Release();
			processTableLock -> Release();
			printf("\n %s is the last thread in ALL the processes - Thus Halting the System\n",currentThread -> name);
			interrupt -> Halt();
		}
		processCounterLock -> Release();
	}
	//printf("\n\t** cid %d of pid %d Delete Stack Pages\n",cid,pid);
	myStackLocation = processTable[pid].childStackPageLocation[cid];	// for deleting the stack
	currentThread -> space -> DeleteStackForThread(myStackLocation);		// of the current thread
	// check if this is last thread of the last process
	processTableLock -> Release();
	currentThread -> Finish();
}

/*
*	Yield System Call : this yields the current thread
*/
void Yield_Syscall()
{
	currentThread -> Yield();
}

/*
*Function used by user program to print any string to the console
*/
int WriteToConsole_Syscall(unsigned int vaddr, int arg1, int arg2, int arg3)
{
	char *buffer;
	int outLength = 400;
	if ( !(buffer = new char[outLength]) ) {
		printf("%s","Error allocating kernel buffer in Write To Console\n");
		return -1;
    }
	//-------------
	bool result;
    int n=0;
    unsigned int index1 = 0;			// The number of bytes copied in
	do{
		copyin(vaddr, 1, buffer + index1);
		vaddr++;
		index1++;
		if(index1 >= 400 && buffer[index1 -1] != '\0')
		{
			printf("\nThe length of string passed to Write To Console is greater than the Max limit\n");
			delete[] buffer;
			return -1;
		}
	}while((*(buffer + (index1 -1))) != '\0');
	
	// logic to get the arguments if user passes more than 3 arguments in the syscall
	int resultarray[6];
	int index = arg3 -1;
	int len = arg3;
	if(arg2 == COMBINED_PARAM)
	{
		while(len != 0)
		{
			resultarray[index] = arg1 % 100;
			arg1 /= 100;
			index--;
			len--;
		}
	}
	//copyin(vaddr, arg1, buffer);
	//-------------
	int argNum = 1, val = 0, index2 = 0, resultIndex = 0;
	//while(buffer[index] != '\0')
	while( buffer[index2] != '\0')
	{
		if(( buffer[index2] == '%') && ( buffer[index2+1] == 'd'))
		{
			if(arg2 == COMBINED_PARAM)
			{
				val = resultarray[resultIndex];
				resultIndex++;
			}
			else
			{
				if(argNum == 1)
					val = arg1;
				if(argNum == 2)
					val = arg2;
				if(argNum == 3)
					val = arg3;
				argNum++;	
			}
			printf("%d",val);
			index2++; index2++;
		}	
		else{
			printf("%c",buffer[index2]);
			index2++;
		}
	}
	//fflush(stdout);
	delete[] buffer;
	//Write_Syscall(vaddr,arg1,ConsoleOutput);
		fflush(stdout);
	return 1;
}

/*
*	Down System Call: This is used to call down on a semaphore
*/
int Down_Syscall(int semID)
{
	//acquire kernelSemaphoreTableLock for mutual exclusion
	kernelSemaphoreTableLock -> Acquire();
	if(semID >= MAX_SEM || semID < 0){
		printf("\nThe Sem ID provided is out of bounds - Down Semaphore Syscall\n");
		kernelSemaphoreTableLock -> Release();
		return -1;
	}
	//check if the address space is not null
	if(kernelSemaphoreTable[semID].addrSpace == NULL)
	{
		printf("\nThe semaphore does not exist - Down Semaphore Syscall\n");
		kernelSemaphoreTableLock -> Release();
		return -1;
	}
	//check if the thread has same addr space as that of semaphore
	if(currentThread -> space != kernelSemaphoreTable[semID].addrSpace){
		printf("\nYou are in the different address space than that of the Semaphore - Down Semaphore Syscall\n");
		kernelSemaphoreTableLock -> Release();
		return -1;
	}
	kernelSemaphoreTableLock -> Release();
	kernelSemaphoreTable[semID].systemCallSemaphore -> P();
	return 1;
}

/*
*	Up System Call: This is used to call up on a semaphore
*/
int Up_Syscall(int semID)
{
	//acquire kernelSemaphoreTableLock for mutual exclusion
	kernelSemaphoreTableLock -> Acquire();
	if(semID >= MAX_SEM || semID < 0){
		printf("\nThe Sem ID provided is out of bounds - Up Semaphore Syscall\n");
		kernelSemaphoreTableLock -> Release();
		return -1;
	}
	//check if the address space is not null
	if(kernelSemaphoreTable[semID].addrSpace == NULL)
	{
		printf("\nThe semaphore does not exist - - Up Semaphore Syscall\n");
		kernelSemaphoreTableLock -> Release();
		return -1;
	}
	//check if the thread has same addr space as that of semaphore
	if(currentThread -> space != kernelSemaphoreTable[semID].addrSpace){
		printf("\nYou are in the different address space than that of the Semaphore - Up Semaphore Syscall\n");
		kernelSemaphoreTableLock -> Release();
		return -1;
	}
	kernelSemaphoreTableLock -> Release();
	kernelSemaphoreTable[semID].systemCallSemaphore -> V();
	return 1;
}

/*
*	CreateSem System Call: This is used to create a semaphore
*/
int CreateSem_Syscall(unsigned int vaddr, int semaphoreLen, int initValue)
{
	//acquire kernelSemaphoreTableLock for mutual exclusion
	kernelSemaphoreTableLock -> Acquire();
	char *semaphoreName;
	//validate the length of semaphore
	if(semaphoreLen < 0)
	{
		printf("\nYou have passed a nevative value for the length - Create Semaphore Syscall\n");
		kernelSemaphoreTableLock -> Release();
		return -1;
	}
	//allocate space for it
	if ( !(semaphoreName = new char[semaphoreLen]) ) {
		printf("%s","Error allocating kernel buffer for write - Create Semaphore Syscall\n");
		kernelSemaphoreTableLock -> Release();
		return -1;
	}
	else 
	{
		if ( copyin(vaddr,semaphoreLen,semaphoreName) == -1 ) 
		{
			printf("%s","Bad pointer passed to to write: data not written - - Create Semaphore Syscall\n");
			delete[] semaphoreName;
			kernelSemaphoreTableLock -> Release();
			return -1;
		}
	}
	//check if it is allowed to create a new semaphore
	if(nextSemaphoreLocation >= MAX_SEM)
	{
		printf("\nThe MAX Number of Semaphore limit is exceeded - Create Semaphore Syscall\n");
		delete[] semaphoreName;
		kernelSemaphoreTableLock -> Release();
		return -1;
	}
	//create semaphore here
	kernelSemaphoreTable[nextSemaphoreLocation].systemCallSemaphore = new Semaphore(semaphoreName, initValue);
	//assign the address space of thread to semaphore
	kernelSemaphoreTable[nextSemaphoreLocation].addrSpace = currentThread -> space;
	int returnValue = nextSemaphoreLocation;
	nextSemaphoreLocation++;
	kernelSemaphoreTableLock -> Release();
	return (returnValue); // return a handle to the semaphore - the semaphore id
}

/*
*	DeleteSem System Call: This is used to delete a semaphore
*/
int DeleteSem_Syscall(int semID)
{
	//acquire kernelSemaphoreTableLock for mutual exclusion
	kernelSemaphoreTableLock -> Acquire();
	//check if the semaphore ID passed is valid or not
	if(semID >= MAX_SEM || semID < 0)
	{
		printf("\nThe semaphore id provided is out of bounds - Delete Semaphore Syscall\n");
		kernelSemaphoreTableLock -> Release();
		return -1;
	}
	//check that the addr space of semaphore with that Id is not null
	if(kernelSemaphoreTable[semID].addrSpace == NULL)
	{
		printf("\nThe Semaphore does not exist - Delete Semaphore Syscall\n");
		kernelSemaphoreTableLock -> Release();
		return -1;
	}
	// check if the thread belongs to the same address space
	if(currentThread -> space != kernelSemaphoreTable[semID].addrSpace)
	{
		printf("\nYou are in a different address space than that of the Semaphore - Delete Semaphore Syscall\n");
		kernelSemaphoreTableLock -> Release();
		return -1;
	}
	//mark semaphore to be deleted
	if((kernelSemaphoreTable[semID].systemCallSemaphore -> isQueueEmpty()) == false)
	{
		kernelSemaphoreTable[semID].isToBeDeleted = true;
		kernelSemaphoreTableLock -> Release();
		return 1;
	}
	else
	{
		kernelSemaphoreTable[semID].isToBeDeleted = false;
		kernelSemaphoreTable[semID].systemCallSemaphore -> Semaphore::~Semaphore();
		kernelSemaphoreTable[semID].addrSpace = NULL;
		kernelSemaphoreTable[semID].systemCallSemaphore = NULL;
	}
	kernelSemaphoreTableLock -> Release();
	return 1; // return true if Semaphore deleted
}

/*
*	Random System Call: Generates a random value based on the seed provided by the user and then returns that generated value to the user
*/
int Random_Syscall(int randomRange)
{
	//compute a readom value between 0 and value passed in randomRange
	int randomValue;
	randomValue = rand()%randomRange;
	return randomValue;		
}

/*
*	DisplayBitMap System Call: Displays the current allocation of pages to a process
*/
void DisplayBitMap_Syscall()
{
	currentThread -> space -> DisplayBitMapOfThread();
}

/*
*	ReadInput System Call: This is used to read a input provided by user on the console
*/
int ReadInput_Syscall()
{
	//scan the value from console
	int retval;
	scanf("%d",&retval);
	return retval;
}

void ExceptionHandler(ExceptionType which) {
    
	int type = machine->ReadRegister(2); // Which syscall?
    int rv=0; 	// the return value from a syscall

    if ( which == SyscallException ) {
	switch (type) {
	    default:
		DEBUG('a', "Unknown syscall - shutting down.\n");
	    case SC_Halt:
		DEBUG('a', "Shutdown, initiated by user program.\n");
		interrupt->Halt();
		break;
	    case SC_Create:
		DEBUG('a', "Create syscall.\n");
		Create_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
	    case SC_Open:
		DEBUG('a', "Open syscall.\n");
		rv = Open_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
		break;
	    case SC_Write:
		DEBUG('a', "Write syscall.\n");
		Write_Syscall(machine->ReadRegister(4),
			      machine->ReadRegister(5),
			      machine->ReadRegister(6));
		break;
	    case SC_Read:
		DEBUG('a', "Read syscall.\n");
		rv = Read_Syscall(machine->ReadRegister(4),
			      machine->ReadRegister(5),
			      machine->ReadRegister(6));
		break;
	    case SC_Close:
		DEBUG('a', "Close syscall.\n");
		Close_Syscall(machine->ReadRegister(4));
		break;

		case SC_Fork:
		DEBUG('a', "Fork syscall.\n");
		rv = Fork_Syscall(machine->ReadRegister(4));
		break;
		
		case SC_Yield:
		DEBUG('a', "Yield syscall.\n");
		Yield_Syscall();
		break;
		
		#ifdef NETWORK

		case SC_CreateLock:
		DEBUG('a', "CreateLock syscall.\n");
		rv = CreateLock_Syscall(machine->ReadRegister(4), 
								machine->ReadRegister(5));
		break;
		
		case SC_CreateCV:
		DEBUG('a', "CreateCV syscall.\n");
		rv = CreateCV_Syscall(machine->ReadRegister(4), 
								machine->ReadRegister(5));
		break;
		
		case SC_Acquire:
		DEBUG('a', "Acquire syscall.\n");
		rv = Acquire_Syscall(machine->ReadRegister(4));
		break;
		
		case SC_CreateMV:
		DEBUG('a', "Create Monitor Variable syscall.\n");
		rv = CreateMV_Syscall(machine->ReadRegister(4),
								machine->ReadRegister(5),
								machine->ReadRegister(6));
		break;
		
		case SC_DeleteMV:
		DEBUG('a', "Delete Monitor Variable syscall.\n");
		rv = DeleteMV_Syscall(machine->ReadRegister(4));
		break;		
		
		
		
		case SC_SetMV:
		DEBUG('a', "Set Monitor Variable syscall.\n");
		rv = SetMV_Syscall(machine->ReadRegister(4), 
							machine->ReadRegister(5), 
							machine->ReadRegister(6));
		break;		
		
		case SC_GetMV:
		DEBUG('a', "Get Monitor Variable syscall.\n");
		rv = GetMV_Syscall(machine->ReadRegister(4),
							machine->ReadRegister(5));
		break;		
		
		case SC_Release:
		DEBUG('a', "Release syscall.\n");
		rv = Release_Syscall(machine->ReadRegister(4));
		break;
		
		case SC_Wait:
		DEBUG('a', "Waite syscall.\n");
		rv = Wait_Syscall(machine->ReadRegister(4),
						  machine->ReadRegister(5));
		break;
		
		case SC_Signal:
		DEBUG('a', "Signal syscall.\n");
		rv = Signal_Syscall(machine->ReadRegister(4),
							machine->ReadRegister(5));
		break;
		
		case SC_Broadcast:
		DEBUG('a', "Broadcast syscall.\n");
		rv = Broadcast_Syscall(machine->ReadRegister(4),
							   machine->ReadRegister(5));
		break;
		
		case SC_DeleteLock:
		DEBUG('a', "DeleteLock syscall.\n");
		rv = DeleteLock_Syscall(machine->ReadRegister(4));
		break;
		
		case SC_DeleteCV:
		DEBUG('a', "DeleteCV syscall.\n");
		rv = DeleteCV_Syscall(machine->ReadRegister(4));
		break;
		#endif
		
		case SC_Exec:
		DEBUG('a', "Exec syscall.\n");
		rv = Exec_Syscall(machine->ReadRegister(4));
		break;
		
		case SC_Exit:
		DEBUG('a', "Exit syscall.\n");
		Exit_Syscall(machine->ReadRegister(4));
		break;
		
		case SC_WriteToConsole:
		DEBUG('a', "WriteToConsole syscall.\n");
		rv = WriteToConsole_Syscall(machine->ReadRegister(4),
								machine->ReadRegister(5),
								machine->ReadRegister(6),
								machine->ReadRegister(7));
		break;
		
		case SC_Down:
		DEBUG('a', "Down syscall.\n");
		rv = Down_Syscall(machine->ReadRegister(4));
		break;
		
		case SC_Up:
		DEBUG('a', "Up syscall.\n");
		rv = Up_Syscall(machine->ReadRegister(4));
		break;
		
		case SC_CreateSem:
		DEBUG('a', "Create Semaphore syscall.\n");
		rv = CreateSem_Syscall(machine->ReadRegister(4),machine->ReadRegister(5),machine->ReadRegister(6));
		break;
		
		case SC_DeleteSem:
		DEBUG('a', "Delete Semaphore syscall.\n");
		rv = DeleteSem_Syscall(machine->ReadRegister(4));
		break;
		
		case SC_Random:
		DEBUG('a', "Random syscall.\n");
		rv = Random_Syscall(machine->ReadRegister(4));
		break;
		
		case SC_DisplayBitMap:
		DEBUG('a', "DisplayBitMap syscall.\n");
		DisplayBitMap_Syscall();
		break;
		
		case SC_ReadInput:
		DEBUG('a', "ReadInput syscall.\n");
		rv = ReadInput_Syscall();
		break;
	}

	// Put in the return value and increment the PC
	machine->WriteRegister(2,rv);
	machine->WriteRegister(PrevPCReg,machine->ReadRegister(PCReg));
	machine->WriteRegister(PCReg,machine->ReadRegister(NextPCReg));
	machine->WriteRegister(NextPCReg,machine->ReadRegister(PCReg)+4);
	return;
    } else if( which == PageFaultException)
	{
		HandlePageFault();		// Call the page fault handler as the required page is not in the TLB
		return;
	}
	
	else {
      cout<<"Unexpected user mode exception - which:"<<which<<"  type:"<< type<<endl;
      interrupt->Halt();
    }
}
