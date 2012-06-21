/*
 * MaxLockCVMaker.c
 *
 *  Created on: Jun 20, 2012
 *      Author: Keith
 */



#include "syscall.h"

void addRand() {
	int x = (RandInt() % 10);
	int y = (RandInt() % 10);
	int res = x + y;

	NPrint("%d + %d = %d\n", sizeof("%d + %d = %d\n"), NEncode2to1(x, y), res);
	Exit(0);
}

int main()
{
	int v[2][MAX_LOCKS_CONDITIONS];
	int i = 0;
	for(i = 0; i < MAX_LOCKS_CONDITIONS; i++) {
		v[0][i] = CreateLock();
		v[1][i] = CreateCondition();
	}

	Exec("../test/MakeSomeLocksAndCVs", sizeof("../test/MakeSomeLocksAndCVs"), "MakeSomeLocksAndCVs main", sizeof("MakeSomeLocksAndCVs main"));

	NPrint("Done creating MAX locks and CVs\n", sizeof("Done creating MAX locks and CVs\n"), 0, 0);
    Exit(0);
}






