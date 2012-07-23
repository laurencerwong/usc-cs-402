// Threadtest.cc
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield,
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.
#include "copyright.h"
#include "system.h"
#include "stdlib.h"
#include <iostream>
//#include "time.h"UNPRIVILEGED_CUSTOMER
#include "../userprog/syscall.h"
//#include <queue>

using namespace std;

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
	int num;

	for (num = 0; num < 5; num++) {
		printf("*** thread %d looped %d times\n", which, num);
		currentThread->Yield();
	}
}

//----------------------------------------------------------------------
// ThreadTest
// 	Set up a ping-pong between two threads, by forking a thread
//	To call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest()
{
	DEBUG('t', "Entering SimpleTest");

	Thread *t = new Thread("forked thread");

	t->Fork(SimpleThread, 1);
	SimpleThread(0);
}

//	Simple test cases for the threads assignment.
//

#include "copyright.h"
#include "system.h"
#ifdef CHANGED
#include "synch.h"
#endif

#ifdef CHANGED
// --------------------------------------------------
// Test Suite
// --------------------------------------------------


// --------------------------------------------------
// Test 1 - see TestSuite() for details
// --------------------------------------------------
Semaphore t1_s1("t1_s1",0);       // To make sure t1_t1 acquires the
// lock before t1_t2
Semaphore t1_s2("t1_s2",0);       // To make sure t1_t2 Is waiting on the
// lock before t1_t3 releases it
Semaphore t1_s3("t1_s3",0);       // To make sure t1_t1 does not release the
// lock before t1_t3 tries to acquire it
Semaphore t1_done("t1_done",0);   // So that TestSuite knows when Test 1 is
// done
Lock t1_l1("t1_l1");		  // the lock tested in Test 1

// --------------------------------------------------
// t1_t1() -- test1 thread 1
//     This is the rightful lock owner
// --------------------------------------------------
void t1_t1() {
	t1_l1.Acquire();
	t1_s1.V();  // Allow t1_t2 to try to Acquire Lock

	printf ("%s: Acquired Lock %s, waiting for t3\n",currentThread->getName(),
			t1_l1.getName());
	t1_s3.P();
	printf ("%s: working in CS\n",currentThread->getName());
	for (int i = 0; i < 1000000; i++) ;
	printf ("%s: Releasing Lock %s\n",currentThread->getName(),
			t1_l1.getName());
	t1_l1.Release();
	t1_done.V();
}

// --------------------------------------------------
// t1_t2() -- test1 thread 2
//     This thread will wait on the held lock.
// --------------------------------------------------
void t1_t2() {

	t1_s1.P();	// Wait until t1 has the lock
	t1_s2.V();  // Let t3 try to acquire the lock

	printf("%s: trying to acquire lock %s\n",currentThread->getName(),
			t1_l1.getName());
	t1_l1.Acquire();

	printf ("%s: Acquired Lock %s, working in CS\n",currentThread->getName(),
			t1_l1.getName());
	for (int i = 0; i < 10; i++)
		;
	printf ("%s: Releasing Lock %s\n",currentThread->getName(),
			t1_l1.getName());
	t1_l1.Release();
	t1_done.V();
}

// --------------------------------------------------
// t1_t3() -- test1 thread 3
//     This thread will try to release the lock illegally
// --------------------------------------------------
void t1_t3() {

	t1_s2.P();	// Wait until t2 is ready to try to acquire the lock

	t1_s3.V();	// Let t1 do it's stuff
	for ( int i = 0; i < 3; i++ ) {
		printf("%s: Trying to release Lock %s\n",currentThread->getName(),
				t1_l1.getName());
		t1_l1.Release();
	}
}

// --------------------------------------------------
// Test 2 - see TestSuite() for details
// --------------------------------------------------
Lock t2_l1("t2_l1");		// For mutual exclusion
Condition t2_c1("t2_c1");	// The condition variable to test
Semaphore t2_s1("t2_s1",0);	// To ensure the Signal comes before the wait
Semaphore t2_done("t2_done",0);     // So that TestSuite knows when Test 2 is
// done

// --------------------------------------------------
// t2_t1() -- test 2 thread 1
//     This thread will signal a variable with nothing waiting
// --------------------------------------------------
void t2_t1() {
	t2_l1.Acquire();
	printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
			t2_l1.getName(), t2_c1.getName());
	t2_c1.Signal(&t2_l1);
	printf("%s: Releasing Lock %s\n",currentThread->getName(),
			t2_l1.getName());
	t2_l1.Release();
	t2_s1.V();	// release t2_t2
	t2_done.V();
}

// --------------------------------------------------
// t2_t2() -- test 2 thread 2
//     This thread will wait on a pre-signalled variable
// --------------------------------------------------
void t2_t2() {
	t2_s1.P();	// Wait for t2_t1 to be done with the lock
	t2_l1.Acquire();
	printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
			t2_l1.getName(), t2_c1.getName());
	t2_c1.Wait(&t2_l1);
	printf("%s: Releasing Lock %s\n",currentThread->getName(),
			t2_l1.getName());
	t2_l1.Release();
}
// --------------------------------------------------
// Test 3 - see TestSuite() for details
// --------------------------------------------------
Lock t3_l1("t3_l1");		// For mutual exclusion
Condition t3_c1("t3_c1");	// The condition variable to test
Semaphore t3_s1("t3_s1",0);	// To ensure the Signal comes before the wait
Semaphore t3_done("t3_done",0); // So that TestSuite knows when Test 3 is
// done

// --------------------------------------------------
// t3_waiter()
//     These threads will wait on the t3_c1 condition variable.  Only
//     one t3_waiter will be released
// --------------------------------------------------
void t3_waiter() {
	t3_l1.Acquire();
	t3_s1.V();		// Let the signaller know we're ready to wait
	printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
			t3_l1.getName(), t3_c1.getName());
	t3_c1.Wait(&t3_l1);
	printf("%s: freed from %s\n",currentThread->getName(), t3_c1.getName());
	t3_l1.Release();
	t3_done.V();
}


// --------------------------------------------------
// t3_signaller()
//     This threads will signal the t3_c1 condition variable.  Only
//     one t3_signaller will be released
// --------------------------------------------------
void t3_signaller() {

	// Don't signal until someone's waiting

	for ( int i = 0; i < 5 ; i++ )
		t3_s1.P();
	t3_l1.Acquire();
	printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
			t3_l1.getName(), t3_c1.getName());
	t3_c1.Signal(&t3_l1);
	printf("%s: Releasing %s\n",currentThread->getName(), t3_l1.getName());
	t3_l1.Release();
	t3_done.V();
}

// --------------------------------------------------
// Test 4 - see TestSuite() for details
// --------------------------------------------------
Lock t4_l1("t4_l1");		// For mutual exclusion
Condition t4_c1("t4_c1");	// The condition variable to test
Semaphore t4_s1("t4_s1",0);	// To ensure the Signal comes before the wait
Semaphore t4_done("t4_done",0); // So that TestSuite knows when Test 4 is
// done

// --------------------------------------------------
// t4_waiter()
//     These threads will wait on the t4_c1 condition variable.  All
//     t4_waiters will be released
// --------------------------------------------------
void t4_waiter() {
	t4_l1.Acquire();
	t4_s1.V();		// Let the signaller know we're ready to wait
	printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
			t4_l1.getName(), t4_c1.getName());
	t4_c1.Wait(&t4_l1);
	printf("%s: freed from %s\n",currentThread->getName(), t4_c1.getName());
	t4_l1.Release();
	t4_done.V();
}


// --------------------------------------------------
// t2_signaller()
//     This thread will broadcast to the t4_c1 condition variable.
//     All t4_waiters will be released
// --------------------------------------------------
void t4_signaller() {

	// Don't broadcast until someone's waiting

	for ( int i = 0; i < 5 ; i++ )
		t4_s1.P();
	t4_l1.Acquire();
	printf("%s: Lock %s acquired, broadcasting %s\n",currentThread->getName(),
			t4_l1.getName(), t4_c1.getName());
	t4_c1.Broadcast(&t4_l1);
	printf("%s: Releasing %s\n",currentThread->getName(), t4_l1.getName());
	t4_l1.Release();
	t4_done.V();
}
// --------------------------------------------------
// Test 5 - see TestSuite() for details
// --------------------------------------------------
Lock t5_l1("t5_l1");		// For mutual exclusion
Lock t5_l2("t5_l2");		// Second lock for the bad behavior
Condition t5_c1("t5_c1");	// The condition variable to test
Semaphore t5_s1("t5_s1",0);	// To make sure t5_t2 acquires the lock after
// t5_t1

// --------------------------------------------------
// t5_t1() -- test 5 thread 1
//     This thread will wait on a condition under t5_l1
// --------------------------------------------------
void t5_t1() {
	t5_l1.Acquire();
	t5_s1.V();	// release t5_t2
	printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
			t5_l1.getName(), t5_c1.getName());
	t5_c1.Wait(&t5_l1);
	printf("%s: Releasing Lock %s\n",currentThread->getName(),
			t5_l1.getName());
	t5_l1.Release();
}

// --------------------------------------------------
// t5_t1() -- test 5 thread 1
//     This thread will wait on a t5_c1 condition under t5_l2, which is
//     a Fatal error
// --------------------------------------------------
void t5_t2() {
	t5_s1.P();	// Wait for t5_t1 to get into the monitor
	t5_l1.Acquire();
	t5_l2.Acquire();
	printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
			t5_l2.getName(), t5_c1.getName());
	t5_c1.Signal(&t5_l2);
	printf("%s: Releasing Lock %s\n",currentThread->getName(),
			t5_l2.getName());
	t5_l2.Release();
	printf("%s: Releasing Lock %s\n",currentThread->getName(),
			t5_l1.getName());
	t5_l1.Release();
}

// --------------------------------------------------
// TestSuite()
//     This is the main thread of the test suite.  It runs the
//     following tests:
//
//       1.  Show that a thread trying to release a lock it does not
//       hold does not work
//
//       2.  Show that Signals are not stored -- a Signal with no
//       thread waiting is ignored
//
//       3.  Show that Signal only wakes 1 thread
//
//	 4.  Show that Broadcast wakes all waiting threads
//
//       5.  Show that Signalling a thread waiting under one lock
//       while holding another is a Fatal error
//
//     Fatal errors terminate the thread in question.
// --------------------------------------------------
void TestSuite() {
	Thread *t;
	char *name;
	int i;

	// Test 1

	printf("Starting Test 1\n");

	t = new Thread("t1_t1");
	t->Fork((VoidFunctionPtr)t1_t1,0);

	t = new Thread("t1_t2");
	t->Fork((VoidFunctionPtr)t1_t2,0);

	t = new Thread("t1_t3");
	t->Fork((VoidFunctionPtr)t1_t3,0);

	// Wait for Test 1 to complete
	for (  i = 0; i < 2; i++ )
		t1_done.P();

	// Test 2

	printf("Starting Test 2.  Note that it is an error if thread t2_t2\n");
	printf("completes\n");

	t = new Thread("t2_t1");
	t->Fork((VoidFunctionPtr)t2_t1,0);

	t = new Thread("t2_t2");
	t->Fork((VoidFunctionPtr)t2_t2,0);

	// Wait for Test 2 to complete
	t2_done.P();

	// Test 3

	printf("Starting Test 3\n");

	for (  i = 0 ; i < 5 ; i++ ) {
		name = new char [20];
		sprintf(name,"t3_waiter%d",i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr)t3_waiter,0);
	}
	t = new Thread("t3_signaller");
	t->Fork((VoidFunctionPtr)t3_signaller,0);

	// Wait for Test 3 to complete
	for (  i = 0; i < 2; i++ )
		t3_done.P();

	// Test 4

	printf("Starting Test 4\n");

	for (  i = 0 ; i < 5 ; i++ ) {
		name = new char [20];
		sprintf(name,"t4_waiter%d",i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr)t4_waiter,0);
	}
	t = new Thread("t4_signaller");
	t->Fork((VoidFunctionPtr)t4_signaller,0);

	// Wait for Test 4 to complete
	for (  i = 0; i < 6; i++ )
		t4_done.P();

	// Test 5

	printf("Starting Test 5.  Note that it is an error if thread t5_t1\n");
	printf("completes\n");

	t = new Thread("t5_t1");
	t->Fork((VoidFunctionPtr)t5_t1,0);

	t = new Thread("t5_t2");
	t->Fork((VoidFunctionPtr)t5_t2,0);

}
#endif



/*
 *
 *  Supermarket Code
 *
 */
/*
#define MAX_SHELF_QTY 20
#define MAX_TROLLY 40
#define MAX_SALESMEN 3
#define MAX_LOADERS 10
#define MAX_CUSTOMERS 30
#define NUM_ITEMS 10
 */

//
//
//#define MAX_CASHIERS 5
//#define MAX_SALESMEN 3
//#define MAX_LOADERS 5
//#define MAX_DEPARTMENTS 5
//#define MAX_CUSTOMERS 100
//#define MAX_MANAGER_ITEMS 100
//#define MAX_ITEMS 10
//#define MAX_LOADER_JOB_QUEUE 100
//
//int maxShelfQty = 15;
//int numTrollies = 40;
//int numSalesmen = 3;
//int numLoaders = 10;
//int numItems = 10;
//int numDepartments = 3;
//int maxCustomers = 100;
//
//// Enums for the salesmen status
//
//enum SalesmanStatus {SALES_NOT_BUSY, SALES_BUSY, SALES_GO_ON_BREAK, SALES_ON_BREAK, SALES_READY_TO_TALK, SALES_READY_TO_TALK_TO_LOADER, SALES_SIGNALLING_LOADER, LOAD_GET_ITEMS_FROM_MANAGER};
//SalesmanStatus currentSalesStatus [MAX_DEPARTMENTS][MAX_SALESMEN];
//enum WhomIWantToTalkTo {GREETING, COMPLAINING, GOODSLOADER, SALESMAN, MANAGER, UNKNOWN};
//WhomIWantToTalkTo currentlyTalkingTo [MAX_DEPARTMENTS][MAX_SALESMEN];
//
///*Static allocation of cashier variables */
//
//enum CustomerType {PRIVILEGED_CUSTOMER, CUSTOMER};
//CustomerType custType [MAX_CASHIERS];
//
//int total [MAX_CASHIERS];
//int custID [MAX_CASHIERS];
//
//int cashierLinesLock = CreateLock("cashierLineLock", sizeof("cashierLineLock"));
////will be grabbed when a privileged customer checks line lengths/cashier status
////Lock* cashierLinesLock;
//
//
////array of CVs for privileged customers waiting in line
////one for each cashier
////Condition** privilegedCashierLineCV;
//int privilegedCashierLineCV[MAX_CASHIERS];
//
////array of CVs for unprivileged customers waiting in line
////one for each cashier
////Condition** unprivilegedCashierLineCV;
//int unprivilegedCashierLineCV[MAX_CASHIERS];
//
////Lock associated with cashToCustCV
////controls access to each cashier
////Lock** cashierLock;
//int cashierLock[MAX_CASHIERS];
////array of CVs, one for each cashier, that the cashier and
////customer use to communicate during checkout
////Condition** cashierToCustCV;
//int cashierToCustCV[MAX_CASHIERS];
//
//enum CashierStatus {CASH_NOT_BUSY, CASH_BUSY, CASH_ON_BREAK, CASH_GO_ON_BREAK, CASH_READY_TO_TALK};
////array of current cashier statuses
//CashierStatus cashierStatus[MAX_CASHIERS];
//
////array of privileged line counts
//int privilegedLineCount[MAX_CASHIERS];
//
////number of privileged customers
//int privCustomers[MAX_CUSTOMERS];
////array of unprivileged line counts
//int unprivilegedLineCount[MAX_CASHIERS];
//
////array of ints representing the item a customer has handed to a cashier
//int cashierDesk[MAX_CASHIERS];
//
////array representing money in each cash register
//int cashRegister[MAX_CASHIERS];
//
////next cashier index for the locks
//int nextCashierIndex = 0;
//
//int cashierIndexLock = CreateLock("cashierIndexLock", sizeof("cashierIndexLock"));
//
////temporary global variable
//int customerCash = 20;
//
////number of cashiers
//int cashierNumber = 5;
//
////number of customers
//int custNumber = 20;
//
//
//
///*Statically allocating things for the salesman*/
//int nextSalesmanIndex = 0;
//
//int salesmanIndexLock = CreateLock("salesmanIndexLock", sizeof("salsemanIndexLock"));
//int departmentIndexLock = CreateLock("departmentIndexLock", sizeof("departmentIndexLock"));
//
//int nextDepartmenIndex = 0;
//
//int salesCustNumber [MAX_DEPARTMENTS][MAX_SALESMEN]; //The array of customer indicies that the customers update
//int salesDesk [MAX_DEPARTMENTS][MAX_SALESMEN];
//int salesBreakBoard [MAX_DEPARTMENTS][MAX_SALESMEN];
////Condition ***salesBreakCV;
//int salesBreakCV[MAX_DEPARTMENTS][MAX_SALESMEN];
//
//int greetingCustWaitingLineCount[MAX_DEPARTMENTS]; //How many customers need to be greeted
//int complainingCustWaitingLineCount[MAX_DEPARTMENTS]; //How many customers need to be helped with empty shelves
//int loaderWaitingLineCount[MAX_DEPARTMENTS];	//How many loaders are waiting on salesmen
//
//int individualSalesmanLock[MAX_DEPARTMENTS][MAX_SALESMEN]; //The lock that each salesman uses for his own "desk"
//int salesLock[MAX_DEPARTMENTS]; //The lock that protects the salesmen statuses and the line for all three
//
//
//int salesmanCV[MAX_DEPARTMENTS][MAX_SALESMEN]; //The condition variable for one on one interactions with the Salesman
//int greetingCustCV[MAX_DEPARTMENTS]; //The condition var that represents the line all the greeters stay in
//int complainingCustCV[MAX_DEPARTMENTS]; //Represents the line that the complainers stay in
//int loaderCV[MAX_DEPARTMENTS];	//Represents the line that loaders wait in
//
////WhomIWantToTalkTo *loaderCurrentlyTalkingTo = new WhomIWantToTalkTo[numLoaders];
//
//
//enum LoaderStatus {LOAD_NOT_BUSY, LOAD_STOCKING, LOAD_HAS_BEEN_SIGNALLED};
//LoaderStatus loaderStatus[MAX_LOADERS];
//
////Lock *inactiveLoaderLock;
//int inactiveLoaderLock = CreateLock("inactiveLoaderLock", sizeof("inactiveLoaderLock"));
//
////Condition *inactiveLoaderCV;
//int inactiveLoaderCV = CreateCondition("inactiveLoaderCV", sizeof("inactiveLoaderCV"));
//
////queue<int> loaderJobQueue;
//int loaderJobQueue [MAX_LOADER_JOB_QUEUE];
//
////Lock ***shelfLock;
//int shelfLock [MAX_DEPARTMENTS][MAX_ITEMS];
//
////Condition ***shelfCV;
//int shelfCV [MAX_DEPARTMENTS][MAX_ITEMS];
//
//int shelfInventory [MAX_DEPARTMENTS][MAX_ITEMS];
//
////Lock *stockRoomLock;
//int stockRoomLock = CreateLock("stockRoomLock", sizeof("stockRoomLock"));
//
//int nextLoaderIndex = 0;
//int loaderIndexLock = CreateLock("loaderIndexLock", sizeof("loaderIndexLock"));
//
////Manager data
//int cashierFlags[MAX_CASHIERS]; //will be set to a customer ID in the slot corresponding to the
////index of the cashier who sets the flag.
//
////Lock* managerLock; //will be the lock behind the manager CV
//int managerLock = CreateLock("managerLock", sizeof("managerLock"));
////Condition* managerCV; //will be the CV the manager and customer use to communicate
//int managerCV = CreateCondition("managerCV", sizeof("managerCV"));
//int managerDesk = 0; //will be place customer puts items to show to manager
//
////manager stores items taken from customer so they can be taken by Loader
////queue<int> managerItems;
//int managerItems [MAX_MANAGER_ITEMS];
////lock to protect managerItems;
////Lock* managerItemsLock;
//int managerItemsLock = CreateLock("managerItemsLock", sizeof("managerItemsLock"));
//
//int cashierTotals[MAX_CASHIERS];
//int salesmenOnBreak[MAX_SALESMEN];
//int salesmenOnBreakIndex = 0;
//int numSalesmenOnBreak[MAX_DEPARTMENTS];
//
////Trolleys
//int trollyCount = numTrollies;
//
////Lock controlling access to trollyCount;
////Lock* trollyLock;
//int trollyLock = CreateLock("trollyLock", sizeof("trollyLock"));
//
////CV for customers to wait on if no trollies are there
////associated with trollyLock
////Condition* trollyCV;
//int trollyCV = CreateCondition("trollyCV", sizeof("trollyCV"));
//
////Discarded trollies, waiting to be replaced by Goods Loader
//int displacedTrollyCount = 0;
//
////Lock controlling access to displaced trolly count
////Lock* displacedTrollyLock;
//int displacedTrollyLock = CreateLock("displacedTrollyLock", sizeof("displacedTrollyLock"));
//
////which goodsloader is currently in the stockroom
//int currentLoaderInStock = -1;
//
////Lock for currentLockInStock
////Lock *currentLoaderInStockLock;
//int currentLoaderInStockLock = CreateLock("currentLoaderInStockLock", sizeof("currentLoaderInStockLock"));
//
////Control variable for current loader in stock
////Condition *currentLoaderInStockCV;
//int currentLoaderInStockCV = CreateCondition("currentLoaderInStockCV", sizeof("currentLoaderInStockCV"));
//
////Control variable for waiting for the stock room
////Condition *stockRoomCV;
//int stockRoomCV = CreateCondition("stockRoomCV", sizeof("stockRoomCV"));
//
////Number of goodsloaders waiting for the stockroom
//int waitingForStockRoomCount = 0;
//
////Which customers are privileged or not
//
//int customersDone = 0;
//
//int testNumber = -1;
////queue<int> cashiersOnBreak; //allows manager to remember which cashiers he's sent on break
//int cashiersOnBreak[MAX_CASHIERS];
//
//int cashiersOnBreakIndex = 0;/*use this to tell where you are in the array*/
//
//int numCashiersOnBreak = 0;
//
//int cashierBreakBoard[MAX_CASHIERS];
////Function prototypes if needed
//
///*Statically allocate customer stuff*/
//int numItemsToBuy [MAX_CUSTOMERS];
//int itemsToBuy [MAX_CUSTOMERS][MAX_ITEMS];
//int qtyItemsToBuy [MAX_CUSTOMERS][MAX_ITEMS];
//int itemsInCart [MAX_CUSTOMERS][MAX_ITEMS];
//int qtyItemsInCart [MAX_CUSTOMERS][MAX_ITEMS];
//int myCash [MAX_CUSTOMERS];
//int type [MAX_CUSTOMERS];
//int nextCustomerIndex = 0;
//int customerIndexLock = CreateLock("customerIndexLock", sizeof("customerIndexLock"));
//
//
//
//
//void initCashierArrays(int numCashiers){
//	for(int i = 0; i < numCashiers; i++){
//		total[i] = 0;
//		custID[i] = 0;
//                unprivilegedCashierLineCV[i] = CreateCondition("privilegedCashierLineCV", sizeof("privilegedCashierLineCV"));
//        /*don't need to use * at the beginning of all the arrays*/
//		privilegedCashierLineCV[i] = CreateCondition("privilegedCashierLineCV", sizeof("privilegedCashierLineCV"));
//		cashierLock[i] = CreateLock("cashierLock", sizeof("cashierLock"));
//		cashierToCustCV[i] = CreateCondition("cashierToCustCV", sizeof("cashierToCustCV"));
//		privilegedLineCount[i] = 0;
//		unprivilegedLineCount[i] = 0;
//		cashierDesk[i] = 0;
//		cashRegister[i] = 0;
//                cashiersOnBreak[i] = -1;
//		cashierBreakBoard[i] = -1;
//
//	}
//}
//
//void initSalesmanArrays(){
//	for(int i = 0; i < numDepartments; i++){
//		greetingCustWaitingLineCount[i] = 0;
//		complainingCustWaitingLineCount[i] = 0;
//		loaderWaitingLineCount[i] = 0;
//		salesLock[i] = CreateLock("salesLock", sizeof("salesLock"));
//		greetingCustCV[i] = CreateCondition("greetingCustCV", sizeof("greetingCustCV"));
//		complainingCustCV[i] = CreateCondition("complainingCustCV", sizeof("complainingCustCV"));
//		loaderCV[i] = CreateCondition("loaderCV", sizeof("loaderCV"));
//		for(int j = 0; j < numSalesmen; j++){
//			salesCustNumber[i][j] = 0;
//			salesDesk[i][j] = 0;
//			salesBreakBoard[i][j] = 0;
//			salesBreakCV[i][j] = CreateCondition("salesBreakCV", sizeof("salesBreakCV"));
//			individualSalesmanLock[i][j] = CreateLock("individualSalesmanLock", sizeof("individualSalesmanLock"));
//			salesmanCV[i][j] = CreateCondition("salesmanCV", sizeof("salesmanCV"));
//			currentSalesStatus[i][j] = SALES_BUSY;
//			currentlyTalkingTo[i][j] = UNKNOWN;
//		}
//	}
//}
//
//void initLoaderArrays(){
//	for(int i = 0; i < MAX_LOADER_JOB_QUEUE; i++){
//		loaderJobQueue[i] = 0;
//	}
//	for(int i = 0; i < numDepartments; i++)	{
//		for(int j = 0; j < numItems; j++){
//			shelfLock[i][j] = CreateLock("shelfLock", sizeof("shelfLock"));
//			shelfCV[i][j] = CreateCondition("shelfCV", sizeof("shelfCV"));
//			shelfInventory[i][j] = 10; //default inventory
//		}
//	}
//}
//
//void initLoaderArraysWithQty(){
//	for(int i = 0; i < MAX_LOADER_JOB_QUEUE; i++){
//		loaderJobQueue[i] = 0;
//	}
//	for(int i = 0; i < numDepartments; i++)	{
//		for(int j = 0; j < numItems; j++){
//			shelfLock[i][j] = CreateLock("shelfLock", sizeof("shelfLock"));
//			shelfCV[i][j] = CreateCondition("shelfCV", sizeof("shelfCV"));
//			shelfInventory[i][j] = 10; //default inventory
//		}
//	}
//}
//
//void initManagerArrays(){
//	for(int i = 0; i < MAX_MANAGER_ITEMS; i++){
//	  managerItems[i] = -1;
//	}
//        for(int i = 0; i < MAX_CASHIERS; i++){
//          cashierTotals[i] = 0;
//        }
//        for(int i = 0; i < MAX_SALESMEN; i++){
//          salesmenOnBreak[i] = -1;
//        }
//        for(int i = 0;i < MAX_DEPARTMENTS; i++){
//          numSalesmenOnBreak[i] = 0;
//        }
//}
//
//void initCustomerArrays(){
//    for(int i = 0; i < MAX_CUSTOMERS; i++){
//        numItemsToBuy[i] = 0;
//        myCash[i] = 0;
//        type[i] = 0;
//        for(int j = 0; j < MAX_ITEMS; j++){
//            itemsToBuy[i][j] = 0;
//            qtyItemsToBuy[i][j] = 0;
//            itemsInCart[i][j] = 0;
//            qtyItemsInCart[i][j] = 0;
//        }
//    }
//}
//
////void initShelves() {
////	char* name;
////	stockRoomLock = new Lock("Stock room lock");
////
////	shelfLock = new Lock**[numDepartments];
////	shelfCV = new Condition**[numDepartments];
////	shelfInventory = new int*[numDepartments];
////
////	for(int i = 0; i < numDepartments; i++) {
////		shelfLock[i] = new Lock*[numItems];
////		shelfCV[i] = new Condition*[numItems];
////		shelfInventory[i] = new int[numItems];
////	}
////
////	for(int i = 0; i < numDepartments; i++) {
////		for(int j = 0; j < numItems; j++) {
////			name = new char[20];
////			sprintf(name, "shelfLock dept%d item%d", i, j);
////			shelfLock[i][j] = new Lock("name");
////			name = new char[20];
////			sprintf(name, "shelCV dept%d item%d", i, j);
////			shelfCV[i][j] = new Condition("name");
////			shelfInventory[i][j] = 0;
////		}
////	}
////}
////
////void initShelvesWithQty(int q) {
////	//cout << "Initializing shelves with qty: " << q << endl;
////
////	stockRoomLock = new Lock("Stock room lock");
////
////	shelfLock = new Lock**[numDepartments];
////	shelfCV = new Condition**[numDepartments];
////	shelfInventory = new int*[numDepartments];
////
////	for(int i = 0; i < numDepartments; i++) {
////		shelfLock[i] = new Lock*[numItems];
////		shelfCV[i] = new Condition*[numItems];
////		shelfInventory[i] = new int[numItems];
////	}
////
////	for(int i = 0; i < numDepartments; i++) {
////		for(int j = 0; j < numItems; j++) {
////			shelfLock[i][j] = new Lock("shelfLock");
////			shelfCV[i][j] = new Condition("shelfCV");
////			shelfInventory[i][j] = q;
////		}
////	}
////}
////
////void initSalesmen(){
////	//cout << "Sales init level 1..." << endl;
////	currentSalesStatus = new SalesmanStatus*[numDepartments];
////	currentlyTalkingTo = new WhomIWantToTalkTo*[numDepartments];
////	salesCustNumber = new int*[numDepartments];
////	salesDesk = new int*[numDepartments];
////	salesBreakBoard = new int*[numDepartments];
////	salesBreakCV = new Condition**[numDepartments];
////	greetingCustWaitingLineCount = new int[numDepartments]; //How many customers need to be greeted
////	complainingCustWaitingLineCount = new int[numDepartments]; //How many customers need to be helped with empty shelves
////	loaderWaitingLineCount = new int[numDepartments];	//How many loaders are waiting on salesmen
////
////	individualSalesmanLock = new Lock**[numDepartments]; //The lock that each salesman uses for his own "desk"
////	salesLock = new Lock*[numDepartments]; //The lock that protects the salesmen statuses and the line for all three
////
////	salesmanCV = new Condition**[numDepartments]; //The condition variable for one on one interactions with the Salesman
////
////	greetingCustCV = new Condition*[numDepartments];
////	complainingCustCV = new Condition*[numDepartments];
////	loaderCV = new Condition*[numDepartments];
////
////	//cout << "Sales init level 2..." << endl;
////	for(int i = 0; i < numDepartments; i++) {
////		currentSalesStatus[i] = new SalesmanStatus[numSalesmen];
////		currentlyTalkingTo[i] = new WhomIWantToTalkTo[numSalesmen];
////		salesCustNumber[i] = new int[numSalesmen];
////		salesDesk[i] = new int[numSalesmen];
////		salesBreakBoard[i] = new int[numSalesmen];
////		salesBreakCV[i] = new Condition*[numSalesmen];
////
////		individualSalesmanLock[i] = new Lock*[numSalesmen];
////		salesLock[i] = new Lock("Overall salesman lock");
////
////		salesmanCV[i] = new Condition*[numSalesmen]; //The condition variable for one on one interactions with the Salesman
////
////		greetingCustCV[i] = new Condition ("Greeting Customer Condition Variable"); //The condition var that represents the line all the greeters stay in
////		complainingCustCV[i] = new Condition("Complaining Customer Condition Variable"); //Represents the line that the complainers stay in
////		loaderCV[i] = new Condition("GoodsLoader Condition Variable");	//Represents the line that loaders wait in
////
////		greetingCustWaitingLineCount[i] = 0;
////		complainingCustWaitingLineCount[i] = 0;
////		loaderWaitingLineCount[i] = 0;
////	}
////
////	//cout << "Sales init level 3..." << endl;
////	for(int i = 0; i < numDepartments; i++) {
////		for(int j = 0; j < numSalesmen; j++){
////			currentSalesStatus[i][j] = SALES_BUSY;
////			currentlyTalkingTo[i][j] = UNKNOWN;
////			salesCustNumber[i][j] = 0;
////			salesBreakBoard[i][j] = 0;
////			individualSalesmanLock[i][j] = new Lock(("Indivisdual Salesman Lock"));
////			salesmanCV[i][j] = new Condition(("Salesman Condition Variable"));
////			salesBreakCV[i][j] = new Condition("Salesman Break CV");
////		}
////	}
////	//cout << "Done initializing sales" << endl;
////}
//
////void initLoaders() {
////	//cout << "Initializing loaders..." << endl;
////	inactiveLoaderLock = new Lock("Lock for loaders waiting to be called on");
////	inactiveLoaderCV = new Condition("CV for loaders waiting to be called on");
////	currentLoaderInStock = -1;
////	currentLoaderInStockLock = new Lock("cur loader in stock lock");
////	currentLoaderInStockCV = new Condition("Current Loader in Stock CV");
////	stockRoomCV = new Condition("Stock Room CV");
////	waitingForStockRoomCount = 0;
////	loaderStatus = new LoaderStatus[numLoaders];
////
////	for(int i = 0; i < numLoaders; i++){
////		loaderStatus[i] = LOAD_NOT_BUSY;
////	}
////}
////
////void initTrolly() {
////	//cout << "Initializing trollies..." << endl;
////
////	char* name;
////
////	trollyCount = numTrollies;
////	displacedTrollyCount = 0;
////	name = new char[20];
////	name = "trolly lock";
////	trollyLock = new Lock(name);
////	name = new char[20];
////	name = "trolly CV";
////	trollyCV = new Condition(name);
////	name = new char[20];
////	name = "displaced trolly lock";
////	displacedTrollyLock = new Lock(name);
////}
////
////void initCustomerCashier(){
////	//cout << "Initializing CustomerCashier..." << endl;
////
////	for(int i = 0; i < maxCustomers; i++){
////		privCustomers[i] = 0; //sets all the customers to unprivileged
////	}
////	char* name;
////	name = new char[20];
////	name = "cashier line lock";
////	customersDone =0;
////	//	cashierLinesLock = new Lock(name);
////	name = new char[20];
////	//name = "manaager items lock";
////	managerItemsLock = new Lock(name);
////	name = new char[20];
////	name = "inactive loader cv";
////	inactiveLoaderCV = new Condition(name);
////	name = new char[20];
////	name = "inactive loader lock";
////	inactiveLoaderLock = new Lock(name);
////	//privilegedCashierLineCV = new Condition*[cashierNumber];
////	//unprivilegedCashierLineCV = new Condition*[cashierNumber];
////	//cashierLock = new Lock*[cashierNumber];
////	//cashierToCustCV = new Condition*[cashierNumber];
////	//cashierStatus = new CashierStatus[cashierNumber];
////	//privilegedLineCount = new int[cashierNumber];
////	//unprivilegedLineCount = new int[cashierNumber];
////	//cashierDesk = new int[cashierNumber];
////	//cashierFlags = new int[cashierNumber];
////	//cashRegister = new int[cashierNumber];
////	cashierBreakBoard = new int[cashierNumber];
////	displacedTrollyCount = 0;
////	name = new char[20];
////	name = "trolly lock";
////	trollyLock = new Lock(name);
////	name = new char[20];
////	name = "trolly CV";
////	trollyCV = new Condition(name);
////	name = new char[20];
////	name = "displaced trolly lock";
////	displacedTrollyLock = new Lock(name);
////
////	initTrolly();
////
////	for(int i = 0; i < cashierNumber; i++){
////		name = new char [20];
////		sprintf(name,"priv cashier line CV %d",i);
////		//privilegedCashierLineCV[i] = new Condition(name);
////	}
////	for(int i = 0; i < cashierNumber; i++){
////		name = new char[20];
////		sprintf(name, "unpriv cashier line CV %d", i);
////		//unprivilegedCashierLineCV[i] = new Condition(name);
////	}
////	for(int i = 0; i < cashierNumber; i++){
////		name = new char[20];
////		sprintf(name, "cashier lock %d", i);
////		//cashierLock[i] = new Lock(name);
////	}
////	for(int i = 0; i < cashierNumber; i++){
////		name = new char[20];
////		sprintf(name, "cashier cust CV %d", i);
////		//cashierToCustCV[i] = new Condition(name);
////	}
////	for(int i = 0; i < cashierNumber; i++){
////		cashierStatus[i] = CASH_BUSY;
////		privilegedLineCount[i] = 0;
////		unprivilegedLineCount[i] = 0;
////		cashierFlags[i] = -1;
////		cashierDesk[i] = -2;
////		cashRegister[i] = 0;
////		cashierBreakBoard[i] = 0;
////	}
////
////	name = new char[20];
////	name = "manager lock";
////	//managerLock = new Lock(name);
////	name = new char[20];
////	name = "manager CV";
////	//managerCV = new Condition(name);
////}
//
////gets the department that a given item will be in
//
//
//int getDepartmentFromItem(int itemNum) {
//	return itemNum % numDepartments;
//}
//
////constructs an argument (for a salesman) that combines 2 numbers into one, here for dept and id
//int constructSalesArg(int dept, int id) {	// 16 bit number: [15:8] are dept num and [7:0] are id num
//	int val = 0;
//	val = (dept << 8) | id;
//	return val;
//}
//
////takes a value made by the constructSalesArg function and extracts the department and id from it, putting tem at the target location
//void deconstructSalesArg(int val, int target[2]) {
//	int dept = 0;
//	int id = 0;
//
//	dept = (val & 0x0000ff00) >> 8;
//	id = (val & 0x000000ff);
//
//	target[0] = id;
//	target[1] = dept;
//}
///*
////Creates salesmen threads given the total number of departments in the store, and the number of salesmen per department
//void createSalesmen(int numDepts, int numSalesPerDept) {
//
//	//cout << "starting to create salesmen" << endl;
//	int salesID = 0;
//
//	for(int i = 0; i < numDepts; i++) {
//		for(int j = 0; j < numSalesPerDept; j++) {
//			Thread * t;
//			char* name;
//			int arg = 0;
//
//			//cout << "preparing to construct... " << i << " " << j << endl;
//			salesID = j;
//			arg = constructSalesArg(i, salesID);	//used to get dept and id info into one number
//			name = new char [20];
//			sprintf(name,"sales%d",i);
//			t = new Thread(name);
//			t->Fork((VoidFunctionPtr)Salesman, arg);
//			delete name;
//
//			//cout << "created sales with dept: " << i << " and ID: " << salesID << endl;
//			//salesID++;
//		}
//	}
//}
//*/
//
///*Customer's function		//__CUST__*/
//void Customer(){
//
//  int myID, r, targetDepartment, currentDepartment, mySalesIndex, someoneIsFree,
//    i, j, k, shelfNum, mySalesID, myCashier, minLineValue, minCashierID,
//    *linesIAmLookingAt, *linesIAmLookingAtCV, amountOwed;
//
//
//   Acquire(customerIndexLock);
//   myID = nextCustomerIndex;
//   nextCustomerIndex++;
//   Release(customerIndexLock);
//   /*//choose the items we want to buy
//	int numItemsToBuy = 3;
//	int *itemsToBuy;
//	int *qtyItemsToBuy;
//	int *itemsInCart;
//	int *qtyItemsInCart;
//	int myCash;*/
//
//	//setup some initialization for specific tests
//	if(testNumber == 8 || testNumber ==10){
//		numItemsToBuy[myID] = 1;
//		myCash[myID] = customerCash;
//	}
//	else if(testNumber != -1 && testNumber !=4 ){
//		numItemsToBuy[myID] = 2;
//		myCash[myID] = customerCash;
//	}
//	else{
//		//numItemsToBuy = (rand() % (numItems - 1)) + 1;
//		numItemsToBuy[myID] = (rand() % numItems);
//		myCash[myID] = rand() % 200;
//	}
//
//	/*itemsToBuy = new int[numItemsToBuy];
//	qtyItemsToBuy = new int[numItemsToBuy];
//	itemsInCart = new int[numItemsToBuy];
//	qtyItemsInCart = new int[numItemsToBuy];*/
//
//	/*char* type = new char[20];
//         int privileged;*/
//
//	//---------Randomly generate whether this customer is privileged--------------
//	srand(myID + time(NULL));
//	r = rand() % 10; //random value to set Customer either as privileged or unprivileged
//	if(r < 2){				//30% chance customer is privileged
//           /*privileged = 1;*/
//           type[myID] = 1;
//	}
//	else /*privileged = 0;	//70% chance this customer is unprivileged*/
//           type[myID] = 0;
//
//	//set char array for I/O purposes
//	/*if(privileged){
//		type = "PrivilegedCustomer";
//	}
//	else type = "Customer";*/
//
//	//privileged = 0;
//	//--------------End of privileged/unprivileged decion--------------------------
//
//	//Decide what to buy
//	if(testNumber != -1){
//		for( i = 0; i < numItemsToBuy[myID]; i++) {
//		  itemsToBuy[myID][i] = i;
//			qtyItemsToBuy[myID][i] = 2;
//			itemsInCart[myID][i] = -1;
//			qtyItemsInCart[myID][i] = 0;
//		}
//	}
//	else{
//		for ( i = 0; i < numItemsToBuy[myID]; i++){
//			//itemsToBuy[i] = rand() % numItems;//getDepartmentFromItem(rand() % numItems);
//                  itemsToBuy[myID][i] = getDepartmentFromItem(rand() % numItems);
//			qtyItemsToBuy[myID][i] = (rand()% numItems);
//			itemsInCart[myID][i] = -1;
//			qtyItemsInCart[myID][i] = 0;
//		}
//	}
//        /*
//	cout << type << " [" << myID << "] will be buying:" << endl;
//	cout << "Item - Qty" << endl;
//	for(int j = 0; j < numItemsToBuy[myID]; j++) {
//		cout << "  " << itemsToBuy[j] << " - " << qtyItemsToBuy[j] << endl;
//                }*/
//
//
//	//ENTERS STORE
//
//	cout << type << " [" << myID << "] enters the SuperMarket" << endl;
//	cout << type << " [" << myID << "] wants to buy [" << numItemsToBuy << "] no.of items" << endl;
//
//	/*trollyLock->Acquire();
//	while(trollyCount == 0) {
//		cout << type << " [" << myID << "] gets in line for a trolly" << endl;
//		trollyCV->Wait(trollyLock);
//	}
//	trollyCount--;
//	cout << type << " [" << myID << "] has a trolly for shopping" << endl;
//	trollyLock->Release();*/
//
//
//       Acquire(trollyLock);
//       while(trollyCount == 0){
//           Wait(trollyCV, trollyLock);
//       }
//       trollyCount--;
//       Release(trollyLock);
//
//	 targetDepartment = -1;
//	 currentDepartment = -1;
//
//	 for( i = 0; i < numItemsToBuy[myID]; i++) {	//goes through everything on our grocery list
//
//		targetDepartment = getDepartmentFromItem(itemsToBuy[myID][i]);	//selects a department
//
//		//if necessary, change departments
//		if(targetDepartment != currentDepartment) {
//                  /*	cout << type << " [" << myID << "] has finished shopping in department [" << currentDepartment << "]" << endl;
//			cout << type << " [" << myID << "] wants to shop in department [" << targetDepartment << "]" << endl;*/
//			/*salesLock[targetDepartment]->Acquire();*/
//                       Acquire(salesLock[targetDepartment]);
//			 mySalesIndex = -1;
//
//			//printf("cust %d is about to select salesman, greeting line is: %d\n", myID,  greetingCustWaitingLineCount[targetDepartment]);
//
//			//Selects a salesman
//			 someoneIsFree = -1;
//			for( j = 0; j < numSalesmen; j++) {
//				if(currentSalesStatus[targetDepartment][j] == SALES_NOT_BUSY) {
//					someoneIsFree = j;
//					mySalesIndex = j;
//					currentSalesStatus[targetDepartment][j] = SALES_BUSY;
//					currentlyTalkingTo[targetDepartment][j] = GREETING;
//					break;
//				}
//			}
//
//			if(someoneIsFree == -1){	//no one is free, so wait in the greeting line
//				greetingCustWaitingLineCount[targetDepartment]++;
//				cout << type << " [" << myID << "] gets in line for the Department [" << targetDepartment << "]" << endl;
//				/*greetingCustCV[targetDepartment]->Wait(salesLock[targetDepartment]);*/
//                               Wait(greetingCustCV[targetDepartment], salesLock[targetDepartment]);
//				for( j = 0; j < numSalesmen; j++){
//					if(currentSalesStatus[targetDepartment][j] == SALES_READY_TO_TALK){
//						mySalesIndex = j;
//						currentSalesStatus[targetDepartment][j] = SALES_BUSY;
//						currentlyTalkingTo[targetDepartment][j] = GREETING;
//						break;
//					}
//				}
//			}
//
//			/*individualSalesmanLock[targetDepartment][mySalesIndex]->Acquire(); //Acquire the salesman's "desk" lock*/
//                       Acquire(individualSalesmanLock[targetDepartment][mySalesIndex]);
//			cout << type << " [" << myID << "] is interacting with DepartmentSalesman[" << mySalesIndex << "] of Department[" << targetDepartment << "]" << endl;
//			/*salesLock[targetDepartment]->Release();*/
//                       Release(salesLock[targetDepartment]);
//			salesCustNumber[targetDepartment][mySalesIndex] = myID; //Sets the customer number of the salesman to this customer's index
//
//			/*salesmanCV[targetDepartment][mySalesIndex]->Signal(individualSalesmanLock[targetDepartment][mySalesIndex]);*/
//                       Signal(salesmanCV[targetDepartment][mySalesIndex], individualSalesmanLock[targetDepartment][mySalesIndex]);
//			/*salesmanCV[targetDepartment][mySalesIndex]->Wait (individualSalesmanLock[targetDepartment][mySalesIndex]);*/
//                       Wait(salesmanCV[targetDepartment][mySalesIndex], individualSalesmanLock[targetDepartment][mySalesIndex]);
//                       /*individualSalesmanLock[targetDepartment][mySalesIndex]->Release();*/
//                       Release(individualSalesmanLock[targetDepartment][mySalesIndex]);
//
//
//		}
//
//		currentDepartment = targetDepartment;
//
//		//BEGINS SHOPPING
//
//		for( shelfNum = 0; shelfNum < numItems; shelfNum++) {
//			if(shelfNum != itemsToBuy[myID][i]) {
//				continue;
//			}
//
//			cout << type << " ["<< myID << "] wants to buy [" << shelfNum << "]-[" << qtyItemsToBuy[i] << "]" << endl;
//			while(qtyItemsInCart[i] < qtyItemsToBuy[i]) {
//                           /*shelfLock[currentDepartment][shelfNum]->Acquire();*/
//                           Acquire(shelfLock[currentDepartment][shelfNum]);
//
//				if(shelfInventory[currentDepartment][shelfNum] > qtyItemsToBuy[myID][i]) {	//if there are enough items, take what i need
//					cout << type << " ["<< myID << "] has found ["
//							<< shelfNum << "] and placed [" << qtyItemsToBuy[i] << "] in the trolly" << endl;
//					if(testNumber == 8) cout << "There were " << shelfInventory[currentDepartment][shelfNum] << " before Customer " << myID << " took the item(s)." << endl;
//
//					shelfInventory[currentDepartment][shelfNum] -= qtyItemsToBuy[myID][i];
//					itemsInCart[myID][i] = shelfNum;
//					qtyItemsInCart[myID][i] += qtyItemsToBuy[myID][i];
//					/*shelfLock[currentDepartment][shelfNum]->Release();*/
//                                       Release(shelfLock[currentDepartment][shelfNum]);
//				}
//				else {	//We are out of this item, go tell sales!
//					cout << type << " [" << myID << "] was not able to find item " << shelfNum <<
//							" and is searching for department salesman " << currentDepartment << endl;
//					/*shelfLock[currentDepartment][shelfNum]->Release();*/
//                                       Release(shelfLock[currentDepartment][shelfNum]);
//					/*salesLock[currentDepartment]->Acquire();*/
//                                       Acquire(salesLock[currentDepartment]);
//
//					 mySalesID = -1;
//
//					for( j = 0; j < numSalesmen; j++) {	//see if there is a free salesman to go to
//						//nobody waiting, sales free
//						if(currentSalesStatus[currentDepartment][j] == SALES_NOT_BUSY) {
//							mySalesID = j;
//							break;
//						}
//					}
//					if(mySalesID == -1) {	//no salesmen are free, I have to wait in line
//						complainingCustWaitingLineCount[currentDepartment]++;
//						/*complainingCustCV[currentDepartment]->Wait(salesLock[currentDepartment]);*/
//                                               Wait(complainingCustCV[currentDepartment], salesLock[currentDepartment]);
//
//						//find the salesman who just signalled me
//						for( k = 0; k < numSalesmen; k++) {
//							if(currentSalesStatus[currentDepartment][k] == SALES_READY_TO_TALK) {
//								mySalesID = k;
//								break;
//							}
//						}
//					}
//
//					//I'm now talking to a salesman
//					currentSalesStatus[currentDepartment][mySalesID] = SALES_BUSY;
//					/*salesLock[currentDepartment]->Release();*/
//                                       Release(salesLock[currentDepartment]);
//					/*individualSalesmanLock[currentDepartment][mySalesID]->Acquire();*/
//                                       Acquire(individualSalesmanLock[currentDepartment][mySalesID]);
//					cout << type << " [" << myID << "] is asking for assistance "
//							"from DepartmentSalesman [" << mySalesID << "]" << endl;
//
//					salesCustNumber[currentDepartment][mySalesID] = myID;
//
//
//					//now proceed with interaction to tell sales we are out
//					currentlyTalkingTo[currentDepartment][mySalesID] = COMPLAINING;
//					salesDesk[currentDepartment][mySalesID] = shelfNum;
//
//					/*salesmanCV[currentDepartment][mySalesID]->Signal(individualSalesmanLock[currentDepartment][mySalesID]);*/
//                                       Signal(salesmanCV[currentDepartment][mySalesID], individualSalesmanLock[currentDepartment][mySalesID]);
//					/*salesmanCV[currentDepartment][mySalesID]->Wait(individualSalesmanLock[currentDepartment][mySalesID]);	//wait for sales to tell me to wait on shelf*/
//                                       Wait(salesmanCV[currentDepartment][mySalesID], individualSalesmanLock[currentDepartment][mySalesID]);
//					/*shelfLock[currentDepartment][shelfNum]->Acquire();*/
//                                       Acquire(shelfLock[currentDepartment][shelfNum]);
//					/*individualSalesmanLock[currentDepartment][mySalesID]->Release();*/
//                                       Release(individualSalesmanLock[currentDepartment][mySalesID]);
//
//					//now i go wait on the shelf
//					/*shelfCV[currentDepartment][shelfNum]->Wait(shelfLock[currentDepartment][shelfNum]);*/
//                                       Wait(shelfCV[currentDepartment][shelfNum], shelfLock[currentDepartment][shelfNum]);
//					cout << "DepartmentSalesman [" << mySalesID << "] informs the " << type << " [" << myID << "] that [" << shelfNum << "] is restocked." << endl;
//
//					//now restocked, continue looping until I have all of what I need
//					cout << type << " [" <<  myID << "] has received assistance about restocking of item [" <<
//							shelfNum << "] from DepartmentSalesman [" << mySalesID << "]" << endl;
//					/*shelfLock[currentDepartment][shelfNum]->Release();*/
//                                       Release(shelfLock[currentDepartment][shelfNum]);
//				}
//			}	//end while loop to get enough of a given item
//		}	//end looking through shelves
//	}	//end going through grocery list
//
//
//	//========================================================
//
//
//	 myCashier; //ID of cashier I speak to
//
//	//--------------Begin looking for a cashier-------------------------------------
//	/*cashierLinesLock->Acquire(); //acquire locks to view line counts and cashier statuses*/
//       Acquire(cashierLinesLock);
//	do{		//loop allows us to rechoose a line if our cashier goes on break
//          /*printf("%s [%d] is looking for the Cashier.\n", type, myID );*/
//
//		//Find if a cashier is free (if one is, customer doesn't need to wait in line)
//		for( i = 0; i < cashierNumber; i++ ){
//			//if I find a cashier who is free, I will:
//			if(cashierStatus[i] == CASH_NOT_BUSY){
//				myCashier = i; //remember who he is
//				/*cashierLock[i]->Acquire(); //get his lock before I wake him up*/
//                               Acquire(cashierLock[i]);
//				cashierStatus[i] = CASH_BUSY; //prevent others from thinking he's free
//				/*printf("%s [%d] chose Cashier [%d] with line of length [0].\n", type, myID, myCashier);*/
//				break; //stop searching through lines
//			}
//
//			//---------------Find shortest line---------------------------
//			else if (i == cashierNumber - 1){
//			  //set the pointers depending on which customer type i am dealing with
//				if(type[myID]){
//					linesIAmLookingAt = privilegedLineCount;
//					linesIAmLookingAtCV= privilegedCashierLineCV;
//				}
//				else{
//					linesIAmLookingAt = unprivilegedLineCount;
//					linesIAmLookingAtCV = unprivilegedCashierLineCV;
//				}
//
//				//from here on, privilegedCustomers and unprivileged customers execute same code because
//				//of the temporary variables linesIAmLookingAt (which is unprivilegedLineCount or privilegedLineCount)
//				//and linesIAmLookingAtCV (which is un/privilegedCashierLineCV)
//
//				minLineValue = custNumber; //set a default min value
//				//find the minimum line value and remember the cashier associated with it
//				for( j = 0; j < cashierNumber; j++){
//					if(linesIAmLookingAt[j] < minLineValue && cashierStatus[j] != CASH_ON_BREAK && cashierStatus[j] != 	CASH_GO_ON_BREAK){
//						//must also check if that cashier is on break
//						minLineValue = linesIAmLookingAt[j];
//						minCashierID = j;
//					}
//				}
//				myCashier = minCashierID;
//				/*printf("%s [%d] chose Cashier [%d] of line length [%d].\n", type, myID, myCashier, linesIAmLookingAt[minCashierID]);*/
//				linesIAmLookingAt[minCashierID]++;
//				//linesIAmLookingAtCV[minCashierID]->Wait(cashierLinesLock); //wait in line
//                                Wait(linesIAmLookingAt[minCashierID], cashierLinesLock);
//				linesIAmLookingAt[myCashier]--; //i have been woken up, remove myself from line
//			}
//			//-------------End find shortest line----------------------
//
//		}
//	}while(cashierStatus[myCashier] == CASH_ON_BREAK); //customer will repeat finding a cashier algorithm if he was woken up because the cashier is going on break
//
//	//----------------End looking for cashier--------------------------------------------
//
//
//	//code after this means I have been woken up after getting to the front of the line
//	/*cashierLock[myCashier]->Acquire(); //disallow others from getting access to my cashier*/
//       Acquire(cashierLock[myCashier]);
//	/*cashierLinesLock->Release();	//allow others to view monitor variable now that I've
//       //my claim on this cashier*/
//       Release(cashierLinesLock);
//	cashierDesk[myCashier] = myID;	//tell cashier who I am
//	/*cashierToCustCV[myCashier]->Signal(cashierLock[myCashier]);	//signal cashier I am at his desk*/
//       Signal(cashierToCustCV[myCashier], cashierLock[myCashier]);
//	/*cashierToCustCV[myCashier]->Wait(cashierLock[myCashier]);	//wait for his acknowlegdment*/
//       Wait(cashierToCustCV[myCashier], cashierLock[myCashier]);
//	cashierDesk[myCashier] = type[myID]; //now tell him that whether or not I am privileged
//	/*cashierToCustCV[myCashier]->Signal(cashierLock[myCashier]);	//signal cashier I've passed this information*/
//       Signal(cashierToCustCV[myCashier], cashierLock[myCashier]);
//	/*cashierToCustCV[myCashier]->Wait(cashierLock[myCashier]);	//wait for his acknowledgment*/
//       Wait(cashierToCustCV[myCashier], cashierLock[myCashier]);
//
//	//When I get here, the cashier is ready for items
//
//	//---------------------------Begin passing items to cashier---------------------------
//	//cycle through all items in my inventory, handing one item to cashier at a time
//	for( i = 0; i < numItemsToBuy[myID]; i++){ //cycle through all types of items
//		for( j = 0; j < qtyItemsInCart[myID][i]; j++){ //be sure we report how many of each type
//			cashierDesk[myCashier] = itemsInCart[myID][i]; //tells the cashier what type of item
//			/*cashierToCustCV[myCashier]->Signal(cashierLock[myCashier]); //signal cashier item is on his desk*/
//                       Signal(cashierToCustCV[myCashier], cashierLock[myCashier]);
//			/*cashierToCustCV[myCashier]->Wait(cashierLock[myCashier]); //wait for his acknowledgement*/
//                       Wait(cashierToCustCV[myCashier], cashierLock[myCashier]);
//		}
//	}
//	cashierDesk[myCashier] = -1; //Tells cashier that I have reached the last item in my inventory
//        /*	cashierToCustCV[myCashier]->Signal(cashierLock[myCashier]); //tell cashier
//                cashierToCustCV[myCashier]->Wait(cashierLock[myCashier]); //wait for him to put the amount on desk*/
//        Signal(cashierToCustCV[myCashier], cashierLock[myCashier]);
//        Wait(cashierToCustCV[myCashier], cashierLock[myCashier]);
//	//------------------------End passing items to cashier--------------------------------
//
//
//	//when I get here, the cashier has loaded my total
//	//If I don't have enough money, leave the error flag -1 on the cashier's desk
//	if(cashierDesk[myCashier] > myCash[myID]){
//          /*printf("%s [%d] cannot pay [%d]\n", type, myID, cashierDesk[myCashier]);*/
//		cashierDesk[myCashier] = -1;
//		/*cashierToCustCV[myCashier]->Signal(cashierLock[myCashier]);
//                  cashierToCustCV[myCashier]->Wait(cashierLock[myCashier]);*/
//                Signal(cashierToCustCV[myCashier], cashierLock[myCashier]);
//                Wait(cashierToCustCV[myCashier], cashierLock[myCashier]);
//		//manager-customer interaction
//		/*managerLock->Acquire();
//                  cashierLock[myCashier]->Release();*/
//                Acquire(managerLock);
//                Release(cashierLock[myCashier]);
//		/*printf("%s [%d] is waiting for Manager for negotiations\n", type, myID);*/
//		/*managerCV->Wait(managerLock);*/
//                Wait(managerCV, managerLock);
//
//		//-----------------------Begin passing items to manager---------------------
//		for( i = 0; i < numItemsToBuy[myID]; i++){
//			while(qtyItemsInCart[myID][i] > 0){
//				if(managerDesk < myCash[myID]){
//					break;
//				}
//				qtyItemsInCart[myID][i] --;
//				managerDesk = itemsInCart[myID][i];
//				/*printf("%s [%d] tells Manager to remove [%d] from trolly.\n", type, myID, itemsInCart[i]);*/
//				/*managerCV->Signal(managerLock);
//				managerCV->Wait(managerLock);*/
//                                Signal(managerCV, managerLock);
//                                Wait(managerCV, managerLock);
//			}
//		}
//		managerDesk = -1; //notifies the manager I'm done
//		/*managerCV->Signal(managerLock);
//                  managerCV->Wait(managerLock);*/
//                Signal(managerCV, managerLock);
//                Wait(managerCV, managerLock);
//		//--------------------End of passing items to manager---------------
//
//		amountOwed = managerDesk;	//if I still can't afford anything, amountOwed will be 0
//		myCash[myID] -= amountOwed;	//updating my cash amount because I am paying manager
//		managerDesk = amountOwed;	//technically redundant, but represents me paying money
//		/*printf("%s [%d] pays [%d] to Manager after removing items and is waiting for receipt from Manager.\n", type, myID, amountOwed);*/
//		//need receipt
//		/*managerCV->Signal(managerLock);
//                  managerCV->Wait(managerLock);*/
//                Signal(managerCV, managerLock);
//                Wait(managerCV, managerLock);
//		//got receipt, I can now leave
//		/*managerLock->Release();*/
//                Release(managerLock);
//		/*cout << "Customer [" << myID << "] got receipt from Manager and is now leaving." << endl;*/
//	}
//	//if I do have money, I just need to update my cash and leave the money there
//	else{
//		myCash[myID] -= cashierDesk[myCashier];
//		//Now I wait for my receipt
//		/*cashierToCustCV[myCashier]->Signal(cashierLock[myCashier]);
//		printf("%s [%d] pays [%d] to Cashier [%d] and is now waiting for receipt.\n", type, myID, cashierDesk[myCashier], myCashier);
//		cashierToCustCV[myCashier]->Wait(cashierLock[myCashier]);
//		//now I've received my receipt and should release the cashier
//		cout << type << " [" << myID << "] got receipt from Cashier [" << myCashier << "] and is now leaving." << endl;
//
//		cashierToCustCV[myCashier]->Signal(cashierLock[myCashier]);
//		cashierLock[myCashier]->Release();*/
//                Signal(cashierToCustCV[myCashier], cashierLock[myCashier]);
//                Wait(cashierToCustCV[myCashier], cashierLock[myCashier]);
//                Signal(cashierToCustCV[myCashier], cashierLock[myCashier]);
//                Release(cashierLock[myCashier]);
//		myCashier = -1; //so I can't accidentally tamper with the cashier I chose anymore
//	}
//
//	//------------------------------Begin replace trolly--------------
//	//no need for CV since sequencing doesn't matter
//	/*displacedTrollyLock->Acquire();
//	displacedTrollyCount++;
//	displacedTrollyLock->Release();*/
//        Acquire(displacedTrollyLock);
//        displacedTrollyLock++;
//        Release(displacedTrollyLock);
//	//-----------------------------End replace trolly----------------------
//
//	customersDone++;	//increment the total customers done count
//
//	//some cleanup
//	/*delete itemsToBuy;
//	delete qtyItemsToBuy;
//	delete itemsInCart;
//	delete qtyItemsInCart;*/
//
//}
//
///*gets the prices for each type of item*/
//int scan(int item){
//	switch(item){
//	case 0: return 5;
//	case 1: return 2;
//	case 2: return 3;
//	case 3: return 8;
//	case 4: return 7;
//	case 5: return 3;
//	case 6: return 10;
//	case 7: return 4;
//	default: return 1;
//	}
//}
//
//
///*Manager code*/
//void manager(){
//	int totalRevenue = 0; //will track the total sales of the day
//	int counter = 0;
//	int i, numFullLines, numAnyLines, wakeCashier, chance, r, dept, arg,
//	  wakeSalesman, targets[2], customerID, cashierID, amountOwed, custTypeID,
//	  managerCustType,
//	/*int *cashierTotals = new int[cashierNumber];*/
//
//	//initializes cashier totals, by which we keep track of how much money each cashier has had
//	//in their registers over time
//	/*for(int i = 0; i < cashierNumber; i++){
//		cashierTotals[i] = 0;
//                }*/
//
//
//	/*queue<int> Break;
//          int* numSalesmenOnBreak = new int[numDepartments];*/
//
//	srand(time(NULL));
//	while(true){
//
//		//------------------Check if all customers have left store----------------
//		//if all customers have left, print out totals and terminate simulation since manager will be the only
//		//ready thread
//		if(customersDone == custNumber){
//			for( i = 0; i < cashierNumber; i++){
//                          /*cashierLock[i]->Acquire();*/
//                          Acquire(cashierLock[i]);
//				//-----------------------------Start empty cashier drawers------------------------------------
//				if(cashRegister[i] > 0){
//					totalRevenue += cashRegister[i];
//					cashierTotals[i] += cashRegister[i];
//					/*cout << "Manager emptied Counter " << i << " drawer." << endl;*/
//					cashRegister[i] = 0;
//					/*cout << "Manager has total sale of $" << totalRevenue << "." << endl;
//                                          cashierLock[i]->Release();*/
//                                        Release(cashierLock[i]);
//				}
//			}
//			/*for(int i = 0; i < cashierNumber; i++){
//				cout << "Total Sale from Counter [" << i << "] is $[" << cashierTotals[i] << "]." << endl;
//			}
//			cout << "Total Sale of the entire store is $[" << totalRevenue << "]." << endl;*/
//			break;
//		}
//		//------------------End check if all customers have left store------------
//
//		//-----------------Have loader check trolleys---------------------------
//		/*inactiveLoaderLock->Acquire();
//		inactiveLoaderCV->Signal(inactiveLoaderLock); //wake up goods loader to do a regular check of trolley and manager items
//		inactiveLoaderLock->Release();*/
//                Acquire(inactiveLoaderLock);
//                Signal(inactiveLoaderCV, inactiveLoaderLock);
//		if(counter != 100000000) counter ++;
//		//I don't need to acquire a lock because I never go to sleep
//		//Therefore, it doesn't matter if a cashierFlag is changed on this pass,
//		//I will get around to it
//		if(counter % 10 == 0){
//			//	cout << customersDone << endl;
//			/*if(testNumber != 5 && testNumber != 6 && testNumber != 10) cout <<"-------Total Sale of the entire store until now is $" << totalRevenue <<"---------" << endl;*/
//		}
//
//		/*cashierLinesLock->Acquire(); //going to be checking line counts and statuses, so need this lock*/
//                Acquire(cashierLinesLock);
//
//		 numFullLines = 0; //will be used to figure out whether to bring cashiers back from break
//		 numAnyLines = 0;
//		for ( i = 0; i < cashierNumber; i++){
//			if(privilegedLineCount[i]) numAnyLines++;
//			if(unprivilegedLineCount[i]) numAnyLines++;
//			//a line is "full" if it has more than 3 customers (if each cashier has a line of size 3, we want to bring back a cashier
//			if(privilegedLineCount[i] >= 3) numFullLines ++;
//			if(unprivilegedLineCount[i] >= 3) numFullLines ++;
//		}
//
//		//--------------------------Begin bring cashier back from break--------------------
//		if(numFullLines > (cashierNumber - numCashiersOnBreak) && /*cashiersOnBreak.size()){ //bring back cashier if there are more lines with 3 customers than there are cashiers and if there are cashiers on break*/
//		   numCashiersOnBreak){
//
//		  wakeCashier = cashiersOnBreak[cashiersOnBreakIndex];
//		  cashiersOnBreakIndex++;
//		  if(cashierStatus[wakeCashier] == CASH_ON_BREAK){
//		    /*cashierLinesLock->Release();*/
//		    Release(cashierLinesLock);
//		    /*cashierLock[wakeCashier]->Acquire();*/
//		    Acquire(cashierLock[wakeCashier]);
//		    if (numAnyLines)cout << "Manager brings back Cashier " << wakeCashier << " from break." << endl;
//		    /*cashierToCustCV[wakeCashier]->Signal(cashierLock[wakeCashier]); //this is the actual act of bring a cashier back from break*/
//		    Signal(cashierToCustCV[wakeCashier], cashierLock[wakeCashier]);
//		    /*cashierLock[wakeCashier]->Release();*/
//		    Release(cashierLock[wakeCashier]);
//		    //bookkeeping
//		    numCashiersOnBreak--;
//		    /*cashiersOnBreak.pop();*/
//		    cashiersOnBreakIndex++;
//		    if(cashiersOnBreakIndex == MAX_CASHIERS){
//		      cashiersOnBreakIndex = 0;
//		    }
//
//		  }
//
//
//		}
//		else /*cashierLinesLock->Release();*/
//		  Release(cashierLinesLock);
//
//		/*cashierLinesLock->Acquire();*/
//		Acquire(cashierLinesLock);
//
//
//		//---------------------------End Bring cashier back from break--------------------------
//
//		//---------------------------Begin send cashiers on break-------------------------------
//		 chance = rand() %10;
//
//		if( chance == 1  && numCashiersOnBreak < cashierNumber -2){ //.001% chance of sending cashier on break
//			//generate cashier index
//			 r = rand() % cashierNumber;
//			if(cashierStatus[r] != CASH_ON_BREAK && cashierStatus[r] != CASH_GO_ON_BREAK){
//				if(cashierStatus[r] == CASH_NOT_BUSY) {
//				  /*cashierLock[r]->Acquire();*/
//				  Acquire(cashierLock[r]);
//					cashierDesk[r] = -2;
//					cashierStatus[r] = CASH_GO_ON_BREAK;
//					/*cashierToCustCV[r]->Signal(cashierLock[r]);*/
//					Signal(cashierToCustCV[r], cashierLock[r]);
//
//					/*cashierLock[r]->Release();*/
//					Release(cashierLock[r]);
//				}
//				else cashierStatus[r] = CASH_GO_ON_BREAK;
//				cout << "Manager sends Cashier [" << r << "] on break." << endl;
//				if(testNumber == 5) cout << "Manager has iterated " << counter << " times at this point." << endl;
//				/*cashiersOnBreak.push(r);*/
//				cashiersOnBreak[cashiersOnBreakIndex] = r;
//				cashiersOnBreakIndex++;
//				numCashiersOnBreak++;
//
//
//			}
//		}
//
//		//-----------------------------End send cashiers on break-------------------------------------
//		/*cashierLinesLock->Release();*/
//		Release(cashierLinesLock);
//
//		//__SALES_BREAK__
//		//-----------------------------Begin bringing salesmen back from break-------------
//		 dept = 0;
//		/*if(salesmenOnBreak.size()){*/
//		if(numSalesmenOnBreak){
//
//		  /*int arg = salesmenOnBreak.front();*/
//		   arg = salesmenOnBreak[salesmenOnBreakIndex];
//			 targets[2];
//			deconstructSalesArg(arg, targets);
//			 wakeSalesman = targets[0];
//			dept = targets[1];
//			/*salesmenOnBreak.pop();*/
//			salesmenOnBreakIndex++;
//			/*salesLock[dept]->Acquire();*/
//			Acquire(salesLock[dept]);
//			if((greetingCustWaitingLineCount[dept] + complainingCustWaitingLineCount[dept] + loaderWaitingLineCount[dept]) > 0 && currentSalesStatus[dept][wakeSalesman] == SALES_ON_BREAK){
//				salesBreakBoard[dept][wakeSalesman] = 0;
//				cout << "Manager brings back Salesman [" << wakeSalesman << "] from break." << endl;
//				/*	salesBreakCV[dept][wakeSalesman]->Signal(salesLock[dept]);*/
//				Signal(salesBreakCV[dept][wakeSalesman], salesLock[dept]);
//				numSalesmenOnBreak[dept]--;
//			}
//			else{
//			  /*salesmenOnBreak.push(arg);*/
//			  salesmenOnBreak[salesmenOnBreakIndex] = arg;
//			  salesmenOnBreakIndex++;
//			}
//			/*salesLock[dept]->Release();*/
//			Release(salesLock[dept]);
//		}
//
//		//------------------------------end bringing salesmen back from break--------------
//
//		//------------------------------Begin putting salesmen on break------------------
//		dept = rand() % numDepartments;
//		/*salesLock[dept]->Acquire();*/
//		Acquire(salesLock[dept]);
//		if (chance == 1 && numSalesmenOnBreak[dept] < numSalesmen -1) {
//			 r = rand() % numSalesmen;
//			if(!salesBreakBoard[dept][r] && currentSalesStatus[dept][r] != SALES_ON_BREAK && currentSalesStatus[dept][r] != SALES_GO_ON_BREAK) {
//				salesBreakBoard[dept][r] = 1;
//				cout << "Manager sends Salesman [" << r << "] of dept " << dept << " on break." << endl;
//				/*individualSalesmanLock[dept][r]->Acquire();
//				if(currentSalesStatus[dept][r] == SALES_NOT_BUSY) {
//					salesmanCV[dept][r]->Signal(individualSalesmanLock[dept][r]);
//					currentSalesStatus[dept][r] = SALES_ON_BREAK;
//				}
//				individualSalesmanLock[dept][r]->Release();*/
//				/*salesmenOnBreak.push(constructSalesArg(dept, r)); //function that uses bit operations to store dept and salesman index*/
//																//in one int so I can get it from my queue later when I take a Salesman off break
//				 arg = constructSalesArg(dept, r);
//				salesmenOnBreak[salesmenOnBreakIndex] = arg;
//				salesmenOnBreakIndex++;
//				if(salesmenOnBreakIndex == MAX_SALESMEN){
//				  salesmenOnBreakIndex = 0;
//				}
//				numSalesmenOnBreak[dept]++;
//			}
//		}
//		/*salesLock[dept]->Release();*/
//		Release(salesLock[dept]);
//		//-----------------------------End send salesmen on break
//
//		for(i = 0; i < cashierNumber; i++){
//
//		  /*cashierLock[i]->Acquire(); //acquiring this lock prevents race conditions*/
//										//only manager and cashier i ever attempt to grab this lock
//		  Acquire(cashierLock[i]);
//			//-----------------------------Start empty cashier drawers------------------------------------
//			if(cashRegister[i] > 0){
//				totalRevenue += cashRegister[i];
//				cashierTotals[i] += cashRegister[i]; //how we remember totals for each cashier
//				cout << "Manager emptied Counter [" << i << "] drawer." << endl;
//				cashRegister[i] = 0;
//				cout << "Manager has total sale of $[" << totalRevenue << "]." << endl;
//				/*cashierLock[i]->Release();*/
//				Release(cashierLock[i]);
//				break;
//			}
//			//---------------------------end empty cashier drawers---------------------------------
//
//			//------------------------------Start deal with broke customers-------------------------
//			else if(cashierFlags[i] != -1){ //cashier flags is [i] is change to the Customre at the i-th
//											//cashier desk so I know who to go to and deal with
//				cout <<"Manager got a call from Cashier [" << i << "]." << endl;
//				customerID = cashierFlags[i];
//				cashierID = i;
//
//				/*managerLock->Acquire(); //manager lock protects the manager's interactions*/
//				Acquire(managerLock);
//
//				/*cashierToCustCV[cashierID]->Signal(cashierLock[cashierID]); //wakes up customer, who also waits first in this interaction
//				cashierToCustCV[cashierID]->Signal(cashierLock[cashierID]); //wakes up cashier
//				cashierLock[i]->Release();
//				managerCV->Wait(managerLock);*/
//				Signal(cashierToCustCV[cashierID], cashierLock[cashierID]);
//				Signal(cashierToCustCV[cashierID], cashierLock[cashierID]);
//				Release(cashierLock[i]);
//				Wait(managerCV, managerLock);
//				amountOwed = cashierFlags[i];
//				cashierFlags[i] = -1; //reset cashierFlags
//				custTypeID = managerDesk;
//				/*char* custType = new char[20];*/
//				managerCustType = -1;
//				if(custTypeID == 0){
//					managerCustType = 0;
//				}
//				else{
//				  managerCustType = 1;
//				}
//				managerDesk = amountOwed;
//				/*managerCV->Signal(managerLock);*/
//				Signal(managerCV, managerLock);
//				/*managerCV->Wait(managerLock);*/
//				Wait(managerCV, managerLock);
//				while(managerDesk != -1 ){ //when managerDesk == -1, customer is out of items or can afford money
//					amountOwed -= scan(managerDesk);
//					/*managerItemsLock->Acquire();*/
//					Acquire(managerItemsLock);
//					managerItems.push(managerDesk);
//					/*managerItemsLock->Release();*/
//					Release(managerItemsLock);
//					cout << "Manager removes [" << managerDesk << "] from the trolly of " << custType << " [" << customerID << "]."<<endl;
//					managerDesk = amountOwed; //giving customer new subtotal
//									//customer will put back -1 if out of items
//									// or if they can now afford
//					/*managerCV->Signal(managerLock);*/
//					Signal(managerCV, managerLock);
//					/*managerCV->Wait(managerLock);*/
//					Wait(managerCV, managerLock);
//				}
//				//inactiveLoaderLock->Acquire();
//				//inactiveLoaderCV->Signal(inactiveLoaderLock);
//				//inactiveLoaderLock->Release();
//				//now customer has reached end of items or has enough money
//				//I give him total
//				managerDesk = amountOwed;
//				/*managerCV->Signal(managerLock);*/
//				Signal(managerCV, managerLock);
//				/*managerCV->Wait(managerLock); //wait for his response, i.e. paying money*/
//				Wait(managerCV, managerLock);
//				totalRevenue += managerDesk;
//
//				//Now give the customer his receipt
//				cout << "Manager gives receipt to " << custType << " [" << customerID << "]." << endl;
//				managerDesk = -1;
//				/*managerCV->Signal(managerLock);*/
//				Signal(managerCV, managerLock);
//				//release manager lock
//				/*managerLock->Release();*/
//				Release(managerLock);
//				break;
//
//
//			}
//			//----------------------------End deal with broke customers--------------------
//			/*else cashierLock[i]->Release();*/
//			else
//			  Release(cashierLock[i]);
//		}
//	}
//}
//
//
//
//
////cashier code
//
//void cashier(){
//  int myCounter, ;
//	/*set the cashier's counter*/
//	Acquire(cashierIndexLock);
//	myCounter = nextCashierIndex;
//	nextCashierIndex++;
//	Release(cashierIndexLock);
//
//	cashierStatus[myCounter] = CASH_NOT_BUSY;
//	while(true){
//		//char* custType = new char[20];
//		//cashierLinesLock->Acquire();
//		Acquire(cashierLinesLock);
//		//check my status to see if I've already been set to busy by a customer
//		if(cashierStatus[myCounter] == CASH_GO_ON_BREAK){
//			//cout << "Cashier [" << myCounter << " acknowledges he will go on break" << endl;
//
//
//		cout << "Cashier [" << myCounter << "] is going on break." << endl;
//		cashierStatus[myCounter] = CASH_ON_BREAK;
//		//unprivilegedCashierLineCV[myCounter]->Broadcast(cashierLinesLock);
//		Broadcast(unprivilegedCashierLineCV[myCounter], cashierLinesLock);
//		//privilegedCashierLineCV[myCounter]->Broadcast(cashierLinesLock);
//		Broadcast(privilegedCashierLineCV[myCounter], cashierLinesLock);
//
//		//cashierLock[myCounter]->Acquire();
//		Acquire(cashierLock[myCounter]);
//		//cashierLinesLock->Release();
//		Release(cashierLinesLock);
//
//		//cashierToCustCV[myCounter]->Wait(cashierLock[myCounter]);
//		Wait(cashierToCustCV[myCounter], cashierLock[myCounter]);
//		//cashierLinesLock->Acquire();
//		Acquire(cashierLinesLock);
//		cashierStatus[myCounter] = CASH_NOT_BUSY;
//		cout << "Cashier [" << myCounter << "] was called from break by Manager to work." << endl;
//	}
//	//check if my lines have anyone in it
//	//set my state so approaching Customers can wait in or engage me, as apropriate
//	if(privilegedLineCount[myCounter]){
//		//privilegedCashierLineCV[myCounter]->Signal(cashierLinesLock);
//          Signal(privilegedCashierLineCV[myCounter], cashierLinesLock);
//		//custType = "Privileged Customer";
//		custType[myCounter] = PRIVILEGED_CUSTOMER;
//	}
//	else if(unprivilegedLineCount[myCounter]){
//		//cout << "Signalling customer in line" << endl;
//		//unprivilegedCashierLineCV[myCounter]->Signal(cashierLinesLock);
//          Signal(unprivilegedCashierLineCV[myCounter], cashierLinesLock);
//		//custType = "Customer";
//		custType[myCounter] = CUSTOMER;
//	}
//	else{
//		cashierStatus[myCounter] = CASH_NOT_BUSY; //means I can be approached without it being necessary to wait in line
//	}
//	//whether or not I'm ready for a customer, I can get my lock and go to sleep
//	//cashierLock[myCounter]->Acquire();
//	Acquire(cashierLock[myCounter]);
//	//cashierLinesLock->Release();
//	Release(cashierLinesLock);
//	//cashierToCustCV[myCounter]->Signal(cashierLock[myCounter]);
//	Signal(cashierToCustCV[myCounter], cashierLock[myCounter]);
//	//cashierToCustCV[myCounter]->Wait(cashierLock[myCounter]);
//	Wait(cashierToCustCV[myCounter], cashierLock[myCounter]);
//	//the next time I'm woken up (assuming it is by a customer, not a manager
//	//I will be totaling items
//	//when I get here, there will be an item to scan
//	//int total = 0;
//	total[myCounter] = 0;
//	//int custID = cashierDesk[myCounter];
//	custID[myCounter] = cashierDesk[myCounter];
//	if(custID[myCounter] == -2){
//		//cashierLock[myCounter]->Release();
//		Release(cashierLock[myCounter]);
//		continue;
//	}
//	//cashierToCustCV[myCounter]->Signal(cashierLock[myCounter]);
//	Signal(cashierToCustCV[myCounter], cashierLock[myCounter]);
//	//cashierToCustCV[myCounter]->Wait(cashierLock[myCounter]);
//	Wait(cashierToCustCV[myCounter], cashierLock[myCounter]);
//
//	if(cashierDesk[myCounter] == 1){
//		//custType = "PrivilegedCustomer";
//		custType[myCounter] = PRIVILEGED_CUSTOMER;
//	}
//	else{
//		//custType = "Customer";
//		custType[myCounter] = CUSTOMER;
//	}
//
//	//cashierToCustCV[myCounter]->Signal(cashierLock[myCounter]);
//	Signal(cashierToCustCV[myCounter], cashierLock[myCounter]);
//	//cashierToCustCV[myCounter]->Wait(cashierLock[myCounter]);
//	Wait(cashierToCustCV[myCounter], cashierLock[myCounter]);
//	while(cashierDesk[myCounter] != -1){ //-1 means we're done scanning
//
//
//		cout << "Cashier [" << myCounter << "] got [" << cashierDesk[myCounter] << "] from trolly of " << custType << " [" << custID << "]." << endl;
//		total[myCounter] += scan(cashierDesk[myCounter]);
//		//cashierToCustCV[myCounter]->Signal(cashierLock[myCounter]);
//		Signal(cashierToCustCV[myCounter], cashierLock[myCounter]);
//		//cashierToCustCV[myCounter]->Wait(cashierLock[myCounter]);
//		Wait(cashierToCustCV[myCounter], cashierLock[myCounter]);
//	}
//	//now I'm done scanning, so I tell the customer the total
//	/*cout << "Cashier [" << myCounter << "] tells " << custType << " [" << custID << "] total cost is $[" << total << "]." << endl;*/
//	cashierDesk[myCounter] = total[myCounter];
//	//cashierToCustCV[myCounter]->Signal(cashierLock[myCounter]);
//	Signal(cashierToCustCV[myCounter], cashierLock[myCounter]);
//	//cashierToCustCV[myCounter]->Wait(cashierLock[myCounter]);
//	Wait(cashierToCustCV[myCounter], cashierLock[myCounter]);
//	if(cashierDesk[myCounter] == -1){
//          /*		cout << "Cashier [" << myCounter << "] asks " << custType << " [" << custID << "] to wait for Manager." << endl;
//                        cout << "Cashier [" << myCounter << "] informs the Manager that " << custType << " [" << custID << "] does not have enough money." << endl;*/
//		cashierFlags[myCounter] = custID[myCounter];
//		//cout << "cashier is goign to sleep" << endl;
//		//cashierToCustCV[myCounter]->Wait(cashierLock[myCounter]);
//		Wait(cashierToCustCV[myCounter], cashierLock[myCounter]);
//		//Set the manager desk to 0 or 1 to tell the manager which type of
//		//customer he is dealing with
//		//if(!strcmp(custType, "Privileged Customer")){
//		if(custType[myCounter] == PRIVILEGED_CUSTOMER){
//			managerDesk = 1;
//		}
//		//if(!strcmp(custType, "Customer")){
//		if(custType[myCounter] == CUSTOMER){
//			managerDesk = 0;
//		}
//		cout << " total " << total << endl;
//		cashierFlags[myCounter] = total[myCounter]; //inform manager of the total the customer owes
//		//managerLock->Acquire();
//		Acquire(managerLock);
//		//managerCV->Signal(managerLock); //wake up manager, who was waiting for this information
//		Signal(managerCV, managerLock);
//		//managerLock->Release();
//		Release(managerLock);
//		//when I am woken up, the manager has taken over so I can free myself for my
//		//next customer
//	}
//	else{
//		//add value to cash register
//		cashRegister[myCounter] += cashierDesk[myCounter];
//		cout << "Cashier [" << myCounter << "] got money $[" << cashierDesk[myCounter] << "] from " << custType << " [" << custID << "]." << endl;
//		//giving the customer a receipt
//		cout << "Cashier [" << myCounter << "] gave the receipt to " << custType << " [" << custID << "] and tells him to leave" << endl;
//		//cashierToCustCV[myCounter]->Signal(cashierLock[myCounter]);
//		Signal(cashierToCustCV[myCounter], cashierLock[myCounter]);
//		//wait for customer to acknowledge getting receipt and release lock
//		//cashierToCustCV[myCounter]->Wait(cashierLock[myCounter]);
//		Wait(cashierToCustCV[myCounter], cashierLock[myCounter]);
//	}
//	cashierDesk[myCounter] = 0;
//	//cashierLock[myCounter]->Release();
//	Release(cashierLock[myCounter]);
//
//	//done, just need to loop back and check for more customers
//	//delete custType;
//	custType[myCounter] = CUSTOMER;
//	}
//}
//
//
//
////Salesman Code		__SALESMAN__
////void Salesman (int myIndex){
//void Salesman() {
//  int myIndex, myCustNumber, itemOutOfStock, itemRestocked, loaderNumber, myLoaderID,
//    i;
//  SalesmanStatus prev;
//  Acquire(salesmanIndexLock);
//  myIndex = nextSalesmanIndex;
//  nextSalesmanIndex++;
//  Release(salesmanIndexLock);
//  int myDept;
//  Acquire(departmentIndexLock);
//  myDept = nextDepartmenIndex;
//  nextDepartmenIndex++;
//  Release(departmentIndexLock);
////	int argTargets[2];
////	deconstructSalesArg(arg, argTargets);	//gets the index and department from the number passed in
////	int myIndex = argTargets[0];
////	int myDept = argTargets[1];
//
//	while(true) {
//          //salesLock[myDept]->Acquire();
//          Acquire(salesLock[myIndex]);
//
//		//go on break if the manager has left me a note saying to
//		if(salesBreakBoard[myDept][myIndex] == 1) {
//			cout << "Sales " << myIndex << " in department " << myDept << " going on break" << endl;
//			prev = currentSalesStatus[myDept][myIndex];
//			currentSalesStatus[myDept][myIndex] = SALES_ON_BREAK;
//			salesBreakBoard[myDept][myIndex] = 0;
//			//salesBreakCV[myDept][myIndex]->Wait(salesLock[myDept]);
//			Wait (salesBreakCV[myDept][myIndex],salesLock[myDept]);
//			currentSalesStatus[myDept][myIndex] = prev;
//			/*cout << "Sales " << myIndex << " in department " << myDept << " back from break" << endl;*/
//		}
//
//		//Check if there is someone in a line and wake them up
//		if(greetingCustWaitingLineCount[myDept] > 0){	//greeting
//			currentSalesStatus[myDept][myIndex] = SALES_READY_TO_TALK;
//			//greetingCustCV[myDept]->Signal(salesLock[myDept]);
//			Signal(greetingCustCV[myDept], salesLock[myDept]);
//			greetingCustWaitingLineCount[myDept]--;
//		}
//		else if(loaderWaitingLineCount[myDept] > 0) {	//loader
//			currentSalesStatus[myDept][myIndex] = SALES_READY_TO_TALK_TO_LOADER;
//			//loaderCV[myDept]->Signal(salesLock[myDept]);
//			Signal(loaderCV[myDept], salesLock[myDept]);
//			loaderWaitingLineCount[myDept]--;
//		}
//		else if(complainingCustWaitingLineCount[myDept] > 0) {	//complaining
//			currentSalesStatus[myDept][myIndex] = SALES_READY_TO_TALK;
//			//complainingCustCV[myDept]->Signal(salesLock[myDept]);
//                        Signal(complainingCustCV[myDept], salesLock[myDept]);
//			complainingCustWaitingLineCount[myDept]--;
//		}
//
//		else{	//not busy
//			currentlyTalkingTo[myDept][myIndex] = UNKNOWN;
//			currentSalesStatus[myDept][myIndex] = SALES_NOT_BUSY;
//		}
//
//		//individualSalesmanLock[myDept][myIndex]->Acquire();
//                Acquire(individualSalesmanLock[myDept][myIndex]);
//		//salesLock[myDept]->Release();
//                Release(salesLock[myDept]);
//		//salesmanCV[myDept][myIndex]->Wait(individualSalesmanLock[myDept][myIndex]);	//Wait for cust/loader to walk up to me
//                Wait (salesmanCV[myDept][myIndex], individualSalesmanLock[myDept][myIndex]);
//
//		if(currentlyTalkingTo[myDept][myIndex] == GREETING) {	//a greeting customer came up to me
//			myCustNumber = salesCustNumber[myDept][myIndex];
//			if(privCustomers[myCustNumber] == 1){
//				cout << "DepartmentSalesman [" << myIndex << "] welcomes PrivilegeCustomer [" << myCustNumber << "] to Department [" << myDept << "]." << endl;
//			}
//			else{
//				cout << "DepartmentSalesman [" << myIndex << "] welcomes Customer [" << myCustNumber << "] to Department [" << myDept << "]." << endl;
//			}
//			//salesmanCV[myDept][myIndex]->Signal(individualSalesmanLock[myDept][myIndex]);
//                        Signal (salesmanCV[myDept][myIndex], individualSalesmanLock[myDept][myIndex]);
//			//individualSalesmanLock[myDept][myIndex]->Release();
//                        Release(individualSalesmanLock[myDept][myIndex]);
//		}
//		else if(currentlyTalkingTo[myDept][myIndex] == COMPLAINING) {	//a complaining customer came up to me
//			myCustNumber = salesCustNumber[myDept][myIndex];
//			itemOutOfStock = salesDesk[myDept][myIndex];
//
//			if(privCustomers[myCustNumber] == 1){
//				cout << "DepartmentSalesman [" << myIndex << "] is informed by PrivilegeCustomer [" << myCustNumber << "] that [" << itemOutOfStock << "] is out of stock." << endl;
//			}
//			else{
//				cout << "DepartmentSalesman [" << myIndex << "] is informed by Customer [" << myCustNumber << "] that [" << itemOutOfStock << "] is out of stock." << endl;
//			}
//			//individualSalesmanLock[myDept][myIndex]->Acquire();
//                        Acquire(individualSalesmanLock[myDept][myIndex]);
//			//salesmanCV[myDept][myIndex]->Signal(individualSalesmanLock[myDept][myIndex]);	//tell cust to wait for the item
//                        Signal(salesmanCV[myDept][myIndex], individualSalesmanLock[myDept][myIndex]);
//
//			//tell goods loader
//			//salesLock[myDept]->Acquire();
//                        Acquire(salesLock[myDept]);
//			salesDesk[myDept][myIndex] = itemOutOfStock;	//Might not be necessary, because we never really took it off the desk
//
//			if(loaderWaitingLineCount[myDept] > 0) {	//if we can, get a loader from line
//				loaderWaitingLineCount[myDept]--;
//				currentSalesStatus[myDept][myIndex] = SALES_READY_TO_TALK_TO_LOADER;
//				//loaderCV[myDept]->Signal(salesLock[myDept]);	//get a loader from line
//                                Signal(loaderCV[myDept], salesLock[myDept]);
//				//salesLock[myDept]->Release();
//                                Release(salesLock[myDept]);
//			}
//			else {	// no one was in line, so go to the inactive loaders
//				currentSalesStatus[myDept][myIndex] = SALES_SIGNALLING_LOADER;
//				//salesLock[myDept]->Release();
//                                Release(salesLock[myDept]);
//
//				//inactiveLoaderLock->Acquire();
//                                Acquire(inactiveLoaderLock);
//				//inactiveLoaderCV->Signal(inactiveLoaderLock);	//call a loader over
//                                Signal(inactiveLoaderCV, inactiveLoaderLock);
//				//inactiveLoaderLock->Release();
//                                Release(inactiveLoaderLock);
//			}
//
//			//salesmanCV[myDept][myIndex]->Wait(individualSalesmanLock[myDept][myIndex]);	//wait for the OK from loader (no matter where he came from)
//                        Wait(salesmanCV[myDept][myIndex], individualSalesmanLock[myDept][myIndex]);
//
//			//check to see if a loader already restocked something
//			if(salesDesk[myDept][myIndex] != -1) {		//loader had finished stocking something and tells me about it
//				itemRestocked = salesDesk[myDept][myIndex];
//				//salesmanCV[myDept][myIndex]->Signal(individualSalesmanLock[myDept][myIndex]);
//                                Signal(salesmanCV[myDept][myIndex], individualSalesmanLock[myDept][myIndex]);
//				//salesmanCV[myDept][myIndex]->Wait(individualSalesmanLock[myDept][myIndex]);
//                                Wait(salesmanCV[myDept][myIndex], individualSalesmanLock[myDept][myIndex]);
//				loaderNumber = salesDesk[myDept][myIndex];
//				//salesmanCV[myDept][myIndex]->Signal(individualSalesmanLock[myDept][myIndex]);
//                                Signal(salesmanCV[myDept][myIndex], individualSalesmanLock[myDept][myIndex]);
//				cout << "DepartmentSalesman [" << myIndex << "] is informed by the GoodsLoader [" << loaderNumber << "] that [" << itemRestocked << "] is restocked." << endl;
//				//shelfLock[myDept][itemRestocked]->Acquire();
//                                Acquire(shelfLock[myDept][itemRestocked]);
//				//shelfCV[myDept][itemRestocked]->Broadcast(shelfLock[myDept][itemRestocked]);
//                                Broadcast(shelfCV[myDept][itemRestocked], shelfLock[myDept][itemRestocked]);
//				//shelfLock[myDept][itemRestocked]->Release();
//                                Release(shelfLock[myDept][itemRestocked]);
//				//can't know the cust number, becaue we broadcast here!
//				//DepartmentSalesman [identifier] informs the Customer/PrivilegeCustomer [identifier] that [item] is restocked.
//			}
//
//
//			//inactiveLoaderLock->Acquire();
//                        Acquire(inactiveLoaderLock);
//			myLoaderID = -1;
//
//			for(i = 0; i < numLoaders; i++) {	//find the loader who i just sent off to restock and change his status
//				if(loaderStatus[i] == LOAD_HAS_BEEN_SIGNALLED) {
//					myLoaderID = i;
//					loaderStatus[i] = LOAD_STOCKING;
//					break;
//				}
//			}
//
//			//inactiveLoaderLock->Release();
//                        Release(inactiveLoaderLock);
//
//			cout << "DepartmentSalesman [" << myIndex << "] informs the GoodsLoader [" << myLoaderID << "] that [" << itemOutOfStock << "] is out of stock." << endl;
//			//salesmanCV[myDept][myIndex]->Signal(individualSalesmanLock[myDept][myIndex]);
//                        Signal(salesmanCV[myDept][myIndex], individualSalesmanLock[myDept][myIndex]);
//			//individualSalesmanLock[myDept][myIndex]->Release();
//                        Release(individualSalesmanLock[myDept][myIndex]);
//
//		}
//		else if(currentlyTalkingTo[myDept][myIndex] == GOODSLOADER) {	//a loader came up to talk to me
//			itemRestocked = salesDesk[myDept][myIndex];
//			//salesmanCV[myDept][myIndex]->Signal(individualSalesmanLock[myDept][myIndex]);
//                        Signal(salesmanCV[myDept][myIndex], individualSalesmanLock[myDept][myIndex]);
//			//salesmanCV[myDept][myIndex]->Wait(individualSalesmanLock[myDept][myIndex]);
//                        Wait(salesmanCV[myDept][myIndex], individualSalesmanLock[myDept][myIndex]);
//			loaderNumber = salesDesk[myDept][myIndex];
//			//salesmanCV[myDept][myIndex]->Signal(individualSalesmanLock[myDept][myIndex]);
//                        Signal(salesmanCV[myDept][myIndex], individualSalesmanLock[myDept][myIndex]);
//			cout << "DepartmentSalesman [" << myIndex << "] is informed by the GoodsLoader [" << loaderNumber << "] that [" << itemRestocked << "] is restocked." << endl;
//			//shelfLock[myDept][itemRestocked]->Acquire();
//                        Acquire(shelfLock[myDept][itemRestocked]);
//			//shelfCV[myDept][itemRestocked]->Broadcast(shelfLock[myDept][itemRestocked]);	//tell customers who were waiting on an item that it was restocked
//                        Broadcast(shelfCV[myDept][itemRestocked], shelfLock[myDept][itemRestocked]);
//			//shelfLock[myDept][itemRestocked]->Release();
//                        Release(shelfLock[myDept][itemRestocked]);
//			//individualSalesmanLock[myDept][myIndex]->Release();
//                        Release(individualSalesmanLock[myDept][myIndex]);
//			//can't know the cust number, becaue we broadcast here!
//			//DepartmentSalesman [identifier] informs the Customer/PrivilegeCustomer [identifier] that [item] is restocked.
//		}
//		else{
//                  //individualSalesmanLock[myDept][myIndex]->Release();
//                  Release(individualSalesmanLock[myDept][myIndex]);
//		}
//	}
//}
//
//
////Goods loader code			__LOADER__
//void GoodsLoader() {
//	int myID;
//	Acquire(loaderIndexLock);
//	myID = nextLoaderIndex;
//	nextLoaderIndex++;
//	Release(loaderIndexLock);
//	int currentDept = -1;	//the department i am dealing with
//	int mySalesID = -1;		//the sales i am helping
//	int shelf = -1;			//the shelf i am dealing with
//
//
//	//bool foundNewOrder = false;	//true if i go to help a salesman, and he was signalling for a loader to restock something
//	int foundNewOrder = 0;
//	int i, j, inHands, restoreTrollies, qtyInHands, newShelfToStock;
//	//inactiveLoaderLock->Acquire();
//	Acquire(inactiveLoaderLock);
//	//normal action loop
//	while(true) {
//		if(!foundNewOrder) {	//if i don't have a new order (from my last run) go to sleep
//			loaderStatus[myID] = LOAD_NOT_BUSY;
//			if(mySalesID != -1){
//				cout << "GoodsLoader [" << myID << "] is waiting for orders to restock." << endl;
//			}
//			//inactiveLoaderCV->Wait(inactiveLoaderLock);
//			Wait(inactiveLoaderCV, inactiveLoaderLock);
//			//at this point, I have just been woken up from the inactive loaders waiting area
//		}
//		foundNewOrder = 0;	//initialize that I have not found a new order for this run yet
//
//		loaderStatus[myID] = LOAD_HAS_BEEN_SIGNALLED;
//		mySalesID = -50;
//
//
//		//look through all departments to find out who signalled me
//		for(j = 0; j < numDepartments; j++) {
//			//salesLock[j]->Acquire();
//			Acquire(salesLock[j]);
//			for(i = 0; i < numSalesmen; i++) {
//				if(currentSalesStatus[j][i] == SALES_SIGNALLING_LOADER) {	//i found a person who was signalling for a loader!
//					mySalesID = i;
//					currentDept = j;
//					currentSalesStatus[currentDept][mySalesID] = SALES_BUSY;
//					break;
//				}
//			}
//			//salesLock[j]->Release();
//			Release(salesLock[j]);
//			if(mySalesID != -50) {	//used to break the second level of for loop if i found a salesman who needs me
//				break;
//			}
//		}
//		//inactiveLoaderLock->Release();
//		Release(inactiveLoaderLock);
//
//		if(mySalesID == -50){ //the loader was signaled by the manager to get trollys (signalled, but no salesmen are signalling for me)
//			//managerItemsLock->Acquire();
//			Acquire(managerItemsLock);
//			inHands = 0;
//			i = 0;
//			while(!managerItems[i] == -1){
//				//simulates the manager putting the items in the stockroom
//				//inHands = managerItems.front();
//				inHands = managerItems[i];
//				//managerItems.pop(); //remove the item from the manager's item list
//				managerItems[i] = -1;
//				i++;
//				//stockRoomLock->Acquire();
//				Acquire(stockRoomLock);
//				cout << "Goodsloader [" << myID << "] put item [" << inHands << "] back in the stock room from the manager" << endl;
//				inHands = 0;
//				//stockRoomLock->Release();
//				Release(stockRoomLock);
//			}
//			//managerItemsLock->Release();
//			Release(managerItemsLock);
//
//			//moves trollies back to the front of the store
//			//displacedTrollyLock->Acquire();
//			Acquire(displacedTrollyLock);
//			restoreTrollies = 0;
//			if(displacedTrollyCount > 0){
//				restoreTrollies = displacedTrollyCount;
//				displacedTrollyCount = 0;
//			}
//			//displacedTrollyLock->Release();
//			Release(displacedTrollyLock);
//
//			if(restoreTrollies != 0) {
//				//trollyLock->Acquire();
//				Acquire(trollyLock);
//				trollyCount += restoreTrollies;
//				//trollyCV->Broadcast(trollyLock);	//inform everyone that there are now more trollies in the front
//				Broadcast(trollyCV, trollyLock);
//				//trollyLock->Release();
//				Release(trollyLock);
//			}
//
//		}
//		else{	//It was not the manager who signalled me
//			//individualSalesmanLock[currentDept][mySalesID]->Acquire();
//			Acquire(individualSalesmanLock[currentDept][mySalesID]);
//			shelf = salesDesk[currentDept][mySalesID];	//read the shelf that needs stocking in from the desk
//			salesDesk[currentDept][mySalesID] = -1;		//write back that i was not on a previous job
//
//			cout << "GoodsLoader [" << myID << "] is informed by DepartmentSalesman [" << mySalesID <<
//					"] of Department [" << currentDept << "] to restock [" << shelf << "]" << endl;
//
//			//salesmanCV[currentDept][mySalesID]->Signal(individualSalesmanLock[currentDept][mySalesID]);	//tell him i'm here
//			Signal(salesmanCV[currentDept][mySalesID], individualSalesmanLock[currentDept][mySalesID]);
//			//salesmanCV[currentDept][mySalesID]->Wait(individualSalesmanLock[currentDept][mySalesID]);
//			Wait(salesmanCV[currentDept][mySalesID], individualSalesmanLock[currentDept][mySalesID]);
//			//individualSalesmanLock[currentDept][mySalesID]->Release();
//			Release(individualSalesmanLock[currentDept][mySalesID]);
//
//
//			//Restock items
//			qtyInHands = 0;
//			//shelfLock[currentDept][shelf]->Acquire();
//			Acquire(shelfLock[currentDept][shelf]);
//			while(shelfInventory[currentDept][shelf] < maxShelfQty) {
//				//shelfLock[currentDept][shelf]->Release();
//				Release(shelfLock[currentDept][shelf]);
//
//				//currentLoaderInStockLock->Acquire(); //acquires the lock in order to access the number of waiting loaders and the id of the current loader in the stock room
//				Acquire(currentLoaderInStockLock);
//				if(currentLoaderInStock != -1 || waitingForStockRoomCount > 0){//if either someone is in line or there is a loader in the stock room...
//					cout << "GoodsLoader [" << myID << "] is waiting for GoodsLoader [" << currentLoaderInStock << "] to leave the StockRoom." << endl;
//					waitingForStockRoomCount++;
//					//currentLoaderInStockCV->Wait(currentLoaderInStockLock); //wait to be let into the stock room
//					Wait(currentLoaderInStockCV, currentLoaderInStockLock);
//				}
//				//stockRoomLock->Acquire(); //acquire the lock to the actual stockroom
//				Acquire(stockRoomLock);
//				if(currentLoaderInStock != -1){ //if someone was in the stockroom, change the current loader to this loader's id, wake him up, and then wait for him to acknowledge
//					currentLoaderInStock = myID;
//					//stockRoomCV->Signal(stockRoomLock);
//					Signal(stockRoomCV, stockRoomLock);
//					//stockRoomCV->Wait(stockRoomLock);
//					Wait(stockRoomCV, stockRoomLock);
//				}
//				else{ //if no one was in the stockroom, then just set the current loader to this loader's id
//					currentLoaderInStock = myID;
//				}
//				cout << "GoodsLoader [" << myID << "] is in the StockRoom and got [" << shelf << "]" << endl;
//
//				//currentLoaderInStockLock->Release(); //releasing this allows others to check the current loader and get in line
//				Release(currentLoaderInStockLock);
//				qtyInHands++; //grab an item
//				//currentLoaderInStockLock->Acquire();
//				Acquire(currentLoaderInStockLock);
//				if(waitingForStockRoomCount > 0){ //if there are people in line, then signal the next person in line and decrement the line count
//					//currentLoaderInStockCV->Signal(currentLoaderInStockLock);
//					Signal(currentLoaderInStockCV, currentLoaderInStockLock);
//					waitingForStockRoomCount--;
//					//currentLoaderInStockLock->Release();
//					Release(currentLoaderInStockLock);
//					//stockRoomCV->Wait(stockRoomLock); //wait for the loader that was woke up to put their id into current loader and then print that this loader is leaving
//					Wait(stockRoomCV, stockRoomLock);
//					cout << "GoodsLoader [" << myID << "] leaves StockRoom." << endl;
//					//stockRoomCV->Signal(stockRoomLock); //wake up the last loader so they can get into the stockroom
//					Signal(stockRoomCV, stockRoomLock);
//				}
//				else{
//					currentLoaderInStock = -1; //if no one is in line, then set the current loader to -1, which signifies that the stockroom was empty prior
//					cout << "GoodsLoader [" << myID << "] leaves StockRoom." << endl;
//					//currentLoaderInStockLock->Release();
//					Release(currentLoaderInStockLock);
//				}
//				//stockRoomLock->Release();
//				Release(stockRoomLock);
//				for(j = 0; j < 5; j++) {
//					currentThread->Yield();
//				}
//
//				//check the shelf i am going to restock
//				//shelfLock[currentDept][shelf]->Acquire();
//				Acquire(shelfLock[currentDept][shelf]);
//				if(testNumber == 8) cout << "GoodsLoader [" << myID << "] is in the act of restocking shelf [" << shelf << "] in Department [" << currentDept << "]." << endl;
//				if(testNumber == 8) cout << "The shelf had " << shelfInventory[currentDept][shelf] << "  but GoodsLoader " << myID <<" has added one more." << endl;
//
//				if(shelfInventory[currentDept][shelf] == maxShelfQty) {	//check to see if a shelf needs more items
//					cout << "GoodsLoader [" << myID << "] has restocked [" << shelf << "] in Department [" << currentDept << "]." << endl;
//					qtyInHands = 0;
//					break;
//				}
//				shelfInventory[currentDept][shelf] += qtyInHands;	//put more items on it
//				qtyInHands = 0;
//			}
//			//shelfLock[currentDept][shelf]->Release();
//			Release(shelfLock[currentDept][shelf]);
//
//
//			//We have finished restocking.  now wait in line/inform sales
//			//salesLock[currentDept]->Acquire();
//			Acquire(salesLock[currentDept]);
//
//			mySalesID = -50;
//			//first look for someone who is signalling for a loader, since we can multipurpose and take a new job, while also informing that we finished a previous one
//			for(i = 0; i < numSalesmen; i++) {
//				if(currentSalesStatus[currentDept][i] == SALES_SIGNALLING_LOADER) {
//					//all salesmen are busy trying to get loaders, but the loaders are out and trying to tell them that a job was finished
//					mySalesID = i;
//
//					//Ready to go talk to sales
//					newShelfToStock = salesDesk[currentDept][mySalesID];
//
//					currentlyTalkingTo[currentDept][mySalesID] = GOODSLOADER;
//					currentSalesStatus[currentDept][mySalesID] = SALES_BUSY;
//					//salesLock[currentDept]->Release();
//					Release(salesLock[currentDept]);
//					//individualSalesmanLock[currentDept][mySalesID]->Acquire();
//					Acquire(individualSalesmanLock[currentDept][mySalesID]);
//					salesDesk[currentDept][mySalesID] = shelf;
//					//salesmanCV[currentDept][mySalesID]->Signal(individualSalesmanLock[currentDept][mySalesID]);
//					Signal(salesmanCV[currentDept][mySalesID], individualSalesmanLock[currentDept][mySalesID]);
//					//salesmanCV[currentDept][mySalesID]->Wait(individualSalesmanLock[currentDept][mySalesID]);
//					Wait(salesmanCV[currentDept][mySalesID], individualSalesmanLock[currentDept][mySalesID]);
//					salesDesk[currentDept][mySalesID] = myID;
//					//salesmanCV[currentDept][mySalesID]->Signal(individualSalesmanLock[currentDept][mySalesID]);
//					Signal(salesmanCV[currentDept][mySalesID], individualSalesmanLock[currentDept][mySalesID]);
//					//individualSalesmanLock[currentDept][mySalesID]->Release();
//					Release(individualSalesmanLock[currentDept][mySalesID]);
//
//					//i have talked to sales and they had a new job for me
//					shelf = newShelfToStock;
//					//foundNewOrder = true;
//					foundNewOrder = 1;
//					break;
//				}
//			}
//			if(mySalesID == -50) {
//				//no one was signalling, so look for free salesmen to go report to
//				for(i = 0; i < numSalesmen; i++) {
//					if(currentSalesStatus[currentDept][i] == SALES_NOT_BUSY) {
//						mySalesID = i;
//
//						//Ready to go talk to sales
//						currentlyTalkingTo[currentDept][mySalesID] = GOODSLOADER;
//						currentSalesStatus[currentDept][mySalesID] = SALES_BUSY;
//						//salesLock[currentDept]->Release();
//						Release(salesLock[currentDept]);
//						//individualSalesmanLock[currentDept][mySalesID]->Acquire();
//						Acquire(individualSalesmanLock[currentDept][mySalesID]);
//						salesDesk[currentDept][mySalesID] = shelf;
//						//salesmanCV[currentDept][mySalesID]->Signal(individualSalesmanLock[currentDept][mySalesID]);
//						Signal(salesmanCV[currentDept][mySalesID], individualSalesmanLock[currentDept][mySalesID]);
//						//salesmanCV[currentDept][mySalesID]->Wait(individualSalesmanLock[currentDept][mySalesID]);
//						Wait(salesmanCV[currentDept][mySalesID], individualSalesmanLock[currentDept][mySalesID]);
//						salesDesk[currentDept][mySalesID] = myID;
//						//salesmanCV[currentDept][mySalesID]->Signal(individualSalesmanLock[currentDept][mySalesID]);
//						Signal(salesmanCV[currentDept][mySalesID], individualSalesmanLock[currentDept][mySalesID]);
//						//individualSalesmanLock[currentDept][mySalesID]->Release();
//						Release(individualSalesmanLock[currentDept][mySalesID]);
//						break;
//					}
//				}
//			}
//
//
//			if(!foundNewOrder) {	//i have to get in line, because i didn't find a new order from someone signalling
//									//(i would have given them my information when i took their order)
//				if(mySalesID == -50) {	//if i have STILL not found anyone, then i do need to get in line
//					loaderWaitingLineCount[currentDept]++;
//
//					//loaderCV[currentDept]->Wait(salesLock[currentDept]);//get in line		//STUCK HERE
//					Wait(loaderCV[currentDept], salesLock[currentDept]);
//
//					for(i = 0; i < numSalesmen; i++) {	//find the salesman who signalled me out of line
//						if(currentSalesStatus[currentDept][i] == SALES_READY_TO_TALK_TO_LOADER) {
//							mySalesID = i;
//
//							//Ready to go talk to a salesman
//							currentlyTalkingTo[currentDept][mySalesID] = GOODSLOADER;
//							currentSalesStatus[currentDept][mySalesID] = SALES_BUSY;
//
//							//salesLock[currentDept]->Release();
//							Release(salesLock[currentDept]);
//							//individualSalesmanLock[currentDept][mySalesID]->Acquire();
//							Acquire(individualSalesmanLock[currentDept][mySalesID]);
//							salesDesk[currentDept][mySalesID] = shelf;
//							//salesmanCV[currentDept][mySalesID]->Signal(individualSalesmanLock[currentDept][mySalesID]);
//							Signal(salesmanCV[currentDept][mySalesID], individualSalesmanLock[currentDept][mySalesID]);
//							//salesmanCV[currentDept][mySalesID]->Wait(individualSalesmanLock[currentDept][mySalesID]);
//							Wait(salesmanCV[currentDept][mySalesID], individualSalesmanLock[currentDept][mySalesID]);
//							salesDesk[currentDept][mySalesID] = myID;
//							//salesmanCV[currentDept][mySalesID]->Signal(individualSalesmanLock[currentDept][mySalesID]);
//							Signal(salesmanCV[currentDept][mySalesID], individualSalesmanLock[currentDept][mySalesID]);
//							//individualSalesmanLock[currentDept][mySalesID]->Release();
//							Release(individualSalesmanLock[currentDept][mySalesID]);
//
//							break;
//						}
//					}
//				}
//			}
//		}
//
//		//Look at all depts to see if anyone else has a job that i should be aware of
//		//inactiveLoaderLock->Acquire();
//		Acquire(inactiveLoaderLock);
//		for(j = 0; j < numDepartments; j++) {
//			//salesLock[j]->Acquire();
//			Acquire(salesLock[j]);
//			for(i = 0; i < numSalesmen; i++) {
//				if(currentSalesStatus[j][i] == SALES_SIGNALLING_LOADER) {
//					//foundNewOrder = true;
//					foundNewOrder = 1;
//					break;
//				}
//			}
//			//salesLock[j]->Release();
//			Release(salesLock[j]);
//			if(foundNewOrder) {	//break out of the outer loop if we did in fact find someone
//				break;
//			}
//		}
//	}
//}
//
//
////================
//// Tests
////================
//
////Test for greeting the customer
///*
//void TestGreetingCustomer(int NUM_CUSTOMERS, int NUM_SALESMEN){
//	Thread *t;
//	char *name;
//
//	printf ("Initializing Variables for 'Greeting Customer Test'\n");
//
//	initTrolly();
//	initSalesmen();
//	initShelvesWithQty(16);
//
//	printf ("Starting 'Greeting Customer Test'\n");
//
//	//Fork Salesmen Threads
//
//	for(int i = 0; i < NUM_SALESMEN; i++){
//		name = new char [20];
//		sprintf(name,"salesman_thread_%d",i);
//		t = new Thread(name);
//		t->Fork((VoidFunctionPtr)Salesman,i);
//	}
//
//	//Fork Customer Threads
//
//	for(int i = 0; i < NUM_CUSTOMERS; i++){
//		name = new char [20];
//		sprintf(name,"customer_thread_%d",i);
//		t = new Thread(name);
//		t->Fork((VoidFunctionPtr)Customer,i);
//	}
//
//	for(int i = 0; i < numLoaders; i++) {
//		name = new char [20];
//		sprintf(name,"loader_thread_%d",i);
//		t = new Thread(name);
//		t->Fork((VoidFunctionPtr)GoodsLoader,i);
//	}
//}
//
//void testCreatingSalesmenWithDepartments() {
//	cout << "initializing salesmen..." << endl;
//	initSalesmen();
//	cout << "creating salesmen..." << endl;
//	createSalesmen(numDepartments, numSalesmen);
//}
//
//void testCustomerEnteringStoreAndPickingUpItems() {
//	initSalesmen();
//	initShelvesWithQty(10);
//	initLoaders();
//	initCustomerCashier();
//	initTrolly();
//
//	char* name;
//	Thread * t;
//
//	createSalesmen(numDepartments, numSalesmen);
//
//	for(int i = 0; i < custNumber; i++){
//		name = new char [20];
//		sprintf(name,"cust%d",i);
//		t = new Thread(name);
//		t->Fork((VoidFunctionPtr)Customer, i);
//	}
//	for(int i = 0; i < cashierNumber; i++) {
//		name = new char [20];
//		sprintf(name,"cash%d",i);
//		t = new Thread(name);
//		t->Fork((VoidFunctionPtr)cashier, i);
//	}
//	for(int i = 0; i < numLoaders; i++) {
//		name = new char [20];
//		sprintf(name,"loader%d",i);
//		t = new Thread(name);
//		t->Fork((VoidFunctionPtr)GoodsLoader, i);
//	}
//	name = new char [20];
//	sprintf(name,"manager");
//	t = new Thread(name);
//	t->Fork((VoidFunctionPtr)manager, 0);
//
//	cout << "Done creating threads" << endl;
//}
//
//void testCustomerGettingInLine(){
//	testNumber = 1;
//	customerCash = 100000;
//	cashierNumber = 5;
//	custNumber = 10;
//	numSalesmen = 1;
//	numDepartments = 1;
//	numLoaders = 1;
//	numTrollies = 20;
//	initCustomerCashier();
//	char* name;
//	cout << "Test 1: please note that all cashier lines are set to be 5 except for cashier 4's line for unprivileged customers, which is set to 1 ";
//	cout << " and cashier 3's line for unprivileged customer, which is set to 3.";
//	cout << "\tThe privileged status of Customers is set randomly, so we will create enough ";
//	cout << "customer threads to likely fill up a line and start choosing other lines" << endl;
//	initShelves();
//	initShelvesWithQty(30);
//	initSalesmen();
//	initLoaders();
//	initTrolly();
//	for(int i = 0; i < cashierNumber; i++){
//		cashierStatus[i] = CASH_BUSY;
//		privilegedLineCount[i] = 5;
//		unprivilegedLineCount[i] = 5;
//	}
//	unprivilegedLineCount[cashierNumber-1] = 1;
//	unprivilegedLineCount[cashierNumber-2] = 3;
//	Thread * t;
//	name = new char[20];
//	name = "sales0";
//	t = new Thread(name);
//	t->Fork((VoidFunctionPtr)Salesman, 0);
//	for(int i = 0; i < custNumber; i++){
//		name = new char [20];
//		sprintf(name,"cust%d",i);
//		t = new Thread(name);
//		t->Fork((VoidFunctionPtr)Customer, i);
//	}
//}
//
//void testCustLeavesAfterReceiptAndCashierHandlingOneAtATime(){
//	testNumber = 3;
//	customerCash = 10000;
//	cashierNumber = 5;
//	custNumber = 20;
//	numSalesmen = 5;
//	numDepartments = 1;
//	numTrollies = 20;
//	numLoaders = 1;
//	initCustomerCashier();
//	initShelves();
//	initShelvesWithQty(30);
//	initSalesmen();
//	initLoaders();
//	initTrolly();
//	char* name;
//	Thread* t;
//	for(int i = 0; i < cashierNumber; i++){
//		name = new char[20];
//		sprintf(name, "cashier%d", i);
//		t = new Thread(name);
//		t->Fork((VoidFunctionPtr)cashier, i);
//	}
//	for(int i = 0; i < custNumber; i++){
//		name = new char [20];
//		sprintf(name,"cust%d",i);
//		t = new Thread(name);
//		t->Fork((VoidFunctionPtr)Customer, i);
//	}
//	createSalesmen(1, 5);
//}
//
//void testCustomerCheckOutWithoutMoney(){
//	testNumber = 2;
//	customerCash = 0;
//	cashierNumber = 5;
//	custNumber = 5;
//	numSalesmen = 5;
//	numDepartments = 1;
//	numTrollies = 20;
//	numLoaders = 1;
//	initCustomerCashier();
//	initShelves();
//	initShelvesWithQty(30);
//	initSalesmen();
//	initLoaders();
//	initTrolly();
//	char* name;
//	Thread* t;
//	name = new char [20];
//	name = "manager thread";
//	t = new Thread(name);
//	t->Fork((VoidFunctionPtr)manager, 0);
//	for(int i = 0; i < cashierNumber; i++){
//		name = new char[20];
//		sprintf(name, "cashier%d", i);
//		t = new Thread(name);
//		t->Fork((VoidFunctionPtr)cashier, i);
//	}
//	for(int i = 0; i < custNumber; i++){
//		name = new char [20];
//		sprintf(name,"cust%d",i);
//		t = new Thread(name);
//		t->Fork((VoidFunctionPtr)Customer, i);
//	}
//	createSalesmen(1, 5);
//}
//
//void testCashiersScanUntilTrollyIsEmpty(){
//	testNumber = 4;
//	customerCash = 10000;
//	cashierNumber = 1;
//	custNumber = 1;
//	numSalesmen = 1;
//	numDepartments = 1;
//	numTrollies = 20;
//	numLoaders = 1;
//	initCustomerCashier();
//	initShelves();
//	initShelvesWithQty(30);
//	initSalesmen();
//	initLoaders();
//	initTrolly();
//	char* name;
//	Thread* t;
//	name = new char[20];
//	sprintf(name, "cashier%d", 0);
//	t = new Thread(name);
//	t->Fork((VoidFunctionPtr)cashier, 0);
//	name = new char [20];
//	sprintf(name,"cust%d",0);
//	t = new Thread(name);
//	t->Fork((VoidFunctionPtr)Customer, 0);
//	createSalesmen(1, 1);
//}
//
//void testPutCashiersOnBreak(){
//	testNumber = 5;
//	initCustomerCashier();
//	customerCash = 10000;
//	cashierNumber = 5;
//	custNumber = 1;
//	numSalesmen = 1;
//	numDepartments = 1;
//	numTrollies = 20;
//	numLoaders = 1;
//	testNumber = 5;
//	initCustomerCashier();
//	initShelves();
//	initShelvesWithQty(30);
//	initSalesmen();
//	initLoaders();
//	initTrolly();
//	char* name;
//	Thread* t;
//
//	for(int i = 0; i < cashierNumber; i++){
//		name = new char[20];
//		sprintf(name, "cashier%d", i);
//		t = new Thread(name);
//		t->Fork((VoidFunctionPtr)cashier, i);
//	}
//	name = new char[20];
//	name = "manager thread";
//	t = new Thread(name);
//	t->Fork((VoidFunctionPtr)manager, 0);
//
//}
//
//void testBringCashiersBackFromBreak(){
//	testNumber = 6;
//	customerCash = 10000;
//	cashierNumber = 4;
//	custNumber = 30;
//	numSalesmen = 5;
//	numDepartments = 1;
//	numTrollies = 20;
//	numLoaders = 1;
//	initShelvesWithQty(100);
//	initSalesmen();
//	initLoaders();
//	initTrolly();
//	char* name;
//	Thread* t;
//	initCustomerCashier();
//
//	for(int i = 0; i < cashierNumber; i++){
//		//if(i != 1 || i != 3){
//			unprivilegedLineCount[i] = 3;
//			privilegedLineCount[i] = 3;
//
//	}
//
//	name = new char[20];
//	name = "manager thread";
//	t = new Thread(name);
//	t->Fork((VoidFunctionPtr)manager, 0);
//	for(int i = 0; i < cashierNumber; i++){
//		name = new char[20];
//		sprintf(name, "cashier%d", i);
//		cashierStatus[i] = CASH_NOT_BUSY;
//		t = new Thread(name);
//		t->Fork((VoidFunctionPtr)cashier, i);
//	}
//
//
//}
//
//void testRevenueAlwaysTheSame(){
//	testNumber = 7;
//	testCustomerEnteringStoreAndPickingUpItems();
//}
//
//void testGoodsLoadersCustomersDontFightOverShelves(){
//	testNumber = 8;
//	maxShelfQty = 5;
//	customerCash = 10000;
//	cashierNumber = 5;
//	custNumber = 10;
//	numSalesmen = 1;
//	numDepartments = 1;
//	numItems = 1;
//	numTrollies = 1;
//	numLoaders = 1;
//	initCustomerCashier();
//	initShelves();
//	initShelvesWithQty(0);
//	initSalesmen();
//	initLoaders();
//	initTrolly();
//	char* name;
//	Thread* t;
//	initCustomerCashier();
//	createSalesmen(numDepartments, numSalesmen);
//	for(int i = 0; i < custNumber; i++){
//		name = new char [20];
//		sprintf(name,"cust%d",i);
//		t = new Thread(name);
//		t->Fork((VoidFunctionPtr)Customer, i);
//	}
//	for(int i = 0; i < numLoaders; i++) {
//		name = new char [20];
//		sprintf(name,"loader%d",i);
//		t = new Thread(name);
//		t->Fork((VoidFunctionPtr)GoodsLoader, i);
//	}
//	name = new char[20];
//	name = "manager thread";
//	t = new Thread(name);
//	t->Fork((VoidFunctionPtr)manager, 0);
//	for(int i = 0; i < cashierNumber; i++){
//		name = new char[20];
//		sprintf(name, "cashier%d", i);
//		t = new Thread(name);
//		t->Fork((VoidFunctionPtr)cashier, i);
//	}
//
//}
//
//void testOneLoaderInStockRoom(){
//	testNumber = 9;
//	maxShelfQty = 100;
//	customerCash = 10000;
//	cashierNumber = 5;
//	custNumber = 4;
//	numSalesmen = 4;
//	numDepartments = 1;
//	numTrollies = 30;
//	numLoaders = 5;
//	initCustomerCashier();
//	initShelves();
//	initShelvesWithQty(0);
//	initSalesmen();
//	initLoaders();
//	initTrolly();
//	char* name;
//	Thread* t;
//	initCustomerCashier();
//	createSalesmen(numDepartments, numSalesmen);
//	for(int i = 0; i < custNumber; i++){
//		name = new char [20];
//		sprintf(name,"cust%d",i);
//		t = new Thread(name);
//		t->Fork((VoidFunctionPtr)Customer, i);
//	}
//	for(int i = 0; i < numLoaders; i++) {
//		name = new char [20];
//		sprintf(name,"loader%d",i);
//		t = new Thread(name);
//		t->Fork((VoidFunctionPtr)GoodsLoader, i);
//	}
//	name = new char[20];
//	name = "manager thread";
//	t = new Thread(name);
//	t->Fork((VoidFunctionPtr)manager, 0);
//	for(int i = 0; i < cashierNumber; i++){
//		name = new char[20];
//		sprintf(name, "cashier%d", i);
//		t = new Thread(name);
//		t->Fork((VoidFunctionPtr)cashier, i);
//	}
//}
//
//void testCustWaitForRestock(){
//	testNumber = 10;
//	testNumber = 10;
//	maxShelfQty = 3;
//	customerCash = 10000;
//	cashierNumber = 1;
//	custNumber = 1;
//	numSalesmen = 1;
//	numDepartments = 1;
//	numTrollies = 1;
//	numLoaders = 1;
//	numItems = 1;
//	initCustomerCashier();
//	initShelves();
//	initShelvesWithQty(0);
//	initSalesmen();
//	initLoaders();
//	initTrolly();
//	char* name;
//	Thread* t;
//	initCustomerCashier();
//	createSalesmen(numDepartments, numSalesmen);
//	for(int i = 0; i < custNumber; i++){
//		name = new char [20];
//		sprintf(name,"cust%d",i);
//		t = new Thread(name);
//		t->Fork((VoidFunctionPtr)Customer, i);
//	}
//	for(int i = 0; i < numLoaders; i++) {
//		name = new char [20];
//		sprintf(name,"loader%d",i);
//		t = new Thread(name);
//		t->Fork((VoidFunctionPtr)GoodsLoader, i);
//	}
//	name = new char[20];
//	name = "manager thread";
//	t = new Thread(name);
//	t->Fork((VoidFunctionPtr)manager, 0);
//	for(int i = 0; i < cashierNumber; i++){
//		name = new char[20];
//		sprintf(name, "cashier%d", i);
//		t = new Thread(name);
//		t->Fork((VoidFunctionPtr)cashier, i);
//	}
//}
//
//void printInitialConditions() {
//	cout << "Test " << testNumber << " beginning with initial conditions:" << endl;
//	cout << "Number of Cashiers = [" << cashierNumber << "]" << endl;
//	cout << "Number of Goods Loaders = [" << numLoaders << "]" << endl;
//	cout << "Number of PrivilegedCustomers = [" << "Randomly generated, 30% chance" << "]" << endl;
//	cout << "Number of Customers = [" << custNumber << "]" << endl;
//	cout << "Number of Managers = [" << "1" << "]" << endl;
//	cout << "Number of DepartmentSalesmen = [" << numSalesmen << "]" << endl;
//
//    cout << "Items:" << endl;
//    cout << "  Number - Price" << endl;
//    for(int i = 0; i < numItems; i++) {
//    	cout << "  " << i << " - " << scan(i) << endl;
//    }
//    cout << endl;
//
//}
//*/
void Problem2(){
}
  /*
	cout << "Menu:" << endl;
	cout << "1. Test customer choosing from cashier lines" << endl;
	cout << "2. Test manager only interacting with one customer or cashier at a time." << endl;
	cout << "3. Test cashier receipt reception and cashier waiting for cusotmer to leave" << endl;
	cout << "4. Test cashiers scan all items" <<endl;
	cout << "5. Test cashiers being sent on break" << endl;
	cout << "6. Test cashiers being brough back from break" << endl;
	cout << "7. Test sales never suffering a race condition" << endl;
	cout << "8. Test Goods Loaders don't restock when a Customer is trying to get an item" << endl;
	cout << "9. Test only one Goods Loader enters the stock room at a time" << endl;
	cout << "10. Test Customrs wait for items to be restocked" << endl;
	cout << "11. Run full simulation" << endl;
	cout << "12. Run full simulation with some predetermined values" << endl;
	// put your necessary menu options here
	cout << "Please input the number option you wish to take: " << endl;
	int choice = 12;


  	while(true){
		cin >> choice;
		if(cin.fail()){
			cin.clear();
			cin.ignore(100, '\n');
			cout << "Not a valid menu option. Please try again: ";
			continue;
		}
		else if(choice > 12 || choice < 1){ //change this if you add more options
			cout << "Not a valid menu option. Please try again: ";
			continue;
		}
		else break;
	}

	switch (choice){
	case 1:
		printInitialConditions();
		testCustomerGettingInLine();
		break;
	case 2:
		printInitialConditions();
		testCustomerCheckOutWithoutMoney();
		break;
	case 3:
		customerCash = 100000;
		cashierNumber = 4;
		custNumber = 30;
		printInitialConditions();
		testCustLeavesAfterReceiptAndCashierHandlingOneAtATime();
		break;
	case 4:
		printInitialConditions();
		testCashiersScanUntilTrollyIsEmpty();
		break;
	case 5:
		printInitialConditions();
		testPutCashiersOnBreak();
		break;
	case 6:
		printInitialConditions();
		testBringCashiersBackFromBreak();
		break;
	case 7:
		printInitialConditions();
		testRevenueAlwaysTheSame();
		break;
	case 8:
		printInitialConditions();
		testGoodsLoadersCustomersDontFightOverShelves();
		break;
	case 9:
		printInitialConditions();
		testOneLoaderInStockRoom();
		break;
	case 10:
		printInitialConditions();
		testCustWaitForRestock();
		break;
	case 11:
		custNumber = 0;
		customerCash = 0;
		numTrollies = 0;
		numSalesmen = 0;
		numDepartments = 0;
		cashierNumber = 0;
		numLoaders = 0;

		while (custNumber < 1){
			cout << "Please input the number of customers you would like (at least 1)" <<endl;
			cin >> custNumber;
		}
		while(numTrollies < 1){
			cout << "Please input the number of trollies you would like (must be at least than 1)" << endl;
			cin >> numTrollies;
		}
		while(numDepartments < 1 || numDepartments > 5){
			cout << "Please input the number of departments you would like (between 1 and 5)" << endl;
			cin >> numDepartments;
		}
		while(numSalesmen < 1 || numSalesmen > 3){
			cout << "Please input the number of salesmen per department you would like (between 1 and 3)" << endl;
			cin >> numSalesmen;
		}
		while(cashierNumber < 1 || cashierNumber > 5){
			cout << "Please input the number of cashiers (between 1 and 5)" << endl;
			cin >> cashierNumber;
		}
		while(numLoaders < 1 || numLoaders > 5){
			cout << "Please input the number of GoodsLoaders (between 1 and 5" << endl;
			cin >> numLoaders;
		}

		cout << custNumber << endl;
		cout << numTrollies << endl;
		cout << numDepartments << endl;
		cout << numSalesmen << endl;
		cout << cashierNumber << endl;
		cout << numLoaders << endl;

		printInitialConditions();
		testCustomerEnteringStoreAndPickingUpItems();
		break;

	case 12:
		testNumber = 12;
		custNumber = 30;
		customerCash = 100;
		numTrollies = 20;
		numDepartments = 3;
		numSalesmen = 3;
		cashierNumber = 3;
		numLoaders = 3;

		printInitialConditions();
		testCustomerEnteringStoreAndPickingUpItems();
		break;

		//add cases here for your test
	default: break;
	}
}
*/
