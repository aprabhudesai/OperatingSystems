// synch.cc 
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks 
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
    while (value == 0) { 			// semaphore not available
	queue->Append((void *)currentThread);	// so go to sleep
	currentThread->Sleep();
    } 
    value--; 					// semaphore available, 
						// consume its value
    
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
	scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments 
// Note -- without a correct implementation of Condition::Wait(), 
// the test case in the network assignment won't work!
Lock::Lock(char* debugName) {
	name = debugName;
	lockQueue = new List;  //create the lock queue
	lockOwner = NULL;	// make lock owner null,
		//as no thread has the lock acquired when object created
	lockStatus = FREE;
}
Lock::~Lock() {
	delete lockQueue;
}
bool Lock::isHeldByCurrentThread(){
	if(currentThread == lockOwner)
		return true;
	else return false;
}

bool Lock::isQueueEmpty()
{
	if(lockQueue -> IsEmpty() == true)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void Lock::Acquire() {
	//disable interrupt to perform atomic operations in single thread
	IntStatus oldIntStatus = interrupt -> SetLevel(IntOff);

	if(isHeldByCurrentThread()){
		interrupt -> SetLevel(oldIntStatus);
		return;
	}
	if(lockStatus == FREE){
		lockStatus = BUSY;
		lockOwner = currentThread;
	}
	else{
		lockQueue -> Append((void *)currentThread);
			// add to wait queue as lock is not available
		currentThread->Sleep(); // put thread to sleep
	}

	//enable interrupts
	interrupt -> SetLevel(oldIntStatus);
}
void Lock::Release() {
	//disable interrupt to perform atomic operations in single thread
	IntStatus oldIntStatus = interrupt -> SetLevel(IntOff);

	if(!isHeldByCurrentThread()){
		printf("\n%s is not the lock owner, so it cannot release the lock\n", currentThread -> getName());
		//enable interrupts
		interrupt -> SetLevel(oldIntStatus);
		return;
	}
	if(!(lockQueue -> IsEmpty())){
		Thread* nextQThread = (Thread *)lockQueue -> Remove();
		//nextThread is the next thread to acquire the lock from waiting queue
		scheduler -> ReadyToRun(nextQThread);
		lockOwner = nextQThread;
	}
	else{
		lockStatus = FREE;
		lockOwner = NULL;
	}
	//enable interrupts
	interrupt -> SetLevel(oldIntStatus);
}

Condition::Condition(char* debugName) {
	name = debugName;
	waitLock = NULL;
	condWaitQueue = new List;
}
Condition::~Condition() {
	delete condWaitQueue;
}

bool Condition::isQueueEmpty()
{
	if((condWaitQueue -> IsEmpty()) == true)
		return true;
	else
		return false;
}

void Condition::Wait(Lock* conditionLock) {
	//ASSERT(FALSE);

	//disable interrupt to perform atomic operations in single thread
	IntStatus oldIntStatus = interrupt -> SetLevel(IntOff);

	//Check if current thread has acquired the lock
	if(!(conditionLock -> isHeldByCurrentThread())){
		printf("%s thread does not has the lock", currentThread -> getName());
		interrupt -> SetLevel(oldIntStatus);
			return;
	}

	// if conditionLock is null, then thread cannot wait, its a invalid condition
	if(conditionLock == NULL){
		printf("\nThread %s cannot wait for condition as condition lock is NULL \n",currentThread -> getName());
		//enable interrupts
		interrupt -> SetLevel(oldIntStatus);
		return;
	}

	//First Thread with conditionLock calling wait
	if(waitLock == NULL)
		waitLock = conditionLock;

	//waitLock and conditionLock do not match
	if(waitLock != conditionLock){
		printf("\nThread %s cannot wait for condition as condition Lock is not same as waitLock \n",currentThread -> getName());
		//enable interrupts
		interrupt -> SetLevel(oldIntStatus);
		return;
	}

	//exiting the monitor before going to sleep
	conditionLock -> Release();

	// add the current thread to condition variable's wait queue
	condWaitQueue -> Append((void *)currentThread);
	//put the thread to sleep
	currentThread -> Sleep();

	//re-acquire the lock after waking up
	conditionLock -> Acquire();

	//enable interrupts
	interrupt -> SetLevel(oldIntStatus);
}
void Condition::Signal(Lock* conditionLock) {
	//disable interrupt to perform atomic operations in single thread
	IntStatus oldIntStatus = interrupt -> SetLevel(IntOff);

	//Check if current thread has acquired the lock
	if(!(conditionLock -> isHeldByCurrentThread())){
		printf("%s thread does not has the lock", currentThread -> getName());
		interrupt -> SetLevel(oldIntStatus);
		return;
	}

	if(condWaitQueue -> IsEmpty()){
		//enable interrupts
		interrupt -> SetLevel(oldIntStatus);
		return;
	}

	if(waitLock != conditionLock){
		printf("\nThread %s cannot signal for condition as condition Lock is not same as waitLock \n",currentThread -> getName());
		//enable interrupts
		interrupt -> SetLevel(oldIntStatus);
		return;
	}

	// remove next thread waiting in condition queue for its chance to perform mutual exclusive operations
	Thread *nextThread = (Thread*)condWaitQueue -> Remove();
	//add it to scheduler's ready queue
	scheduler -> ReadyToRun(nextThread);

	//if no more thread waiting for condition
	if(condWaitQueue -> IsEmpty())
		waitLock = NULL;

	//enable interrupts
	interrupt -> SetLevel(oldIntStatus);
}
void Condition::Broadcast(Lock* conditionLock) {
	//signal all thread waiting for same condition to occur
	//example: waiting for movie to end

	//Check if current thread has acquired the lock
	if(!(conditionLock -> isHeldByCurrentThread())){
		printf("%s thread does not has the lock", currentThread -> getName());
		return;
	}

	while(!(condWaitQueue -> IsEmpty())){
		Signal(conditionLock);
	}
}
