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
	cv = CreateCondition("A CV", sizeof("A CV"));
	var = CreateMV("var", sizeof("var"), 3, 0);

	NPrint("got lock %d, CV %d, and MV %d", sizeof("Got lock %d, CV %d, and MV %d"), NEncode2to1(lock, cv), NEncode2to1(var, 0));

	NPrint("about to acquire\n", sizeof("about to acquire\n"), 0, 0);
	Acquire(lock);
	NPrint("acquired lock %d\n", sizeof("acquired lock %d\n"), lock, 0);

	/*for(i = 0; i < 3; i++){
		NPrint("var value at entry %d is %d\n", sizeof("var value at entry %d is %d\n"), NEncode2to1(i, GetMV(var, i)), 0);
	}*/
	for(i = 0; i < 3; i++){
		SetMV(var, i, i);
		NPrint("var value at entry %d was set to %d\n", sizeof("var value at entry %d was set to %d\n"), NEncode2to1(i, i), 0);
	}
	for(i = 0; i < 3; i++){
		NPrint("var value at entry %d is %d\n", sizeof("var value at entry %d is %d\n"), NEncode2to1(i, GetMV(var, i)), 0);
	}

	Release(lock);
	NPrint("released lock %d\n", sizeof("released lock %d\n"), lock, 0);

	DestroyMV(mv);
	NPrint("destroyed CV %d\n", sizeof("destroyed CV %d\n"), cv, 0);
	DestroyLock(lock);
	NPrint("destroyed lock %d\n", sizeof("destroyed lock %d\n"), lock, 0);
	DestroyCondition(cv);
	NPrint("destroyed MV %d\n", sizeof("destroyed MV %d\n"), mv, 0);

	NPrint("Test server create/destroy complete\n", sizeof("Test server create/destroy complete\n"), 0, 0);

	Exit(0);
	return 0;
}
