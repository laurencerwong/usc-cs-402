// threadtest.cc 
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
#include "time.h"
#include <queue>

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

int maxShelfQty = 20;
int numTrollies = 40;
int numSalesmen = 3;
int numLoaders = 10;
int numItems = 10;
int numDepartments = 3;
int maxCustomers = 100;

// Enums for the salesmen status

enum SalesmanStatus {SALES_NOT_BUSY, SALES_BUSY, SALES_GO_ON_BREAK, SALES_ON_BREAK, SALES_READY_TO_TALK, SALES_SIGNALLING_LOADER, LOAD_GET_ITEMS_FROM_MANAGER};
SalesmanStatus *currentSalesStatus = new SalesmanStatus[numSalesmen];

enum WhomIWantToTalkTo {GREETING, COMPLAINING, GOODSLOADER, SALESMAN, MANAGER, UNKNOWN};
WhomIWantToTalkTo *currentlyTalkingTo = new WhomIWantToTalkTo[numSalesmen];
WhomIWantToTalkTo *loaderCurrentlyTalkingTo = new WhomIWantToTalkTo[numLoaders];

int *salesCustNumber = new int[numSalesmen]; //The array of customer indicies that the customers update
int *salesDesk = new int[numSalesmen];
int *salesBreakBoard = new int[numSalesmen];
Condition **salesBreakCV = new Condition*[numSalesmen];

int greetingCustWaitingLineCount = 0; //How many customers need to be greeted
int complainingCustWaitingLineCount = 0; //How many customers need to be helped with empty shelves
int loaderWaitingLineCount = 0;	//How many loaders are waiting on salesmen

Lock **individualSalesmanLock = new Lock*[numSalesmen]; //The lock that each salesman uses for his own "desk"
Lock *salesLock = new Lock("Overall sales lock"); //The lock that protects the salesmen statuses and the line for all three

Condition **salesmanCV = new Condition*[numSalesmen]; //The condition variable for one on one interactions with the Salesman
Condition *greetingCustCV = new Condition ("Greeting Customer Condition Variable"); //The condition var that represents the line all the greeters stay in
Condition *complainingCustCV = new Condition("Complaining Customer Condition Variable"); //Represents the line that the complainers stay in
Condition *loaderCV = new Condition("GoodsLoader Condition Variable");	//Represents the line that loaders wait in

enum LoaderStatus {LOAD_NOT_BUSY, LOAD_STOCKING, LOAD_HAS_BEEN_SIGNALLED};
LoaderStatus *loaderStatus = new LoaderStatus[numLoaders];

Lock *inactiveLoaderLock = new Lock("Lock for loaders waiting to be called on");
Condition *inactiveLoaderCV = new Condition("CV for loaders waiting to be called on");

Lock **shelfLock = new Lock*[numItems];
Condition **shelfCV = new Condition*[numItems];
Lock *stockRoomLock = new Lock("Stock room lock");

int *shelfInventory = new int[numItems];


//will be grabbed when a privileged customer checks line lengths/cashier status
Lock* cashierLinesLock;


//array of CVs for privileged customers waiting in line
//one for each cashier
Condition** privilegedCashierLineCV;

//array of CVs for unprivileged customers waiting in line
//one for each cashier
Condition** unprivilegedCashierLineCV;

//Lock associated with cashToCustCV
//controls access to each cashier
Lock** cashierLock;
//array of CVs, one for each cashier, that the cashier and
//customer use to communicate during checkout
Condition** cashierToCustCV;

enum CashierStatus {CASH_NOT_BUSY, CASH_BUSY, CASH_ON_BREAK, CASH_GO_ON_BREAK, CASH_READY_TO_TALK};
//array of current cashier statuses
CashierStatus* cashierStatus;

//array of privileged line counts
int* privilegedLineCount;

//array of unprivileged line counts
int* unprivilegedLineCount;

//number of cashiers
int cashierNumber = 5;

//number of customers
int custNumber = 20;

//array of ints representing the item a customer has handed to a cashier
int* cashierDesk;

//array representing money in each cash register
int* cashRegister;

//temporary global variable
int customerCash = 20;
//Manager data
int* cashierFlags; //will be set to a customer ID in the slot corresponding to the
//index of the cashier who sets the flag.

Lock* managerLock; //will be the lock behind the manager CV
Condition* managerCV; //will be the CV the manager and customer use to communicate
int managerDesk; //will be place customer puts items to show to manager

//Trolleys
int trollyCount = numTrollies;

//Lock controlling access to trollyCount;
Lock* trollyLock;

//CV for customers to wait on if no trollies are there
//associated with trollyLock
Condition* trollyCV;

//Discarded trollies, waiting to be replaced by Goods Loader
int displacedTrollyCount;

//Lock controlling access to displaced trolly count
Lock* displacedTrollyLock;

//manager stores items taken from customer so they can be taken by Loader
queue<int> managerItems;
//lock to protect managerItems;
Lock* managerItemsLock;

//which goodsloader is currently in the stockroom
int currentLoaderInStock = -1;

//Lock for currentLockInStock
Lock *currentLoaderInStockLock = new Lock("cur loader in stock lock");

//Which customers are privileged or not
int *privCustomers = new int[maxCustomers];
int customersDone;

void initShelves() {
	for(int i = 0; i < numItems; i++) {
		shelfLock[i] = new Lock("shelfLock");
		shelfCV[i] = new Condition("shelfCV");
		shelfInventory[i] = 0;
	}
}

void initShelvesWithQty(int q) {
	for(int i = 0; i < numItems; i++) {
		shelfLock[i] = new Lock("shelfLock");
		shelfCV[i] = new Condition("shelfCV");
		shelfInventory[i] = q;
	}
}

void initSalesmen(){
	for(int i = 0; i < numSalesmen; i++){
		currentSalesStatus[i] = SALES_BUSY;
		currentlyTalkingTo[i] = UNKNOWN;
		salesBreakBoard[i] = 0;
		individualSalesmanLock[i] = new Lock(("Salesman Lock"));
		salesmanCV[i] = new Condition(("Salesman Control Variable"));
		salesCustNumber[i] = 0;
		char* name = new char[20];
		sprintf(name, "Sales break CV %d", i);
		salesBreakCV[i] = new Condition(name);
	}
}

void initLoaders() {
	for(int i = 0; i < numLoaders; i++){
		loaderStatus[i] = LOAD_NOT_BUSY;
	}
}

void initTrolly() {
	char* name;

	trollyCount = numTrollies;
	displacedTrollyCount = 0;
	name = new char[20];
	name = "trolly lock";
	trollyLock = new Lock(name);
	name = new char[20];
	name = "trolly CV";
	trollyCV = new Condition(name);
	name = new char[20];
	name = "displaced trolly lock";
	displacedTrollyLock = new Lock(name);
}

void initCustomerCashier(){
	for(int i = 0; i < maxCustomers; i++){
		privCustomers[i] = 0; //sets all the customers to unprivileged
	}
	char* name;
	name = new char[20];
	name = "cashier line lock";
	customersDone =0;
	cashierLinesLock = new Lock(name);
	name = new char[20];
	name = "manaager items lock";
	managerItemsLock = new Lock(name);
	inactiveLoaderCV = new Condition(name);
	inactiveLoaderLock = new Lock(name);
	privilegedCashierLineCV = new Condition*[cashierNumber];
	unprivilegedCashierLineCV = new Condition*[cashierNumber];
	cashierLock = new Lock*[cashierNumber];
	cashierToCustCV = new Condition*[cashierNumber];
	cashierStatus = new CashierStatus[cashierNumber];
	privilegedLineCount = new int[cashierNumber];
	unprivilegedLineCount = new int[cashierNumber];
	cashierDesk = new int[cashierNumber];
	cashierFlags = new int[cashierNumber];
	cashRegister = new int[cashierNumber];

	displacedTrollyCount = 0;
	name = new char[20];
	name = "trolly lock";
	trollyLock = new Lock(name);
	name = new char[20];
	name = "trolly CV";
	trollyCV = new Condition(name);
	name = new char[20];
	name = "displaced trolly lock";
	displacedTrollyLock = new Lock(name);

	initTrolly();

	for(int i = 0; i < cashierNumber; i++){
		name = new char [20];
		sprintf(name,"priv cashier line CV %d",i);
		privilegedCashierLineCV[i] = new Condition(name);
	}
	for(int i = 0; i < cashierNumber; i++){
		name = new char[20];
		sprintf(name, "unpriv cashier line CV %d", i);
		unprivilegedCashierLineCV[i] = new Condition(name);
	}
	for(int i = 0; i < cashierNumber; i++){
		name = new char[20];
		sprintf(name, "cashier lock %d", i);
		cashierLock[i] = new Lock(name);
	}
	for(int i = 0; i < cashierNumber; i++){
		name = new char[20];
		sprintf(name, "cashier cust CV %d", i);
		cashierToCustCV[i] = new Condition(name);
	}
	for(int i = 0; i < cashierNumber; i++){
		cashierStatus[i] = CASH_BUSY;
		privilegedLineCount[i] = 0;
		unprivilegedLineCount[i] = 0;
		cashierFlags[i] = -1;
		cashierDesk[i] = -2;
		cashRegister[i] = 0;
	}

	name = new char[20];
	name = "manager lock";
	managerLock = new Lock(name);
	name = new char[20];
	name = "manager CV";
	managerCV = new Condition(name);
}

int getDepartmentFromItem(int itemNum) {
	return itemNum % numDepartments;
}

void CustomerP1(int myIndex) {
	//choose the items we want to buy
	int numItemsToBuy = 3;	//TODO actually decide how many items and which to buy
	int *itemsToBuy;
	int *qtyItemsToBuy;
	int *itemsInCart;
	int *qtyItemsInCart;
	itemsToBuy = new int[numItemsToBuy];
	qtyItemsToBuy = new int[numItemsToBuy];
	itemsInCart = new int[numItemsToBuy];
	qtyItemsInCart = new int[numItemsToBuy];

	for(int i = 0; i < numItemsToBuy; i++) {
		//TODO- Put a random item/qty picker here
		itemsToBuy[i] = i;
		qtyItemsToBuy[i] = 2;
		itemsInCart[i] = -1;
		qtyItemsInCart[i] = 0;
	}

	//ENTERS STORE

	cout << "Customer "<< myIndex <<" enters the SuperMarket" << endl;
	cout << "Customer " << myIndex << " wants to buy " << numItemsToBuy << " items" << endl;

	/*
	cout << "Customer " << myIndex << " has grocery list:" << endl;
	cout << "Wanted  qty     Has  qty" << endl;
	for(int k = 0; k < numItemsToBuy; k++) {
		cout << itemsToBuy[k] << "       " << qtyItemsToBuy[k] << "       " <<
				itemsInCart[k] << "    " << qtyItemsInCart[k] << endl;
	}
	cout << "end cust list ---" << endl;
	 */

	trollyLock->Acquire();
	if(trollyCount == 0) {
		trollyCV->Wait(trollyLock);
	}
	trollyCount--;
	trollyLock->Release();

	salesLock->Acquire();
	int mySalesIndex = -1;

	//printf("about to select salesman, greeting line is: %d\n", greetingCustWaitingLineCount);

	//Selects a salesman
	if(greetingCustWaitingLineCount > 0){	//there is a line
		greetingCustWaitingLineCount++;
		greetingCustCV->Wait(salesLock);

		for(int i = 0; i < numSalesmen; i++){
			if(currentSalesStatus[i] == SALES_READY_TO_TALK){
				mySalesIndex = i;
				currentSalesStatus[i] = SALES_BUSY;
				currentlyTalkingTo[i] = GREETING;
				break;
			}
		}
	}
	else {	//there is no line
		//printf("There was no line for cust %d\n", myIndex);
		for(int i = 0; i < numSalesmen; i++){
			if(currentSalesStatus[i] == SALES_NOT_BUSY){
				mySalesIndex = i;
				currentSalesStatus[i] = SALES_BUSY;
				currentlyTalkingTo[i] = GREETING;
				break;
			}
		}
		if(mySalesIndex == -1) {	//no line, but all are busy
			//printf("But all sales were busy... getting in line\n");
			greetingCustWaitingLineCount++;
			greetingCustCV->Wait(salesLock);

			for(int i = 0; i < numSalesmen; i++){
				if(currentSalesStatus[i] == SALES_READY_TO_TALK){
					mySalesIndex = i;
					currentSalesStatus[i] = SALES_BUSY;
					currentlyTalkingTo[i] = GREETING;
					break;
				}
			}
		}
	}

	//printf("about to  get desk lock\n");

	individualSalesmanLock[mySalesIndex]->Acquire(); //Acquire the salesman's "desk" lock
	salesLock->Release();
	salesCustNumber[mySalesIndex] = myIndex; //Sets the customer number of the salesman to this customer's index
	salesmanCV[mySalesIndex]->Signal(individualSalesmanLock[mySalesIndex]);
	salesmanCV[mySalesIndex]->Wait (individualSalesmanLock[mySalesIndex]);
	individualSalesmanLock[mySalesIndex]->Release();

	//BEGINS SHOPPING

	for(int i = 0; i < numItemsToBuy; i++) {	//goes through everything on our grocery list
		for(int shelfNum = 0; shelfNum < numItems; shelfNum++) {
			if(shelfNum != itemsToBuy[i]) {
				continue;
			}

			cout << "Customer " << myIndex << " wants to buy " << qtyItemsToBuy[i] << " of item " << shelfNum << endl;
			while(qtyItemsInCart[i] < qtyItemsToBuy[i]) {
				shelfLock[shelfNum]->Acquire();
				if(shelfInventory[shelfNum] > qtyItemsToBuy[i]) {
					cout << "Customer " << myIndex << " has found item "
							<< shelfNum << " and placed " << qtyItemsToBuy[i] << " in the trolly" << endl;


					shelfInventory[shelfNum] -= qtyItemsToBuy[i];
					itemsInCart[i] = shelfNum;
					qtyItemsInCart[i] += qtyItemsToBuy[i];
					shelfLock[shelfNum]->Release();
					//cout << "Think I need: " << qtyItemsToBuy[i] << "  Got: " << qtyItemsInCart[i] << endl;
				}
				else {	//We are out of this item, go tell sales!
					cout << "Customer " << myIndex << " was not able to find item " << shelfNum <<
							" and is searching for department salesman " << "DEPT NUM" << endl;	//TODO dept num
					shelfLock[shelfNum]->Release();
					salesLock->Acquire();

					int mySalesID = -1;

					for(int j = 0; j < numSalesmen; j++) {
						//nobody waiting, sales free
						if(complainingCustWaitingLineCount == 0 && currentSalesStatus[j] == SALES_NOT_BUSY) {
							mySalesID = j;

							break;
						}
					}
					if(mySalesID == -1) {	//no salesmen are free, I have to wait in line
						cout << "Customer " << myIndex << " gets in line for a salesman in department " << "DNUM" << endl;
						complainingCustWaitingLineCount++;
						complainingCustCV->Wait(salesLock);

						//find the salesman who just signalled me

						for(int k = 0; k < numSalesmen; k++) {
							if(currentSalesStatus[k] == SALES_READY_TO_TALK) {
								mySalesID = k;
								break;
							}
						}
					}

					individualSalesmanLock[mySalesID]->Acquire();
					cout << "Customer " << myIndex << " is asking for assistance "
							"from DepartmentSalesman " << mySalesID << endl;
					currentSalesStatus[mySalesID] = SALES_BUSY;
					salesCustNumber[mySalesID] = myIndex;	//
					salesLock->Release();

					//now proceed with interaction to tell sales we are out
					currentlyTalkingTo[mySalesID] = COMPLAINING;
					salesDesk[mySalesID] = shelfNum;

					salesmanCV[mySalesID]->Signal(individualSalesmanLock[mySalesID]);
					salesmanCV[mySalesID]->Wait(individualSalesmanLock[mySalesID]);
					individualSalesmanLock[mySalesID]->Release();
					//now i go wait on the shelf
					cout << "Customer " << myIndex << " is now waiting for item " <<
							shelfNum << " to be restocked" << endl;

					shelfLock[shelfNum]->Acquire();
					shelfCV[shelfNum]->Wait(shelfLock[shelfNum]);
					//now restocked, continue looping until I have all of what I need
					cout << "Customer " << myIndex << " has received assistance about restocking of item " <<
							shelfNum << " from DepartmentSalesman " << mySalesID << endl;
				}
			}	//end while loop to get enough of a given item
		}	//end looking through shelves
	}	//end going through grocery list

	cout << "Customer " << myIndex << " is done getting items:" << endl;
	cout << "Wanted  qty     Has  qty" << endl;
	for(int k = 0; k < numItemsToBuy; k++) {
		cout << itemsToBuy[k] << "       " << qtyItemsToBuy[k] << "       " <<
				itemsInCart[k] << "    " << qtyItemsInCart[k] << endl;
	}
	cout << "end cust list ---" << endl;

	trollyLock->Acquire();
	displacedTrollyCount++;
	trollyLock->Release();

	//cleanup
	delete itemsToBuy;
	delete qtyItemsToBuy;
	delete itemsInCart;
	delete qtyItemsInCart;
}



//Customer's function		//__CUST__
void Customer(int myID){

	//choose the items we want to buy
	int numItemsToBuy = 3;	//TODO actually decide how many items and which to buy
	int *itemsToBuy;
	int *qtyItemsToBuy;
	int *itemsInCart;
	int *qtyItemsInCart;
	itemsToBuy = new int[numItemsToBuy];
	qtyItemsToBuy = new int[numItemsToBuy];
	itemsInCart = new int[numItemsToBuy];
	qtyItemsInCart = new int[numItemsToBuy];

	//---------Randomly generate whether this customer is privileged--------------
	char* type = new char[20];
	int privileged;
	srand(myID + time(NULL));
	int r = rand() % 10; //random value to set Customer either as privileged or unprivileged
	if(r < 2){//30% chance customer is privileged
		privileged = 1;
	}
	else privileged = 0; //70% chance this customer is unprivileged

	//set char array for I/O purposes
	if(privileged){
		type = "Privileged Customer";
	}
	else type = "Customer";
	privileged = 0;
	//--------------End of privileged/unprivileged decion--------------------------


	for(int i = 0; i < numItemsToBuy; i++) {
		//TODO- Put a random item/qty picker here
		itemsToBuy[i] = i;
		qtyItemsToBuy[i] = 2;
		itemsInCart[i] = -1;
		qtyItemsInCart[i] = 0;
	}

	//ENTERS STORE

	cout << type << " [" << myID << "] enters the SuperMarket" << endl;
	cout << type << " [" << myID << "] wants to buy [" << numItemsToBuy << "] no.of items" << endl;

	/*
	cout << "Customer " << myIndex << " has grocery list:" << endl;
	cout << "Wanted  qty     Has  qty" << endl;
	for(int k = 0; k < numItemsToBuy; k++) {
		cout << itemosToBuy[k] << "       " << qtyItemsToBuy[k] << "       " <<
				itemsInCart[k] << "    " << qtyItemsInCart[k] << endl;
	}
	cout << "end cust list ---" << endl;
	*/

	trollyLock->Acquire();
	//cout << "there are: " << trollyCount << " trollies" << endl;
	while(trollyCount == 0) {
		cout << type << " [" << myID << "] gets in line for a trolly" << endl;
		trollyCV->Wait(trollyLock);
	}
	trollyCount--;
	cout << type << " [" << myID << "] has a trolly for shopping" << endl;
	trollyLock->Release();

	salesLock->Acquire();
	int mySalesIndex = -1;

	//printf("cust %d is about to select salesman, greeting line is: %d\n", myID,  greetingCustWaitingLineCount);

	//Selects a salesman
	if(greetingCustWaitingLineCount > 0){	//there is a line
		greetingCustWaitingLineCount++;
		cout << type << " [" << myID << "] gets in line for the Department [" << "dept#" << "]" << endl;
		greetingCustCV->Wait(salesLock);

		for(int i = 0; i < numSalesmen; i++){
			if(currentSalesStatus[i] == SALES_READY_TO_TALK){
				mySalesIndex = i;
				currentSalesStatus[i] = SALES_BUSY;
				currentlyTalkingTo[i] = GREETING;
				break;
			}
		}
	}
	else {	//there is no line
		//printf("There was no line for cust %d\n", myID);
		for(int i = 0; i < numSalesmen; i++){
			if(currentSalesStatus[i] == SALES_NOT_BUSY){
				mySalesIndex = i;
				currentSalesStatus[i] = SALES_BUSY;
				currentlyTalkingTo[i] = GREETING;
				break;
			}
		}
		if(mySalesIndex == -1) {	//no line, but all are busy
			//printf("But all sales were busy... getting in line\n");
			greetingCustWaitingLineCount++;
			greetingCustCV->Wait(salesLock);

			for(int i = 0; i < numSalesmen; i++){
				if(currentSalesStatus[i] == SALES_READY_TO_TALK){
					mySalesIndex = i;
					currentSalesStatus[i] = SALES_BUSY;
					currentlyTalkingTo[i] = GREETING;
					break;
				}
			}
		}
	}

	//cout << "cust " << myID << " about to get desk lock" << endl;

	individualSalesmanLock[mySalesIndex]->Acquire(); //Acquire the salesman's "desk" lock
	cout << type << " [" << myID << "] is interacting with DepartmentSalesman[" << mySalesIndex << "] of Department[" << "dept#" << "]" << endl;
	salesLock->Release();
	salesCustNumber[mySalesIndex] = myID; //Sets the customer number of the salesman to this customer's index

	/*for(int j = 0; j < numSalesmen; j++) {
		cout << "sales status " << currentSalesStatus[j] << "  sales talk " << currentlyTalkingTo[j] << endl;
	}*/

	salesmanCV[mySalesIndex]->Signal(individualSalesmanLock[mySalesIndex]);
	salesmanCV[mySalesIndex]->Wait (individualSalesmanLock[mySalesIndex]);
	individualSalesmanLock[mySalesIndex]->Release();

	//BEGINS SHOPPING

	for(int i = 0; i < numItemsToBuy; i++) {	//goes through everything on our grocery list
		for(int shelfNum = 0; shelfNum < numItems; shelfNum++) {
			if(shelfNum != itemsToBuy[i]) {
				continue;
			}

			cout << type << " ["<< myID << "] wants to buy [" << shelfNum << "]-[" << qtyItemsToBuy[i] << "]." << endl;
			while(qtyItemsInCart[i] < qtyItemsToBuy[i]) {
				shelfLock[shelfNum]->Acquire();
				if(shelfInventory[shelfNum] > qtyItemsToBuy[i]) {
					cout << type << " ["<< myID << "] has found ["
						 << shelfNum << "] and placed [" << qtyItemsToBuy[i] << "] in the trolly" << endl;


					shelfInventory[shelfNum] -= qtyItemsToBuy[i];
					itemsInCart[i] = shelfNum;
					qtyItemsInCart[i] += qtyItemsToBuy[i];
					shelfLock[shelfNum]->Release();
					//cout << "Think I need: " << qtyItemsToBuy[i] << "  Got: " << qtyItemsInCart[i] << endl;
				}
				else {	//We are out of this item, go tell sales!
					cout << type << " [" << myID << "] was not able to find item " << shelfNum <<
							" and is searching for department salesman " << "DEPT NUM" << endl;	//TODO dept num
					shelfLock[shelfNum]->Release();
					salesLock->Acquire();

					int mySalesID = -1;

					for(int j = 0; j < numSalesmen; j++) {
						//nobody waiting, sales free
						if(complainingCustWaitingLineCount == 0 && currentSalesStatus[j] == SALES_NOT_BUSY) {
							mySalesID = j;

							break;
						}
					}
					if(mySalesID == -1) {	//no salesmen are free, I have to wait in line
//						cout << type << " ["<< myID << "] gets in line for a salesman in department " << "dept#" << endl;
						complainingCustWaitingLineCount++;
						complainingCustCV->Wait(salesLock);

						//find the salesman who just signalled me

						for(int k = 0; k < numSalesmen; k++) {
							if(currentSalesStatus[k] == SALES_READY_TO_TALK) {
								mySalesID = k;
								break;
							}
						}
					}

					individualSalesmanLock[mySalesID]->Acquire();
					cout << type << " [" << myID << "] is asking for assistance "
							"from DepartmentSalesman [" << mySalesID << "]" << endl;
					currentSalesStatus[mySalesID] = SALES_BUSY;
					salesCustNumber[mySalesID] = myID;	//
					salesLock->Release();

					//now proceed with interaction to tell sales we are out
					currentlyTalkingTo[mySalesID] = COMPLAINING;
					salesDesk[mySalesID] = shelfNum;

					salesmanCV[mySalesID]->Signal(individualSalesmanLock[mySalesID]);
					salesmanCV[mySalesID]->Wait(individualSalesmanLock[mySalesID]);
					individualSalesmanLock[mySalesID]->Release();
					//now i go wait on the shelf
				/*	cout << type << " [" <<  myID << "] is now waiting for item [" <<
							shelfNum << "] to be restocked" << endl;
							*/
					shelfLock[shelfNum]->Acquire();
					shelfCV[shelfNum]->Wait(shelfLock[shelfNum]);
					cout << "DepartmentSalesman [" << mySalesID << "] informs the " << type << " [" << myID << "] that [" << shelfNum << "] is restocked." << endl;
					//now restocked, continue looping until I have all of what I need
					cout << type << " [" <<  myID << "] has received assistance about restocking of item [" <<
							shelfNum << "] from DepartmentSalesman [" << mySalesID << "]" << endl;
				}
			}	//end while loop to get enough of a given item
		}	//end looking through shelves
	}	//end going through grocery list

	/*cout << type << " [" << myID << "] is done getting items:" << endl;
	cout << "Wanted  qty     Has  qty" << endl;
	for(int k = 0; k < numItemsToBuy; k++) {
		cout << itemsToBuy[k] << "       " << qtyItemsToBuy[k] << "       " <<
				itemsInCart[k] << "    " << qtyItemsInCart[k] << endl;
	}
	cout << "end cust list ---" << endl;*/

	
	//========================================================
	
	
	int myCashier; //ID of cashier I speak to
	int myCash = customerCash;
	
	//int *itemsInCart;
	//int *qtyItemsInCart;
	//int numItemsToBuy = 3;
	//itemsInCart = new int[numItemsToBuy];
	//qtyItemsInCart = new int[numItemsToBuy];
	/*for(int i = 0; i < numItemsToBuy; i++) {
		itemsInCart[i] = i;
		qtyItemsInCart[i] = 3 - i;
	}*/

	//--------------Begin looking for a cashier-------------------------------------
	cashierLinesLock->Acquire(); //acquire locks to view line counts and cashier statuses
	do{
	printf("%s [%d] is looking for the Cashier.\n", type, myID );

	//Find if a cashier is free (if one is, customer doesn't need to wait in line)
	for(int i = 0; i < cashierNumber; i++ ){
		//if I find a cashier who is free, I will:
		if(cashierStatus[i] == CASH_NOT_BUSY){
			myCashier = i; //remember who he is
			cashierLock[i]->Acquire(); //get his lock before I wake him up
			cashierStatus[i] = CASH_BUSY; //prevent others from thinking he's free
			printf("%s [%d] chose Cashier [%d] with line of length [0].\n", type, myID, myCashier);
			break; //stop searching through lines
		}

		//---------------Find shortest line---------------------------
		else if (i == cashierNumber - 1){
			int minLineValue;
			int minCashierID = 0;
			int* linesIAmLookingAt; //temporary pointer to line counts so the code for
					//the two types of customer can be identical
			Condition **linesIAmLookingAtCV;//temporary pointer to the CVs, again for
					//the purpose of having same code for both types of customers

			if(privileged){
				linesIAmLookingAt = privilegedLineCount;
				linesIAmLookingAtCV = privilegedCashierLineCV;
			}
			else{
				linesIAmLookingAt = unprivilegedLineCount;
				linesIAmLookingAtCV = unprivilegedCashierLineCV;
			}

			//from here on, privilegedCustomers and unprivileged customers execute same code because
			//of the temporary variables linesIAmLookingAt (which is unprivilegedLineCount or privilegedLineCount)
			//and linesIAmLookingAtCV (which is un/privilegedCashierLineCV)

			minLineValue = custNumber; //set a default min value
			//find the minimum line value and remember the cashier associated with it
			for(int j = 0; j < cashierNumber; j++){
				if(linesIAmLookingAt[j] < minLineValue && cashierStatus[j] != CASH_ON_BREAK && cashierStatus[j] != 	CASH_GO_ON_BREAK){
					//must also check if that cashier is on break
					minLineValue = linesIAmLookingAt[j];
					minCashierID = j;
				}
			}
			myCashier = minCashierID;
			printf("%s [%d] chose Cashier [%d] of line length [%d].\n", type, myID, myCashier, linesIAmLookingAt[minCashierID]);
			linesIAmLookingAt[minCashierID]++;
			linesIAmLookingAtCV[minCashierID]->Wait(cashierLinesLock); //wait in line
			linesIAmLookingAt[myCashier]--; //i have been woken up, remove myself from line



		}
		//-------------End find shortest line----------------------

	}
	}while(cashierStatus[myCashier] == CASH_ON_BREAK); //customer will repeat finding a cashier algorithm if he was woken up because the cashier is going on break

	//----------------End looking for cashier--------------------------------------------


	//code after this means I have been woken up after getting to the front of the line
	cashierLock[myCashier]->Acquire(); //disallow others from getting access to my cashier
	//printf("%s %d is now engaged with Cashier %d after waiting in line.\n", type, myID, myCashier);
	cashierLinesLock->Release(); //allow others to view monitor variable now that I've staked
							//my claim on this cashier
	cashierDesk[myCashier] = myID; //tell cashier who I am
	cashierToCustCV[myCashier]->Signal(cashierLock[myCashier]); //signal cashier I am at his desk
	cashierToCustCV[myCashier]->Wait(cashierLock[myCashier]); //wait for his acknowlegdment
	cashierDesk[myCashier] = privileged; //now tell him that whether or not I am privileged
	cashierToCustCV[myCashier]->Signal(cashierLock[myCashier]); //signal cashier I've passed this information
	cashierToCustCV[myCashier]->Wait(cashierLock[myCashier]); //wait for his acknowledgment

	//When I get here, the cashier is ready for items
	//---------------------------Begin passing items to cashier---------------------------
	//cycle through all items in my inventory, handing one item to cashier at a time
	for(int i = 0; i < numItemsToBuy; i++){ //cycle through all types of items
		for(int j = 0; j < qtyItemsInCart[i]; j++){ //be sure we report how many of each type
			cashierDesk[myCashier] = itemsInCart[i]; //tells the cashier what type of item
			cashierToCustCV[myCashier]->Signal(cashierLock[myCashier]); //signal cashier item is on his desk
			cashierToCustCV[myCashier]->Wait(cashierLock[myCashier]); //wait for his acknowledgement
		}
	}
	cashierDesk[myCashier] = -1; //Tells cashier that I have reached the last item in my inventory
	cashierToCustCV[myCashier]->Signal(cashierLock[myCashier]); //tell cashier
	cashierToCustCV[myCashier]->Wait(cashierLock[myCashier]); //wait for him to put the amount on desk
	//------------------------End passing items to cashier--------------------------------


	//when I get here, the cashier has loaded my total
	//If I don't have enough money, leave the error flag -1 on the cashier's desk
	if(cashierDesk[myCashier] > myCash){
		printf("%s [%d] cannot pay [%d]\n", type, myID, cashierDesk[myCashier]);
		cashierDesk[myCashier] = -1;
		cashierToCustCV[myCashier]->Signal(cashierLock[myCashier]);
		cashierToCustCV[myCashier]->Wait(cashierLock[myCashier]);
		//TODO manager-customer interaction
		managerLock->Acquire();
		cashierLock[myCashier]->Release();
		printf("%s [%d] is waiting for Manager for negotiations\n", type, myID);
		managerCV->Wait(managerLock);

		//-----------------------Begin passing items to manager---------------------
		for(int i = 0; i < numItemsToBuy; i++){
			while(qtyItemsInCart[i] > 0){
//				cout << "manager desk " << managerDesk << endl;
				if(managerDesk < myCash){

					break;
				}
				qtyItemsInCart[i] --;
				managerDesk = itemsInCart[i];
				printf("%s [%d] tells Manager to remove [%d] from trolly.\n", type, myID, itemsInCart[i]);
				managerCV->Signal(managerLock);
				managerCV->Wait(managerLock);
			}
		}
		managerDesk = -1; //notifies the manager I'm done
		managerCV->Signal(managerLock);
		managerCV->Wait(managerLock);
		//--------------------End of passing items to manager---------------

		int amountOwed = managerDesk; //if I still can't afford anything, amountOwed will be 0
		myCash -= amountOwed; //updating my cash amount because I am paying manager
		managerDesk = amountOwed; //technically redundant, but represents me paying money
		printf("%s [%d] pays [%d] to Manager after removing items and is waiting for receipt from Manager.\n", type, myID, amountOwed);
		//need receipt
		managerCV->Signal(managerLock);
		managerCV->Wait(managerLock);
		//got receipt, I can now leave
		managerLock->Release();
		cout << "Customer [" << myID << "] got receipt from Manager and is now leaving." << endl;
	}
	//if I do have money, I just need to update my cash and leave
	//the money there
	else{
		myCash -= cashierDesk[myCashier];
		//Now I wait for my receipt
		cashierToCustCV[myCashier]->Signal(cashierLock[myCashier]);
		printf("%s [%d] pays [%d] to Cashier [%d] and is now waiting for receipt.\n", type, myID, cashierDesk[myCashier], myCashier);
		cashierToCustCV[myCashier]->Wait(cashierLock[myCashier]);
		//now I've received my receipt and should release the cashier
		cout << type << " [" << myID << "] got receipt from Cashier [" << myCashier << "] and is now leaving." << endl;
		cashierLock[myCashier]->Release();
		cashierToCustCV[myCashier]->Signal(cashierLock[myCashier]);
		myCashier = -1; //so I can't accidentally tamper with the cashier I chose anymore
	}

	//------------------------------Begin replace trolly--------------
	//no need for CV since sequencing doesn't matter
	displacedTrollyLock->Acquire();
	displacedTrollyCount++;
	displacedTrollyLock->Release();
	//-----------------------------End replace trolly----------------------

	customersDone++;
	
	
	//cleanup
	delete itemsToBuy;
	delete qtyItemsToBuy;
	delete itemsInCart;
	delete qtyItemsInCart;
}

//----------------------------------------------------
//customer method with cashier
//---------------------------------------------------
void customer(int myID){

	int myCashier; //ID of cashier I speak to
	char* type = new char[20];
	int myCash = customerCash;
	int *itemsInCart;
	int *qtyItemsInCart;
	int numItemsToBuy = 3;
	int privileged;
	itemsInCart = new int[numItemsToBuy];
	qtyItemsInCart = new int[numItemsToBuy];
	for(int i = 0; i < numItemsToBuy; i++) {
		//TODO- Put a random item/qty picker here

		itemsInCart[i] = i;
		qtyItemsInCart[i] = 3 - i;
	}

	//---------Randomly generate whether this customer is privileged--------------
	srand(myID + time(NULL));
	int r = rand() % 10; //random value to set Customer either as privileged or unprivileged
	if(r < 2){//30% chance customer is privileged
		privileged = 1;
	}
	else privileged = 0; //70% chance this customer is unprivileged

	//set char array for I/O purposes
	if(privileged){
		type = "Privileged Customer";
	}
	else type = "Customer";
	privileged = 0;
	//--------------End of privileged/unprivileged decion--------------------------

	//--------------Begin looking for a cashier-------------------------------------
	cashierLinesLock->Acquire(); //acquire locks to view line counts and cashier statuses
	do{
		printf("%s %d is looking for the cashier.\n", type, myID );

		//Find if a cashier is free (if one is, customer doesn't need to wait in line)
		for(int i = 0; i < cashierNumber; i++ ){
			//if I find a cashier who is free, I will:
			if(cashierStatus[i] == CASH_NOT_BUSY){
				myCashier = i; //remember who he is
				cashierLock[i]->Acquire(); //get his lock before I wake him up
				cashierStatus[i] = CASH_BUSY; //prevent others from thinking he's free
				printf("%s %d chose Cashier %d who is free.\n", type, myID, myCashier);
				break; //stop searching through lines
			}

			//---------------Find shortest line---------------------------
			else if (i == cashierNumber - 1){
				int minLineValue;
				int minCashierID = 0;
				int* linesIAmLookingAt; //temporary pointer to line counts so the code for
				//the two types of customer can be identical
				Condition **linesIAmLookingAtCV;//temporary pointer to the CVs, again for
				//the purpose of having same code for both types of customers

				if(privileged){
					linesIAmLookingAt = privilegedLineCount;
					linesIAmLookingAtCV = privilegedCashierLineCV;
				}
				else{
					linesIAmLookingAt = unprivilegedLineCount;
					linesIAmLookingAtCV = unprivilegedCashierLineCV;
				}

				//from here on, privilegedCustomers and unprivileged customers execute same code because
				//of the temporary variables linesIAmLookingAt (which is unprivilegedLineCount or privilegedLineCount)
				//and linesIAmLookingAtCV (which is un/privilegedCashierLineCV)

				minLineValue = custNumber; //set a default min value
				//find the minimum line value and remember the cashier associated with it
				for(int j = 0; j < cashierNumber; j++){
					if(linesIAmLookingAt[j] < minLineValue && cashierStatus[j] != CASH_ON_BREAK && cashierStatus[j] != 	CASH_GO_ON_BREAK){
						//must also check if that cashier is on break
						minLineValue = linesIAmLookingAt[j];
						minCashierID = j;
					}
				}
				myCashier = minCashierID;
				printf("%s %d chose Cashier %d of line length %d.\n", type, myID, myCashier, linesIAmLookingAt[minCashierID]);
				linesIAmLookingAt[minCashierID]++;
				linesIAmLookingAtCV[minCashierID]->Wait(cashierLinesLock); //wait in line
				linesIAmLookingAt[myCashier]--;


				//when customer is woken up, there are two cases: 1. I am at front of the line, or 2., my cashier is going on break
				//this decremented line count if the second case


			}
			//-------------End find shortest line----------------------

		}
	}while(cashierStatus[myCashier] == CASH_ON_BREAK); //customer will repeat finding a cashier algorithm if he was woken up because the cashier is going on break

	//----------------End looking for cashier--------------------------------------------


	//code after this means I have been woken up after getting to the front of the line
	cashierLock[myCashier]->Acquire(); //disallow others from getting access to my cashier
	printf("%s %d is now engaged with Cashier %d after waiting in line.\n", type, myID, myCashier);
	cashierLinesLock->Release(); //allow others to view monitor variable now that I've staked
	//my claim on this cashier
	cashierDesk[myCashier] = myID; //tell cashier who I am
	cashierToCustCV[myCashier]->Signal(cashierLock[myCashier]); //signal cashier I am at his desk
	cashierToCustCV[myCashier]->Wait(cashierLock[myCashier]); //wait for his acknowlegdment
	cashierDesk[myCashier] = privileged; //now tell him that whether or not I am privileged
	cashierToCustCV[myCashier]->Signal(cashierLock[myCashier]); //signal cashier I've passed this information
	cashierToCustCV[myCashier]->Wait(cashierLock[myCashier]); //wait for his acknowledgment

	//When I get here, the cashier is ready for items
	//---------------------------Begin passing items to cashier---------------------------
	//cycle through all items in my inventory, handing one item to cashier at a time
	for(int i = 0; i < numItemsToBuy; i++){ //cycle through all types of items
		for(int j = 0; j < qtyItemsInCart[i]; j++){ //be sure we report how many of each type
			cashierDesk[myCashier] = itemsInCart[i]; //tells the cashier what type of item
			cashierToCustCV[myCashier]->Signal(cashierLock[myCashier]); //signal cashier item is on his desk
			cashierToCustCV[myCashier]->Wait(cashierLock[myCashier]); //wait for his acknowledgement
		}
	}
	cashierDesk[myCashier] = -1; //Tells cashier that I have reached the last item in my inventory
	cashierToCustCV[myCashier]->Signal(cashierLock[myCashier]); //tell cashier
	cashierToCustCV[myCashier]->Wait(cashierLock[myCashier]); //wait for him to put the amount on desk
	//------------------------End passing items to cashier--------------------------------


	//when I get here, the cashier has loaded my total
	//If I don't have enough money, leave the error flag -1 on the cashier's desk
	if(cashierDesk[myCashier] > myCash){
		cashierDesk[myCashier] = -1;
		cashierToCustCV[myCashier]->Signal(cashierLock[myCashier]);
		cashierToCustCV[myCashier]->Wait(cashierLock[myCashier]);
		//TODO manager-customer interaction
		managerLock->Acquire();
		cashierLock[myCashier]->Release();
		managerCV->Wait(managerLock);

		//-----------------------Begin passing items to manager---------------------
		for(int i = 0; i < numItemsToBuy; i++){
			while(qtyItemsInCart[i] > 0){
				cout << "manager desk " << managerDesk << endl;
				if(managerDesk < myCash){

					break;
				}
				qtyItemsInCart[i] --;
				managerDesk = itemsInCart[i];
				managerCV->Signal(managerLock);
				managerCV->Wait(managerLock);
			}
		}
		managerDesk = -1; //notifies the manager I'm done
		managerCV->Signal(managerLock);
		managerCV->Wait(managerLock);
		//--------------------End of passing items to manager---------------

		int amountOwed = managerDesk; //if I still can't afford anything, amountOwed will be 0
		myCash -= amountOwed; //updating my cash amount because I am paying manager
		managerDesk = amountOwed; //technically redundant, but represents me paying money
		printf("%s %d pays %d and is now waiting for receipt.\n", type, myID, amountOwed);
		//need receipt
		managerCV->Signal(managerLock);
		managerCV->Wait(managerLock);
		//got receipt, I can now leave
		managerLock->Release();
		cout << type << " " << myID << " got receipt from Manager and is now leaving." << endl;
	}
	//if I do have money, I just need to update my cash and leave
	//the money there
	else{
		myCash -= cashierDesk[myCashier];
		//Now I wait for my receipt
		cashierToCustCV[myCashier]->Signal(cashierLock[myCashier]);
		printf("%s %d pays %d and is now waiting for receipt.\n", type, myID, cashierDesk[myCashier]);
		cashierToCustCV[myCashier]->Wait(cashierLock[myCashier]);
		//now I've received my receipt and should release the cashier
		cout << type << " " << myID << " got receipt from Cashier " << myCashier << " and is now leaving." << endl;
		cashierLock[myCashier]->Release();
		cashierToCustCV[myCashier]->Signal(cashierLock[myCashier]);
		myCashier = -1; //so I can't accidentally tamper with the cashier I chose anymore
	}

	//------------------------------Begin replace trolly--------------
	//no need for CV since sequencing doesn't matter
	displacedTrollyLock->Acquire();
	displacedTrollyCount++;
	displacedTrollyLock->Release();
	//-----------------------------End replace trolly----------------------

	customersDone++;
}

int scan(int item){
	switch(item){
	case 0: return 5;
	case 1: return 2;
	case 2: return 3;
	case 3: return 8;
	default: return 0;
	}
}



void manager(){
	int totalRevenue = 0; //will track the total sales of the day
	int counter = 0;
	int *cashierTotals = new int[cashierNumber];

	//initializes cashier totals, by which we keep track of how much money each cashier has had
	//in their registers over time
	for(int i = 0; i < cashierNumber; i++){
		cashierTotals[i] = 0;
	}
	queue<int> cashiersOnBreak; //allows manager to remember which cashiers he's sent on break
	int numCashiersOnBreak = 0;
	queue<int> salesmenOnBreak;
	int numSalesmenOnBreak = 0;


	while(true){

		//------------------Check if all customers have left store----------------
		//if all customers have left, print out totals and terminate simulation since manager will be the only
		//ready thread
		if(customersDone == custNumber){
			for(int i = 0; i < cashierNumber; i++){
				cashierLock[i]->Acquire();
				//-----------------------------Start empty cashier drawers------------------------------------
				if(cashRegister[i] > 0){
					totalRevenue += cashRegister[i];
					cashierTotals[i] += cashRegister[i];
					cout << "Manager emptied Counter " << i << " drawer." << endl;
					cashRegister[i] = 0;
					cout << "Manager has total sale of $" << totalRevenue << "." << endl;
					cashierLock[i]->Release();
				}
			}
			for(int i = 0; i < cashierNumber; i++){
				cout << "Total Sale from Counter [" << i << "] is $[" << cashierTotals[i] << "]." << endl;
			}
			cout << "Total Sale of the entire store is $[" << totalRevenue << "]." << endl;
			break;
		}
		//------------------End check if all customers have left store------------

		//-----------------Have loader check trolleys---------------------------
		inactiveLoaderCV->Signal(inactiveLoaderLock); //wake up goods loader to do a regular check of trolley and manager items
		counter ++;
		//I don't need to acquire a lock because I never go to sleep
		//Therefore, it doesn't matter if a cashierFlag is changed on this pass,
		//I will get around to it
		if(counter > 10){
			counter = 0;
		//	cout << customersDone << endl;
			cout <<"-------Total Sale of the entire store until now is $" << totalRevenue <<"---------" << endl;
		}

		cashierLinesLock->Acquire(); //going to be checking line counts and statuses, so need this lock

		int numFullLines = 0; //will be used to figure out whether to bring cashiers back from break
		int numAnyLines = 0;
		for (int i = 0; i < cashierNumber; i++){
			if(privilegedLineCount[i]) numAnyLines++;
			if(unprivilegedLineCount[i]) numAnyLines++;
			//a line is "full" if it has more than 3 customers (if each cashier has a line of size 3, we want to bring back a cashier
			if(privilegedLineCount[i] > 3) numFullLines ++;
			if(unprivilegedLineCount[i] > 3) numFullLines ++;
		}

		//--------------------------Begin bring cashier back from break--------------------
		if(numFullLines > (cashierNumber - cashiersOnBreak.size()) && cashiersOnBreak.size()){ //bring back cashier if there are more lines with 3 customers than there are cashiers and if there are cashiers on break

			int wakeCashier = cashiersOnBreak.front();
			if(cashierStatus[wakeCashier] == CASH_ON_BREAK && !unprivilegedLineCount[wakeCashier] && !privilegedLineCount[wakeCashier]){
			if (numAnyLines)cout << "Manager brings back Cashier " << wakeCashier << " from break." << endl;
			cashierStatus[wakeCashier] = CASH_NOT_BUSY; //set this since any cashier that had breaked will have lines = 0
			cashierToCustCV[wakeCashier]->Signal(cashierLock[wakeCashier]); //this is the actual act of bring a cashier back from break

			//bookkeeping
			numCashiersOnBreak--;
			cashiersOnBreak.pop();
				
			}
			cashierLinesLock->Release();

		}
		else cashierLinesLock->Release();
		//---------------------------End Bring cashier back from break--------------------------

		//---------------------------Begin send cashiers on break-------------------------------
		cashierLinesLock->Acquire();
		srand(counter);
		int chance = rand() % 5;
		if( chance == 1  && cashiersOnBreak.size() < cashierNumber -2){ //.001% chance of sending cashier on break
			//generate cashier index
			int r = rand() % cashierNumber;
			if(cashierStatus[r] != CASH_ON_BREAK && cashierStatus[r] != CASH_GO_ON_BREAK){
				cashierStatus[r] = CASH_GO_ON_BREAK;
				/*if(numAnyLines)*/cout << "Manager sends Cashier [" << r << "] on break." << endl;
				cashiersOnBreak.push(r);
				numCashiersOnBreak++;


			}
		}
		//-----------------------------End send cashiers on break-------------------------------------
		cashierLinesLock->Release();

		//-----------------------------Begin bringing salesmen back from break-------------

		salesLock->Acquire();
		if((greetingCustWaitingLineCount + complainingCustWaitingLineCount + loaderWaitingLineCount) > 0 && salesmenOnBreak.size()){
			int wakeSalesman = cashiersOnBreak.front();
			if(currentSalesStatus[wakeSalesman] == SALES_ON_BREAK){
				salesBreakBoard[wakeSalesman] = 0;
				cout << "Manager brings back Cashier " << wakeSalesman << " from break." << endl;
				salesBreakCV[wakeSalesman]->Signal(salesLock);
				numSalesmenOnBreak--;
				salesmenOnBreak.pop();
			}
		}
		salesLock->Release();
		//------------------------------end bringing salesmen back from break--------------

		//------------------------------Begin putting salesmen on break------------------
		salesLock->Acquire();
		if (chance == 1 && numSalesmenOnBreak < numSalesmen -1) {
			int r = rand() % numSalesmen;
			if(!salesBreakBoard[r] && currentSalesStatus[r] != SALES_ON_BREAK && currentSalesStatus[r] != SALES_GO_ON_BREAK) {
				salesBreakBoard[r] = 1;
				cout << "Manager sends Salesman " << r << " on break." << endl;
				individualSalesmanLock[r]->Acquire();
				if(currentlyTalkingTo[r] == UNKNOWN){
					salesmanCV[r]->Signal(individualSalesmanLock[r]);
					currentSalesStatus[r] = SALES_ON_BREAK;
				}
				individualSalesmanLock[r]->Release();
				salesmenOnBreak.push(r);
				numSalesmenOnBreak++;
			}
		}
		salesLock->Release();
		//-----------------------------End send salesmen on break




		for(int i = 0; i < cashierNumber; i++){

			cashierLock[i]->Acquire();
			//-----------------------------Start empty cashier drawers------------------------------------
			if(cashRegister[i] > 0){
				totalRevenue += cashRegister[i];
				cashierTotals[i] += cashRegister[i];
				cout << "Manager emptied Counter [" << i << "] drawer." << endl;
				cashRegister[i] = 0;
				cout << "Manager has total sale of $[" << totalRevenue << "]." << endl;
				cashierLock[i]->Release();
				break;
			}
			//---------------------------end empty cashier drawers---------------------------------

			//------------------------------Start deal with broke customers-------------------------
			else if(cashierFlags[i] != -1){
				cout <<"Manager got a call from Cashier [" << i << "]." << endl;
				int customerID = cashierFlags[i];
				int cashierID = i;

				managerLock->Acquire();
				cashierToCustCV[cashierID]->Signal(cashierLock[cashierID]); //wakes up customer, who also waits first in this interaction
				cashierToCustCV[cashierID]->Signal(cashierLock[cashierID]); //wakes up cashier
				//cout <<"manager has signallled cusstomer and cashier" << endl;
				cashierLock[i]->Release();
				managerCV->Wait(managerLock);
				int amountOwed = cashierFlags[i];
				cashierFlags[i] = -1; //reset cashierFlags
				cout << " amount owed " << amountOwed << endl;
				int custTypeID = managerDesk;
				cout << "custTypeID " << custTypeID << endl;
				char* custType = new char[20];
				if(custTypeID == 0){
					custType = "Customer";
				}
				else{
					custType = "Privileged Customer";
				}
				managerDesk = amountOwed;
				managerCV->Signal(managerLock);
				managerCV->Wait(managerLock);
				while(managerDesk != -1 ){ //when managerDesk == -1, customer is out of items or can afford money
					amountOwed -= scan(managerDesk);
					managerItemsLock->Acquire();
					managerItems.push(managerDesk);
					managerItemsLock->Release();
					cout << "new amount owed " << amountOwed << endl;
					cout << "Manager removes [" << managerDesk << "] from the trolly of " << custType << " [" << customerID << "]."<<endl;
					managerDesk = amountOwed; //giving customer new subtotal
									//customer will put back -1 if out of items
									// or if they can now afford
					managerCV->Signal(managerLock);
					managerCV->Wait(managerLock);
				}
				inactiveLoaderCV->Signal(inactiveLoaderLock);
				//now customer has reached end of items or has enough money
				//I give him total
				managerDesk = amountOwed;
				managerCV->Signal(managerLock);
				managerCV->Wait(managerLock); //wait for his response, i.e. paying money
				totalRevenue += managerDesk;

				//Now give the customer his receipt
				cout << "Manager gives receipt to " << custType << " [" << customerID << "]." << endl;
				managerDesk = -1;
				managerCV->Signal(managerLock);
				//release manager lock
				managerLock->Release();
				break;


			}
			//----------------------------End deal with broke customers--------------------
			else cashierLock[i]->Release();
		}
	}
}




//cashier code

void cashier(int myCounter){
	while(true){
		char* custType = new char[20];
		cashierLinesLock->Acquire();
		//check my status to see if I've already been set to busy by a customer
		if(cashierStatus[myCounter] == CASH_GO_ON_BREAK){
			//cout << "Cashier [" << myCounter << " acknowledges he will go on break" << endl;



		unprivilegedCashierLineCV[myCounter]->Broadcast(cashierLinesLock);
		privilegedCashierLineCV[myCounter]->Broadcast(cashierLinesLock);
		//cout << myCounter << " just broadcast! and my line had " << unprivilegedLineCount[myCounter] << " and " << privilegedLineCount[myCounter] << endl;
		cout << "Cashier [" << myCounter << "] is going on break." << endl;
		cashierStatus[myCounter] = CASH_ON_BREAK;
		cashierLock[myCounter]->Acquire();
		cashierLinesLock->Release();
		cashierToCustCV[myCounter]->Wait(cashierLock[myCounter]);
		cout << "Cashier [" << myCounter << "] was called from break by Manager to work." << endl;
		continue;
	}
	//check if my lines have anyone in it

	if(privilegedLineCount[myCounter]){
		cashierStatus[myCounter] = CASH_BUSY;
		privilegedCashierLineCV[myCounter]->Signal(cashierLinesLock);
		custType = "Privileged Customer";
	}
	else if(unprivilegedLineCount[myCounter]){
		cashierStatus[myCounter] = CASH_BUSY;
		//cout << "Signalling customer in line" << endl;
		unprivilegedCashierLineCV[myCounter]->Signal(cashierLinesLock);
		custType = "Customer";
	}
	else{
		cashierStatus[myCounter] = CASH_NOT_BUSY;
	}
	//whether or not I'm ready for a customer, I can get my lock and go to sleep
	cashierLock[myCounter]->Acquire();
	cashierLinesLock->Release();
	cashierToCustCV[myCounter]->Signal(cashierLock[myCounter]);
	//cout << "Cashier falling asleep" << endl;
	cashierToCustCV[myCounter]->Wait(cashierLock[myCounter]);
	//the next time I'm woken up (assuming it is by a customer, not a manager
	//I will be totaling items
	//when I get here, there will be an item to scan
	int total = 0;
	int custID = cashierDesk[myCounter];
	cashierToCustCV[myCounter]->Signal(cashierLock[myCounter]);
	cashierToCustCV[myCounter]->Wait(cashierLock[myCounter]);
	if(cashierDesk[myCounter] == 1){
		custType = "PrivilegedCustomer";
	}
	else{
		custType = "Customer";
	}
	cashierToCustCV[myCounter]->Signal(cashierLock[myCounter]);
	cashierToCustCV[myCounter]->Wait(cashierLock[myCounter]);
	while(cashierDesk[myCounter] != -1){ //-1 means we're done scanning


		cout << "Cashier [" << myCounter << "] got [" << cashierDesk[myCounter] << "] from trolly of " << custType << " [" << custID << "]." << endl;
		total += scan(cashierDesk[myCounter]);
		cashierToCustCV[myCounter]->Signal(cashierLock[myCounter]);
		cashierToCustCV[myCounter]->Wait(cashierLock[myCounter]);
	}
	//now I'm done scanning, so I tell the customer the total
	cout << "Cashier [" << myCounter << "] tells " << custType << " [" << custID << "] total cost is $[" << total << "]." << endl;
	cashierDesk[myCounter] = total;
	cashierToCustCV[myCounter]->Signal(cashierLock[myCounter]);

	cashierToCustCV[myCounter]->Wait(cashierLock[myCounter]);
	if(cashierDesk[myCounter] == -1){
		cout << "Cashier [" << myCounter << "] asks " << custType << " [" << custID << "] to wait for Manager." << endl;
		cout << "Cashier [" << myCounter << "] informs the Manager that " << custType << " [" << custID << "] does not have enough money." << endl;
		cashierFlags[myCounter] = custID;
		//cout << "cashier is goign to sleep" << endl;
		cashierToCustCV[myCounter]->Wait(cashierLock[myCounter]);
		//Set the manager desk to 0 or 1 to tell the manager which type of
		//customer he is dealing with
		if(!strcmp(custType, "Privileged Customer")){
			managerDesk = 1;
		}
		if(!strcmp(custType, "Customer")){
			managerDesk = 0;
		}
		cout << " total " << total << endl;
		cashierFlags[myCounter] = total; //inform manager of the total the customer owes
		managerCV->Signal(managerLock); //wake up manager, who was waiting for this information
		//when I am woken up, the manager has taken over so I can free myself for my
		//next customer
	}
	else{
		//add value to cash register
		cashRegister[myCounter] += cashierDesk[myCounter];
		cout << "Cashier [" << myCounter << "] got money $[" << cashierDesk[myCounter] << "] from " << custType << " [" << custID << "]." << endl;
		//giving the customer a receipt
		cout << "Cashier [" << myCounter << "] gave the receipt to " << custType << " [" << custID << "] and tells him to leave" << endl;
		cashierToCustCV[myCounter]->Signal(cashierLock[myCounter]);
		//wait for customer to acknowledge getting receipt and release lock
		cashierToCustCV[myCounter]->Wait(cashierLock[myCounter]);
	}
	cashierDesk[myCounter] = 0;
	cashierLock[myCounter]->Release();

	//done, just need to loop back and check for more customers
	delete custType;
	}
}



//Salesman Code		__SALESMAN__
void Salesman (int myIndex){

	while(true) {
		salesLock->Acquire();

		//Check if there is someone in line
		//and wake them up

		if(salesBreakBoard[myIndex] == 1) {	//go on break
			cout << "Sales " << myIndex << " going on break" << endl;
			currentSalesStatus[myIndex] = SALES_ON_BREAK;

			salesBreakCV[myIndex]->Wait(salesLock);
			salesBreakBoard[myIndex] = 0;
			currentSalesStatus[myIndex] = SALES_NOT_BUSY;
			cout << "Sales " << myIndex << " back from break" << endl;
		}

		if(greetingCustWaitingLineCount > 0){
			//cout << "Salesman going to greet cust" << endl;
			currentSalesStatus[myIndex] = SALES_READY_TO_TALK;
			greetingCustCV->Signal(salesLock);
			greetingCustWaitingLineCount--;
			//currentlyTalkingTo[myIndex] = GREETING;
		}
		else if(loaderWaitingLineCount > 0) {
			currentSalesStatus[myIndex] = SALES_READY_TO_TALK;
			//cout << "Signalling loader waiting" << endl;
			loaderCV->Signal(salesLock);
			loaderWaitingLineCount--;
		}
		else if(complainingCustWaitingLineCount > 0) {
			//cout << "Salesman going to help a complaing cust" << endl;
			currentSalesStatus[myIndex] = SALES_READY_TO_TALK;
			complainingCustCV->Signal(salesLock);
			complainingCustWaitingLineCount--;
		}

		else{
			//cout << "not busy" << endl;
			currentlyTalkingTo[myIndex] = UNKNOWN;
			currentSalesStatus[myIndex] = SALES_NOT_BUSY;
		}

		individualSalesmanLock[myIndex]->Acquire();
		salesLock->Release();
		salesmanCV[myIndex]->Wait(individualSalesmanLock[myIndex]);	//Wait for cust/loader to walk up to me?
		//cout << "Someone " << currentlyTalkingTo[myIndex] << " came up to salesman " << myIndex << endl;

		if(currentlyTalkingTo[myIndex] == GREETING) {
			//individualSalesmanLock[myIndex]->Acquire();
			//salesLock->Release();
			//salesmanCV[myIndex]->Wait(individualSalesmanLock[myIndex]);	//Wait for cust to walk up to me?
			int myCustNumber = salesCustNumber[myIndex]; //not sure if custNumberForSales needs to be an array
			if(privCustomers[myCustNumber] == 1){
				cout << "DepartmentSalesman [" << myIndex << "] welcomes PrivilegeCustomer [" << myCustNumber << "] to Department [" << "dept#" << "]." << endl;
			}
			else{
				cout << "DepartmentSalesman [" << myIndex << "] welcomes Customer [" << myCustNumber << "] to Department [" << "dept#" << "]." << endl;
			}
			salesmanCV[myIndex]->Signal(individualSalesmanLock[myIndex]);
			individualSalesmanLock[myIndex]->Release();
			//cout << "just signalled on index: " << myIndex << endl;
		}
		else if(currentlyTalkingTo[myIndex] == COMPLAINING) {
			//individualSalesmanLock[myIndex]->Acquire();
			//salesLock->Release();
			//salesmanCV[myIndex]->Wait(individualSalesmanLock[myIndex]);	//Wait for cust to walk up to me?
			int myCustNumber = salesCustNumber[myIndex]; //not sure if custNumberForSales needs to be an array
			int itemOutOfStock = salesDesk[myIndex];

			if(privCustomers[myCustNumber] == 1){
				cout << "DepartmentSalesman [" << myIndex << "] is informed by PrivilegeCustomer [" << myCustNumber << "] that [" << itemOutOfStock << "] is out of stock." << endl;
			}
			else{
				cout << "DepartmentSalesman [" << myIndex << "] is informed by Customer [" << myCustNumber << "] that [" << itemOutOfStock << "] is out of stock." << endl;
			}
			salesmanCV[myIndex]->Signal(individualSalesmanLock[myIndex]);	//tell cust to wait

			//TODO tell goods loader
			salesLock->Acquire();
			currentSalesStatus[myIndex] = SALES_SIGNALLING_LOADER;
			salesDesk[myIndex] = itemOutOfStock;	//Might not be necessary, because we never really took it off the desk
			salesLock->Release();
			individualSalesmanLock[myIndex]->Acquire();
			//cout << "signalling a loader" << endl;
			inactiveLoaderCV->Signal(inactiveLoaderLock);	//call a loader over?
			salesmanCV[myIndex]->Wait(individualSalesmanLock[myIndex]);
			inactiveLoaderLock->Acquire();
			individualSalesmanLock[myIndex]->Release();	//???
			int myLoaderID = -1;

			/*
			/*for(int i = 0; i < numLoaders; i++) {
				cout << "Load status: " << loaderStatus[i] << endl;
			}*/

			for(int i = 0; i < numLoaders; i++) {
				if(loaderStatus[i] == LOAD_HAS_BEEN_SIGNALLED) {
					myLoaderID = i;
					loaderStatus[i] = LOAD_STOCKING;
					break;
				}
			}
			//cout << "Salesman " << myIndex << " is waiting for loader " << myLoaderID << endl;
			inactiveLoaderLock->Release();

			//cout << "Loader " << myLoaderID << " has arrived at salesman " << myIndex << "!" << endl;
			cout << "DepartmentSalesman [" << myIndex << "] informs the GoodsLoader [" << myLoaderID << "] that [" << itemOutOfStock << "] is out of stock." << endl;
			salesmanCV[myIndex]->Signal(individualSalesmanLock[myIndex]);

		}
		else if(currentlyTalkingTo[myIndex] == GOODSLOADER) {
			//individualSalesmanLock[myIndex]->Acquire();
			//salesLock->Release();
			//salesmanCV[myIndex]->Wait(individualSalesmanLock[myIndex]);	//Wait for cust/loader to walk up to me?

			int itemRestocked = salesDesk[myIndex];
			salesmanCV[myIndex]->Signal(individualSalesmanLock[myIndex]);
//			salesmanCV[myIndex]->Wait(individualSalesmanLock[myIndex]);
//			int loaderNumber = salesDesk[myIndex];
//			salesmanCV[myIndex]->Signal(individualSalesmanLock[myIndex]);
			cout << "DepartmentSalesman [" << myIndex << "] is informed by the GoodsLoader [" << "loaderNumber" << "] that [" << itemRestocked << "] is restocked." << endl;
			shelfCV[itemRestocked]->Broadcast(shelfLock[itemRestocked]);
			//DepartmentSalesman [identifier] informs the Customer/PrivilegeCustomer [identifier] that [item] is restocked.
		}
		else{
			individualSalesmanLock[myIndex]->Release();
		}
	}
}

//Goods loader code			__LOADER__
//Goods loader code			__LOADER__
void GoodsLoader(int myID) {
	int mySalesID = -1;
	while(true) {
		inactiveLoaderLock->Acquire();
		loaderStatus[myID] = LOAD_NOT_BUSY;
		if(mySalesID != -1){
			cout << "GoodsLoader [" << myID << "] is waiting for orders to restock." << endl;
		}
		inactiveLoaderCV->Wait(inactiveLoaderLock);
//		cout << "Loader " << myID << " has been summoned" << endl;

		loaderStatus[myID] = LOAD_HAS_BEEN_SIGNALLED;
		int shelf = -1;
		mySalesID = -1;
		inactiveLoaderLock->Release();

		salesLock->Acquire();
		for(int i = 0; i < numSalesmen; i++) {
			if(currentSalesStatus[i] == SALES_SIGNALLING_LOADER) {
				mySalesID = i;
				currentSalesStatus[mySalesID] = SALES_BUSY;
				break;
			}
		}

		if(mySalesID == -1){ //if the loader was signaled by the manager to get trollys
			salesLock->Release();
			managerItemsLock->Acquire();
			int inHands = 0;
			while(!managerItems.empty()){
				//simulates the manager putting the items in the stockroom
				inHands = managerItems.front();
				managerItems.pop(); //remove the item from the manager's item list
				stockRoomLock->Acquire();
				inHands = 0;
				stockRoomLock->Release();
			}
			managerItemsLock->Release();
			displacedTrollyLock->Acquire();
			int restoreTrollies = 0;
			if(displacedTrollyCount > 0){
				restoreTrollies = displacedTrollyCount;
			}
			displacedTrollyLock->Release();

			if(restoreTrollies != 0) {
				trollyLock->Acquire();
				trollyCount += restoreTrollies;
				trollyCV->Broadcast(trollyLock);
				trollyLock->Release();
			}
		}
		else{
			shelf = salesDesk[mySalesID];
			cout << "GoodsLoader [" << myID << "] is informed by DepartmentSalesman [" << mySalesID <<
					"] of Department [" << "dept#" << "] to restock [" << shelf << "]." << endl;
			//inactiveLoaderLock->Acquire();
			//inactiveLoaderLock->Release();

			individualSalesmanLock[mySalesID]->Acquire();
			salesLock->Release();
			salesmanCV[mySalesID]->Signal(individualSalesmanLock[mySalesID]);	//tell him i'm here
			salesmanCV[mySalesID]->Wait(individualSalesmanLock[mySalesID]);
			individualSalesmanLock[mySalesID]->Release();


			//restock
			int qtyInHands = 0;
			for(int i = 0; i < maxShelfQty; i++) {
				//currentLoaderInStockLock->Acquire();
				//if(currentLoaderInStock == -1){
				//	currentLoaderInStock = myID;
					//cout << "GoodsLoader [" << myID << "] is setting the currentLoaderInLock" << endl;
				//}
				//else{
					cout << "GoodsLoader [" << myID << "] is waiting for GoodsLoader [" << currentLoaderInStock << "] to leave the StockRoom." << endl;
				//}
				//Simulates a store room like the spec says
				stockRoomLock->Acquire();
				//currentLoaderInStockLock->Release();
				cout << "GoodsLoader [" << myID << "] is in the StockRoom and got [" << shelf << "]." << endl;
				qtyInHands++;
				//currentLoaderInStockLock->Acquire();
				//currentLoaderInStock = -1; //lets other goodsloaders change it
				stockRoomLock->Release();
				//currentLoaderInStockLock->Release();
				cout << "GoodsLoader [" << myID << "] leaves StockRoom." << endl;

				//cout << "about to yield" << endl;
					for(int j = 0; j < 5; j++) {
					currentThread->Yield();
				}

				shelfLock[shelf]->Acquire();
				if(shelfInventory[shelf] == maxShelfQty) {
					cout << "GoodsLoader [" << myID << "] has restocked [" << shelf << "] in Department [" << "dept#" << "]." << endl;
					shelfLock[shelf]->Release();
					qtyInHands = 0;
					break;
				}
				shelfInventory[shelf] += qtyInHands;
				qtyInHands = 0;
				shelfLock[shelf]->Release();
			}

			//wait in line/inform sales
			salesLock->Acquire();
			//cout << " I have acquired saleslock and am awaiting to notify a salesman" << endl;
			mySalesID = -1;
			for(int i = 0; i < numSalesmen; i++) {
				if(currentSalesStatus[i] == SALES_NOT_BUSY) {
					//cout << " a salesman wasn't busy" << endl;
					mySalesID = i;
					break;
				}
			}
			if(mySalesID == -1) {
				loaderWaitingLineCount++;
				//cout << "about to wait" << endl;
				loaderCV->Wait(salesLock);
				//cout << " i was woken up " << endl;
				for(int i = 0; i < numSalesmen; i++) {
					if(currentSalesStatus[i] == SALES_READY_TO_TALK) {
						mySalesID = i;
						break;
					}
				}
			}

			//printf("I am loader %d, and I want to talk to sales %d\n", myID, mySalesID);

			//Ready to go talk to sales
			individualSalesmanLock[mySalesID]->Acquire();
			cout << "Goods loader has acquired individualSalesmanLock" << endl;
			currentlyTalkingTo[mySalesID] = GOODSLOADER;
			currentSalesStatus[mySalesID] = SALES_BUSY;
			salesLock->Release();
			salesDesk[mySalesID] = shelf;
//			salesmanCV[mySalesID]->Signal(individualSalesmanLock[mySalesID]);
//			salesmanCV[mySalesID]->Wait(individualSalesmanLock[mySalesID]);
//			salesDesk[mySalesID] = myID;
			salesmanCV[mySalesID]->Signal(individualSalesmanLock[mySalesID]);
			individualSalesmanLock[mySalesID]->Release();
		}
	}
}


//================
// Tests
//================

//Test for greeting the customer
void TestGreetingCustomer(int NUM_CUSTOMERS, int NUM_SALESMEN){
	Thread *t;
	char *name;

	printf ("Initializing Variables for 'Greeting Customer Test'\n");

	initTrolly();
	initSalesmen();
	initShelvesWithQty(16);

	printf ("Starting 'Greeting Customer Test'\n");

	//Fork Salesmen Threads

	for(int i = 0; i < NUM_SALESMEN; i++){
		name = new char [20];
		sprintf(name,"salesman_thread_%d",i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr)Salesman,i);
	}

	//Fork Customer Threads

	for(int i = 0; i < NUM_CUSTOMERS; i++){
		name = new char [20];
		sprintf(name,"customer_thread_%d",i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr)Customer,i);
	}

	for(int i = 0; i < numLoaders; i++) {
		name = new char [20];
		sprintf(name,"loader_thread_%d",i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr)GoodsLoader,i);
	}
}

void testCustomerEnteringStoreAndPickingUpItems() {
	initSalesmen();
	initShelvesWithQty(10);
	initLoaders();
	initCustomerCashier();
	initTrolly();


	char* name;

	Thread * t;
	for(int i = 0; i < numSalesmen; i++){
		name = new char [20];
		sprintf(name,"sales%d",i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr)Salesman, i);
		delete name;
	}
	for(int i = 0; i < custNumber; i++){
		name = new char [20];
		sprintf(name,"cust%d",i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr)Customer, i);
		delete name;
	}
	for(int i = 0; i < cashierNumber; i++) {
		name = new char [20];
		sprintf(name,"cash%d",i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr)cashier, i);
		delete name;
	}
	for(int i = 0; i < numLoaders; i++) {
		name = new char [20];
		sprintf(name,"loader%d",i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr)GoodsLoader, i);
		delete name;
	}
	name = new char [20];
	sprintf(name,"manager");
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)manager, 0);
	delete name;
}

void testCustomerGettingInLine(){
	initCustomerCashier();
	char* name;
	for(int i = 0; i < cashierNumber; i++){
		cashierStatus[i] = CASH_BUSY;
		privilegedLineCount[i] = 5;
		unprivilegedLineCount[i] = 5;
	}
	unprivilegedLineCount[cashierNumber-1] = 1;
	Thread * t;
	for(int i = 0; i < custNumber; i++){
		name = new char [20];
		sprintf(name,"cust%d",i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr)customer, i);
		delete name;
	}
}

void testCustomerCheckOutWithMoney(){
	initCustomerCashier();
	char* name;
	Thread* t;
	name = new char [20];
	name = "manager thread";
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)manager, 0);
	for(int i = 0; i < cashierNumber; i++){
		name = new char [20];
		sprintf(name, "cashier%d", i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr)cashier, i);
	}
	for(int i = 0; i < custNumber; i++){
		name = new char [20];
		sprintf(name,"cust%d",i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr)customer, i);
	}
}

void testCustomerCheckOutWithoutMoney(){
	initCustomerCashier();
	char* name;
	Thread* t;
	name = new char [20];
	name = "manager thread";
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)manager, 0);
	for(int i = 0; i < cashierNumber; i++){
		name = new char[20];
		sprintf(name, "cashier%d", i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr)cashier, i);
	}
	for(int i = 0; i < custNumber; i++){
		name = new char [20];
		sprintf(name,"cust%d",i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr)customer, i);
	}
	name = new char[20];

}

void testMakeCashiersBreak(){
	initCustomerCashier();
	char* name;
	Thread* t;
	name = new char[20];
	name = "manager thread";
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)manager, 0);
	for(int i = 0; i < cashierNumber; i++){
		name = new char[20];
		sprintf(name, "cashier%d", i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr)cashier, i);
	}
	t->Fork((VoidFunctionPtr)customer, 0);
	cout << "before yield" << endl;
	for(int i = 0; i < 3; i++){
		currentThread->Yield();
	}
	cout << "after yield" << endl;
	for(int i = 1; i < custNumber; i++){
		cout << "making cust" << endl;
		name = new char[20];
		sprintf(name, "cust%d", i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr)cashier, i);
	}
	while(true);
}
void Problem2(){
	cout << "Menu:" << endl;
	cout << "1. Test customer choosing from cashier lines" << endl;
	cout << "2. Test customer-cashier interaction" << endl;
	cout << "3. Test customer without enough money" << endl;
	cout << "4. Test cashiers going on break and coming off break" <<endl;
	cout << "5. Test customers entering store and getting their items from shelves" << endl;
	cout << "6. Test customers entering store and being greeted" << endl;
	// put your necessary menu options here
	cout << "Please input the number option you wish to take: " << endl;
	int choice;
	while(true){
		cin >> choice;
		if(cin.fail()){
			cin.clear();
			cin.ignore(100, '\n');
			cout << "Not a valid menu option. Please try again: ";
			continue;
		}
		else if(choice > 6 || choice < 1){ //change this if you add more options
			cout << "Not a valid menu option. Please try again: ";
			continue;
		}
		else break;
	}
	switch (choice){
	case 1:
		customerCash = 100000;
		cashierNumber = 5;
		custNumber = 6;
		testCustomerGettingInLine();
		break;
	case 2:
		customerCash = 100000;
		cashierNumber = 4;
		custNumber = 30;
		testCustomerCheckOutWithMoney();
		break;
	case 3:
		customerCash = 6;
		cashierNumber = 4;
		custNumber = 10;
		testCustomerCheckOutWithoutMoney();
		break;
	case 4:
		customerCash = 0;
		cashierNumber = 3;
		custNumber = 20;
		char* name = new char[20];
		name = "blah";
		Thread* thread = new Thread(name);
		//thread->Fork((VoidFunctionPtr)testMakeCashiersBreak, 0);
		testMakeCashiersBreak();
		break;
	case 5:
		custNumber = 12;
		customerCash = 25;
		numTrollies = 40;
		testCustomerEnteringStoreAndPickingUpItems();
		break;
	case 6:
		TestGreetingCustomer(custNumber, numSalesmen);
		break;
		//add cases here for your test
	default: break;
	}
}

