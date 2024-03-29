/*
 * test_server_acquire_release.c
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
	cv = CreateCondition("A CV", sizeof("A CV"));
	var = CreateMV("var", sizeof("var"), 3, 0);

	NPrint("got lock %d, CV %d, and MV %d\n", sizeof("Got lock %d, CV %d, and MV %d\n"), NEncode2to1(lock, cv), NEncode2to1(var, 0));

	NPrint("about to acquire\n", sizeof("about to acquire\n"), 0, 0);
	Acquire(lock);
	NPrint("acquired lock %d\n", sizeof("acquired lock %d\n"), lock, 0);
	Release(lock);
	NPrint("released lock %d\n", sizeof("released lock %d\n"), lock, 0);

	NPrint("Test server acquire/release complete\n", sizeof("Test server acquire/release complete\n"), 0, 0);

	Exit(0);
	return 0;
}
