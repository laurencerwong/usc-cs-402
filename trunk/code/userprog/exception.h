/*
 * exception
 *
 *  Created on: Jun 9, 2012
 *      Author: Laurence, Rob
 */

#ifndef EXCEPTION_
#define EXCEPTION_

#include "bitmap.h"

typedef struct Lock_Entry{
	Lock *lock;
	AddrSpace *lockSpace;
	bool isToBeDeleted;
} LockEntry;

typedef struct Condition_Entry{
	Condition *condition;
	AddrSpace *conditionSpace;
	bool isToBeDeleted;
} ConditionEntry;

BitMap *lockMap;
BitMap *conditionMap;

LockEntry *lockTable;
ConditionEntry *conditionTable;

int lockArraySize = 0;
int conditionArraySize = 0;

#endif /* EXCEPTION_ */
