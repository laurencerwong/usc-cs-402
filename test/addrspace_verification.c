/*
 * addrspace_verification.c
 *
 *  Created on: Jun 20, 2012
 *      Author: Robert
 */
#include "syscall.h"

int main(){
	/*test that all lock operation verify address space */
	/*We should get errors for each function call */
	Acquire(0);
	Release(0);
	DestroyLock(0);

	/*test that all condition operations verify address space */
	/*We should get errors for each function call */
	Wait(0);
	Signal(0);
	Broadcast(0);
	DestroyCondition(0);
	Exit(0);
}
