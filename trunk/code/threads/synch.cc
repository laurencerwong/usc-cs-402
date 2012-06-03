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
	state = FREE; //not allowing more than one thread to acquire one Lock
										//to conform to the monitor paradigm
	queue = new List;
}

//---------------------------------------------------------------
//Deallocates Lock, assuming no thread is still trying to use lock
//--------------------------------------------------------------
Lock::~Lock() {
	delete queue;
	delete name;
}
//----------------------------------------------------------------

//--------------------------------------------------------------
//Lock::Acquire
//Allow thread to acquire the Lock, also allows us to remember which
//Thread called this function
//----------------------------------------------------------------
void Lock::Acquire() {

	IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts so the current thread
														//cannot be switched out
	
  if(isHeldByCurrentThread()){ //thread already has possession, if allowed to acquire again,
	  	  	  	  	  	  	  //thread would be blocked entire process indefinitely since interrupts are switched off
  	(void) interrupt->SetLevel(oldLevel);		//so restore interrupts
  	return;																	//and do not wait
  }
  if(state == FREE){	//lock is free to be taken
						state = BUSY;
						threadWithPossession = currentThread; //make this thread the holder
	}
  else{
  	queue->Append( (void *) currentThread); //go into waiting queue
  	currentThread->Sleep();	//kick self out of processor to wait
  }
  (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

void Lock::Release() {
	Thread* thread;
	IntStatus oldLevel = interrupt->SetLevel(IntOff);
	if(!isHeldByCurrentThread()){ //shouldn't allow synchronization to be messed up if a thread calls release accidentally or maliciously
		printf("Thread %s calling Release() on Lock %s does not have lock\n", currentThread->getName(), name);
		(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
		return; //kicks out a thread that didn't acquire Lock
	}
	if(!queue->IsEmpty()){ //check this condition to avoid segmentation faults or bus errors
		thread = (Thread *) queue->Remove();
		threadWithPossession = thread; //give next thread immediate possession of the lock
		scheduler->ReadyToRun(thread); //add to the system's ready queue
	}
	else{ //make sure lock can be grabbed by anyone if no one was waiting in te ready queue
		state = FREE;
		threadWithPossession = NULL;
	}
	(void) interrupt->SetLevel(oldLevel); //restore interrupts
	
}

//--------------------------------------------------------------
//Allows us to use the external system variable Thread* currentThread
//to check against the thread that last acquired the lock.
//---------------------------------------------------------
bool Lock::isHeldByCurrentThread(){
	return currentThread == threadWithPossession;
}

Condition::Condition(char* debugName) { 
	name = debugName;
	queue = new List;
}
Condition::~Condition() { 
	delete queue;
}
void Condition::Wait(Lock* conditionLock) { 
	
	IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts so this operation is atomic
														//(can't be switched out)
	//a null argument would be improper, so don't allow current thread to execute rest of function
	if(conditionLock == NULL){
		printf("Argument passed to Condition::Wait() was null, returning without performing wait\n");
		(void) interrupt->SetLevel(oldLevel);
		return;
	}
	//set the current lock for the condition variable if we don't already remember it
	if(waitingLock == NULL){
		waitingLock = conditionLock;
	}
	//check that the lock passed in is the lock associated with this CV.  Otherwise, it would be improper
	//to allow the thread to release the lock, and would mess up the semantics of the CV
	if(conditionLock != waitingLock){
		printf("Argument passed to Condition::Wait() was the incorrect lock, returning without performing wait\n");
		(void) interrupt->SetLevel(oldLevel);
	}
	//OK to wait

	conditionLock->Release(); //release lock
	queue->Append((void *) currentThread);
	currentThread->Sleep(); //will sleep. current thread will be not regain processor until it is at the front of queue
							//and Signal is called on this CV
	conditionLock->Acquire(); //reacquire lock before returning ensures mutual exclusivity until current thread manually releases lock
  (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts (atomic operation finished)
  
}
void Condition::Signal(Lock* conditionLock) { 
	Thread *thread;
	IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts so this opertion is atomic
														//(can't be switch out
	if(queue->IsEmpty()) return; //don't need to worry if an extraneous call to signal, just return
	if(waitingLock != conditionLock){ //check that it is the proper lock, otherwise the thread woken up will try
										//to acquire a lock that may not be free and be blocked as a result
		printf("Argument passed to Condition::Signal() was the incorrect lock, returning without performing wait\n");
		(void) interrupt->SetLevel(oldLevel); //restore interrupts (atomic operation done)
		return;
	}
	thread = (Thread *) queue->Remove(); //get the next thread waiting on CV
	scheduler->ReadyToRun(thread); //add to system's ready queue
	if(queue->IsEmpty()){
		waitingLock = NULL; //if queue is empty, we no longer need to remember a lock
							//and can allow a different lock to be passed in on the next Wait
	}
	(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}
void Condition::Broadcast(Lock* conditionLock) { 
	IntStatus oldLevel = interrupt->SetLevel(IntOff);
	while(!queue->IsEmpty()) Signal(conditionLock); //all operations we need are in Signal, just need to call over and over
	(void) interrupt->SetLevel(oldLevel);
}
