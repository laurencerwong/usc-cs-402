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
	int numClients = CreateMV("number of clients", sizeof("number of clients"), 0, 0);
	int counterIsInitialized = CreateMV("boolean", sizeof("boolean"), 1, 0);
	Acquire(lock);
	SetMV(numClients, 0, GetMV(counter, 0) + 1);
	NPrint("Printing when counter is %d\n", sizeof("Printing when counter is %d\n"), GetMV(counter, 0));
	NPrint("Now incrementing counter\n", sizeof("NOw incrementing counter\n"));
	SetMV(GetMV(counter, 0) + 1);
	Wait(condition, lock);
	NPrint("Woken up after wait, counter is now %d", GetMV(counter, 0));
	NPrint("Now incrementing counter\n", sizeof("NOw incrementing counter\n"));
	SetMV(counter, 0, GetMV(counter, 0) + 1);
	Release(lock);
	NPrint("Released lock", sizeof("Released lock"));

}
