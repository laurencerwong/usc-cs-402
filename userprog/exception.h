/*
 * exception
 *
 *  Created on: Jun 9, 2012
 *      Author: Laurence, Rob
 */

#ifndef EXCEPTION_
#define EXCEPTION_

#include "bitmap.h"
#include "synch.h"
#include "addrspace.h"
#include <queue>

typedef struct Lock_Entry{
	Lock *lock;
	AddrSpace *lockSpace;
	bool isToBeDeleted;
	int acquireCount;
} LockEntry;

typedef struct Condition_Entry{
	Condition *condition;
	AddrSpace *conditionSpace;
	bool isToBeDeleted;
} ConditionEntry;

typedef struct Semaphore_Entry{
	Semaphore *semaphore;
	AddrSpace *semaphoreSpace;
	bool isToBeDeleted;
} SemaphoreEntry;

BitMap *lockMap;
BitMap *conditionMap;
BitMap *semaphoreMap;
BitMap *queueMap;

LockEntry *lockTable;
ConditionEntry *conditionTable;
SemaphoreEntry *semaphoreTable;
queue<int> *queueTable;


int lockArraySize = 0;
int conditionArraySize = 0;
int semaphoreArraySize = 0;
int queueArraySize = 0;



#endif /* EXCEPTION_ */
