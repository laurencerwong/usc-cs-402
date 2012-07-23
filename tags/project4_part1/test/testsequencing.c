/*
 * testbroadcast.c
 *
 *  Created on: Jul 9, 2012
 *      Author: Robert
 */


int main(){
	int lock = CreateLock("A lock", sizeof("A lock"));
	int condition = CreateCondition("A condition", sizeof("A condition"));
	int counter = CreateMV("Counter", sizeof("Counter"), 1, 0);
	int numClients = CreateMV("number of clients", sizeof("number of clients"), 1, 0);
	int counterIsInitialized = CreateMV("boolean", sizeof("boolean"), 1, 0);
	Acquire(lock);
	SetMV(numClients, 0, GetMV(counter, 0) + 1);
	NPrint("Printing when counter is %d\n", sizeof("Printing when counter is %d\n"), GetMV(counter, 0));
	NPrint("Now incrementing counter\n", sizeof("Now incrementing counter\n"));
	SetMV(counter, 0, GetMV(counter, 0) + 1);
	NPrint("Waiting...\n", sizeof("Waiting...\n"), 0, 0);
	Wait(condition, lock);
	NPrint("Woken up after wait, counter is now %d\n", sizeof("Woken up after wait, counter is now %d\n"), GetMV(counter, 0));
	NPrint("Now incrementing counter\n", sizeof("Now incrementing counter\n"));
	SetMV(counter, 0, GetMV(counter, 0) + 1);
	Release(lock);
	NPrint("Released lock\n", sizeof("Released lock\n"), 0, 0);

}
