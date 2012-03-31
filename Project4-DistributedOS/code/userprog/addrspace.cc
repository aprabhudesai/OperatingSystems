// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "noff.h"
#include "table.h"
//#include "synch.h"

extern "C" { int bzero(char *, int); };

Table::Table(int s) : map(s), table(0), lock(0), size(s) {
    table = new void *[size];
    lock = new Lock("TableLock");
}

Table::~Table() {
    if (table) {
	delete table;
	table = 0;
    }
    if (lock) {
	delete lock;
	lock = 0;
    }
}

void *Table::Get(int i) {
    // Return the element associated with the given if, or 0 if
    // there is none.

    return (i >=0 && i < size && map.Test(i)) ? table[i] : 0;
}

int Table::Put(void *f) {
    // Put the element in the table and return the slot it used.  Use a
    // lock so 2 files don't get the same space.
    int i;	// to find the next slot

    lock->Acquire();
    i = map.Find();
    lock->Release();
    if ( i != -1)
	table[i] = f;
    return i;
}

void *Table::Remove(int i) {
    // Remove the element associated with identifier i from the table,
    // and return it.

    void *f =0;

    if ( i >= 0 && i < size ) {
	lock->Acquire();
	if ( map.Test(i) ) {
	    map.Clear(i);
	    f = table[i];
	    table[i] = 0;
	}
	lock->Release();
    }
    return f;
}

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void 
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}


//----------------------------------------------------------------------
// Function AddrSpace::DisplayBitMapOfThread
//  Deallocate physical pages of the entire process 
//----------------------------------------------------------------------

void AddrSpace::DisplayBitMapOfThread()
{
	pageTableLock -> Acquire();
	physicalPageBitMapLock -> Acquire();
	printf("\n----------------------------------------------------------\n");
	printf("\n	Page Table Entries For Process %d	\n", currentThread -> pID);
	printf("\n----------------------------------------------------------\n");
    for(unsigned int i=0; i< numPages ; i++)
	{
		printf("\nVirtual Page = %d 	Physical Page = %d\n",pageTable[i].virtualPage, pageTable[i].physicalPage);
	}
	physicalPageBitMapLock -> Acquire();
	pageTableLock -> Release();
}


//remove stack allocation on process ends its work
//----------------------------------------------------------------------
// Function AddrSpace::DeleteStackForThread
//  Deallocate physical pages of the entire process 
//----------------------------------------------------------------------

void AddrSpace::DeleteStackForThread(int StackVirtualPageNo){

	pageTableLock -> Acquire();
	physicalPageBitMapLock -> Acquire();
	int noOfStackPages = 8;
    while(noOfStackPages > 0) 
	{
		//bzero(pageTable[StackVirtualPageNo].physicalPage * PageSize,PageSize)
		if(pageTable[StackVirtualPageNo].valid == true)
		{
			physicalPageBitMap -> Clear(pageTable[StackVirtualPageNo].physicalPage);	// Clear the physical page to indicate that it is available for use
			pageTable[StackVirtualPageNo].valid == false;								// Set the valid bit to be false
		}
		StackVirtualPageNo--;					
		noOfStackPages--;
		numOfPhysPages--;				//decrease the number of physical pages that the process has been allocated
	}
	physicalPageBitMapLock -> Release();
	pageTableLock -> Release();
}

//----------------------------------------------------------------------
// Function AddrSpace::DeleteProcessPhysicalPages
//  Deallocate physical stack pages of the thread 
//----------------------------------------------------------------------


void AddrSpace::DeleteProcessPhysicalPages(){

	pageTableLock -> Acquire();
	physicalPageBitMapLock -> Acquire();
	int index = 0;
	int noOfTotalPages = numOfCodeDataPages;
    while(index < noOfTotalPages) 
	{
		//bzero(pageTable[index].physicalPage * PageSize,PageSize)
		if(pageTable[index].valid == true)
		{
			physicalPageBitMap -> Clear(pageTable[index].physicalPage);		// Clear the physical page to indicate that it is available for use
			pageTable[index].valid == false;								// set the valid bit to false
		}
		index++;
	}
	physicalPageBitMapLock -> Release();
	pageTableLock -> Release();
}

//----------------------------------------------------------------------
// Function AddrSpace::stackPageAllocation
//  Allocate physical pages to the thread and associate them to the 
//  corresponding virtual pages. Then add these entries to the page table
//  of the process. This function returns the page number of the last allocated 
//  page. This number is used to initialize the StackReg for the thread
//----------------------------------------------------------------------

int AddrSpace::stackPageAllocation()
{
	pageTableLock -> Acquire();
	physicalPageBitMapLock -> Acquire();
	if((numPages + 8) >= NumPhysPages)						// Error checking - if the maximum page limit is reached
	{
		printf("\nMaximum page limit reached\n");
		physicalPageBitMapLock -> Release();
		pageTableLock -> Release();
		interrupt -> Halt();								// Halt the system as the process can no longer execute
		return -1;
	}
	int j = 0;
	unsigned int i=0;
	newPageTable = new TranslationEntry[numPages+8];
	for(i=0; i< numPages ; i++)
	{
		newPageTable[i].virtualPage = pageTable[i].virtualPage;			//Create a new page table to copy the content of the current page table
		newPageTable[i].physicalPage = pageTable[i].physicalPage;		// this is done so that we can allocate stack pages dynamically
		newPageTable[i].valid = pageTable[i].valid;
		newPageTable[i].use = pageTable[i].use;
		newPageTable[i].dirty = pageTable[i].dirty;
		newPageTable[i].readOnly = pageTable[i].readOnly; 
	}
	
	while( j < 8)
	{
		int freePhysicalPage = physicalPageBitMap -> Find();			// Find a free physical page using bitmap
		newPageTable[pageTableIndex].virtualPage = pageTableIndex;
		newPageTable[pageTableIndex].physicalPage = freePhysicalPage;
		bzero(&(machine -> mainMemory[(newPageTable[pageTableIndex].physicalPage * PageSize)]), PageSize);		// Clear the main memory before allocating the new page for stack
		newPageTable[pageTableIndex].valid = true;
		newPageTable[pageTableIndex].use = false;
		newPageTable[pageTableIndex].dirty = false;
		newPageTable[pageTableIndex].readOnly = false;  // if the code segment was entirely on 
					// a separate page, we could set its 
					// pages to be read-only
		pageTableIndex++;
		numPages++;
		numOfPhysPages++;
		j++;
	}
	
	delete[] pageTable;
	pageTable = new TranslationEntry[numPages];
	pageTable = newPageTable;
	machine -> pageTable = newPageTable;				// make the newly created page table as the machine's page table
	machine -> pageTableSize = numPages;				// inform the machine about the new number of pages after stack allocation
	physicalPageBitMapLock -> Release();
	pageTableLock -> Release();
	return numPages;
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	"executable" is the file containing the object code to load into memory
//
//      It's possible to fail to fully construct the address space for
//      several reasons, including being unable to allocate memory,
//      and being unable to read key parts of the executable.
//      Incompletely consretucted address spaces have the member
//      constructed set to false.
//----------------------------------------------------------------------

AddrSpace::AddrSpace(OpenFile *executable) : fileTable(MaxOpenFiles) {
    
	processCounterLock -> Acquire();
	
	if(processIdCounter >= MAX_PROCESSES)
	{
		printf("\nMax process limit reached\n");
		processCounterLock -> Release();
		interrupt -> Halt();
	}
	/*else
	{
		processCounterLock -> Release();
	}*/
	NoffHeader noffH;
    unsigned int i, size;
    // Don't allocate the input or output to disk files
    fileTable.Put(0);
    fileTable.Put(0);

    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && 
		(WordToHost(noffH.noffMagic) == NOFFMAGIC))
    	SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);

    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size ;
	
	CodeSize = noffH.code.size;												// Store the size of the code segment
	
	DataSize = noffH.initData.size + noffH.uninitData.size;					// Store the size of the data segment
	
	numPages = divRoundUp(size, PageSize) + divRoundUp(UserStackSize,PageSize);		// calculate the number of pages required for the process
	
	numOfPhysPages = numPages;												// assign the total number of pages to the number of physical pages of the process. This variable is used for error checking
	
	numOfCodeDataPages = divRoundUp(size, PageSize);
	
    size = numPages * PageSize;

    DEBUG('a', "Initializing address space, num pages %d, size %d\n", 
					numPages, size);
	
// first, set up the translation 
	pageTableLock = new Lock("pageTableLock");
	pageTableLock -> Acquire();
    pageTable = new TranslationEntry[numPages];
	pageTableIndex = 0;
	physicalPageBitMapLock -> Acquire();
	
	int numOfFreePhysPages = physicalPageBitMap -> NumClear();						// check if the number of clear pages is less than the required number of pages
	if(numOfFreePhysPages - numPages < 0)											// if yes then report Error to the user and Halt the machine
	{
		printf("\nMax physical page limit reached - no more free pages available\n");
		physicalPageBitMapLock -> Release();
		interrupt -> Halt();
	}
	
    for (i = 0; i < numPages; i++) 
	{
		int freePhyPage = physicalPageBitMap -> Find();
		pageTable[pageTableIndex].virtualPage = i;						
		pageTable[pageTableIndex].physicalPage = freePhyPage;											// find a free physical for the corresponding virtual page
		bzero(&(machine -> mainMemory[(pageTable[pageTableIndex].physicalPage * PageSize)]), PageSize);
		pageTable[pageTableIndex].valid = true;
		pageTable[pageTableIndex].use = false;
		pageTable[pageTableIndex].dirty = false;
		pageTable[pageTableIndex].readOnly = false;  // if the code segment was entirely on 
					// a separate page, we could set its 
					// pages to be read-only
		pageTableIndex++;
	}
	physicalPageBitMapLock -> Release();

// We need to copy one page at a time to the machine memory

	for(i=0;i < numOfCodeDataPages;i++)
	{
			DEBUG('a', "Initializing code segment, at 0x%x, size %d\n", noffH.code.virtualAddr, noffH.code.size);		// copying the code + data page by page
			executable->ReadAt(&(machine->mainMemory[((pageTable[i].physicalPage) * PageSize)]),
			PageSize , noffH.code.inFileAddr);
			noffH.code.inFileAddr = noffH.code.inFileAddr + PageSize;
	}	
	pageTableLock -> Release();
	processCounterLock -> Release();
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
//
// 	Dealloate an address space.  release pages, page tables, files
// 	and file tables
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
    delete pageTable;
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);	

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
	//machine->WriteRegister(StackReg, topOfProcessStack - 16);
    DEBUG('a', "Initializing stack register to %x\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState() 
{}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
}
