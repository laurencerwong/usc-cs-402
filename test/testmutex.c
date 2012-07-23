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
	int j = 0;
	/*NPrint("about to acquire\n", sizeof("about to acquire\n"), 0, 0);*/
	Acquire(lock);
	/*NPrint("acquired, about to setMV\n", sizeof("acquired, about to setMV\n"), 0, 0);*/
	SetMV(counter, 0, GetMV(counter, 0) + 1);
	/*NPrint("setMV, about to loop\n", sizeof("setMV, about to loop\n"), 0, 0);*/
	for(i = 0; i < 30; i++){
		NPrint("counter value is %d\n", sizeof("counter value is %d\n"), GetMV(counter, 0), 0);
		for(j = 0; j < 2000; j++){};
	}
	Release(lock);
	/*NPrint("Released lock\n", sizeof("Released lock\n"));*/

	Exit(0);
	return 0;
}
