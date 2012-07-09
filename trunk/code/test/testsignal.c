/*
 * testsignal.c
 *
 *  Created on: Jul 9, 2012
 *      Author: Robert
 */


int main(){
	int i = 0;
	int lock = CreateLock("A lock", sizeof("A lock"));
	int condition = CreateCondition("A condition", sizeof("A condition"));
	int numClients = CreateMV("number of clients", sizeof("number of clients"), 0, 0);
	Acquire(lock);
	NPrint("I have lock\n", sizeof("I have lock\n"));
	for(i = 0; i < GetMV(numClients, 0); i++){
		NPrint("Doing signal\n", sizeof("Doing broadcast\n"));
		Signal(condition, lock);
	}
	NPrint("Release lock\n");
	Release(lock);
}
