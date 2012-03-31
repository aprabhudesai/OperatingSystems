// system.h 
//	All global variables used in Nachos are defined here.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef SYSTEM_H
#define SYSTEM_H

#include "copyright.h"
#include "utility.h"
#include "thread.h"
#include "scheduler.h"
#include "interrupt.h"
#include "stats.h"
#include "timer.h"

#define MAX_CHILD 5000
#define MAX_PROCESSES 2000
#define SERVER_COUNT 3

// Initialization and cleanup routines
extern void Initialize(int argc, char **argv); 	// Initialization,
						// called before anything else
extern void Cleanup();				// Cleanup, called when
						// Nachos is done.

extern Thread *currentThread;			// the thread holding the CPU
extern Thread *threadToBeDestroyed;  		// the thread that just finished
extern Scheduler *scheduler;			// the ready list
extern Interrupt *interrupt;			// interrupt status
extern Statistics *stats;			// performance metrics
extern Timer *timer;				// the hardware alarm clock

extern int testcase;
extern int noOfServers;
#ifdef USER_PROGRAM
#include "machine.h"
#include "bitmap.h"
#include "synch.h"
extern Machine* machine;	// user program memory and registers
// BitMap to find the available physical page numbers
extern BitMap *physicalPageBitMap;
extern Lock *physicalPageBitMapLock;

extern int processIdCounter; // For assigning the process Id to every new process getting created
extern int totalNumOfProcesses;
extern Lock *processCounterLock; // For accessing the process Id counter and total number of processes
struct processInfo		// Process table entries structure
{
	int pid;
	int noOfThreads;
	int noOfWaitingThreads;
	AddrSpace *space;
	int childStackPageLocation[MAX_CHILD];
	int childIdCounter;
};

extern processInfo processTable[MAX_PROCESSES];

extern Lock *processTableLock;

#endif

#ifdef FILESYS_NEEDED 		// FILESYS or FILESYS_STUB 
#include "filesys.h"
extern FileSystem  *fileSystem;
#endif

#ifdef FILESYS
#include "synchdisk.h"
extern SynchDisk   *synchDisk;
#endif

#ifdef NETWORK
#include "post.h"
#include "synch.h"
extern PostOffice* postOffice;
extern Lock *mailBoxLock;
extern int mailBoxCounter;
extern int netname;
#endif

#endif // SYSTEM_H
