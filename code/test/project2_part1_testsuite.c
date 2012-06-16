/*
 * project2_part1_testsuite.cc
 *
 *  Created on: Jun 9, 2012
 *      Author: Laurence, Rob
 */

/*#include "syscall.h"*/
#include "../userprog/syscall.h"

int condition;
int lock;
int testCounter;
void forkTestPrint()
{
	/*char message[14] = "Print message";
	NPrint(message, 19, 0, 0);*/
	NPrint("A thread printing...\n", sizeof("Thread printing...\n"), 0, 0);
	Exit(0);
}

void execTestPrint1() {
	NPrint("Exec 1 printing!\n", sizeof("Exec 1 printing!\n"), 0, 0);
}
void execTestPrint2() {
	NPrint("Exec 2 printing!\n", sizeof("Exec 2 printing!\n"), 0, 0);
}
void execTestPrint3() {
	NPrint("Exec 3 printing!\n", sizeof("Exec 3 printing!\n"), 0, 0);
}

void testExecAndFork()
{
	int i;
	/*Exec(forkTestPrint)*/
	for(i = 0; i < 5; i++) {
		Fork(forkTestPrint);
	}
}


void testYieldCompanion(){
	NPrint("testYieldCompanion is printing\n", sizeof("testYieldCompanion is printing\n"), 0, 0);
	Exit(0);
}
/*We will test yield by forking to another function, which prints a different statement than this function.
 * This function will call yield after it forks.  When tested WITHOUT AN RS VALUE, the forked method should always print first
 */
void testYield(){
	Fork(testYieldCompanion);
	Yield();
	NPrint("testYield is printing\n", sizeof("testYield is printing\n"), 0, 0);
	Exit(0);
}

/*We will create some locks, but then try to access invalid locks in acquires, releases, and destroys
 * in order to exercise the validation in lock syscalls
 */
void testLockArrayBoundaries(){
	int lock1 = CreateLock();
	int lock2 = CreateLock();
	/*This is the only thread running, so we know the only two valid indices are 0 and 1*/

	/*First, we check acquire, to be sure its validation works */
	Acquire(3); /*To exercise the "lock exists" validation */
	Acquire(-1); /*To excercise the "invalid index" validation, lower bound */
	Acquire(1000); /*To exercise the "invalid index" validation, upper bound */

	/*Now lets check Release's validation */
	Release(3); /*To exercise the "lock exists" validation */
	Release(-1); /*To excercise the "invalid index" validation, lower bound */
	Release(1000); /*To exercise the "invalid index" validation, upper bound */

	/*Now lets check DestroyLock's Validation */
	DestroyLock(3); /*To exercise the "lock exists" validation */
	DestroyLock(-1); /*To excercise the "invalid index" validation, lower bound */
	DestroyLock(1000); /*To exercise the "invalid index" validation, upper bound */
}

/*We will test the destruction of a lock in 2 ways: creating a lock and destroying it without an Acquire(),
 * in order to see that it will be destroyed upon our call to DestroyLock since no thread possesses the lock.
 * To test that it is destroyed, we will try to Acquire() the lock.
 * The second way we will test destroying the lock is to create it, acquire it, and then try to destroy it.  We will then
 * acquire it again, which if the lock still exists, won't give us any error messages.  We will then release the lock,
 * which should destroy it, and test this by trying another acquire.
 */

void testDestroyLock(){
	/*Method 1*/
	int lock = CreateLock();
	DestroyLock(lock);
	Acquire(lock); /*This system call should give an error */

	/*Method 2*/
	lock = CreateLock();
	Acquire(lock);
	DestroyLock(lock);
	Acquire(lock); /*this should still be valid */
	Release(lock); /*at this point the kernel should destroy the lock */
	Acquire(lock); /*this should give an error */


}

void startTestMutex(){
	int i;
	testCounter = 0;
	for(i = 0; i < 10; i++){
		NPrint("startTestMutex() forking a thread\n", sizeof("startTestMutex() forking a thread\n"), 0, 0);
	}
}

void testMutex(){
	Acquire(lock);
	NPrint("Incrementing testCounter from %d to %d\n", sizeof("Incrementing testCounter from %d to %d\n"), testCounter, testCounter + 1);
	testCounter++;
	Release(lock);
}


/*We will fork many threads to this same function.  Basically, they will all try to acquire a lock and then print.
 * Only one print should ever happen, because we won't ever release the lock
 */
void testAcquire(){
	Acquire(lock);
	NPrint("testAcquire now printing\n", sizeof("testAcquire now printing\n"), 0, 0);
}

void startTestAcquire(){
	int i;
	lock = CreateLock();
	for(i = 0; i < 10; i++){
		NPrint("startTestAcquire() forking a thread\n", sizeof("startTestAcquire() forking a thread\n"), 0, 0);
		Fork(testAcquire);
	}
}

/*We will create some condition variables, but then try to access invalid locks in acquires, releases, and destroys
 * in order to exercise the validation in condition syscalls
 */
void testConditionArrayBoundaries(){
	int condition1 = CreateCondition();
	int condition2 = CreateCondition();
	int lock1 = CreateLock();
	int lock2 = CreateLock();
	/*This is the only thread running, so we know the only two valid indices are 0 and 1 */

	/*Exercise Wait's validation procedures */
	Wait(3, lock1);
	Wait(-1, lock1);
	Wait(10000, lock1);

	/*Excercise Wait's validation procedures */
	Signal(3, lock1);
	Signal(-1, lock1);
	Signal(1000000, lock1);

	/*Exercise Broadcast's validation procedures */
	Broadcast(3, lock1);
	Broadcast(-1, lock1);
	Broadcast(1000000, lock1);

	/*Exercise DestroyCondition's validation procedures */
	DestroyCondition(3);
	DestroyCondition(-1);
	DestroyCondition(1000000);

	/*Now do valid operations */
	Acquire(lock1);
	Signal(condition1, lock1);
	Broadcast(condition1, lock1);
	Wait(condition1, lock1);

}

/*We will test that within Condition system calls, locks are still validated */
void testLockBoundariesWithConditions(){
	int condition1 = CreateCondition();
	int lock1 = CreateCondition();

	/*Exercise lock validation procedures in Signal */
	Signal(condition1, 1);
	Signal(condition1, -1);
	Signal(condition1, 30304);

	/*Exercise lock validation procedures in Broadcast */
	Broadcast(condition1, 1);
	Broadcast(condition1, -1);
	Broadcast(condition1, 30304);

	/*Exercise lock validation proceudres in Wait*/
	Wait(condition1, 1);
	Wait(condition1, -1);
	Wait(condition1, 30304);

	/*One last check, make sure a previously existing but now destroyed lock doesn't
	 * pass validation in all Condition operations
	 */
	DestroyLock(lock1);
	Signal(condition1, lock1);
	Broadcast(condition1, lock1);
	Wait(condition1, lock1);
}

/*This method is a companition to DestroyCondition so that we can test proper lock deletion when someone is waiting on a condition
 *
 */
void testDestroyConditionCompanion(){
	Signal(condition, lock);
	Wait(condition, lock);
}
/*We will test the destruction of a condition in 2 ways: creating a condition and destroying it without a wait,
 * in order to see that it will be destroyed upon our call to DestroyCondition since no thread is waiting on the condition.
 * To test that it is destroyed, we will try to Signal() the condition.
 * The second way we will test destroying the Condition is to create it, have a thread wait on it, and then try to destroy it.  We will then
 * signal it again, which if the condition still exists, won't give us any error messages. We will then do another wait on the condition, but it should now be destroyed.
 * A third test will create the condition, fork to another method that will wait on the condition, and then we will destroy the lock, check if it still exists while a thread
 * waits on an associated condition by Acquiring, and then wake up the thread on the condition.  Finally, we test if the lock has now been deleted by calling acquire on it, which should
 * give us an error.
 */
void testDestroyCondition(){
	/*Method 1*/
	condition = CreateCondition();
	lock = CreateLock();
	Acquire(lock);
	DestroyCondition(condition);
	Signal(condition, lock); /*This system call should give an error */

	/*Method 2*/
	condition = CreateCondition();
	Fork(testDestroyConditionCompanion);
	Wait(condition, lock);
	DestroyCondition(condition);
	Signal(condition);
	Wait(condition);

	/*Method 3*/
	condition = CreateCondition();
	Fork(testDestroyConditionCompanion);
	Wait(condition, lock);
	DestroyLock(lock);
	Acquire(lock);
	Signal(condition);
	Acquire(lock);

}

void testConditionSequencingCompanion(){
	int j;
	Acquire(lock);
	for(j = 0; j < 10; j++){
		testCounter++;
		Signal(condition, lock);
		Wait(condition, lock);
	}
}

void testConditionSequencing(){
	int i;
	lock = CreateLock();
	condition = CreateCondition();
	testCounter = 0;
	Acquire(lock);
	NPrint("testConditionSequencing has value of %d\n", sizeof("testConditionSequencing has value of %d\n"), testCounter, 0);
	Fork(testConditionSequencingCompanion);
	for(i = 0; i < 10; i++){
		Signal(condition, lock);
		Wait(condition, lock);
		NPrint("testConditionSequencing has value of %d\n", sizeof("testConditionSequencing has value of %d\n"), testCounter, 0);
	}
}


void testNEncode2to1() {
	int e = NEncode2to1(5, 10);
	NPrint("NEncode2to1 test starting...\n", sizeof("NEncode2to1 test starting...\n"), 0, 0);
	NPrint("Encoded value for v1 = 5 and v2 = 10: %d\n", sizeof("Encoded value for v1 = 5 and v2 = 10: %d\n"), e, 0);
	NPrint("NEncode2to1 test complete!\n\n", sizeof("NEncode2to1 test complete!\n\n"), 0, 0);
}

void testNPrint()
{
	int enc1 = 0;
	int enc2 = 0;
	NPrint("NPrint test starting\n", sizeof("NPrint test starting\n"), 0, 0);

	NPrint("Testing printing...\n", sizeof("Testing printing...\n"), 0, 0);
	NPrint("Testing printing the number 7: %d\n", sizeof("Testing printing the number 7: %d\n"), 7, 0);
	NPrint("Testing printing the first 4 numbers constants: %d, %d, %d, %d\n", sizeof("Testing printing the first 4 numbers constants: %d, %d, %d, %d\n"),
			65536, 262147);
	NPrint("Testing printing the first 4 numbers encoded: %d, %d, %d, %d\n", sizeof("Testing printing the first 4 numbers encoded: %d, %d, %d, %d\n"),
			NEncode2to1(0, 1), NEncode2to1(3, 4));
	NPrint("Values should have been equal\n", sizeof("Values should have been equal\n"), 0, 0);

	NPrint("NPrint test complete!\n\n", sizeof("NPrint test complete!\n\n"), 0, 0);
	/*Exit(0);*/
}

void testExec() {
	NPrint("Exec test starting...\n", sizeof("Exec test starting...\n"), 0, 0);

	Exec("../test/sort", sizeof("../test/sort"));
	NPrint("Exec test: sort executed\n", sizeof("Exec test: sort complete\n"), 0, 0);
	Exec("../test/matmult", sizeof("../test/matmult"));
	NPrint("Exec test: matmult executed\n", sizeof("Exec test: matmult complete\n"), 0, 0);

	NPrint("Exec test complete!\n\n", sizeof("Exec test complete!\n\n"), 0, 0);
	/*Exec("halt", 4);*/
}

int main(int argc, char** argv) {

	/*char *buffer = "something";		 //What even is this?
	Write(buffer, sizeof(buffer[]), 1);	 //are these arguments in the right place? */

	testNEncode2to1();
	testNPrint();
	testExec();

	return 0;
	Exit(0);
}
