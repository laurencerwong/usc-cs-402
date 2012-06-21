/*
 * MaxLockCVMaker.c
 *
 *  Created on: Jun 20, 2012
 *      Author: Keith
 */



#include "syscall.h"

int main()
{
	int i = 0;
	for(i = 0; i < MAX_LOCKS_CONDITIONS; i++) {
		NPrint("Making a lock and CV pass #%d\n", sizeof("Making a lock and CV pass #%d\n"), i, 0);
		CreateLock("lock", sizeof("lock"));
		/*NPrint("Making cv pass %d\n", sizeof("Making cv pass %d\n"), i, 0);*/
		CreateCondition("cv", sizeof("cv"));
		/*NPrint("Done making stuff on pass %d\n", sizeof("Done Making stuff on pass %d\n"), i, 0);*/
	}

	NPrint("About to exec something to make a few more locks and CVs\n", sizeof("About to exec something to make a few more locks and CVs\n"), 0, 0);
	Exec("../test/MakeSomeLocksAndCVs", sizeof("../test/MakeSomeLocksAndCVs"), "MakeSomeLocksAndCVs main", sizeof("MakeSomeLocksAndCVs main"));

	NPrint("Done creating MAX locks and CVs\n", sizeof("Done creating MAX locks and CVs\n"), 0, 0);
    Exit(0);
}






