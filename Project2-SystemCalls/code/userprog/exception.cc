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

//-------------------------------------------------
//	Data structure to hold the lock information
//-------------------------------------------------

struct kernelLock
{
	Lock *systemCallLock;
	AddrSpace* addrSpace;
	bool isToBeDeleted;
};

kernelLock kernelLockTable[MAX_LOCKS];
static int nextLockLocation = 0;	//This is the location of the lock in the lock table

//Lock for the kernel lock table to ensure mutually exclusive access
Lock *kernelLockTableLock;

//--------------------------------------------------

/*  Kernel Table to hold Condition Variables used by Threads
and Address Space of the User Program to which the thread belongs */
struct kernelCV{
	Condition *systemCallCV;
	AddrSpace *addrspace;
	bool toBeDeleted;
};
/* Define Kernel CV Table for the struct declaration */
kernelCV kernelCVTable[MAX_SYSTEM_CV];
/* location to store the next Condition Variable accessed in KernelCVTable	*/
static int nextCVLocation = 0;


/* Lock needed to provide mutual exclusive access to KernelCVTable	*/
Lock *kernelCVTableLock = NULL;

//----------------------------------------------------



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

int CreateLock_Syscall(unsigned int vaddr, int lockLen)
{
	char *lockName;
	
	if(lockLen < 0)
	{
		printf("\nYou have passed a nevative value for the length!!!\n");
		return -1;
	}
	
	if ( !(lockName = new char[lockLen]) ) {
		printf("%s","Error allocating kernel buffer for write!\n");
		return -1;
	}
	else 
	{
		if ( copyin(vaddr,lockLen,lockName) == -1 ) 
		{
			printf("%s","Bad pointer passed to to write: data not written\n");
			delete[] lockName;
			return -1;
		}
	}
	/*for(int i=0;i < lockLen; i++)
	{
		*(lockName + i) = (char *) (vaddr + i); 
	}
	*(lockName + i) = '\0';
	*/
	kernelLockTableLock -> Acquire();
	
	if(nextLockLocation >= MAX_LOCKS)
	{
		printf("\nThe MAX Number of lock limit is exceeded!!!\n");
		kernelLockTableLock -> Release();
		return -1;
	}
	
	kernelLockTable[nextLockLocation].systemCallLock = new Lock(lockName);
	kernelLockTable[nextLockLocation].addrSpace = currentThread -> space;
	nextLockLocation++;
	kernelLockTableLock -> Release();
	return (nextLockLocation -1); // return a handle to the lock - the lock id
}

/* Create Condition Variable System Call	*/
int CreateCV_Syscall(unsigned int vaddr, int CVLen)
{
	kernelCVTableLock -> Acquire();
	
	char *CVName;
	if(CVLen <= 0){
		printf("%s","Condition Variable Name Length error - CreateCV System Call\n");
		kernelCVTableLock -> Release();
		return -1;
	}
	if ( !(CVName = new char[CVLen]) ) {
		printf("%s","Error allocating kernel buffer for CreateCV System Call!\n");
		kernelCVTableLock -> Release();
		return -1;
	}
	else{
		if ( copyin(vaddr,CVLen,CVName) == -1 ) {
			printf("%s","Bad pointer passed to to CreateLock: data not written\n");
			delete[] CVName;
			kernelCVTableLock -> Release();
			return -1;
		}
	}
	
	if(nextCVLocation == MAX_SYSTEM_CV){
		printf("%s","Kernel out of Memory - CreateCV System Call\n");
		delete[] CVName;
		kernelCVTableLock -> Release();
		return -1;
	}
	
	kernelCVTable[nextCVLocation].systemCallCV = new Condition(CVName);
	kernelCVTable[nextCVLocation].addrspace = currentThread -> space;
	nextCVLocation++;
	kernelCVTableLock -> Release();
	return (nextCVLocation - 1);
}


int Acquire_Syscall(int lockID)
{
	if(lockID >= MAX_LOCKS)
	{
		printf("\nThe lock id provided is out of bounds\n");
		return -1;
	}
	
	kernelLockTableLock -> Acquire();
	
	if(kernelLockTable[lockID].addrSpace == NULL)
	{
		printf("\nThe lock does not exist\n");
		kernelLockTableLock -> Release();
		return -1;
	}
	
	// chech if the thread belongs to the same address space
	if(currentThread -> space != kernelLockTable[lockID].addrSpace)
	{
		printf("\nYou are in a different address space than that of the lock\n");
		kernelLockTableLock -> Release();
		return -1;
	}
	
	kernelLockTable[lockID].systemCallLock -> Acquire();
	kernelLockTableLock -> Release();
	return 1; // return true if lock acquired
}

int Release_Syscall(int lockID)
{
	if(lockID >= MAX_LOCKS)
	{
		printf("\nThe lock id provided is out of bounds\n");
		return -1;
	}
	
	kernelLockTableLock -> Acquire();
	
	if(kernelLockTable[lockID].addrSpace == NULL)
	{
		printf("\nThe lock does not exist\n");
		kernelLockTableLock -> Release();
		return -1;
	}
	
	// chech if the thread belongs to the same address space
	if(currentThread -> space != kernelLockTable[lockID].addrSpace)
	{
		printf("\nYou are in a different address space than that of the lock\n");
		kernelLockTableLock -> Release();
		return -1;
	}
	
	kernelLockTable[lockID].systemCallLock -> Release();
	kernelLockTableLock -> Release();
	return 1; // return true if lock released
}

/* Condition Variable Wait System Call */
int Wait_Syscall(int cvID, int lockID){
	
	if(lockID >= MAX_LOCKS){
		printf("%s","\nThe lock id provided is out of bounds - WaitCV System Call\n");
		return -1;
	}
	if(cvID >= MAX_SYSTEM_CV){
		printf("%s","Wait CV System Call out of bound parameter\n");
		return -1;
	}
	
	kernelCVTableLock -> Acquire();
	kernelLockTableLock -> Acquire();	
    
	if(kernelLockTable[lockID].addrSpace == NULL){
		printf("%s","\nThe Lock requested Does not exist - Wait CV System Call\n");
		kernelLockTableLock -> Release();
		kernelCVTableLock -> Release();
		return -1;
	}
	if(kernelCVTable[cvID].addrspace == NULL){
		printf("%s","Wait CV System Call : Condition Variable Not available\n");
		kernelLockTableLock -> Release();
		kernelCVTableLock -> Release();
		return -1;
	}
	if(currentThread -> space != kernelLockTable[lockID].addrSpace){
		printf("%s","\nIllegal Lock Access - Wait CV System Call\n");
		kernelLockTableLock -> Release();
		kernelCVTableLock -> Release();
		return -1;
	}
	if(kernelCVTable[cvID].addrspace != currentThread -> space){
		printf("%s","Wait CV System Call Illegal parameter Access\n");
		kernelLockTableLock -> Release();
		kernelCVTableLock -> Release();
		return -1;
	}
	kernelLockTableLock -> Release();
	kernelCVTableLock -> Release();
	kernelCVTable[cvID].systemCallCV -> Wait(kernelLockTable[lockID].systemCallLock);
	return 1;
}

/* Condition Variable Signal System Call */
int Signal_Syscall(int cvID, int lockID){
	
	if(lockID >= MAX_LOCKS){
		printf("%s","\nThe lock id provided is out of bounds - WaitCV System Call\n");
		return -1;
	}
	if(cvID >= MAX_SYSTEM_CV){
		printf("%s","Signal CV System Call out of bound parameter\n");
		return -1;
	}
	
	kernelCVTableLock -> Acquire();
	kernelLockTableLock -> Acquire();	
    
	if(kernelLockTable[lockID].addrSpace == NULL){
		printf("%s","\nThe Lock requested Does not exist - Signal CV System Call\n");
		kernelLockTableLock -> Release();
		kernelCVTableLock -> Release();
		return -1;
	}
	if(kernelCVTable[cvID].addrspace == NULL){
		printf("%s","Signal CV System Call : Condition Variable Not available\n");
		kernelLockTableLock -> Release();
		kernelCVTableLock -> Release();
		return -1;
	}
	if(currentThread -> space != kernelLockTable[lockID].addrSpace){
		printf("%s","\nIllegal Lock Access - Signal CV System Call\n");
		kernelLockTableLock -> Release();
		kernelCVTableLock -> Release();
		return -1;
	}
	if(kernelCVTable[cvID].addrspace != currentThread -> space){
		printf("%s","Signal CV System Call Illegal parameter Access\n");
		kernelLockTableLock -> Release();
		kernelCVTableLock -> Release();
		return -1;
	}
	kernelLockTableLock -> Release();
	kernelCVTableLock -> Release();
	kernelCVTable[cvID].systemCallCV -> Signal(kernelLockTable[lockID].systemCallLock);
	return 1;
}

/* Condition Variable Broadcast System Call */
int Broadcast_Syscall(int cvID, int lockID){
	
	if(lockID >= MAX_LOCKS){
		printf("%s","\nThe lock id provided is out of bounds - WaitCV System Call\n");
		return -1;
	}
	if(cvID >= MAX_SYSTEM_CV){
		printf("%s","Broadcast CV System Call out of bound parameter\n");
		return -1;
	}
	
	kernelCVTableLock -> Acquire();
	kernelLockTableLock -> Acquire();	
    
	if(kernelLockTable[lockID].addrSpace == NULL){
		printf("%s","\nThe Lock requested Does not exist - Broadcast CV System Call\n");
		kernelLockTableLock -> Release();
		kernelCVTableLock -> Release();
		return -1;
	}
	if(kernelCVTable[cvID].addrspace == NULL){
		printf("%s","Broadcast CV System Call : Condition Variable Not available\n");
		kernelLockTableLock -> Release();
		kernelCVTableLock -> Release();
		return -1;
	}
	if(currentThread -> space != kernelLockTable[lockID].addrSpace){
		printf("%s","\nIllegal Lock Access - Broadcast CV System Call\n");
		kernelLockTableLock -> Release();
		kernelCVTableLock -> Release();
		return -1;
	}
	if(kernelCVTable[cvID].addrspace != currentThread -> space){
		printf("%s","Broadcast CV System Call Illegal parameter Access\n");
		kernelLockTableLock -> Release();
		kernelCVTableLock -> Release();
		return -1;
	}
	kernelLockTableLock -> Release();
	kernelCVTableLock -> Release();
	kernelCVTable[cvID].systemCallCV -> Broadcast(kernelLockTable[lockID].systemCallLock);
	return 1;
}

int DeleteLock_Syscall(int lockID)
{
	if(lockID >= MAX_LOCKS)
	{
		printf("\nThe lock id provided is out of bounds\n");
		return -1;
	}
	
	kernelLockTableLock -> Acquire();
	
	if(kernelLockTable[lockID].addrSpace == NULL)
	{
		printf("\nThe lock does not exist\n");
		kernelLockTableLock -> Release();
		return -1;
	}
	
	// chech if the thread belongs to the same address space
	if(currentThread -> space != kernelLockTable[lockID].addrSpace)
	{
		printf("\nYou are in a different address space than that of the lock\n");
		kernelLockTableLock -> Release();
		return -1;
	}
	
	if((kernelLockTable[lockID].systemCallLock -> isQueueEmpty()) == false)
	{
		kernelLockTable[lockID].isToBeDeleted = true;
		kernelLockTableLock -> Release();
		return 1;
	}
	else
	{
		kernelLockTable[lockID].isToBeDeleted = false;
		kernelLockTable[lockID].systemCallLock -> Lock::~Lock();
		kernelLockTable[lockID].addrSpace = NULL;
		kernelLockTable[lockID].systemCallLock = NULL;
	}
	kernelLockTableLock -> Release();
	return 1; // return true if lock deleted
}

/* Delete Condition Variable System Call */
int DeleteCV_Syscall(int cvID){
	
	kernelCVTableLock -> Acquire();
	//Check if cvID is within max Limits
	if(cvID >= MAX_SYSTEM_CV){
		printf("%s","Delete CV System Call out of bound parameter\n");
		kernelCVTableLock -> Release();
		return -1;
	}
	//Check if the Condition Variable user accessing is already deleted or not created
	if(kernelCVTable[cvID].addrspace == NULL){
		printf("%s","Delete CV System Call : Condition Variable Not available\n");
		kernelCVTableLock -> Release();
		return -1;
	}
	//Check if the user program Thread is the owner of the CV
	if(kernelCVTable[cvID].addrspace != currentThread -> space){
		printf("%s","Delete CV System Call Illegal parameter Access\n");
		kernelCVTableLock -> Release();
		return -1;
	}
	//Check before deleting if there are threads waiting in the wait Queue of the CV
	if((kernelCVTable[cvID].systemCallCV -> isQueueEmpty()) == false){
		printf("%s","To be deleted set to true as wait queue is not empty - Delete CV System Call\n");
		kernelCVTable[cvID].toBeDeleted = true;
		kernelCVTableLock -> Release();
		return 1;
	}
	//if none of the above then the CV is deleted
	kernelCVTable[cvID].addrspace = NULL;
	kernelCVTable[cvID].toBeDeleted = false;
	kernelCVTable[cvID].systemCallCV -> Condition::~Condition();
	kernelCVTable[cvID].systemCallCV = NULL;
	kernelCVTableLock -> Release();
	return 1;
}

void kernel_thread(unsigned int vaddr){
	
	int myStackTopVirtualPage = (currentThread -> space) -> stackPageAllocation();
	
	processTable[currentThread -> pID].childThreadTable[currentThread -> cID].myStackVirtualPage = myStackTopVirtualPage - 1;
	
	machine->WriteRegister(PCReg, vaddr);
	machine->WriteRegister(NextPCReg, vaddr + 4);
	
	(currentThread->space) -> RestoreState();
	
	machine->WriteRegister(StackReg, myStackTopVirtualPage * PageSize - 16);
	
    machine->Run();			// jump to the user progam
    //ASSERT(FALSE);

}

void Fork_Syscall(unsigned int vaddr){
	Thread *childThread = new Thread("NewChildThead");

	int cID = -1,pID =-1;
	childThread -> space = currentThread -> space;
	pID = currentThread -> pID;
	
	processTableLock -> Acquire();
	
	cID = processTable[pID].noOfChildProc;
	processTable[pID].noOfChildProc++;
	
	processTableLock -> Release();
	
	childThread -> cID = cID;
	childThread -> pID = pID;

	childThread -> Fork((VoidFunctionPtr)kernel_thread, vaddr);
}

void exec_thread(int Nouse){
	
	(currentThread -> space) -> InitRegisters();		// set the initial register values
    (currentThread -> space) -> RestoreState();		// load page table register
	
	machine -> Run();			// jump to the user progam
	//ASSERT(FALSE);
}


int Exec_Syscall(unsigned int vaddr)
{
	char *fileName;
	int outLength = 100;
	
	AddrSpace *space;
	
	if ( !(fileName = new char[outLength]) ) {
		printf("%s","Error allocating kernel buffer in Exec\n");
		return -1;
    }
	//-------------
	bool result;
    int n=0, index1 = -1;			// The number of bytes copied in
    
	do{
		index1++;
		copyin(vaddr + index1, 1, fileName + index1);
	}while(*(fileName + index1) != '\0' && index1 < outLength);
	
	OpenFile *executable = fileSystem -> Open(fileName);
	
	if (executable == NULL) {
	printf("Unable to open file %s\n", fileName);
	return -1;
    }
    space = new AddrSpace(executable);
    
	Thread *processThread = new Thread("ProcessThread");
	
	processThread -> space = space;
	
    delete executable;			// close file
	
	processIdCounterLock -> Acquire();
	int pid = processIdCounter;
	processIdCounter++;
	processIdCounterLock -> Release();
	
	processTableLock -> Acquire();
	processTable[pid].pID = pid;
	processTable[pid].space = space;
	processTable[pid].noOfChildProc++;
	processTableLock -> Release();
	
	processThread -> pID = pid;
	machine -> WriteRegister(2,pid);
	processThread -> Fork(exec_thread,10);
	return pid;
}

void Exit_Syscall(unsigned int status)
{
	currentThread -> Finish();
}

void Yield_Syscall()
{
	currentThread -> Yield();
}

// Function used by user program to print any string to the console
int WriteToConsole_Syscall(unsigned int vaddr, int arg1, int arg2, int arg3)
{
	char *buffer;
	int outLength = 100;
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
	}while((*(buffer + (index1 -1))) != '\0');
	
	
	//copyin(vaddr, arg1, buffer);
	//-------------
	int argNum = 1, val = 0, index2 = 0;
	//while(buffer[index] != '\0')
	while( buffer[index2] != '\0')
	{
		if(( buffer[index2] == '%') && ( buffer[index2+1] == 'd'))
		{
			if(argNum == 1)
				val = arg1;
			if(argNum == 2)
				val = arg2;
			if(argNum == 3)
				val = arg3;
			argNum++;	
			printf("%d",val);
			index2++; index2++;
		}	
		else{
			printf("%c",buffer[index2]);
			index2++;
		}
	}
	fflush(stdout);
	delete[] buffer;
	
	//Write_Syscall(vaddr,arg1,ConsoleOutput);
		fflush(stdout);
	
	return 1;
}

void ExceptionHandler(ExceptionType which) {
    
	kernelLockTableLock = new Lock("kernelLockTableLock");
	kernelCVTableLock = new Lock("kernelCVTableLock");
	
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
		
		//-------------------------------------------------
		case SC_Fork:
		DEBUG('a', "Fork syscall.\n");
		Fork_Syscall(machine->ReadRegister(4));
		break;
		
		case SC_Yield:
		DEBUG('a', "Yield syscall.\n");
		Yield_Syscall();
		break;
		
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
		
		case SC_Exec:
		DEBUG('a', "Exec syscall.\n");
		Exec_Syscall(machine->ReadRegister(4));
		break;
		
		case SC_Exit:
		DEBUG('a', "Exit syscall.\n");
		Exit_Syscall(machine->ReadRegister(4));
		break;
		
		case SC_WriteToConsole:
		DEBUG('a', "WriteToConsole syscall.\n");
		WriteToConsole_Syscall(machine->ReadRegister(4),
								machine->ReadRegister(5),
								machine->ReadRegister(6),
								machine->ReadRegister(7));
		break;
		//-------------------------------------------------
	}

	// Put in the return value and increment the PC
	machine->WriteRegister(2,rv);
	machine->WriteRegister(PrevPCReg,machine->ReadRegister(PCReg));
	machine->WriteRegister(PCReg,machine->ReadRegister(NextPCReg));
	machine->WriteRegister(NextPCReg,machine->ReadRegister(PCReg)+4);
	return;
    } else {
      cout<<"Unexpected user mode exception - which:"<<which<<"  type:"<< type<<endl;
      interrupt->Halt();
    }
}
