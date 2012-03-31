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
//#include "pagetable.h"
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
//  Deallocate physical pages of the entire process - It clears the main memory 
//	and also invalidates the IPT
//----------------------------------------------------------------------

void AddrSpace::DeleteStackForThread(int StackVirtualPageNo){
	
	iptLock -> Acquire();
	physicalPageBitMapLock -> Acquire();
	pageTableLock -> Acquire();
	int noOfStackPages = 8;
	if(pageReplacementPolicy != 1)
	{
		FIFOLock -> Acquire();
		while(noOfStackPages > 0) 
		{
			if(pageTable[StackVirtualPageNo].valid == true)		// Set the valid bit to be false
			{
				if((ipt[pageTable[StackVirtualPageNo].physicalPage].valid == true) && (ipt[pageTable[StackVirtualPageNo].physicalPage].processId == currentThread -> pID) && (ipt[pageTable[StackVirtualPageNo].physicalPage].use == false))
				{
					physicalPageBitMap -> Clear(pageTable[StackVirtualPageNo].physicalPage);	// Clear the physical page to indicate that it is available for use
					ipt[pageTable[StackVirtualPageNo].physicalPage].valid = false;
					ipt[pageTable[StackVirtualPageNo].physicalPage].dirty = false;
					ipt[pageTable[StackVirtualPageNo].physicalPage].use = false;
					ipt[pageTable[StackVirtualPageNo].physicalPage].virtualPage = -1;
					ipt[pageTable[StackVirtualPageNo].physicalPage].processId = -1;
					for(int i = 0;i < NumPhysPages ; i++)
					{
						int *ptr; 
						if(!(FIFOQueue -> IsEmpty()))
						{
							ptr = (int *) FIFOQueue -> Remove();			// Select the page for eviction to be the first one in the queue
							int pageToReplace = *ptr; 
							if(pageToReplace != pageTable[StackVirtualPageNo].physicalPage)
							{
								FIFOQueue -> Append((void *) ptr);			// Add the selected page to the FIFO Queue if I am not clearing this page
							}												// This is done so that the FIFO Queue will have only those pages that are getting used
						}
					}
				}
				pageTable[StackVirtualPageNo].valid == false;
			}
			StackVirtualPageNo--;					
			noOfStackPages--;
			numOfPhysPages--;				//decrease the number of physical pages that the process has been allocated
		}
		FIFOLock -> Release();
	}
	else
	{
		while(noOfStackPages > 0) 
		{
			if(pageTable[StackVirtualPageNo].valid == true)		// Set the valid bit to be false
			{
				if((ipt[pageTable[StackVirtualPageNo].physicalPage].valid == true) && (ipt[pageTable[StackVirtualPageNo].physicalPage].processId == currentThread -> pID) && (ipt[pageTable[StackVirtualPageNo].physicalPage].use == false))
				{
					physicalPageBitMap -> Clear(pageTable[StackVirtualPageNo].physicalPage);	// Clear the physical page to indicate that it is available for use
					ipt[pageTable[StackVirtualPageNo].physicalPage].valid = false;
					ipt[pageTable[StackVirtualPageNo].physicalPage].dirty = false;
					ipt[pageTable[StackVirtualPageNo].physicalPage].use = false;
					ipt[pageTable[StackVirtualPageNo].physicalPage].virtualPage = -1;
					ipt[pageTable[StackVirtualPageNo].physicalPage].processId = -1;
				}
				pageTable[StackVirtualPageNo].valid == false;
			}
			StackVirtualPageNo--;					
			noOfStackPages--;
			numOfPhysPages--;				//decrease the number of physical pages that the process has been allocated
		}
	}
	pageTableLock -> Release();
	physicalPageBitMapLock -> Release();
	iptLock -> Release();
}

//----------------------------------------------------------------------
// Function AddrSpace::DeleteProcessPhysicalPages
//  Deallocate physical stack pages of the thread - It clears the main memory page 
//	and also invalidates the IPT
//----------------------------------------------------------------------

void AddrSpace::DeleteProcessPhysicalPages(){
	iptLock -> Acquire();
	physicalPageBitMapLock -> Acquire();
	int index = 0;
	if(pageReplacementPolicy != 1)
	{
		FIFOLock -> Acquire();
		for(index = 0; index < NumPhysPages ; index++)
		{	
			if(ipt[index].valid == true && ipt[index].processId == currentThread -> pID && (ipt[index].use == false))
			{
				physicalPageBitMap -> Clear(index);		// Clear the physical page to indicate that it is available for use
				ipt[index].valid = false;
				ipt[index].dirty = false;
				ipt[index].use = false;
				ipt[index].virtualPage = -1;
				ipt[index].processId = -1;
				for(int i = 0;i < NumPhysPages ; i++)
				{
					int *ptr; 
					if(!(FIFOQueue -> IsEmpty()))
					{
						ptr = (int *) FIFOQueue -> Remove();			// Select the page for eviction to be the first one in the queue
						int pageToReplace = *ptr; 
						if(pageToReplace != ipt[index].physicalPage)
						{
							FIFOQueue -> Append((void *) ptr);			// Add the selected page to the FIFO Queue - This is done so that if the page is not mine then it is getting used 
						}												// by some other process an thus has to be in the FIFO Queue
					}
				}
			}
		}
		FIFOLock -> Release();
	}
	else
	{
		for(index = 0; index < NumPhysPages ; index++)
		{	
			if(ipt[index].valid == true && ipt[index].processId == currentThread -> pID && (ipt[index].use == false))
			{
				physicalPageBitMap -> Clear(index);		// Clear the physical page to indicate that it is available for use
				ipt[index].valid = false;
				ipt[index].dirty = false;
				ipt[index].use = false;
				ipt[index].virtualPage = -1;
				ipt[index].processId = -1;
			}
		}
	}
	physicalPageBitMapLock -> Release();
	iptLock -> Release();
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
	int j = 0;
	unsigned int i=0;
	newPageTable = new pageTableEntry[numPages+8];
	for(i=0; i< numPages ; i++)
	{
		newPageTable[i].virtualPage = pageTable[i].virtualPage;			//Create a new page table to copy the content of the current page table
		newPageTable[i].physicalPage = pageTable[i].physicalPage;		// this is done so that we can allocate stack pages dynamically
		newPageTable[i].valid = pageTable[i].valid;
		newPageTable[i].use = pageTable[i].use;
		newPageTable[i].dirty = pageTable[i].dirty;
		newPageTable[i].readOnly = pageTable[i].readOnly; 
		newPageTable[i].diskLocation = pageTable[i].diskLocation;
		newPageTable[i].byteOffset = pageTable[i].byteOffset;
	}
	while( j < 8)
	{
		newPageTable[pageTableIndex].virtualPage = pageTableIndex;
		newPageTable[pageTableIndex].physicalPage = -1;
		bzero(&(machine -> mainMemory[(newPageTable[pageTableIndex].physicalPage * PageSize)]), PageSize);		// Clear the main memory before allocating the new page for stack
		newPageTable[pageTableIndex].valid = false;
		newPageTable[pageTableIndex].use = false;
		newPageTable[pageTableIndex].dirty = false;
		newPageTable[pageTableIndex].readOnly = false;  // if the code segment was entirely on 
					// a separate page, we could set its 
					// pages to be read-only
		newPageTable[i].diskLocation = 2;
		pageTableIndex++;
		numPages++;
		numOfPhysPages++;
		j++;
	}
	delete[] pageTable;
	pageTable = new pageTableEntry[numPages];
	pageTable = newPageTable;
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

AddrSpace::AddrSpace(OpenFile *exec) : fileTable(MaxOpenFiles) {
    
	processCounterLock -> Acquire();
	executable = exec;
	if(processIdCounter >= MAX_PROCESSES)
	{
		printf("\nMax process limit reached\n");
		pageTableLock -> Release();
		processCounterLock -> Release();
		interrupt -> Halt();
	}
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
	numOfCodeInitDataPages = divRoundUp(noffH.code.size + noffH.initData.size , PageSize);
    size = numPages * PageSize;
    DEBUG('a', "Initializing address space, num pages %d, size %d\n", 
					numPages, size);
	pageTableLock = new Lock("pageTableLock");
	pageTableLock -> Acquire();
// first, set up the translation 
    pageTable = new pageTableEntry[numPages];
	pageTableIndex = 0;
    for (i = 0; i < numPages; i++) 
	{
		pageTable[pageTableIndex].virtualPage = i;						
		pageTable[pageTableIndex].physicalPage = -1;											// find a free physical for the corresponding virtual page
		bzero(&(machine -> mainMemory[(pageTable[pageTableIndex].physicalPage * PageSize)]), PageSize);
		pageTable[pageTableIndex].valid = false;
		pageTable[pageTableIndex].use = false;
		pageTable[pageTableIndex].dirty = false;
		pageTable[pageTableIndex].readOnly = false;  // if the code segment was entirely on 
					// a separate page, we could set its 
					// pages to be read-only
		if(i >= numOfCodeInitDataPages)
		{
			pageTable[pageTableIndex].diskLocation = 2;		// indicate that the page in not in executable (not on disk)
		}
		else
		{
			pageTable[pageTableIndex].diskLocation = 1;		// indicate that the page is in the executable
			pageTable[pageTableIndex].byteOffset = noffH.code.inFileAddr + (i * PageSize);
		}
		pageTableIndex++;
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
	IntStatus oldIntStatus = interrupt -> SetLevel(IntOff); 	// Setting the interupts off
	for(int i = 0; i < TLBSize ; i++)
	{
		if(machine -> tlb[i].valid == true)
		{
			ipt[machine->tlb[i].physicalPage].dirty = machine->tlb[i].dirty;
			machine -> tlb[i].valid = false;
		}
	}
	interrupt -> SetLevel(oldIntStatus);		// Restore Interrupts
}
