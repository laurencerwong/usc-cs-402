/*
 * testmutex.c
 *
 *  Created on: Jul 9, 2012
 *      Author: Robert
 */


int main(){
	int lock = CreateLock("A lock", sizeof("A lock"));
	int counter = CreateMV("Counter", sizeof("Counter"), 1, 0);
	int i = 0;
	Acquire(lock);
	SetMV(numClients, 0, GetMV(counter, 0) + 1);
	for(i = 0; i < 30; i++){
		NPrint("counter value is %d\n", sizeof("counter value is %d\n"), GetMV(counter, 0));
	}
	Release(lock);
	NPrint("Released lock", sizeof("Released lock"));
}
