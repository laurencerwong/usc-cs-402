/*
 * MakeSomeLocksAndCVs.c
 *
 *  Created on: Jun 20, 2012
 *      Author: Keith
 */

#include "syscall.h"

int main() {
	int v[2][10];
	int i = 0;

	for(i = 0; i < 10; i++) {
		Yield();
	}

	for(i = 0; i < 10; i++) {
		v[0][i] = CreateLock();
		v[1][i] = CreateCondition();
	}

	NPrint("Done creating some locks and CVs\n", sizeof("Done creating some locks and CVs\n"), 0, 0);
    Exit(0);
}

