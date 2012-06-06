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

int maxShelfQty = 10;
int numTrollies = 40;
int numSalesmen = 3;
int numLoaders = 10;
int numItems = 10;
int numDepartments = 3;
int maxCustomers = 100;

// Enums for the salesmen status

enum SalesmanStatus {SALES_NOT_BUSY, SALES_BUSY, SALES_GO_ON_BREAK, SALES_ON_BREAK, SALES_READY_TO_TALK, SALES_READY_TO_TALK_TO_LOADER, SALES_SIGNALLING_LOADER, LOAD_GET_ITEMS_FROM_MANAGER};
SalesmanStatus **currentSalesStatus;
enum WhomIWantToTalkTo {GREETING, COMPLAINING, GOODSLOADER, SALESMAN, MANAGER, UNKNOWN};
WhomIWantToTalkTo **currentlyTalkingTo;
//WhomIWantToTalkTo *loaderCurrentlyTalkingTo = new WhomIWantToTalkTo[numLoaders];


int **salesCustNumber; //The array of customer indicies that the customers update
int **salesDesk;
int **salesBreakBoard;
Condition ***salesBreakCV;

int *greetingCustWaitingLineCount; //How many customers need to be greeted
int *complainingCustWaitingLineCount; //How many customers need to be helped with empty shelves
int *loaderWaitingLineCount;	//How many loaders are waiting on salesmen

Lock ***individualSalesmanLock; //The lock that each salesman uses for his own "desk"
Lock **salesLock; //The lock that protects the salesmen statuses and the line for all three

Condition ***salesmanCV; //The condition variable for one on one interactions with the Salesman
Condition **greetingCustCV; //The condition var that represents the line all the greeters stay in
Condition **complainingCustCV; //Represents the line that the complainers stay in
Condition **loaderCV;	//Represents the line that loaders wait in

enum LoaderStatus {LOAD_NOT_BUSY, LOAD_STOCKING, LOAD_HAS_BEEN_SIGNALLED};
LoaderStatus *loaderStatus;

Lock *inactiveLoaderLock;
Condition *inactiveLoaderCV;

queue<int> loaderJobQueue;

Lock ***shelfLock;
Condition ***shelfCV;
int **shelfInventory;

Lock *stockRoomLock;

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


//Function prototypes if needed
void Salesman(int);


void initShelves() {
	stockRoomLock = new Lock("Stock room lock");

	shelfLock = new Lock**[numDepartments];
	shelfCV = new Condition**[numDepartments];
	shelfInventory = new int*[numDepartments];

	for(int i = 0; i < numDepartments; i++) {
		shelfLock[i] = new Lock*[numItems];
		shelfCV[i] = new Condition*[numItems];
		shelfInventory[i] = new int[numItems];
	}

	for(int i = 0; i < numDepartments; i++) {
		for(int j = 0; j < numItems; j++) {
			shelfLock[i][j] = new Lock("shelfLock");
			shelfCV[i][j] = new Condition("shelfCV");
			shelfInventory[i][j] = 0;
		}
	}
}

void initShelvesWithQty(int q) {
	cout << "Initializing shelves with qty: " << q << endl;

	stockRoomLock = new Lock("Stock room lock");

	shelfLock = new Lock**[numDepartments];
	shelfCV = new Condition**[numDepartments];
	shelfInventory = new int*[numDepartments];

	for(int i = 0; i < numDepartments; i++) {
		shelfLock[i] = new Lock*[numItems];
		shelfCV[i] = new Condition*[numItems];
		shelfInventory[i] = new int[numItems];
	}

	for(int i = 0; i < numDepartments; i++) {
		for(int j = 0; j < numItems; j++) {
			shelfLock[i][j] = new Lock("shelfLock");
			shelfCV[i][j] = new Condition("shelfCV");
			shelfInventory[i][j] = q;
		}
	}
}

void initSalesmen(){
	cout << "Sales init level 1..." << endl;
	currentSalesStatus = new SalesmanStatus*[numDepartments];
	currentlyTalkingTo = new WhomIWantToTalkTo*[numDepartments];
	salesCustNumber = new int*[numDepartments];
	salesDesk = new int*[numDepartments];
	salesBreakBoard = new int*[numDepartments];
	salesBreakCV = new Condition**[numDepartments];
	greetingCustWaitingLineCount = new int[numDepartments]; //How many customers need to be greeted
	complainingCustWaitingLineCount = new int[numDepartments]; //How many customers need to be helped with empty shelves
	loaderWaitingLineCount = new int[numDepartments];	//How many loaders are waiting on salesmen

	individualSalesmanLock = new Lock**[numDepartments]; //The lock that each salesman uses for his own "desk"
	salesLock = new Lock*[numDepartments]; //The lock that protects the salesmen statuses and the line for all three

	salesmanCV = new Condition**[numDepartments]; //The condition variable for one on one interactions with the Salesman

	greetingCustCV = new Condition*[numDepartments];
	complainingCustCV = new Condition*[numDepartments];
	loaderCV = new Condition*[numDepartments];

	cout << "Sales init level 2..." << endl;
	for(int i = 0; i < numDepartments; i++) {
		currentSalesStatus[i] = new SalesmanStatus[numSalesmen];
		currentlyTalkingTo[i] = new WhomIWantToTalkTo[numSalesmen];
		salesCustNumber[i] = new int[numSalesmen];
		salesDesk[i] = new int[numSalesmen];
		salesBreakBoard[i] = new int[numSalesmen];
		salesBreakCV[i] = new Condition*[numSalesmen];

		individualSalesmanLock[i] = new Lock*[numSalesmen];
		salesLock[i] = new Lock("Overall salesman lock");

		salesmanCV[i] = new Condition*[numSalesmen]; //The condition variable for one on one interactions with the Salesman

		greetingCustCV[i] = new Condition ("Greeting Customer Condition Variable"); //The condition var that represents the line all the greeters stay in
		complainingCustCV[i] = new Condition("Complaining Customer Condition Variable"); //Represents the line that the complainers stay in
		loaderCV[i] = new Condition("GoodsLoader Condition Variable");	//Represents the line that loaders wait in

		greetingCustWaitingLineCount[i] = 0;
		complainingCustWaitingLineCount[i] = 0;
		loaderWaitingLineCount[i] = 0;
	}

	cout << "Sales init level 3..." << endl;
	for(int i = 0; i < numDepartments; i++) {
		for(int j = 0; j < numSalesmen; j++){
			currentSalesStatus[i][j] = SALES_BUSY;
			currentlyTalkingTo[i][j] = UNKNOWN;
			salesCustNumber[i][j] = 0;
			salesBreakBoard[i][j] = 0;
			individualSalesmanLock[i][j] = new Lock(("Indivisdual Salesman Lock"));
			salesmanCV[i][j] = new Condition(("Salesman Condition Variable"));
			salesBreakCV[i][j] = new Condition("Salesman Break CV");
		}
	}
	cout << "Done initializing sales" << endl;
}

void initLoaders() {
	cout << "Initializing loaders..." << endl;
	inactiveLoaderLock = new Lock("Lock for loaders waiting to be called on");
	inactiveLoaderCV = new Condition("CV for loaders waiting to be called on");
	loaderStatus = new LoaderStatus[numLoaders];

	for(int i = 0; i < numLoaders; i++){
		loaderStatus[i] = LOAD_NOT_BUSY;
	}
}

void initTrolly() {
	cout << "Initializing trollies..." << endl;

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
	cout << "Initializing CustomerCashier..." << endl;

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
	name = new char[20];
	name = "inactive loader cv";
	inactiveLoaderCV = new Condition(name);
	name = new char[20];
	name = "inactive loader lock";
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

int constructSalesArg(int dept, int id) {	// 16 bit number: [15:8] are dept num and [7:0] are id num
	int val = 0;
	val = (dept << 8) | id;
	return val;
}

void deconstructSalesArg(int val, int target[2]) {
	int dept = 0;
	int id = 0;

	dept = (val & 0x0000ff00) >> 8;
	id = (val & 0x000000ff);

	target[0] = id;
	target[1] = dept;
}

void createSalesmen(int numDepts, int numSalesPerDept) {

	cout << "starting to create salesmen" << endl;
	int salesID = 0;

	for(int i = 0; i < numDepts; i++) {
		for(int j = 0; j < numSalesPerDept; j++) {
			Thread * t;
			char* name;
			int arg = 0;

			cout << "preparing to construct... " << i << " " << j << endl;
			salesID = j;
			arg = constructSalesArg(i, salesID);
			name = new char [20];
			sprintf(name,"sales%d",i);
			t = new Thread(name);
			t->Fork((VoidFunctionPtr)Salesman, arg);
			delete name;

			cout << "created sales with dept: " << i << " and ID: " << salesID << endl;
			//salesID++;
		}
	}
}

void customer(int id) {

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

	char* type = new char[20];
	int myCash = customerCash;
	int privileged;

	//---------Randomly generate whether this customer is privileged--------------
	srand(myID + time(NULL));
	int r = rand() % 10; //random value to set Customer either as privileged or unprivileged
	if(r < 2){//30% chance customer is privileged
		privileged = 1;
	}
	else privileged = 0; //70% chance this customer is unprivileged

	//set char array for I/O purposes
	if(privileged){
		type = "PrivilegedCustomer";
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




	int targetDepartment = -1;
	int currentDepartment = -1;

	for(int i = 0; i < numItemsToBuy; i++) {	//goes through everything on our grocery list

		targetDepartment = getDepartmentFromItem(itemsToBuy[i]);

		if(targetDepartment != currentDepartment) {

			salesLock[targetDepartment]->Acquire();
			int mySalesIndex = -1;

			//printf("cust %d is about to select salesman, greeting line is: %d\n", myID,  greetingCustWaitingLineCount[targetDepartment]);

			//Selects a salesman
			int someoneIsFree = -1;
			for(int j = 0; j < numSalesmen; j++) {
				if(currentSalesStatus[targetDepartment][j] == SALES_NOT_BUSY) {
					someoneIsFree = j;
					mySalesIndex = j;
					currentSalesStatus[targetDepartment][j] = SALES_BUSY;
					currentlyTalkingTo[targetDepartment][j] = GREETING;
					break;
				}
			}

			if(someoneIsFree == -1){	//no one is free
				greetingCustWaitingLineCount[targetDepartment]++;
				cout << type << " [" << myID << "] gets in line for the Department [" << targetDepartment << "]" << endl;
				greetingCustCV[targetDepartment]->Wait(salesLock[targetDepartment]);

				for(int j = 0; j < numSalesmen; j++){
					if(currentSalesStatus[targetDepartment][j] == SALES_READY_TO_TALK){
						mySalesIndex = j;
						currentSalesStatus[targetDepartment][j] = SALES_BUSY;
						currentlyTalkingTo[targetDepartment][j] = GREETING;
						break;
					}
				}
			}
			/*Rob has removed this else {	//someone is free
				//printf("There was no line for cust %d\n", myID);
				for(int j = 0; j < numSalesmen; j++){
					if(currentSalesStatus[targetDepartment][j] == SALES_NOT_BUSY){
						mySalesIndex = j;
						currentSalesStatus[targetDepartment][j] = SALES_BUSY;
						currentlyTalkingTo[targetDepartment][j] = GREETING;
						break;
					}
				}
			}*/

			//cout << "cust " << myID << " about to get desk lock" << endl;

			individualSalesmanLock[targetDepartment][mySalesIndex]->Acquire(); //Acquire the salesman's "desk" lock
			cout << type << " [" << myID << "] is interacting with DepartmentSalesman[" << mySalesIndex << "] of Department[" << targetDepartment << "]" << endl;
			salesLock[targetDepartment]->Release();
			salesCustNumber[targetDepartment][mySalesIndex] = myID; //Sets the customer number of the salesman to this customer's index
			//cout << "cust " << myID << " about to signal salesman " << mySalesIndex << endl;

			/*for(int j = 0; j < numSalesmen; j++) {
				cout << "sales status " << currentSalesStatus[targetDepartment][j] << "  sales talk " << currentlyTalkingTo[targetDepartment][j] << endl;
			}*/

			salesmanCV[targetDepartment][mySalesIndex]->Signal(individualSalesmanLock[targetDepartment][mySalesIndex]);
			//cout << "cust " << myID << " waiting for response from sales " << mySalesIndex << endl;
			salesmanCV[targetDepartment][mySalesIndex]->Wait (individualSalesmanLock[targetDepartment][mySalesIndex]);
			//cout << "cust " << myID << " starts shopping!" << endl;
			individualSalesmanLock[targetDepartment][mySalesIndex]->Release();

		}

		currentDepartment = targetDepartment;

		//BEGINS SHOPPING

		for(int shelfNum = 0; shelfNum < numItems; shelfNum++) {
			if(shelfNum != itemsToBuy[i]) {
				continue;
			}

			cout << type << " ["<< myID << "] wants to buy [" << shelfNum << "]-[" << qtyItemsToBuy[i] << "]" << endl;
			while(qtyItemsInCart[i] < qtyItemsToBuy[i]) {
				shelfLock[currentDepartment][shelfNum]->Acquire();
				if(shelfInventory[currentDepartment][shelfNum] > qtyItemsToBuy[i]) {
					cout << type << " ["<< myID << "] has found ["
						 << shelfNum << "] and placed [" << qtyItemsToBuy[i] << "] in the trolly" << endl;


					shelfInventory[currentDepartment][shelfNum] -= qtyItemsToBuy[i];
					itemsInCart[i] = shelfNum;
					qtyItemsInCart[i] += qtyItemsToBuy[i];
					shelfLock[currentDepartment][shelfNum]->Release();
					//cout << "Think I need: " << qtyItemsToBuy[i] << "  Got: " << qtyItemsInCart[i] << endl;
				}
				else {	//We are out of this item, go tell sales!
					cout << type << " [" << myID << "] was not able to find item " << shelfNum <<
							" and is searching for department salesman " << currentDepartment << endl;	//TODO dept num
					shelfLock[currentDepartment][shelfNum]->Release();		//BREAK THE SQUARE
					salesLock[currentDepartment]->Acquire();	//BREAK THE SQUARE

					int mySalesID = -1;

					for(int j = 0; j < numSalesmen; j++) {
						//nobody waiting, sales free
						if(currentSalesStatus[currentDepartment][j] == SALES_NOT_BUSY) {
							mySalesID = j;
							break;
						}
					}
					if(mySalesID == -1) {	//no salesmen are free, I have to wait in line
						//cout << "Customer " << myID << " gets in line for a salesman in department " << currentDepartment << endl;
						complainingCustWaitingLineCount[currentDepartment]++;
						complainingCustCV[currentDepartment]->Wait(salesLock[currentDepartment]);

						//find the salesman who just signalled me

						for(int k = 0; k < numSalesmen; k++) {
							if(currentSalesStatus[currentDepartment][k] == SALES_READY_TO_TALK) {
								mySalesID = k;
								break;
							}
						}
					}

					individualSalesmanLock[currentDepartment][mySalesID]->Acquire();
					cout << type << " [" << myID << "] is asking for assistance "
							"from DepartmentSalesman [" << mySalesID << "]" << endl;
					currentSalesStatus[currentDepartment][mySalesID] = SALES_BUSY;
					salesCustNumber[currentDepartment][mySalesID] = myID;	//
					salesLock[currentDepartment]->Release();

					//now proceed with interaction to tell sales we are out
					currentlyTalkingTo[currentDepartment][mySalesID] = COMPLAINING;
					salesDesk[currentDepartment][mySalesID] = shelfNum;

					salesmanCV[currentDepartment][mySalesID]->Signal(individualSalesmanLock[currentDepartment][mySalesID]);
					salesmanCV[currentDepartment][mySalesID]->Wait(individualSalesmanLock[currentDepartment][mySalesID]);	//wait for sales to tell me to wait on shelf
					shelfLock[currentDepartment][shelfNum]->Acquire();
					individualSalesmanLock[currentDepartment][mySalesID]->Release();
					//now i go wait on the shelf
				/*	cout << type << " [" <<  myID << "] is now waiting for item [" <<
							shelfNum << "] to be restocked" << endl;
							*/
					shelfCV[currentDepartment][shelfNum]->Wait(shelfLock[currentDepartment][shelfNum]);
					cout << "DepartmentSalesman [" << mySalesID << "] informs the " << type << " [" << myID << "] that [" << shelfNum << "] is restocked." << endl;
					//now restocked, continue looping until I have all of what I need
					cout << type << " [" <<  myID << "] has received assistance about restocking of item [" <<
							shelfNum << "] from DepartmentSalesman [" << mySalesID << "]" << endl;
					shelfLock[currentDepartment][shelfNum]->Release();
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

	//--------------Begin looking for a cashier-------------------------------------
	cashierLinesLock->Acquire(); //acquire locks to view line counts and cashier statuses
	do{
	////printf("%s [%d] is looking for the Cashier.\n", type, myID );

	//Find if a cashier is free (if one is, customer doesn't need to wait in line)
	for(int i = 0; i < cashierNumber; i++ ){
		//if I find a cashier who is free, I will:
		if(cashierStatus[i] == CASH_NOT_BUSY){
			myCashier = i; //remember who he is
			cashierLock[i]->Acquire(); //get his lock before I wake him up
			cashierStatus[i] = CASH_BUSY; //prevent others from thinking he's free
			////printf("%s [%d] chose Cashier [%d] with line of length [0].\n", type, myID, myCashier);
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
			////printf("%s [%d] chose Cashier [%d] of line length [%d].\n", type, myID, myCashier, linesIAmLookingAt[minCashierID]);
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
		////printf("%s [%d] cannot pay [%d]\n", type, myID, cashierDesk[myCashier]);
		cashierDesk[myCashier] = -1;
		cashierToCustCV[myCashier]->Signal(cashierLock[myCashier]);
		cashierToCustCV[myCashier]->Wait(cashierLock[myCashier]);
		//TODO manager-customer interaction
		managerLock->Acquire();
		cashierLock[myCashier]->Release();
		////printf("%s [%d] is waiting for Manager for negotiations\n", type, myID);
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
				////printf("%s [%d] tells Manager to remove [%d] from trolly.\n", type, myID, itemsInCart[i]);
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
		////printf("%s [%d] pays [%d] to Manager after removing items and is waiting for receipt from Manager.\n", type, myID, amountOwed);
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
		////printf("%s [%d] pays [%d] to Cashier [%d] and is now waiting for receipt.\n", type, myID, cashierDesk[myCashier], myCashier);
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
	cout << "customer " << myID << " is replacing his trolly, and disp count is: " << displacedTrollyCount << " while " << trollyCount << " are still available" << endl;
	displacedTrollyCount++;
	displacedTrollyLock->Release();
	//-----------------------------End replace trolly----------------------

	customersDone++;
	
	cout << "customer " << myID << " has finished, and so far " << customersDone << " are done" << endl;
	
	//cleanup
	delete itemsToBuy;
	delete qtyItemsToBuy;
	delete itemsInCart;
	delete qtyItemsInCart;
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
	int* numSalesmenOnBreak = new int[numDepartments];


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
		if((unsigned int)numFullLines > (cashierNumber - cashiersOnBreak.size()) && cashiersOnBreak.size()){ //bring back cashier if there are more lines with 3 customers than there are cashiers and if there are cashiers on break

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
		if( chance == 1  && cashiersOnBreak.size() < (unsigned int)cashierNumber -2){ //.001% chance of sending cashier on break
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

		//__SALES_BREAK__
		//-----------------------------Begin bringing salesmen back from break-------------

		int dept = 0;

		for(int i = 0; i < numDepartments; i++) {
			salesLock[dept]->Acquire();
			if((greetingCustWaitingLineCount[dept] + complainingCustWaitingLineCount[dept] + loaderWaitingLineCount[dept]) > 0 && numSalesmenOnBreak[dept]){
				int arg = salesmenOnBreak.front();
				int targets[2];
				deconstructSalesArg(arg, targets);
				int wakeSalesman = targets[0];
				dept = targets[1];

				if(currentSalesStatus[dept][wakeSalesman] == SALES_ON_BREAK){
					salesBreakBoard[dept][wakeSalesman] = 0;
					cout << "Manager brings back Cashier " << wakeSalesman << " from break." << endl;
					salesBreakCV[dept][wakeSalesman]->Signal(salesLock[dept]);
					numSalesmenOnBreak[dept]--;
					salesmenOnBreak.pop();
				}
			}
			salesLock[dept]->Release();
		}
		//------------------------------end bringing salesmen back from break--------------

		//------------------------------Begin putting salesmen on break------------------
		dept = rand() % numDepartments;
		salesLock[dept]->Acquire();
		if (chance == 1 && numSalesmenOnBreak[dept] < numSalesmen -1) {
			int r = rand() % numSalesmen;
			if(!salesBreakBoard[dept][r] && currentSalesStatus[dept][r] != SALES_ON_BREAK && currentSalesStatus[dept][r] != SALES_GO_ON_BREAK) {
				salesBreakBoard[dept][r] = 1;
				cout << "Manager sends Salesman " << r << " on break." << endl;
				individualSalesmanLock[dept][r]->Acquire();
				//if(currentlyTalkingTo[dept][r] == UNKNOWN){
				if(currentSalesStatus[dept][r] == SALES_NOT_BUSY) {
					salesmanCV[dept][r]->Signal(individualSalesmanLock[dept][r]);
					currentSalesStatus[dept][r] = SALES_ON_BREAK;
				}
				individualSalesmanLock[dept][r]->Release();
				salesmenOnBreak.push(constructSalesArg(dept, r));
				numSalesmenOnBreak[dept]++;
			}
		}
		salesLock[dept]->Release();
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
		////cout << "Cashier [" << myCounter << "] is going on break." << endl;
		cashierStatus[myCounter] = CASH_ON_BREAK;
		cashierLock[myCounter]->Acquire();
		cashierLinesLock->Release();
		cashierToCustCV[myCounter]->Wait(cashierLock[myCounter]);
		////cout << "Cashier [" << myCounter << "] was called from break by Manager to work." << endl;
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


		////cout << "Cashier [" << myCounter << "] got [" << cashierDesk[myCounter] << "] from trolly of " << custType << " [" << custID << "]." << endl;
		total += scan(cashierDesk[myCounter]);
		cashierToCustCV[myCounter]->Signal(cashierLock[myCounter]);
		cashierToCustCV[myCounter]->Wait(cashierLock[myCounter]);
	}
	//now I'm done scanning, so I tell the customer the total
	////cout << "Cashier [" << myCounter << "] tells " << custType << " [" << custID << "] total cost is $[" << total << "]." << endl;
	cashierDesk[myCounter] = total;
	cashierToCustCV[myCounter]->Signal(cashierLock[myCounter]);

	cashierToCustCV[myCounter]->Wait(cashierLock[myCounter]);
	if(cashierDesk[myCounter] == -1){
		////cout << "Cashier [" << myCounter << "] asks " << custType << " [" << custID << "] to wait for Manager." << endl;
		////cout << "Cashier [" << myCounter << "] informs the Manager that " << custType << " [" << custID << "] does not have enough money." << endl;
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
		////cout << " total " << total << endl;
		cashierFlags[myCounter] = total; //inform manager of the total the customer owes
		managerCV->Signal(managerLock); //wake up manager, who was waiting for this information
		//when I am woken up, the manager has taken over so I can free myself for my
		//next customer
	}
	else{
		//add value to cash register
		cashRegister[myCounter] += cashierDesk[myCounter];
		////cout << "Cashier [" << myCounter << "] got money $[" << cashierDesk[myCounter] << "] from " << custType << " [" << custID << "]." << endl;
		//giving the customer a receipt
		////cout << "Cashier [" << myCounter << "] gave the receipt to " << custType << " [" << custID << "] and tells him to leave" << endl;
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
//void Salesman (int myIndex){
void Salesman(int arg) {
	int argTargets[2];
	deconstructSalesArg(arg, argTargets);
	int myIndex = argTargets[0];
	int myDept = argTargets[1];


	cout << "Salesman has just been created in dept: " << myDept << " with id: " << myIndex << endl;

	while(true) {
		salesLock[myDept]->Acquire();

		//cout << "running sales sched" << endl;

		//Check if there is someone in line
		//and wake them up

		if(salesBreakBoard[myDept][myIndex] == 1) {	//go on break
			cout << "Sales " << myIndex << " in department " << myDept << " going on break" << endl;
			SalesmanStatus prev = currentSalesStatus[myDept][myIndex];
			currentSalesStatus[myDept][myIndex] = SALES_ON_BREAK;
			salesBreakBoard[myDept][myIndex] = 0;
			salesBreakCV[myDept][myIndex]->Wait(salesLock[myDept]);
			currentSalesStatus[myDept][myIndex] = prev;
			cout << "Sales " << myIndex << " in department " << myDept << " back from break" << endl;
		}

		if(greetingCustWaitingLineCount[myDept] > 0){
			cout << "Salesman going to greet cust" << endl;
			currentSalesStatus[myDept][myIndex] = SALES_READY_TO_TALK;
			greetingCustCV[myDept]->Signal(salesLock[myDept]);
			greetingCustWaitingLineCount[myDept]--;
			//currentlyTalkingTo[myDept][myIndex] = GREETING;
		}
		else if(loaderWaitingLineCount[myDept] > 0) {
			currentSalesStatus[myDept][myIndex] = SALES_READY_TO_TALK_TO_LOADER;
			cout << "Signalling loader waiting" << endl;
			loaderCV[myDept]->Signal(salesLock[myDept]);
			loaderWaitingLineCount[myDept]--;
		}
		else if(complainingCustWaitingLineCount[myDept] > 0) {
			cout << "Salesman going to help a complaing cust" << endl;
			currentSalesStatus[myDept][myIndex] = SALES_READY_TO_TALK;
			complainingCustCV[myDept]->Signal(salesLock[myDept]);
			complainingCustWaitingLineCount[myDept]--;
		}

		else{
			cout << "not busy" << endl;
			currentlyTalkingTo[myDept][myIndex] = UNKNOWN;
			currentSalesStatus[myDept][myIndex] = SALES_NOT_BUSY;
		}

		individualSalesmanLock[myDept][myIndex]->Acquire();
		//cout << "got my individual lock" << endl;
		salesLock[myDept]->Release();
		//cout << "sales " << myIndex << " waiting for someone to come up to me" << endl;

		salesmanCV[myDept][myIndex]->Wait(individualSalesmanLock[myDept][myIndex]);	//Wait for cust/loader to walk up to me?	STUCK HERE
		//cout << "Someone " << currentlyTalkingTo[myDept][myIndex] << " came up to salesman " << myIndex << endl;

		if(currentlyTalkingTo[myDept][myIndex] == GREETING) {
			//individualSalesmanLock[myDept][myIndex]->Acquire();
			//salesLock[myDept]->Release();
			//salesmanCV[myDept][myIndex]->Wait(individualSalesmanLock[myDept][myIndex]);	//Wait for cust to walk up to me?
			int myCustNumber = salesCustNumber[myDept][myIndex]; //not sure if custNumberForSales needs to be an array
			if(privCustomers[myCustNumber] == 1){
				cout << "DepartmentSalesman [" << myIndex << "] welcomes PrivilegeCustomer [" << myCustNumber << "] to Department [" << myDept << "]." << endl;
			}
			else{
				cout << "DepartmentSalesman [" << myIndex << "] welcomes Customer [" << myCustNumber << "] to Department [" << myDept << "]." << endl;
			}
			salesmanCV[myDept][myIndex]->Signal(individualSalesmanLock[myDept][myIndex]);
			individualSalesmanLock[myDept][myIndex]->Release();	//???
			//cout << "just signalled on index: " << myIndex << endl;
		}
		else if(currentlyTalkingTo[myDept][myIndex] == COMPLAINING) {
			//individualSalesmanLock[myDept][myIndex]->Acquire();
			//salesLock[myDept]->Release();
			//salesmanCV[myDept][myIndex]->Wait(individualSalesmanLock[myDept][myIndex]);	//Wait for cust to walk up to me?
			int myCustNumber = salesCustNumber[myDept][myIndex]; //not sure if custNumberForSales needs to be an array
			int itemOutOfStock = salesDesk[myDept][myIndex];

			if(privCustomers[myCustNumber] == 1){
				cout << "DepartmentSalesman [" << myIndex << "] is informed by PrivilegeCustomer [" << myCustNumber << "] that [" << itemOutOfStock << "] is out of stock." << endl;
			}
			else{
				cout << "DepartmentSalesman [" << myIndex << "] is informed by Customer [" << myCustNumber << "] that [" << itemOutOfStock << "] is out of stock." << endl;
			}
			individualSalesmanLock[myDept][myIndex]->Acquire();	//TODO check order of this and release prev/next line
			salesmanCV[myDept][myIndex]->Signal(individualSalesmanLock[myDept][myIndex]);	//tell cust to wait

			//TODO tell goods loader
			salesLock[myDept]->Acquire();
			cout << "salesman " << myIndex << " in dept " << myDept << " is trying to get sales lock to signal GL" << endl;
			salesDesk[myDept][myIndex] = itemOutOfStock;	//Might not be necessary, because we never really took it off the desk
			//salesLock[myDept]->Release();
			cout << "salesman " << myIndex << " in dept " << myDept << " is signalling for a loader" << endl;

			//salesLock[myDept]->Acquire();
			if(loaderWaitingLineCount[myDept] > 0) {	//get a loader from line
				loaderWaitingLineCount[myDept]--;
				currentSalesStatus[myDept][myIndex] = SALES_READY_TO_TALK_TO_LOADER;
				cout << "salesman " << myIndex << " in dept " << myDept << " found a loader in line (length " << loaderWaitingLineCount[myDept] << ")" << endl;
				loaderCV[myDept]->Signal(salesLock[myDept]);	//get a loader from line
				salesLock[myDept]->Release();
			}
			else {	// no one in line, go to inactive
				currentSalesStatus[myDept][myIndex] = SALES_SIGNALLING_LOADER;
				salesLock[myDept]->Release();

				inactiveLoaderLock->Acquire();
				cout << "salesman " << myIndex << " in dept " << myDept << " is is getting a loader from inactive" << endl;
				inactiveLoaderCV->Signal(inactiveLoaderLock);	//call a loader over?
				inactiveLoaderLock->Release();
			}

			salesmanCV[myDept][myIndex]->Wait(individualSalesmanLock[myDept][myIndex]);	//wait for the OK! from loader
			cout << "salesman " << myIndex << " in dept " << myDept << " got a loader" << endl;
			//check to see if a loader already restocked something

			if(salesDesk[myDept][myIndex] != -1) {	//loader had finished stocking something too
				int itemRestocked = salesDesk[myDept][myIndex];
				cout << "I found a restocked order! for item " << itemRestocked << endl;
				salesmanCV[myDept][myIndex]->Signal(individualSalesmanLock[myDept][myIndex]);
				salesmanCV[myDept][myIndex]->Wait(individualSalesmanLock[myDept][myIndex]);
				int loaderNumber = salesDesk[myDept][myIndex];
				salesmanCV[myDept][myIndex]->Signal(individualSalesmanLock[myDept][myIndex]);
				cout << "DepartmentSalesman [" << myIndex << "] is informed by the GoodsLoader [" << loaderNumber << "] that [" << itemRestocked << "] is restocked." << endl;
				shelfLock[myDept][itemRestocked]->Acquire();
				cout << "DepartmentSalesman [" << myIndex << "] has acquired shelfLock for restocked iterm" << endl;
				shelfCV[myDept][itemRestocked]->Broadcast(shelfLock[myDept][itemRestocked]);
				shelfLock[myDept][itemRestocked]->Release();
				//individualSalesmanLock[myDept][myIndex]->Release();
				//DepartmentSalesman [identifier] informs the Customer/PrivilegeCustomer [identifier] that [item] is restocked.
			}


			//int myLoaderID = -100;


			inactiveLoaderLock->Acquire();
			//individualSalesmanLock[myDept][myIndex]->Release();
			int myLoaderID = -1;

			//for(int i = 0; i < numLoaders; i++) {
			//	cout << "Load status: " << loaderStatus[i] << endl;
			//}

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
			salesmanCV[myDept][myIndex]->Signal(individualSalesmanLock[myDept][myIndex]);
			individualSalesmanLock[myDept][myIndex]->Release();

		}
		else if(currentlyTalkingTo[myDept][myIndex] == GOODSLOADER) {
			//individualSalesmanLock[myIndex]->Acquire();
			//salesLock->Release();
			//salesmanCV[myIndex]->Wait(individualSalesmanLock[myIndex]);	//Wait for cust/loader to walk up to me?

			int itemRestocked = salesDesk[myDept][myIndex];
			salesmanCV[myDept][myIndex]->Signal(individualSalesmanLock[myDept][myIndex]);
			salesmanCV[myDept][myIndex]->Wait(individualSalesmanLock[myDept][myIndex]);
			int loaderNumber = salesDesk[myDept][myIndex];
			salesmanCV[myDept][myIndex]->Signal(individualSalesmanLock[myDept][myIndex]);
			cout << "DepartmentSalesman [" << myIndex << "] is informed by the GoodsLoader [" << loaderNumber << "] that [" << itemRestocked << "] is restocked." << endl;
			shelfLock[myDept][itemRestocked]->Acquire();
			cout << "DepartmentSalesman [" << myIndex << "] has acquired shelfLock for restocked iterm" << endl;
			shelfCV[myDept][itemRestocked]->Broadcast(shelfLock[myDept][itemRestocked]);
			shelfLock[myDept][itemRestocked]->Release();
			individualSalesmanLock[myDept][myIndex]->Release();
			//DepartmentSalesman [identifier] informs the Customer/PrivilegeCustomer [identifier] that [item] is restocked.
		}
		else{
			individualSalesmanLock[myDept][myIndex]->Release();
		}
	}
}


//Goods loader code			__LOADER__
void GoodsLoader(int myID) {
	int currentDept = -1;	//the department i am dealing with
	int mySalesID = -1;		//the sales i am helping
	int shelf = -1;			//the shelf i am dealing with


	bool salesNeedsMoreLoaders = false;	//unused
	bool foundNewOrder = false;	//true if i go to help a salesman, and he was signalling for a loader to restock something

	inactiveLoaderLock->Acquire();
	//normal action loop
	while(true) {
		//if(!salesNeedsMoreLoaders) {

		if(!foundNewOrder) {	//if i don't have a new order (from my last run) go to sleep
			loaderStatus[myID] = LOAD_NOT_BUSY;
			if(mySalesID != -1){
				cout << "GoodsLoader [" << myID << "] is waiting for orders to restock." << endl;
			}
			inactiveLoaderCV->Wait(inactiveLoaderLock);
			//cout << "Loader " << myID << " has been summoned" << endl;
			//at this point, I have just been woken up from the inactive loaders waiting area
		}
		foundNewOrder = false;	//initialize that I have not found a new order for this run yet

		loaderStatus[myID] = LOAD_HAS_BEEN_SIGNALLED;
		mySalesID = -50;
		inactiveLoaderLock->Release();

		//look through all departments to find out who signalled me
		for(int j = 0; j < numDepartments; j++) {
			salesLock[j]->Acquire();
			//cout << "checking dept " << j << endl;
			for(int i = 0; i < numSalesmen; i++) {
				if(currentSalesStatus[j][i] == SALES_SIGNALLING_LOADER) {	//i found a person who was signalling for a loader!
					mySalesID = i;
					currentDept = j;
					currentSalesStatus[currentDept][mySalesID] = SALES_BUSY;
					break;
				}
			}
			salesLock[j]->Release();
			if(mySalesID != -50) {	//used to break the second level of for loop if i found a salesman who needs me
				break;
			}
		}

		if(mySalesID == -50){ //the loader was signaled by the manager to get trollys (signalled, but no salesmen are signalling for me)
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

			//moves trollies back to the front of the store
			displacedTrollyLock->Acquire();
			int restoreTrollies = 0;
			if(displacedTrollyCount > 0){
				restoreTrollies = displacedTrollyCount;
				displacedTrollyCount = 0;
			}
			displacedTrollyLock->Release();

			if(restoreTrollies != 0) {
				trollyLock->Acquire();
				trollyCount += restoreTrollies;
				cout << "goods loader " << myID << " is replacing " << restoreTrollies << " so now there are " << trollyCount << endl;
				trollyCV->Broadcast(trollyLock);	//inform everyone that there are now more trollies in the front
				trollyLock->Release();
			}

		}
		else{	//It was not the manager who signalled me
			individualSalesmanLock[currentDept][mySalesID]->Acquire();
			shelf = salesDesk[currentDept][mySalesID];	//read the shelf that needs stocking in from the desk
			salesDesk[currentDept][mySalesID] = -1;		//write back that i was not on a previous job

			cout << "GoodsLoader [" << myID << "] is informed by DepartmentSalesman [" << mySalesID <<
					"] of Department [" << currentDept << "] to restock [" << shelf << "]" << endl;

			salesmanCV[currentDept][mySalesID]->Signal(individualSalesmanLock[currentDept][mySalesID]);	//tell him i'm here
			salesmanCV[currentDept][mySalesID]->Wait(individualSalesmanLock[currentDept][mySalesID]);
			individualSalesmanLock[currentDept][mySalesID]->Release();


			//Restock items
			int qtyInHands = 0;
			//for(int i = 0; i < maxShelfQty; i++) {
			shelfLock[currentDept][shelf]->Acquire();
			while(shelfInventory[currentDept][shelf] < maxShelfQty) {
				shelfLock[currentDept][shelf]->Release();

				//currentLoaderInStockLock->Acquire();
				if(currentLoaderInStock == -1){
					currentLoaderInStock = myID;
					//cout << "GoodsLoader [" << myID << "] is setting the currentLoaderInLock" << endl;
				}
				else{
					cout << "GoodsLoader [" << myID << "] is waiting for GoodsLoader [" << currentLoaderInStock << "] to leave the StockRoom." << endl;
				}
				//Simulates a store room
				stockRoomLock->Acquire();
				//currentLoaderInStockLock->Release();
				cout << "GoodsLoader [" << myID << "] is in the StockRoom and got [" << shelf << "]" << endl;
				qtyInHands++;
				stockRoomLock->Release();
				cout << "GoodsLoader [" << myID << "] leaves StockRoom." << endl;
				currentLoaderInStockLock->Acquire();
				currentLoaderInStock = -1;
				currentLoaderInStockLock->Release();
				/*
				 * 				currentLoaderInStockLock->Acquire();
				currentLoaderInStock = -1; //lets other goodsloaders change it
				currentLoaderInStockLock->Release();
				 */
				//cout << "about to yield" << endl;
				for(int j = 0; j < 5; j++) {
					currentThread->Yield();
				}

				//check the shelf i am going to restock
				shelfLock[currentDept][shelf]->Acquire();
				if(shelfInventory[currentDept][shelf] == maxShelfQty) {
					cout << "GoodsLoader [" << myID << "] has restocked [" << shelf << "] in Department [" << currentDept << "]." << endl;
					qtyInHands = 0;
					break;
				}
				shelfInventory[currentDept][shelf] += qtyInHands;
				qtyInHands = 0;
			}
			shelfLock[currentDept][shelf]->Release();


			//We have finished restocking.  now wait in line/inform sales
			salesLock[currentDept]->Acquire();
			cout << "loader " << myID << " I have acquired saleslock and am awaiting to notify a salesman with info " << currentDept << " " << shelf << endl;
			for(int j = 0; j < numSalesmen; j++) {
				cout << "current sales status in department " << currentDept << " are: Sales #" << j << " is status " << currentSalesStatus[currentDept][j] << endl;
			}
			mySalesID = -50;
			//first look for someone who is signalling for a loader, since we can multipurpose and take a new job, while also informing that we finished a previous one
			for(int i = 0; i < numSalesmen; i++) {
				if(currentSalesStatus[currentDept][i] == SALES_SIGNALLING_LOADER) {
					//all salesmen are busy trying to get loaders, but the loaders are out and trying to tell them that a job was finished
					mySalesID = i;
					individualSalesmanLock[currentDept][mySalesID]->Acquire();
					//Ready to go talk to sales

					int newShelfToStock = salesDesk[currentDept][mySalesID];

					cout << "Goods loader " << myID << " has acquired individualSalesmanLock with dept and sales id: " << currentDept << " " << mySalesID << endl;
					currentlyTalkingTo[currentDept][mySalesID] = GOODSLOADER;
					currentSalesStatus[currentDept][mySalesID] = SALES_BUSY;
					salesLock[currentDept]->Release();
					salesDesk[currentDept][mySalesID] = shelf;
					salesmanCV[currentDept][mySalesID]->Signal(individualSalesmanLock[currentDept][mySalesID]);
					salesmanCV[currentDept][mySalesID]->Wait(individualSalesmanLock[currentDept][mySalesID]);
					salesDesk[currentDept][mySalesID] = myID;
					salesmanCV[currentDept][mySalesID]->Signal(individualSalesmanLock[currentDept][mySalesID]);
					individualSalesmanLock[currentDept][mySalesID]->Release();

					shelf = newShelfToStock;
					foundNewOrder = true;
					break;
				}
				/*else if(currentSalesStatus[currentDept][i] == SALES_NOT_BUSY) {
					cout << "salesman d-i " << currentDept << " " << i << "wasn't busy" << endl;
					mySalesID = i;
					break;
				}*/
			}
			if(mySalesID == -50) {
				//no one was signalling, so look for free salesmen to go report to
				for(int i = 0; i < numSalesmen; i++) {
					if(currentSalesStatus[currentDept][i] == SALES_NOT_BUSY) {
						cout << "salesman d-i " << currentDept << " " << i << "wasn't busy" << endl;
						mySalesID = i;
						//Ready to go talk to sales
						individualSalesmanLock[currentDept][mySalesID]->Acquire();
						cout << "Goods loader " << myID << " has acquired individualSalesmanLock" << endl;
						currentlyTalkingTo[currentDept][mySalesID] = GOODSLOADER;
						currentSalesStatus[currentDept][mySalesID] = SALES_BUSY;
						salesLock[currentDept]->Release();
						salesDesk[currentDept][mySalesID] = shelf;
						salesmanCV[currentDept][mySalesID]->Signal(individualSalesmanLock[currentDept][mySalesID]);
						salesmanCV[currentDept][mySalesID]->Wait(individualSalesmanLock[currentDept][mySalesID]);
						salesDesk[currentDept][mySalesID] = myID;
						salesmanCV[currentDept][mySalesID]->Signal(individualSalesmanLock[currentDept][mySalesID]);
						individualSalesmanLock[currentDept][mySalesID]->Release();
						break;
					}
				}
			}


			if(!foundNewOrder) {	//i have to get in line, because i didn't find a new order from someone signalling
									//(i would have given them my information when i took their order)
				if(mySalesID == -50) {	//if i have STILL not found anyone, then i do need to get in line
					loaderWaitingLineCount[currentDept]++;
					cout << "loader " << myID << " about to wait for a salesman in department " << currentDept << endl;

					loaderCV[currentDept]->Wait(salesLock[currentDept]);//get in line		//STUCK HERE

					for(int j = 0; j < numSalesmen; j++) {
						cout << "Salesman " << j << " status for dept: " << currentDept << " is: " << currentSalesStatus[currentDept][j] << "  loader " << myID << " was checking" << endl;
					}

					for(int i = 0; i < numSalesmen; i++) {	//find the salesman who signalled me out of line
						if(currentSalesStatus[currentDept][i] == SALES_READY_TO_TALK_TO_LOADER) {
							mySalesID = i;

							//Ready to go talk to a salesman
							cout << "Goods loader " << myID << " has acquired individualSalesmanLock" << endl;
							currentlyTalkingTo[currentDept][mySalesID] = GOODSLOADER;
							currentSalesStatus[currentDept][mySalesID] = SALES_BUSY;
							individualSalesmanLock[currentDept][mySalesID]->Acquire();
							salesLock[currentDept]->Release();
							salesDesk[currentDept][mySalesID] = shelf;
							salesmanCV[currentDept][mySalesID]->Signal(individualSalesmanLock[currentDept][mySalesID]);
							salesmanCV[currentDept][mySalesID]->Wait(individualSalesmanLock[currentDept][mySalesID]);
							salesDesk[currentDept][mySalesID] = myID;
							salesmanCV[currentDept][mySalesID]->Signal(individualSalesmanLock[currentDept][mySalesID]);
							individualSalesmanLock[currentDept][mySalesID]->Release();

							break;
						}
					}
					cout << "loader " << myID << " was woken up by sales d-i " << currentDept << " " << mySalesID << endl;
				}
			}
		}

		//Look at all depts
		for(int j = 0; j < numDepartments; j++) {
			salesLock[j]->Acquire();
			//cout << "checking dept " << j << endl;
			for(int i = 0; i < numSalesmen; i++) {
				//cout << "i " << i << "  j " << j << endl;
				if(currentSalesStatus[j][i] == SALES_SIGNALLING_LOADER) {
					foundNewOrder = true;
					break;
				}
			}
			salesLock[j]->Release();
			if(foundNewOrder) {
				break;
			}
		}


		//MIGHT*** need an if statement or condition arond this relating to the found new order business
		//int tempSalesID = -1;
		inactiveLoaderLock->Acquire();
		/*for(int j = 0; j < numDepartments; j++) {
			salesLock[j]->Acquire();
			for(int i = 0; i < numSalesmen; i++) {
				if(currentSalesStatus[j][i] == SALES_SIGNALLING_LOADER) {
					tempSalesID = i;
					break;
				}
			}
			salesLock[j]->Release();
			if(tempSalesID != -1) {
				break;
			}
		}*/

		/*
		if(tempSalesID == -1) {
			salesNeedsMoreLoaders = false;
		}
		else {
			salesNeedsMoreLoaders = true;
		}
		*/
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

void testCreatingSalesmenWithDepartments() {
	cout << "initializing salesmen..." << endl;
	initSalesmen();
	cout << "creating salesmen..." << endl;
	createSalesmen(numDepartments, numSalesmen);
}

void testCustomerEnteringStoreAndPickingUpItems() {
	initSalesmen();
	initShelvesWithQty(30);
	initLoaders();
	initCustomerCashier();
	initTrolly();


	char* name;

	Thread * t;
	/*for(int i = 0; i < numSalesmen; i++){
		name = new char [20];
		sprintf(name,"sales%d",i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr)Salesman, i);
		delete name;
	}*/
	createSalesmen(numDepartments, numSalesmen);
	for(int i = 0; i < custNumber; i++){
		name = new char [20];
		sprintf(name,"cust%d",i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr)Customer, i);
	}
	for(int i = 0; i < cashierNumber; i++) {
		name = new char [20];
		sprintf(name,"cash%d",i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr)cashier, i);
	}
	for(int i = 0; i < numLoaders; i++) {
		name = new char [20];
		sprintf(name,"loader%d",i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr)GoodsLoader, i);
	}
	name = new char [20];
	sprintf(name,"manager");
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)manager, 0);

	cout << "Done creating threads" << endl;
}

void testCustomerGettingInLine(){
	testNumber = 1;
	customerCash = 100000;
	cashierNumber = 5;
	custNumber = 10;
	numSalesmen = 1;
	numDepartments = 1;
	numTrollies = 20;
	numLoaders = 1;
	initCustomerCashier();
	char* name;
	cout << "Test 1: please note that all cashier lines are set to be 5 except for cashier 4's line for unprivileged customers, which is set to 1 ";
	cout << " and cashier 3's line for unprivileged customer, which is set to 3.";
	cout << "\tThe privileged status of Customers is set randomly, so we will create enough ";
	cout << "customer threads to likely fill up a line and start choosing other lines" << endl;
	initShelves();
	initShelvesWithQty(30);
	initSalesmen();
	initLoaders();
	initTrolly();
	for(int i = 0; i < cashierNumber; i++){
		cashierStatus[i] = CASH_BUSY;
		privilegedLineCount[i] = 5;
		unprivilegedLineCount[i] = 5;
	}
	unprivilegedLineCount[cashierNumber-1] = 1;
	unprivilegedLineCount[cashierNumber-2] = 3;
	Thread * t;
	name = new char[20];
	name = "sales0";
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)Salesman, 0);
	for(int i = 0; i < custNumber; i++){
		name = new char [20];
		sprintf(name,"cust%d",i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr)Customer, i);
	}
}

void testCustLeavesAfterReceiptAndCashierHandlingOneAtATime(){
	testNumber = 3;
	customerCash = 10000;
	cashierNumber = 5;
	custNumber = 20;
	numSalesmen = 5;
	numDepartments = 1;
	numTrollies = 20;
	numLoaders = 1;
	initCustomerCashier();
	initShelves();
	initShelvesWithQty(30);
	initSalesmen();
	initLoaders();
	initTrolly();
	char* name;
	Thread* t;
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
		t->Fork((VoidFunctionPtr)Customer, i);
	}
	createSalesmen(1, 5);
}

void testCustomerCheckOutWithoutMoney(){
	testNumber = 2;
	customerCash = 0;
	cashierNumber = 5;
	custNumber = 5;
	numSalesmen = 5;
	numDepartments = 1;
	numTrollies = 20;
	numLoaders = 1;
	initCustomerCashier();
	initShelves();
	initShelvesWithQty(30);
	initSalesmen();
	initLoaders();
	initTrolly();
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
		t->Fork((VoidFunctionPtr)Customer, i);
	}
	createSalesmen(1, 5);
}

void testCashiersScanUntilTrollyIsEmpty(){
	testNumber = 4;
	customerCash = 10000;
	cashierNumber = 1;
	custNumber = 1;
	numSalesmen = 1;
	numDepartments = 1;
	numTrollies = 20;
	numLoaders = 1;
	initCustomerCashier();
	initShelves();
	initShelvesWithQty(30);
	initSalesmen();
	initLoaders();
	initTrolly();
	char* name;
	Thread* t;
	name = new char[20];
	sprintf(name, "cashier%d", 0);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)cashier, 0);
	name = new char [20];
	sprintf(name,"cust%d",0);
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)Customer, 0);
	createSalesmen(1, 1);
}
void testPutCashiersOnBreak(){
	testNumber = 5;
	initCustomerCashier();
	customerCash = 10000;
	cashierNumber = 5;
	custNumber = 1;
	numSalesmen = 1;
	numDepartments = 1;
	numTrollies = 20;
	numLoaders = 1;
	testNumber = 5;
	initCustomerCashier();
	initShelves();
	initShelvesWithQty(30);
	initSalesmen();
	initLoaders();
	initTrolly();
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
}

void testBringCashiersBackFromBreak(){
	testNumber = 6;
	customerCash = 10000;
	cashierNumber = 5;
	custNumber = 30;
	numSalesmen = 5;
	numDepartments = 1;
	numTrollies = 20;
	numLoaders = 1;
	initCustomerCashier();
	initShelves();
	initShelvesWithQty(100);
	initSalesmen();
	initLoaders();
	initTrolly();
	char* name;
	Thread* t;
	initCustomerCashier();
	createSalesmen(numDepartments, numSalesmen);
	for(int i = 0; i < custNumber; i++){
		name = new char [20];
		sprintf(name,"cust%d",i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr)Customer, i);
	}
	for(int i = 0; i < numLoaders; i++) {
		name = new char [20];
		sprintf(name,"loader%d",i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr)GoodsLoader, i);
	}
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

}

void testRevenueAlwaysTheSame(){
	testNumber = 7;
	testCustomerEnteringStoreAndPickingUpItems();
}

void testGoodsLoadersCustomersDontFightOverShelves(){
	testNumber = 8;
	maxShelfQty = 100;
	customerCash = 10000;
	cashierNumber = 5;
	custNumber = 100;
	numSalesmen = 1;
	numDepartments = 1;
	numTrollies = 30;
	numLoaders = 1;
	initCustomerCashier();
	initShelves();
	initShelvesWithQty(0);
	initSalesmen();
	initLoaders();
	initTrolly();
	char* name;
	Thread* t;
	initCustomerCashier();
	createSalesmen(numDepartments, numSalesmen);
	for(int i = 0; i < custNumber; i++){
		name = new char [20];
		sprintf(name,"cust%d",i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr)Customer, i);
	}
	for(int i = 0; i < numLoaders; i++) {
		name = new char [20];
		sprintf(name,"loader%d",i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr)GoodsLoader, i);
	}
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

}

void testOneLoaderInStockRoom(){
	testNumber = 9;
	maxShelfQty = 100;
	customerCash = 10000;
	cashierNumber = 5;
	custNumber = 4;
	numSalesmen = 4;
	numDepartments = 1;
	numTrollies = 30;
	numLoaders = 5;
	initCustomerCashier();
	initShelves();
	initShelvesWithQty(0);
	initSalesmen();
	initLoaders();
	initTrolly();
	char* name;
	Thread* t;
	initCustomerCashier();
	createSalesmen(numDepartments, numSalesmen);
	for(int i = 0; i < custNumber; i++){
		name = new char [20];
		sprintf(name,"cust%d",i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr)Customer, i);
	}
	for(int i = 0; i < numLoaders; i++) {
		name = new char [20];
		sprintf(name,"loader%d",i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr)GoodsLoader, i);
	}
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
}

void testCustWaitForRestock(){
	testNumber = 10;
	testNumber = 10;
	maxShelfQty = 1;
	customerCash = 10000;
	cashierNumber = 5;
	custNumber = 4;
	numSalesmen = 4;
	numDepartments = 1;
	numTrollies = 30;
	numLoaders = 1;
	initCustomerCashier();
	initShelves();
	initShelvesWithQty(0);
	initSalesmen();
	initLoaders();
	initTrolly();
	char* name;
	Thread* t;
	initCustomerCashier();
	createSalesmen(numDepartments, numSalesmen);
	for(int i = 0; i < custNumber; i++){
		name = new char [20];
		sprintf(name,"cust%d",i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr)Customer, i);
	}
	for(int i = 0; i < numLoaders; i++) {
		name = new char [20];
		sprintf(name,"loader%d",i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr)GoodsLoader, i);
	}
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
}

void Problem2(){
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
		else if(choice > 11 || choice < 1){ //change this if you add more options
			cout << "Not a valid menu option. Please try again: ";
			continue;
		}
		else break;
	}
	cin >> choice;
	//choice = 6;
	switch (choice){
	case 1:
		testCustomerGettingInLine();
		break;
	case 2:
		testCustomerCheckOutWithoutMoney();
		break;
	case 3:

		customerCash = 100000;
		cashierNumber = 4;
		custNumber = 30;
		testCustLeavesAfterReceiptAndCashierHandlingOneAtATime();
		break;
	case 4:
		testCashiersScanUntilTrollyIsEmpty();
		break;
	case 5:
		testPutCashiersOnBreak();
		break;
	case 6:
		testBringCashiersBackFromBreak();
		break;
	case 7:
		testRevenueAlwaysTheSame();
		break;
	case 8:
		testGoodsLoadersCustomersDontFightOverShelves();
		break;
	case 9:
		testOneLoaderInStockRoom();
		break;
	case 10:
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
		cout <<  numSalesmen << endl;
		cout << cashierNumber << endl;
		cout << numLoaders << endl;
		testCustomerEnteringStoreAndPickingUpItems();
		break;


		//add cases here for your test
	default: break;
	}
}
