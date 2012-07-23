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

	NPrint("Creating some locks and CVs\n", sizeof("Creating some locks and CVs\n"), 0, 0);
	for(i = 0; i < 10; i++) {
		Yield();
	}

	for(i = 0; i < 10; i++) {
		NPrint("Creating additional threads. #%d\n", sizeof("Creating additional threads. #%d\n"), i, 0);
		v[0][i] = CreateLock("some lock", sizeof("some lock"));
		v[1][i] = CreateCondition("some cv", sizeof("some cv"));
	}

	NPrint("Done creating some locks and CVs\n", sizeof("Done creating some locks and CVs\n"), 0, 0);
    Exit(0);
}

