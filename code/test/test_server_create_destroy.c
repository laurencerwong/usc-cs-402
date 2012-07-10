/*
 * test_server_create_destroy.c
 *
 *  Created on: Jul 10, 2012
 *      Author: Keith
 */


int main(){

	int lock, cv, mv;
	int var;
	int temp;
	int i = 0;

	NPrint("Creating Lock, CV, and MV (with 3 entries)\n", sizeof("Creating Lock, CV, and MV (with 3 entries)\n"), 0, 0);
	lock = CreateLock("A lock", sizeof("A lock"));
	cv = CreateCV("A CV", sizeof("A CV"));
	var = CreateMV("var", sizeof("var"), 3, 0);

	NPrint("Got lock %d, CV %d, and MV %d", sizeof("Got lock %d, CV %d, and MV %d"), NEncode2to1(lock, cv), NEncode2to1(var, 0));

	NPrint("about to acquire\n", sizeof("about to acquire\n"), 0, 0);
	Acquire(lock);
	NPrint("acquired lock %d\n", sizeof("acquired lock %d\n"), lock, 0);

	NPrint("acquired lock %d\n", sizeof("acquired lock %d\n"), lock, 0);
	for(i = 0; i < 3; i++){
		NPrint("var value at entry %d is %d\n", sizeof("var value is %d\n"), GetMV(var, i), 0);
	}


	Release(lock);
	NPrint("released lock %d\n", sizeof("released lock %d\n"), lock, 0);

	/*NPrint("acquired, about to setMV\n", sizeof("acquired, about to setMV\n"), 0, 0);*/
	SetMV(var, 0, GetMV(counter, 0) + 1);
	/*NPrint("setMV, about to loop\n", sizeof("setMV, about to loop\n"), 0, 0);*/
	Release(lock);
	/*NPrint("Released lock\n", sizeof("Released lock\n"));*/

	Exit(0);
	return 0;
}
