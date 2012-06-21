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

void testYieldCompanion(){
	NPrint("testYieldCompanion is printing\n", sizeof("testYieldCompanion is printing\n"), 0, 0);
	Exit(0);
}
/*We will test yield by forking to another function, which prints a different statement than this function.
 * This function will call yield after it forks.  When tested WITHOUT AN RS VALUE, the forked method should always print first
 */
void testYield(){
	int i;
	Fork(testYieldCompanion, "testYield companion thread", sizeof("testYield companion thread"));
	Yield();
	NPrint("testYield is printing\n", sizeof("testYield is printing\n"), 0, 0);

}

/*We will create some locks, but then try to access invalid locks in acquires, releases, and destroys
 * in order to exercise the validation in lock syscalls
 */
void testLockArrayBoundaries(){

	int lock1 = CreateLock();
	int lock2 = CreateLock();
	NPrint("Starting testLockArrayBoundaries()\n", sizeof("Starting testLockArrayBoundaries()\n"));
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
	NPrint("Created lock\n", sizeof("Created lock\n"));
	DestroyLock(lock);
	NPrint("Destroyed lock\n", sizeof("Destroyed lock\n"));
	Acquire(lock); /*This system call should give an error */

	/*Method 2*/
	lock = CreateLock();
	NPrint("Created lock\n", sizeof("Created lock\n"));
	Acquire(lock);
	NPrint("Acquired lock\n", sizeof("Acquired lock\n"));
	DestroyLock(lock);
	NPrint("Called destroy lock\n", sizeof("Called destroy lock\n"));
	NPrint("Acquiring lock\n", sizeof("Acquiring lock\n"));
	Acquire(lock); /*this should still be valid */
	NPrint("Releasing lock\n", sizeof("Releasing lock\n"));
	Release(lock); /*at this point the kernel should destroy the lock */
	NPrint("Acquiring lock\n", sizeof("Acquiring lock\n"));
	Acquire(lock); /*this should give an error */


}


/*Only one of these function should be able to increment the testCounter at once. Every value of testCounter should be visited in ascending order */
void testMutex(){
	Acquire(lock);
	NPrint("Incrementing testCounter from %d to %d\n", sizeof("Incrementing testCounter from %d to %d\n"), NEncode2to1(testCounter, testCounter + 1));
	testCounter++;
	Release(lock);
	Exit(0);
}

/*Function that will fork threads to testMutex.  We assume fork is working in order to test this.  We don't care in what order threads are forked,
 * as long as the CS in testMutex() is accessed in accordance with mutual exclusion
 */
void startTestMutex(){
	int i;
	lock = CreateLock("mutex test lock", sizeof("mutex test lock"));
	testCounter = 0;
	for(i = 0; i < 10; i++){
		NPrint("startTestMutex() forking a thread\n", sizeof("startTestMutex() forking a thread\n"), 0, 0);
		Fork(testMutex, "testMutex thread", sizeof("testMutex thread"));
	}
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
		Fork(testAcquire, "testAcquire thread", sizeof("testAcquire thread"));
	}
}

/*We will create some condition variables, but then try to access invalid locks in acquires, releases, and destroys
 * in order to exercise the validation in condition syscalls
 */
void testConditionArrayBoundaries(){
	int condition1 = CreateCondition("condition 1", sizeof("condition 1"));
	int condition2 = CreateCondition("condition 2", sizeof("condition 2"));
	int lock1 = CreateLock("lock 1", sizeof("lock 1"));
	int lock2 = CreateLock("lock 2", sizeof("lock 2"));
	/*This is the only thread running, so we know the only two valid indices are 0 and 1 */

	NPrint("Conditions w/indices %d and &d have been created\n", sizeof("Conditions w/ indicies %d and %d have been created\n"), condition1, condition2);
	NPrint("Locks w/ indices %d and &d have been created\n", sizeof("Locks w/ indices %d and %d have been created\n"), condition1, condition2);
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
	int condition1 = CreateCondition("condition1", sizeof("condition1"));
	int lock1 = CreateLock("lock1", sizeof("lock1"));

	NPrint("Condition w/index %d has been created\n", sizeof("Condition w/ index %d has been created\n"), condition1);
	NPrint("Lock w/ index %d has been created\n", sizeof("Lock w/ index %d has been created\n"), condition1);
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
	NPrint("Companion thread starting\n", sizeof("Companion thread starting\n"));
	Acquire(lock);
	Signal(condition, lock);
	Wait(condition, lock);
	NPrint("Thread destroyConditionCompanion has woken up after Wait\n", sizeof("Thread destroyConditionCompanion has woken up after Wait\n"));
	Release(lock);
	Exit(0);
}
/*We will test the destruction of a condition in 2 ways: creating a condition and destroying it without a wait,
 * in order to see that it will be destroyed upon our call to DestroyCondition since no thread is waiting on the condition.
 * To test that it is destroyed, we will try to Signal() the condition.
 * The second way we will test destroying the Condition is to create it, have a thread wait on it, and then try to destroy it.  We will then
 * signal it again, which if the condition still exists, won't give us any error messages. We will then do another wait on the condition, but it should now be destroyed.
 * A third test will create the condition, fork to another method that will wait on the condition, and then we will destroy the lock, check if it still exists while a thread
 * waits on an associated condition by Acquiring, We should be blocked and not able to acquire.
 */
void testDestroyCondition(){
	/*Method 1*/
	NPrint("\n\nStarting first DestroyCondition test\n", sizeof("\n\nStarting first DestroyCondition test\n"));
	condition = CreateCondition("testCondition", sizeof("testCondition"));
	NPrint("Created condition\n", sizeof("Created condition\n"));
	lock = CreateLock("test lock", sizeof("test lock"));
	NPrint("Created lock\n", sizeof("Created lock\n"));
	Acquire(lock);
	DestroyCondition(condition);
	NPrint("Destroyed condition\n", sizeof("Destroyed condition\n"));
	Signal(condition, lock); /*This system call should give an error */

	/*Method 2*/
	NPrint("\n\nStarting second DestroyCondition test\n", sizeof("\n\nStarting first DestroyCondition test\n"));
	condition = CreateCondition("testCondition", sizeof("testCondition"));
	NPrint("Created condition\n", sizeof("Created condition\n"));
	NPrint("Forking companion thread\n", sizeof("Forking companion thread\n"));
	Fork(testDestroyConditionCompanion, "destroy condition companion", sizeof("destroy condition companion"));
	NPrint("About to call Wait on the condition\n", sizeof("About to call Wait on the condition\n"));
	Wait(condition, lock);
	DestroyCondition(condition);
	NPrint("Destroyed condition\n", sizeof("Destroyed condition\n"));
	NPrint("About to call Signal on the condition\n", sizeof("About to call Signal on the condition\n"));
	Signal(condition, lock);
	Release();
	Yield();
	NPrint("About to call Wait on the condition\n", sizeof("About to call Wait on the condition\n"));
	Wait(condition, lock);

	/*Method 3*/
	Acquire(lock);
	NPrint("\n\nStarting third DestroyCondition test\n", sizeof("\n\nStarting third DestroyCondition test\n"));
	condition = CreateCondition("testCondition", sizeof("testCondition"));
	Fork(testDestroyConditionCompanion, "destroy condition companion", sizeof("destroy condition companion"));
	Wait(condition, lock);
	DestroyLock(lock);
	NPrint("Destroyed lock\n", sizeof("Destroyed lock\n"));
	NPrint("About to call Signal on the condition\n", sizeof("About to call Signal on the condition\n"));
	Signal(condition, lock);
	NPrint("Releaseing lock\n", sizeof("Releasing lock\n"));
	Release(lock);
	Yield();
	Acquire(lock);
	Exit(0);

}

void testConditionSequencingCompanion(){
	int j;
	Acquire(lock);
	for(j = 0; j < 10; j++){
		NPrint("Thread 2 has value of %d\n", sizeof("Thread 2 has value of %d\n"), testCounter);
		testCounter++;
		Signal(condition, lock);
		Wait(condition, lock);

	}
	Exit(0);
}

/*This test emulates the "tennis" depiction of condition based sequencing.  The value of testCounter should increment one by one as it is
 * "hit back and forth" between the two threads we create
 */
void testConditionSequencing(){
	int i;
	lock = CreateLock("test lock", sizeof("test lock"));
	condition = CreateCondition();
	testCounter = 0;
	Acquire(lock);
	Fork(testConditionSequencingCompanion, "test condition sequencing companion thread", sizeof("test condition sequencing companion thread"));
	for(i = 0; i < 10; i++){
		Wait(condition, lock);
		NPrint("Thread 1 has value of %d\n", sizeof("testConditionSequencing has value of %d\n"), testCounter, 0);
		testCounter++;
		Signal(condition, lock);

	}
	Exit(0);
}

void testAddressSpaceVerification(){
	int i;
	condition = CreateCondition();
	lock = CreateLock();
	Exec("../test/addrspace_verification", sizeof("../test/addrspace_verification"), "addrspace verification thread", sizeof("addrspace verification thread"));
	for(i = 0; i < 1000; i++){
		Yield();
	}
}

void testCreatingTooManyLocks(){
	int i;
	for(i = 0; i < 1000; i++){
		CreateLock();
	}
}

void testCreatingTooManyConditions(){
	int i;
	for(i = 0; i < 1000; i++){
		CreateCondition();
	}
}

int main(int argc, char** argv) {
	int choice;
	/*char *buffer = "something";		 //What even is this?
	Write(buffer, sizeof(buffer[]), 1);	 //are these arguments in the right place? */

	NPrint("\n\nRunning Project 2 Test Suite\n", sizeof("\n\nRunning Project 2 Test Suite\n"), 0, 0);
	NPrint("1. Test lock array boundaries\n", sizeof("1. Test lock array boundaries\n"), 0, 0);
	NPrint("2. Test DestroyLock()\n", sizeof("2. Test DestroyLock()\n"), 0, 0);
	NPrint("3. Test Condition array boundaries\n", sizeof("2. Test Condition array boundaries\n"), 0, 0);
	NPrint("4. Test lock array boundaries in Condition operations\n", sizeof("4. Test lock array boundaries in Condition operations\n"), 0, 0);
	NPrint("5. Test DestroyCondition()\n", sizeof("5. Test DestroyCondition()\n"), 0, 0);
	NPrint("6. Test Acquire()\n", sizeof("6. Test Acquire()\n"), 0, 0);
	NPrint("7. Test mutual exclusion with locks\n", sizeof("7. Test mutual exclusion with locks\n"), 0, 0);
	NPrint("8. Test sequencing via condition variables\n", sizeof("8. Test sequencing via condition variables\n"), 0, 0);
	NPrint("9. Test yield\n", sizeof("9. Test yield\n"));
	NPrint("10. Test address space verification in both locks and conditions\n", sizeof("10. Test address space verification in both locks and conditions\n"));
	NPrint("11. Test creating too many locks\n", sizeof("11. Test creating too many locks\n"));
	NPrint("12. Test creating too many conditions\n", sizeof("12. Test creating too many conditions\n"));

	choice = ReadInt("Please enter a menu choice:\n", sizeof("Please enter a menu choice:\n"));
	NPrint("Choice: %d\n", sizeof("Choice: %d\n"), choice, 0);
	switch(choice){
	case 1:
		testLockArrayBoundaries();
		break;
	case 2:
		testDestroyLock();
		break;
	case 3:
		testConditionArrayBoundaries();
		break;
	case 4:
		testLockBoundariesWithConditions();
		break;
	case 5:
		testDestroyCondition();
		break;
	case 6:
		startTestAcquire();
		break;
	case 7:
		startTestMutex();
		break;
	case 8:
		testConditionSequencing();
		break;
	case 9:
		testYield();
		break;
	case 10:
		testAddressSpaceVerification();
		break;
	case 11:
		testCreatingTooManyLocks();
		break;
	case 12:
		testCreatingTooManyConditions();
		break;
	default:
		NPrint("No Choice?\n", sizeof("No Choice?\n"), 0, 0);
		break;
	}
	Exit(0);
}
