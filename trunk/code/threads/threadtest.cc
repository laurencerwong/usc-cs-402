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
#include <iostream>

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

#define MAX_SALESMEN 3
#define MAX_CUSTOMERS 60
#define NUM_ITEMS 10


// Enums for the salesmen status

enum SalesmanStatus {SALES_NOT_BUSY, SALES_BUSY, SALES_ON_BREAK, SALES_READY_TO_TALK};
SalesmanStatus currentSalesStatus[MAX_SALESMEN];

enum WhomIWantToTalkTo {GREETING, COMPLAINING, GOODSLOADER, UNKNOWN};
WhomIWantToTalkTo currentlyTalkingTo[MAX_SALESMEN];

int salesCustNumber[MAX_SALESMEN]; //The array of customer indicies that the customers update
int custNumberForSales[MAX_SALESMEN]; //Array that the salesman copies the salesCustNumber to to keep a local copy

int salesDesk[MAX_SALESMEN];

int greetingCustWaitingLineCount = 0; //How many customers need to be greeted
int complainingCustWaitingLineCount = 0; //How many customers need to be greeted

Lock *individualSalesmanLock[MAX_SALESMEN]; //The lock that each salesman uses for his own "desk"
Lock *salesLock = new Lock("Overall sales lock"); //The lock that protects the salesmen statuses and the line for all three

Condition *salesmanCV[MAX_SALESMEN]; //The condition variable for one on one interactions with the Salesman
Condition *greetingCustCV = new Condition ("Greeting Customer Condition Variable"); //The condition var that represents the line all the greeters stay in
Condition *complainingCustCV = new Condition("Complaining Customer Condition Variable"); //Represents the line that the complainers stay in

Lock *shelfLock[NUM_ITEMS];
Condition *shelfCV[NUM_ITEMS];

int shelfInventory[NUM_ITEMS];


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

enum CashierStatus {CASH_NOT_BUSY, CASH_BUSY, CASH_ON_BREAK, CASH_READY_TO_TALK};
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


void initShelves() {
	for(int i = 0; i < NUM_ITEMS; i++) {
		shelfLock[i] = new Lock("shelfLock");
		shelfCV[i] = new Condition("shelfCV");
		shelfInventory[i] = 0;
	}
}

void initShelvesWithQty(int q) {
	for(int i = 0; i < NUM_ITEMS; i++) {
		shelfLock[i] = new Lock("shelfLock");
		shelfCV[i] = new Condition("shelfCV");
		shelfInventory[i] = q;
	}
}

void initSalesmen(){
	for(int i = 0; i < MAX_SALESMEN; i++){
		currentSalesStatus[i] = SALES_NOT_BUSY;
		currentlyTalkingTo[i] = UNKNOWN;
		individualSalesmanLock[i] = new Lock(("Salesman Lock"));
		salesmanCV[i] = new Condition(("Salesman Control Variable"));
		salesCustNumber[i] = 0;
		custNumberForSales[i] = 0;
	}
}


void initCustomerCashier(){
	char* name;
	name = new char[20];
	name = "cashier line lock";
	cashierLinesLock = new Lock(name);
	privilegedCashierLineCV = new Condition*[cashierNumber];
	unprivilegedCashierLineCV = new Condition*[cashierNumber];
	cashierLock = new Lock*[cashierNumber];
	cashierToCustCV = new Condition*[cashierNumber];
	cashierStatus = new CashierStatus[cashierNumber];
	privilegedLineCount = new int[cashierNumber];
	unprivilegedLineCount = new int[cashierNumber];
	cashierDesk = new int[cashierNumber];
	for(int i = 0; i < cashierNumber; i++){
		name = new char [20];
		sprintf(name,"priv cashier line CV %d",i);
		privilegedCashierLineCV[i] = new Condition(name);
		delete name;
	}
	for(int i = 0; i < cashierNumber; i++){
		name = new char[20];
		sprintf(name, "unpriv cashier line CV %d", i);
		unprivilegedCashierLineCV[i] = new Condition(name);
		delete name;
	}
	for(int i = 0; i < cashierNumber; i++){
			name = new char[20];
			sprintf(name, "cashier lock %d", i);
			cashierLock[i] = new Lock(name);
			delete name;
		}
	for(int i = 0; i < cashierNumber; i++){
			name = new char[20];
			sprintf(name, "cashier cust CV %d", i);
			cashierToCustCV[i] = new Condition(name);
			delete name;
	}
	for(int i = 0; i < cashierNumber; i++){
			cashierStatus[i] = CASH_NOT_BUSY;
			privilegedLineCount[i] = 0;
			unprivilegedLineCount[i] = 0;
	}


}



//Customer's function

void Customer(int myIndex){

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
		qtyItemsInCart[i] = -1;
	}



	salesLock->Acquire();
	int mySalesIndex = 0;

	//Selects a salesman

	if(greetingCustWaitingLineCount > 0){
		greetingCustWaitingLineCount++;
		greetingCustCV->Wait(salesLock);
	}

	for(int i = 0; i < MAX_SALESMEN; i++){
		if(currentSalesStatus[i] == SALES_NOT_BUSY){
			mySalesIndex = i;
			currentSalesStatus[i] = SALES_BUSY;
			currentlyTalkingTo[i] = GREETING;
			break;
		}
	}

	salesLock->Release();
	individualSalesmanLock[mySalesIndex]->Acquire(); //Acquire the salesman's "desk" lock
	salesCustNumber[mySalesIndex] = myIndex; //Sets the customer number of the salesman to this customer's index
	salesmanCV[mySalesIndex]->Signal(individualSalesmanLock[mySalesIndex]);
	salesmanCV[mySalesIndex]->Wait (individualSalesmanLock[mySalesIndex]);
	
	for(int i = 0; i < numItemsToBuy; i++) {
		for(int shelfNum = 0; shelfNum < NUM_ITEMS; shelfNum++) {
			while(qtyItemsInCart[i] < qtyItemsToBuy[i]) {
				shelfLock[shelfNum]->Acquire();
				if(shelfInventory[shelfNum] != 0) {
					shelfInventory[shelfNum]--;
					itemsInCart[i]++;
					shelfLock[shelfNum]->Release();
				}
				else {	//We are out of this item, go tell sales!
					shelfLock[shelfNum]->Release();

					salesLock->Acquire();

					int mySalesID = -1;

					for(int j = 0; j < MAX_SALESMEN; j++) {
						//nobody waiting, sales free
						if(complainingCustWaitingLineCount == 0 && currentSalesStatus[j] == SALES_NOT_BUSY) {
							mySalesID = j;
							break;
						}
					}
					if(mySalesID == -1) {	//no salesmen are free, I have to wait in line
						complainingCustWaitingLineCount++;
						complainingCustCV->Wait(salesLock);

						//find the salesman who just signalled me
						salesLock->Acquire();
						for(int k = 0; k < MAX_SALESMEN; k++) {
							if(currentSalesStatus[k] == SALES_READY_TO_TALK) {
								mySalesID = k;
								break;
							}
						}
					}

					individualSalesmanLock[mySalesID]->Acquire();
					currentSalesStatus[mySalesID] = SALES_BUSY;
					salesLock->Release();

					//now proceed with interaction to tell sales we are out
					currentlyTalkingTo[mySalesID] = COMPLAINING;
					salesDesk[mySalesID] = shelfNum;

					salesmanCV[mySalesID]->Signal(individualSalesmanLock[mySalesID]);
					salesmanCV[mySalesID]->Wait(individualSalesmanLock[mySalesID]);
					individualSalesmanLock[mySalesID]->Release();
					//now i go wait on the shelf

					shelfCV[shelfNum]->Wait(shelfLock[shelfNum]);
					//now restocked, continue looping until I have all of what I need
				}
			}
		}
	}

	delete itemsToBuy;
	delete qtyItemsToBuy;
	delete itemsInCart;
	delete qtyItemsInCart;
}

//----------------------------------------------------
//customer method with cashier
//---------------------------------------------------
void customer(int myID){
	int privileged = 0;
	int myCashier; //ID of cashier I speak to
	char* type = new char[20];
	int myCash = 10000000;
	int *itemsInCart;
	int *qtyItemsInCart;
	int numItemsToBuy = 3;
	itemsInCart = new int[numItemsToBuy];
	qtyItemsInCart = new int[numItemsToBuy];

	for(int i = 0; i < numItemsToBuy; i++) {
		//TODO- Put a random item/qty picker here

		itemsInCart[i] = i
		qtyItemsInCart[i] = 3 - i;
	}
	if(privileged){
		type = "Privileged Customer";
	}
	else type = "Customer";
	cashierLinesLock->Acquire();
	printf("%s %d is looking for the cashier\n", type, myID );
	for(int i = 0; i < cashierNumber; i++ ){
		//if I find a cashier who is free, I will:
		if(cashierStatus[i] == 0){
			myCashier = i; //remember who he is
			cashierStatus[i] = CASH_BUSY; //prevent others from thinking he's free
			cashierLock[i]->Acquire(); //get his lock before I wake him up
			printf("%s %d chose Cashier %d who is free", type, myID, myCashier);
			cashierLinesLock->Release(); //allow other to view monitor vars since they can't
										//affect my operations
			break; //stop searching through lines
		}
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

			minLineValue = linesIAmLookingAt[0]; //set a default min value
			//find the minimum line value and remember the cashier associated with it
			for(int j = 1; j < cashierNumber; j++){
				if(linesIAmLookingAt[j] < minLineValue){
					//must also check if that cashier is on break
					minLineValue = linesIAmLookingAt[j];
					minCashierID = j;
				}
			}
			myCashier = minCashierID;
			printf("%s %d chose Cashier %d of line length %d\n", type, myID, myCashier, linesIAmLookingAt[minCashierID]);
			linesIAmLookingAt[minCashierID]++;
			linesIAmLookingAtCV[minCashierID]->Wait(cashierLinesLock); //wait in line
			//code after this means I have been woken up after getting to the front of the line
			cashierStatus[myCashier] = CASH_BUSY;
			cashierLock[i]->Acquire();
			printf("%s %d is now engaged with Cashier %d after waiting in line", type, myID, myCashier);
			cashierLinesLock->Release(); //allow others to view monitor variable now that I've staked
						//my claim on this cashier
			break;
		}
		cashierDesk[myCashier] = myID;
		cashierToCustCV[myCashier]->Signal(cashierLock[myCashier]);
		cashierToCustCV[myCashier]->Wait(cashierLock[myCashier]);
		//cycle through all items in my inventory, handing one item to cashier ata time
		for(int i = 0; i < numItemsToBuy; i++){
			for(int j = 0; j < numItemsToBuy; j++){
				cashierDesk[i] = itemsInCart[i]; //tells the cashier what type of item
				cashierToCustCV[myCashier]->Signal(cashierLock[myCashier]);
				cashierToCustCV[myCashier]->Wait(cashierLock[myCashier]);
			}
		}
		cashierDesk[i] = -1;
		//when I get here, the cashier has loaded my total
		//If I don't have enough money, leave the error flag -1 on the cashier's desk
		if(cashierDesk[myCashier] > myCash){
			cashierDesk[myCashier] = -1;
			cashierToCustCV[myCashier]->Signal(cashierLock[myCashier]);
			cashierToCustCV[myCashier]->Wait(cashierLock[myCashier]);
			//TODO manager-customer interaction
		}
		//if I do have money, I just need to update my cash and leave
		//the money there
		else{
			myCash -= cashierDesk[myCashier];
			//Now I wait for my receipt
			cashierToCustCV[myCashier]->Signal(cashierLock[myCashier]);
			printf("%s %d pays %d and is now waiting for receipt\n", type, myID, cashierDesk[myCashier]);
			cashierToCustCV[myCashier]->Wait(cashierLock[myCashier]);
			//now I've received my receipt and should release the cashier
			cashierStatus[myCashier] = NOT_BUSY;
			cashierLock[myCashier]->Release();
			myCashier = -1; //so I can't accidentally tamper with the cashier I chose anymore
		}
		//TODO replace trolly and leave store
	}
}


int scan(int item){
	switch(item){
	case 0: return 5;
	case 1: return 2;
	case 2: return 3;
	case 2: return 8;
	default: return 0;
	}
}

//cashier code

void cashier(int myCounter){
	while(true){
	cashierLinesLock->Acquire();
	//check if my lines have anyone in it
	if(privilegedLineCount[myCounter]){
		cashierStatus[myCounter] = BUSY;
		privilegedCashierLineCV[myCounter]->Signal(cashierLinesLock);
	}
	else if(unprivilegedLineCount[myCounter]){
		cashierStatus[myCounter] = BUSY;
		unprivilegedCashierLineCV[myCounter]->Signal(cashierLinesLock);
	}
	else cashierStatus[myCounter] = NOT_BUSY;
	//whether or not I'm ready for a customer, I can get my lock and go to sleep
	cashierLock[myCounter]->Acquire();
	cashierLinesLock->Release();
	cashierToCustCV[myCounter]->Wait(cashierLock[myCounter]);
	//the next time I'm woken up (assuming it is by a customer, not a manager
	//I will be totaling items
	if(cashierDesk[myCounter] == -1){ //case where manager wants me to take a break
		//Be sure to double check race conditions for this
		//make sure manager has set my status to on break
		cashierToCustCV[myCounter]->Wait(cashierLock[myCounter]);
		//to come back from break, manager doesn't have to signal me... it will be like i
		//had no line when checking and went to sleep
	}
	//when I get here, there will be an item to scan
	int total = 0;
	int custID = cashierDesk[myCounter];
	while(cashierDesk[myCounter] != -1){ //-1 means we're done scanning
		cout << "Cashier " << myCounter << " from the trolly of Customer " << custID << endl;
		cashierToCustCV[myCounter]->Signal(cashierLock[myCounter]);
		cashierToCustCV[myCounter]->Wait(cashierLock[myCounter]);
		total += scan(cashierDesk[myCounter]);
		//tell customer I'm ready for next item and wait
		cashierToCustCV[myCounter]->Signal(cashierLock[myCounter]);
		cashierToCustCV[myCounter]->Wait(cashierLock[myCounter]);
	}
	//now I'm done scanning, so I tell the customer the total
	cout << "Cashier " << myCounter << " tells Customer " << custID << " total cost is $" << total << endl;
	cashierDesk[myCounter] = total;
	cashierToCustCV[myCounter]->Signal(cashierLock[myCounter]);
	cashierToCustCV[myCounter]->Wait(cashierLock[myCounter]);
	if(cashierDesk == -1){
		//signal the manager
	}
	else{
		cashierToCustCV[myCounter]->Signal(cashierLock[myCounter]);
		cashierStatus = NOT_BUSY;
	}
	cashierLock[myCounter]->Release();
	//done, just need to loop back and check for more customers
	}
}



//Salesman Code

void Salesman (int myIndex){

	salesLock->Acquire();

	//Check if there is someone in line
	//and wake them up

	if(greetingCustWaitingLineCount > 0){

		greetingCustCV->Signal(salesLock);
		greetingCustWaitingLineCount--;
		currentlyTalkingTo[myIndex] = GREETING;

	}
	else{
		currentlyTalkingTo[myIndex] = UNKNOWN;
	}

	individualSalesmanLock[myIndex]->Acquire();
	salesLock->Release();
	salesmanCV[myIndex]->Wait(individualSalesmanLock[myIndex]);
	custNumberForSales[myIndex] = salesCustNumber[myIndex]; //not sure if custNumberForSales needs to be an array
}



//================
// Tests
//================

//Test for greeting the customer
void TestGreetingCustomer(int NUM_CUSTOMERS, int NUM_SALESMEN){
	Thread *t;
	char *name;

	printf ("Initializing Variables for 'Greeting Customer Test'\n");

	initSalesmen();


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
		t->Fork((VoidFunctionPtr)Salesman,i);
	}
}

void testCustomerEnteringStoreAndPickingUpItems(int numCustomers) {
	initSalesmen();
	initShelvesWithQty(10);

	char* custName;

	Thread * t;
	for(int i = 0; i < numCustomers; i++){
		custName = new char [20];
		sprintf(custName,"cust%d",i);
		t = new Thread(custName);
		t->Fork((VoidFunctionPtr)Customer, i);
		delete custName;
	}
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
	for(int i = 0; i < cashierNumber; i++){
		name = new char [20];
		sprintf(name, "cashier%d", i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr)cashier, i);
		delete name;
	}
	for(int i = 0; i < custNumber; i++){
		name = new char [20];
		sprintf(name,"cust%d",i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr)customer, i);
		delete name;
	}

}

void Problem2(){
		cout << "Menu:" << endl;
		cout << "1. Test customer-cashier" << endl;
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
			else if(choice > 1 || choice < 1){ //change this if you add more options
				cout << "Not a valid menu option. Please try again: ";
				continue;
			}
			else break;
		}
		switch (choice){
		case 1:
				cashierNumber = 5;
				custNumber = 6;
				testCustomerGettingInLine();
				break;
		case 2:
				cashierNumber = 1;
				custnumber = 1;
				testCustomerCheckoutWithMoney();
				break;
		//add cases here for your test
		default: break;
		}
}

