// addrspace.h
//	Data structures to keep track of executing user programs
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include "table.h"
#include "pagetable.h"

#define UserStackSize		1024 	// increase this as necessary!

#define MaxOpenFiles 256
#define MaxChildSpaces 256
#define MAX_NO_OF_THREADS 20

class AddrSpace {
  public:
    unsigned int numPages;		// Number of pages in the virtual
					// address space
	unsigned int numOfPhysPages;	// Number of the physical pages used currently

	unsigned int CodeSize;		// Size of the code segment
	unsigned int DataSize;		// Size of the data segment - Init Data + UnInit Data
	AddrSpace(OpenFile *exec);	// Create an address space,
					// initializing it with the program
					// stored in the file "executable"
    ~AddrSpace();			// De-allocate an address space
    void InitRegisters();		// Initialize user-level CPU registers,
					// before jumping to user code
    void SaveState();			// Save/restore address space-specific
    void RestoreState();		// info on a context switch
    Table fileTable;			// Table of openfiles
	int stackPageAllocation(); // To allocate stack pages dynamically and put them into page table of the process. This returns the value of the last page number.
	void DeleteStackForThread(int);	// To delete the stack pages allocated to the thread
	void DeleteProcessPhysicalPages();	// To delete the pages allocated to the process
	void DisplayBitMapOfThread();	// To display the physical pages allocated to the process
	OpenFile *executable;
	Lock *pageTableLock;			// For mutually exclusive access to the pages table	
	pageTableEntry *pageTable;	// Assume linear page table translation
					// for now!
	pageTableEntry *newPageTable;	// Used whenever a new stack has to be allocated
 private:
	unsigned int numOfCodeDataPages; // Numnber of code + data + stack (1st thread) pages
	unsigned int numCodePages; // Number of pages for code segment in the virtual address space
	unsigned int numInitDataPages; // Number of pages for initialized data segment in the virtual address space
	unsigned int numUnInitDataPages; // Number of pages for uninitialized data segment in the virtual address space
	unsigned int topOfProcessStack; // Pointer to the first location in the process stack
	unsigned int pageTableIndex;
	unsigned int numOfCodeInitDataPages;	// Number of code and initialized data pages
};

#endif // ADDRSPACE_H
