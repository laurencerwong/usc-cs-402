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

#include "bitmap.h"
#include "addrspace.h"
//#include "../userprog/bitmap.h"
//#include "../userprog/addrspace.h"
#include "synch.h"
#include "machine.h"
#include "IPTEntry.h"
//#include <queue>
#include "list.h"
//using namespace std;

#define MAX_THREADS 100
#define MAX_PROCESSES 100

// Initialization and cleanup routines
extern void Initialize(int argc, char **argv); 	// Initialization,
						// called before anything else
extern void Cleanup();				// Cleanup, called when
						// Nachos is done.

//#ifdef CHANGED

struct ProcessEntry {
	int numThreadsAlive;
	int processID;
	int threadStacks[MAX_THREADS];
	int nextThreadID;
};


extern ProcessEntry processTable[MAX_PROCESSES];
extern int nextProcessID;
extern Lock processIDLock;
extern int numLivingProcesses;

extern BitMap *mainMemoryBitmap;
extern Lock *processTableLock;
extern AddrSpace **pageOwners;
extern Lock* ioLock;
extern Lock* lockTableLock;
extern Lock* conditionTableLock;

extern Lock* mailboxIDLock;
extern int nextMailboxID;

extern int currentTLB;

extern IPTEntry* IPT;
extern BitMap *swapMap;
extern OpenFile *swapFile;

extern Lock* iptLock;
extern Lock* swapLock;

extern List *evictionList;

enum EvictionPolicy {FIFO, RAND};
extern EvictionPolicy evictionPolicy;

extern int myMachineID;
extern int totalNumServers;


//#endif

extern Thread *currentThread;			// the thread holding the CPU
extern Thread *threadToBeDestroyed;  		// the thread that just finished
extern Scheduler *scheduler;			// the ready list
extern Interrupt *interrupt;			// interrupt status
extern Statistics *stats;			// performance metrics
extern Timer *timer;				// the hardware alarm clock

#ifdef USER_PROGRAM
#include "machine.h"
extern Machine* machine;	// user program memory and registers
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
extern PostOffice* postOffice;
#endif

#endif // SYSTEM_H
