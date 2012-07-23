/*
 * test_creates.c
 *
 *  Created on: Jul 10, 2012
 *      Author: Keith
 */



int main(){

	int lock1, cv1, mv1;
	int lock2, cv2, mv2;
	int lock3, cv3, mv3;
	int var;
	int temp;
	int i = 0;

	NPrint("Creating Lock, CV, and MV (with 3 entries) SET 1\n", sizeof("Creating Lock, CV, and MV (with 3 entries) SET 1\n"), 0, 0);
	lock1 = CreateLock("A Lock", sizeof("A Lock"));
	cv1 = CreateCondition("A CV", sizeof("A CV"));
	mv1 = CreateMV("A MV", sizeof("A MV"), 3, 0);
	NPrint("got lock %d, CV %d, and MV %d\n", sizeof("Got lock %d, CV %d, and MV %d\n"), NEncode2to1(lock1, cv1), NEncode2to1(mv1, 0));

	NPrint("Creating Lock, CV, and MV (with 3 entries) SET 2\n", sizeof("Creating Lock, CV, and MV (with 3 entries) SET 2\n"), 0, 0);
	lock2 = CreateLock("Another Lock", sizeof("Another Lock"));
	cv2 = CreateCondition("Another CV", sizeof("Another CV"));
	mv2 = CreateMV("Another MV", sizeof("Another MV"), 3, 0);
	NPrint("got lock %d, CV %d, and MV %d\n", sizeof("Got lock %d, CV %d, and MV %d\n"), NEncode2to1(lock2, cv2), NEncode2to1(mv2, 0));

	NPrint("Creating Lock, CV, and MV (with 3 entries) SET 3 (index of each should be same as SET 1)\n",
			sizeof("Creating Lock, CV, and MV (with 3 entries) SET 3 (index of each should be same as SET 1)\n"), 0, 0);
	lock3 = CreateLock("A Lock", sizeof("A Lock"));
	cv3 = CreateCondition("A CV", sizeof("A CV"));
	mv3 = CreateMV("A MV", sizeof("A MV"), 3, 0);
	NPrint("got lock %d, CV %d, and MV %d\n", sizeof("Got lock %d, CV %d, and MV %d\n"), NEncode2to1(lock3, cv3), NEncode2to1(mv3, 0));


	NPrint("Test creates complete\n", sizeof("Test creates complete\n"), 0, 0);

	Exit(0);
	return 0;
}
