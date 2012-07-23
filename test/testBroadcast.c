/*
 * testBroadcast.c
 *
 *  Created on: Jul 9, 2012
 *      Author: Robert
 */


int main(){
	int lock = CreateLock("A lock", sizeof("A lock"));
	int condition = CreateCondition("A condition", sizeof("A condition"));
	Acquire(lock);
	NPrint("I have lock\n", sizeof("I have lock\n"));
	NPrint("Doing broadcast\n", sizeof("Doing broadcast\n"));
	Broadcast(condition, lock);
	NPrint("Release lock\n");
	Release(lock);
}
