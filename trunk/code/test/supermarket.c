#include "../userprog/syscall.h"

/* Prototyping functions */
void Salesman();
void Customer();
void manager();
void cashier();
void GoodsLoader();



#define MAX_CASHIERS 5
#define MAX_SALESMEN 3
#define MAX_LOADERS 5
#define MAX_DEPARTMENTS 5
#define MAX_CUSTOMERS 100
#define MAX_MANAGER_ITEMS 100
#define MAX_ITEMS 10
#define MAX_LOADER_JOB_QUEUE 100
#define MAX_ITEMS_TO_BUY 30
#define NULL 0


int maxShelfQty = 15;
int numTrollies = 40;
int numSalesmen = 3;
int numLoaders = 10;
int numItems = 10;
int numDepartments = 3;

/*  Enums for the salesmen status */

typedef enum {SALES_NOT_BUSY, SALES_BUSY, SALES_GO_ON_BREAK, SALES_ON_BREAK, SALES_READY_TO_TALK, SALES_READY_TO_TALK_TO_LOADER, SALES_SIGNALLING_LOADER, LOAD_GET_ITEMS_FROM_MANAGER} SalesmanStatus;
SalesmanStatus currentSalesStatus [MAX_DEPARTMENTS][MAX_SALESMEN];
typedef enum {GREETING, COMPLAINING, GOODSLOADER, SALESMAN, MANAGER, UNKNOWN} WhomIWantToTalkTo;
WhomIWantToTalkTo currentlyTalkingTo [MAX_DEPARTMENTS][MAX_SALESMEN];

/*Static allocation of cashier variables */

typedef enum {PRIVILEGED_CUSTOMER, CUSTOMER} CustomerType;
CustomerType custType [MAX_CASHIERS];

int total [MAX_CASHIERS];
int custID [MAX_CASHIERS];

int cashierLinesLock;
/* will be grabbed when a privileged customer checks line lengths/cashier status */


/* array of CVs for privileged customers waiting in line */
/* one for each cashier */
int privilegedCashierLineCV[MAX_CASHIERS];

/* array of CVs for unprivileged customers waiting in line */
/* one for each cashier */
int unprivilegedCashierLineCV[MAX_CASHIERS];
 
/* Lock associated with cashToCustCV */
/* controls access to each cashier */
int cashierLock[MAX_CASHIERS];
/* array of CVs, one for each cashier, that the cashier and */
/* customer use to communicate during checkout */
int cashierToCustCV[MAX_CASHIERS];

int numCashiers = 0;

typedef enum {CASH_NOT_BUSY, CASH_BUSY, CASH_ON_BREAK, CASH_GO_ON_BREAK, CASH_READY_TO_TALK} CashierStatus;
/* array of current cashier statuses */
CashierStatus cashierStatus[MAX_CASHIERS];

/* array of privileged line counts */
int privilegedLineCount[MAX_CASHIERS];

/* number of privileged customers */
int privCustomers[MAX_CUSTOMERS];
/* array of unprivileged line counts */
int unprivilegedLineCount[MAX_CASHIERS];

/* array of ints representing the item a customer has handed to a cashier */
int cashierDesk[MAX_CASHIERS];

/* array representing money in each cash register */
int cashRegister[MAX_CASHIERS];

/* next cashier index for the locks */
int nextCashierIndex = 0;

int cashierIndexLock;

/* temporary global variable */
int customerCash = 20;

/* number of cashiers */
int cashierNumber = 5;

/* number of customers */
int custNumber = 20;



/*Statically allocating things for the salesman*/
int nextSalesmanIndex = 0;

int salesmanIndexLock;
int departmentIndexLock;

int nextDepartmenIndex = 0;

int salesCustNumber [MAX_DEPARTMENTS][MAX_SALESMEN]; /* The array of customer indicies that the customers update */
int salesDesk [MAX_DEPARTMENTS][MAX_SALESMEN];
int salesBreakBoard [MAX_DEPARTMENTS][MAX_SALESMEN];
int salesBreakCV[MAX_DEPARTMENTS][MAX_SALESMEN];

int greetingCustWaitingLineCount[MAX_DEPARTMENTS]; /* How many customers need to be greeted */
int complainingCustWaitingLineCount[MAX_DEPARTMENTS]; /* How many customers need to be helped with empty shelves */
int loaderWaitingLineCount[MAX_DEPARTMENTS];	/* How many loaders are waiting on salesmen */

int individualSalesmanLock[MAX_DEPARTMENTS][MAX_SALESMEN]; /* The lock that each salesman uses for his own "desk" */
int salesLock[MAX_DEPARTMENTS]; /* The lock that protects the salesmen statuses and the line for all three */


int salesmanCV[MAX_DEPARTMENTS][MAX_SALESMEN]; /* The condition variable for one on one interactions with the Salesman */
int greetingCustCV[MAX_DEPARTMENTS]; /* The condition var that represents the line all the greeters stay in */
int complainingCustCV[MAX_DEPARTMENTS]; /* Represents the line that the complainers stay in */
int loaderCV[MAX_DEPARTMENTS];	/* Represents the line that loaders wait in */

typedef enum {LOAD_NOT_BUSY, LOAD_STOCKING, LOAD_HAS_BEEN_SIGNALLED} LoaderStatus;
LoaderStatus loaderStatus[MAX_LOADERS];

int inactiveLoaderLock;

int inactiveLoaderCV;

int shelfLock [MAX_DEPARTMENTS][MAX_ITEMS];

int shelfCV [MAX_DEPARTMENTS][MAX_ITEMS];

int shelfInventory [MAX_DEPARTMENTS][MAX_ITEMS];

int stockRoomLock;

int nextLoaderIndex = 0;
int loaderIndexLock;

/* Manager data */
int cashierFlags[MAX_CASHIERS]; /* will be set to a customer ID in the slot corresponding to the */
/* index of the cashier who sets the flag. */

int managerLock;
int managerCV;
int managerDesk = 0; /* will be place customer puts items to show to manager */

/* manager stores items taken from customer so they can be taken by Loader */
/* lock to protect managerItems; */
int managerItems;
int managerItemsLock;

int cashierTotals[MAX_CASHIERS];
int numSalesmenOnBreak[MAX_DEPARTMENTS];

/* Trolleys */
int trollyCount;

/* Lock controlling access to trollyCount; */
int trollyLock;

/* CV for customers to wait on if no trollies are there */
/* associated with trollyLock */
int trollyCV;

/* Discarded trollies, waiting to be replaced by Goods Loader */
int displacedTrollyCount = 0;

/* Lock controlling access to displaced trolly count */
int displacedTrollyLock;

/* which goodsloader is currently in the stockroom */
int currentLoaderInStock = -1;

/* Lock for currentLockInStock */
int currentLoaderInStockLock;

/* Control variable for current loader in stock */
int currentLoaderInStockCV;

/* Control variable for waiting for the stock room */
int stockRoomCV;

/* Number of goodsloaders waiting for the stockroom */
int waitingForStockRoomCount = 0;

/* Which customers are privileged or not */

int customersDone = 0;

int testNumber = -1;

int numCashiersOnBreak = 0;

/* Function prototypes if needed */

/*Statically allocate customer stuff*/
int nextCustomerIndex = 0;
int customerIndexLock;




void initCashierArrays(int numCashiersIn){
    int i;
  cashierLinesLock  = CreateLock("cashierLineLock", sizeof("cashierLineLock"));
  cashierIndexLock = CreateLock("cashierIndexLock", sizeof("cashierIndexLock"));
	for( i = 0; i < numCashiersIn; i++){
		total[i] = 0;
		custID[i] = 0;
		unprivilegedCashierLineCV[i] = CreateCondition("unprivilegedCashierLineCV", sizeof("privilegedCashierLineCV"));
		privilegedCashierLineCV[i] = CreateCondition("privilegedCashierLineCV", sizeof("privilegedCashierLineCV"));
		cashierLock[i] = CreateLock("cashierLock", sizeof("cashierLock"));
		cashierToCustCV[i] = CreateCondition("cashierToCustCV", sizeof("cashierToCustCV"));
		privilegedLineCount[i] = 0;
		unprivilegedLineCount[i] = 0;
		cashierDesk[i] = 0;
		cashRegister[i] = 0;
	}
	NPrint("Finished initializing cashier arrays\n", sizeof("Finished initializing cashier arrays\n"));
}

void initSalesmanArrays(){
  int i, j;
  salesmanIndexLock = CreateLock("salesmanIndexLock", sizeof("salsemanIndexLock"));
  departmentIndexLock = CreateLock("departmentIndexLock", sizeof("departmentIndexLock"));
	for( i = 0; i < numDepartments; i++){
		greetingCustWaitingLineCount[i] = 0;
		complainingCustWaitingLineCount[i] = 0;
		loaderWaitingLineCount[i] = 0;
		salesLock[i] = CreateLock("salesLock", sizeof("salesLock"));
		greetingCustCV[i] = CreateCondition("greetingCustCV", sizeof("greetingCustCV"));
		complainingCustCV[i] = CreateCondition("complainingCustCV", sizeof("complainingCustCV"));
		loaderCV[i] = CreateCondition("loaderCV", sizeof("loaderCV"));
		for( j = 0; j < numSalesmen; j++){
			salesCustNumber[i][j] = 0;
			salesDesk[i][j] = 0;
			salesBreakBoard[i][j] = 0;
			salesBreakCV[i][j] = CreateCondition("salesBreakCV", sizeof("salesBreakCV"));
			individualSalesmanLock[i][j] = CreateLock("individualSalesmanLock", sizeof("individualSalesmanLock"));
			salesmanCV[i][j] = CreateCondition("salesmanCV", sizeof("salesmanCV"));
			currentSalesStatus[i][j] = SALES_BUSY;
			currentlyTalkingTo[i][j] = UNKNOWN;
		}
	}
	NPrint("Finished initializing salesmen arrays\n", sizeof("Finished initializing salesmen arrays\n"));
}

void initLoaderArrays(){
    int i, j;
  inactiveLoaderLock = CreateLock("inactiveLoaderLock", sizeof("inactiveLoaderLock"));
  inactiveLoaderCV = CreateCondition("inactiveLoaderCV", sizeof("inactiveLoaderCV"));
  stockRoomLock = CreateLock("stockRoomLock", sizeof("stockRoomLock"));
  loaderIndexLock = CreateLock("loaderIndexLock", sizeof("loaderIndexLock"));
  currentLoaderInStockLock = CreateLock("currentLoaderInStockLock", sizeof("currentLoaderInStockLock"));
  currentLoaderInStockCV = CreateCondition("currentLoaderInStockCV", sizeof("currentLoaderInStockCV"));
  stockRoomCV = CreateCondition("stockRoomCV", sizeof("stockRoomCV"));
  for( i = 0; i < numDepartments; i++)	{
    for( j = 0; j < numItems; j++){
      shelfLock[i][j] = CreateLock("shelfLock", sizeof("shelfLock"));
      shelfCV[i][j] = CreateCondition("shelfCV", sizeof("shelfCV"));
      shelfInventory[i][j] = 10; /* default inventory */
    }
  }
  NPrint("finished initializing loader arrays\n", sizeof("finished initializing loader arrays\n"));
}

void initLoaderArraysWithQty(){
  int i, j;
	for( i = 0; i < numDepartments; i++)	{
		for( j = 0; j < numItems; j++){
			shelfLock[i][j] = CreateLock("shelfLock", sizeof("shelfLock"));
			shelfCV[i][j] = CreateCondition("shelfCV", sizeof("shelfCV"));
			shelfInventory[i][j] = 10; /* default inventory */
		}
	}
	NPrint("Finished initializing loader arrays with qty\n", sizeof("Finished initializing loader arrays with qty\n"));
}

void initManagerArrays(){
  int i;
  managerLock = CreateLock("managerLock", sizeof("managerLock"));
  managerCV = CreateCondition("managerCV", sizeof("managerCV"));
  managerItemsLock = CreateLock("managerItemsLock", sizeof("managerItemsLock"));
  managerItems = CreateQueue();
        for( i = 0; i < numCashiers; i++){
          cashierTotals[i] = 0;
        }
        for( i = 0;i < numDepartments; i++){
          numSalesmenOnBreak[i] = 0;
        }
	NPrint("Finished initializing manager arrays\n", sizeof("Finished initializing manager arrays\n"));
}

void initCustomerArrays(){
  int i, j;
  customerIndexLock = CreateLock("customerIndexLock", sizeof("customerIndexLock"));
  NPrint("Customer index lock is %d\n", sizeof("Customer index lock is %d\n"), customerIndexLock);
  trollyLock  = CreateLock("trollyLock", sizeof("trollyLock"));
  trollyCount = numTrollies;
  trollyCV = CreateCondition("trollyCV", sizeof("trollyCV"));
  displacedTrollyLock = CreateLock("displacedTrollyLock", sizeof("displacedTrollyLock"));
  NPrint("finished initializing customer arrays\n", sizeof("finished initializing customer arrays\n"));
}


/* gets the department that a given item will be in */


int getDepartmentFromItem(int itemNum) {
	return itemNum % numDepartments;
}

/* constructs an argument (for a salesman) that combines 2 numbers into one, here for dept and id */
int constructSalesArg(int dept, int id) {	/*  16 bit number: [15:8] are dept num and [7:0] are id num */
	int val = 0;
	val = (dept << 8) | id;
	return val;
}

/* takes a value made by the constructSalesArg function and extracts the department and id from it, putting tem at the target location */
void deconstructSalesArg(int val, int target[2]) {
	int dept = 0;
	int id = 0;

	dept = (val & 0x0000ff00) >> 8;
	id = (val & 0x000000ff);

	target[0] = id;
	target[1] = dept;
}
/*
/* Creates salesmen threads given the total number of departments in the store, and the number of salesmen per department */
void createSalesmen(int numDepts, int numSalesPerDept) {
  /* cout << "starting to create salesmen" << endl; */
  int salesID = 0;
  int i, j;
	for(i = 0; i < numDepts; i++) {
		for(j = 0; j < numSalesPerDept; j++) {
		  /* Fork(Salesman); */
		}
	}
}


/*Customer's function		/* __CUST__*/ 
void Customer(){

  int myID, r, targetDepartment, currentDepartment, mySalesIndex, someoneIsFree,
    i, j, k, shelfNum, mySalesID, myCashier, minLineValue, minCashierID,
    *linesIAmLookingAt, *linesIAmLookingAtCV, amountOwed;
  int numItemsToBuy = 3;
  int itemsToBuy[MAX_ITEMS_TO_BUY];
  int qtyItemsToBuy[MAX_ITEMS_TO_BUY];
  int itemsInCart[MAX_ITEMS_TO_BUY];
  int qtyItemsInCart[MAX_ITEMS_TO_BUY];
  int myCash, type;
 
   
   
   Acquire(customerIndexLock);
   myID = nextCustomerIndex;
   nextCustomerIndex++;
   Release(customerIndexLock);
   for(i = 0; i < MAX_ITEMS_TO_BUY; i++){
	   itemsToBuy[i] = -1;
	   qtyItemsToBuy[i] = -1;
	   itemsInCart[i] = -1;
	   qtyItemsInCart[i] = -1;
   }
   /*choose the items we want to buy */

	/* setup some initialization for specific tests */
	if(testNumber == 8 || testNumber ==10){
		numItemsToBuy = 1;
		myCash = customerCash;
	}
	else if(testNumber != -1 && testNumber !=4 ){
		numItemsToBuy = 2;
		myCash = customerCash;
	}
	else{
		numItemsToBuy = (NRand() % numItems);
		myCash = NRand() % 200;
	}

	/* ---------NRandomly generate whether this customer is privileged-------------- */
	NSrand(myID + NTime(NULL));
	r = NRand() % 10; /* NRandom value to set Customer either as privileged or unprivileged */
	if(r < 2){				/* 30% chance customer is privileged */
           type = 1;
	}
	else /*privileged = 0;	/* 70% chance this customer is unprivileged*/
           type = 0;

	/* set char array for I/O purposes */
	/* --------------End of privileged/unprivileged decion-------------------------- */

	/* Decide what to buy */
	if(testNumber != -1){
		for( i = 0; i < numItemsToBuy; i++) {
		  itemsToBuy[i] = i;
			qtyItemsToBuy[i] = 2;
			itemsInCart[i] = -1;
			qtyItemsInCart[i] = 0;
		}
	}
	else{
		for ( i = 0; i < numItemsToBuy; i++){
                  itemsToBuy[i] = getDepartmentFromItem(NRand() % numItems);
			qtyItemsToBuy[i] = (NRand()% numItems);
			itemsInCart[i] = -1;
			qtyItemsInCart[i] = 0;
		}
	}
	if(type){
	  NPrint("Privileged Customer [%d] will be buying:\n", sizeof("Privileged Customer [%d] will be buying:\n"), myID, 0);
	}
	else{
	  NPrint("Customer [%d] will be buying:\n", sizeof("Customer [%d] will be buying:\n"), myID, 0);
	}
	NPrint("Item - Qty\n", sizeof("Item - Qty\n"), 0, 0);
	for(j = 0; j < numItemsToBuy; j++){
	  NPrint("  %d - %d\n", sizeof("  %d - %d\n"), NEncode2to1(itemsToBuy[j], qtyItemsToBuy[j]), 0);
	}

	/* ENTERS STORE */


	if(type){
	  NPrint("Privileged Customer [%d] enters the SuperMarket\n", sizeof("Privileged Customer [%d] enters the SuperMarket\n"), myID, 0);
	  NPrint("Privileged Customer [%d] wants to buy [%d] no. of items\n", sizeof("Privileged Customer [%d] wants to buy [%d] no. of items\n"), NEncode2to1(myID, numItemsToBuy), 0);
	}
	else{
	  NPrint("Customer [%d] enters the SuperMarket\n", sizeof("Customer [%d] enters the SuperMarket\n"), myID, 0);
	  NPrint("Customer [%d] wants to buy [%d] no. of items\n", sizeof("Customer [%d] wants to buy [%d] no. of items\n"), NEncode2to1(myID, numItemsToBuy), 0);
	}
	  
	
       Acquire(trollyLock);
       while(trollyCount == 0){
	 if(type){
	   NPrint("Privileged Customer [%d] gets in line or a trolly\n", sizeof("Privileged Customer [%d] gets in line or a trolly\n"), myID, 0);
	 }
	 else{
	   NPrint("Customer [%d], gets in line for a trolly\n", sizeof("Customer [%d], gets in line for a trolly\n"), myID, 0);
	 }
           Wait(trollyCV, trollyLock);
       }
       trollyCount--;
       Release(trollyLock);

	 targetDepartment = -1;
	 currentDepartment = -1;

	 for( i = 0; i < numItemsToBuy; i++) {	/* goes through everything on our grocery list */

		targetDepartment = getDepartmentFromItem(itemsToBuy[i]);	/* selects a department */

		/* if necessary, change departments */
		if(targetDepartment != currentDepartment) {
		  if(type){
		    NPrint("Privileged Customer [%d] has finished shopping in department [%d]\n", sizeof("Privileged Customer [%d] has finished shopping in department [%d]\n"), NEncode2to1(myID, currentDepartment), 0);
		    NPrint("Privileged Customer [%d] wants to shop in department [%d]\n", sizeof("Privileged Customer [%d] wants to shop in department [%d]\n"), NEncode2to1(myID, targetDepartment), 0);
		  }
		  else{
		    NPrint("Customer [%d] has finished shopping in department [%d]\n", sizeof("Customer [%d] has finished shopping in department [%d]\n"), NEncode2to1(myID, currentDepartment), 0);
		    NPrint("Customer [%d] wants to shop in department [%d]\n", sizeof("Customer [%d] wants to shop in department [%d]\n"), NEncode2to1(myID, targetDepartment), 0);
		  }
                       Acquire(salesLock[targetDepartment]);
			 mySalesIndex = -1;


			/* Selects a salesman */
			 someoneIsFree = -1;
			for( j = 0; j < numSalesmen; j++) {
				if(currentSalesStatus[targetDepartment][j] == SALES_NOT_BUSY) {
					someoneIsFree = j;
					mySalesIndex = j;
					currentSalesStatus[targetDepartment][j] = SALES_BUSY;
					currentlyTalkingTo[targetDepartment][j] = GREETING;
					break;
				}
			}

			if(someoneIsFree == -1){	/* no one is free, so wait in the greeting line */
				greetingCustWaitingLineCount[targetDepartment]++;
				if(type){
				  NPrint("Privileged Customer [%d] gets in line for the Department [%d]\n", sizeof("Privileged Customer [%d] gets in line for the Department [%d]\n"), NEncode2to1(myID, targetDepartment), 0);
				}
				else{				
				  NPrint("Customer [%d] gets in line for the Department [%d]\n", sizeof("Customer [%d] gets in line for the Department [%d]\n"), NEncode2to1(myID, targetDepartment), 0);
				}
                               Wait(greetingCustCV[targetDepartment], salesLock[targetDepartment]);
				for( j = 0; j < numSalesmen; j++){
					if(currentSalesStatus[targetDepartment][j] == SALES_READY_TO_TALK){
						mySalesIndex = j;
						currentSalesStatus[targetDepartment][j] = SALES_BUSY;
						currentlyTalkingTo[targetDepartment][j] = GREETING;
						break;
					}
				}
			}

                       Acquire(individualSalesmanLock[targetDepartment][mySalesIndex]);
		       if(type){
			 NPrint("Privileged Customer [%d] is interacting with DepartmentSalesmen[%d] of Department[%d]\n", sizeof("Privileged Customer [%d] is interacting with DepartmentSalesmen[%d] of Department[%d]\n"), NEncode2to1(myID, mySalesIndex), targetDepartment);
		       }
		       else{
			 NPrint("Customer [%d] is interacting with DepartmentSalesmen[%d] of Department[%d]\n", sizeof("Customer [%d] is interacting with DepartmentSalesmen[%d] of Department[%d]\n"), NEncode2to1(myID, mySalesIndex), targetDepartment);
		       }
                       Release(salesLock[targetDepartment]);
			salesCustNumber[targetDepartment][mySalesIndex] = myID; /* Sets the customer number of the salesman to this customer's index */

                       Signal(salesmanCV[targetDepartment][mySalesIndex], individualSalesmanLock[targetDepartment][mySalesIndex]);
                       Wait(salesmanCV[targetDepartment][mySalesIndex], individualSalesmanLock[targetDepartment][mySalesIndex]);
                       Release(individualSalesmanLock[targetDepartment][mySalesIndex]);


		}

		currentDepartment = targetDepartment;

		/* BEGINS SHOPPING */

		for( shelfNum = 0; shelfNum < numItems; shelfNum++) {
			if(shelfNum != itemsToBuy[i]) {
				continue;
			}

			if(type){
			  NPrint("Privileged Customer [%d] wants to buy [%d]-[%d]\n", sizeof("Privileged Customer [%d] wants to buy [%d]-[%d]\n"), NEncode2to1(myID, shelfNum), qtyItemsToBuy[i]);
			}
			else{
			  NPrint("Customer [%d] wants to buy [%d]-[%d]\n", sizeof("Customer [%d] wants to buy [%d]-[%d]\n"), NEncode2to1(myID, shelfNum), qtyItemsToBuy[i]);
			}
			while(qtyItemsInCart[i] < qtyItemsToBuy[i]) {
                           Acquire(shelfLock[currentDepartment][shelfNum]);

				if(shelfInventory[currentDepartment][shelfNum] > qtyItemsToBuy[i]) {	/* if there are enough items, take what i need */
				  if(type){
				    NPrint("Privileged Customer [%d] has found [%d] and placed [%d] in the trolly\n", sizeof("Privileged Customer [%d] has found [%d] and placed [%d] in the trolly\n"), NEncode2to1(myID, shelfNum), qtyItemsToBuy[i]);
				  }
				  else{
				    NPrint("Customer [%d] has found [%d] and placed [%d] in the trolly\n", sizeof("Customer [%d] has found [%d] and placed [%d] in the trolly\n"), NEncode2to1(myID, shelfNum), qtyItemsToBuy[i]);
				  }
				  if(testNumber == 8)
				    NPrint("There were %d before Customer %d took the item(s).\n", sizeof("There were %d before Customer %d took the item(s).\n"), NEncode2to1(shelfInventory[currentDepartment][shelfNum], myID), 0);

					shelfInventory[currentDepartment][shelfNum] -= qtyItemsToBuy[i];
					itemsInCart[i] = shelfNum;
					qtyItemsInCart[i] += qtyItemsToBuy[i];
                                       Release(shelfLock[currentDepartment][shelfNum]);
				}
				else {	/* We are out of this item, go tell sales! */
				  if(type){
				    NPrint("Privileged Customer [%d] was not able to find item %d and is searching for department salesman %d\n", sizeof("Privileged Customer [%d] was not able to find item %d and is searching for department salesman %d\n"), NEncode2to1(myID, shelfNum), currentDepartment);
				  }
				  else{
				    NPrint("Customer [%d] was not able to find item %d and is searching for department salesman %d\n", sizeof("Customer [%d] was not able to find item %d and is searching for department salesman %d\n"), NEncode2to1(myID, shelfNum), currentDepartment);
				  }
                                       Release(shelfLock[currentDepartment][shelfNum]);
                                       Acquire(salesLock[currentDepartment]);

					 mySalesID = -1;

					for( j = 0; j < numSalesmen; j++) {	/* see if there is a free salesman to go to */
						/* nobody waiting, sales free */
						if(currentSalesStatus[currentDepartment][j] == SALES_NOT_BUSY) {
							mySalesID = j;
							break;
						}
					}
					if(mySalesID == -1) {	/* no salesmen are free, I have to wait in line */
						complainingCustWaitingLineCount[currentDepartment]++;
                                               Wait(complainingCustCV[currentDepartment], salesLock[currentDepartment]);

						/* find the salesman who just signalled me */
						for( k = 0; k < numSalesmen; k++) {
							if(currentSalesStatus[currentDepartment][k] == SALES_READY_TO_TALK) {
								mySalesID = k;
								break;
							}
						}
					}

					/* I'm now talking to a salesman */
					currentSalesStatus[currentDepartment][mySalesID] = SALES_BUSY;
                                       Release(salesLock[currentDepartment]);
                                       Acquire(individualSalesmanLock[currentDepartment][mySalesID]);

				       if(type){
					 NPrint("Privileged Customer [%d] is asking for assistance from DepartmentSalesmen[%d]", sizeof("Privileged Customer [%d] is asking for assistance from DepartmentSalesman[%d]"), NEncode2to1(myID, mySalesID) , 0);
				       }
				       else{
					 NPrint("Customer [%d] is asking for assistance from DepartmentSalesmen[%d]", sizeof("Customer [%d] is asking for assistance from DepartmentSalesman[%d]"), NEncode2to1(myID, mySalesID) , 0);
				       }

					salesCustNumber[currentDepartment][mySalesID] = myID;


					/* now proceed with interaction to tell sales we are out */
					currentlyTalkingTo[currentDepartment][mySalesID] = COMPLAINING;
					salesDesk[currentDepartment][mySalesID] = shelfNum;

                                       Signal(salesmanCV[currentDepartment][mySalesID], individualSalesmanLock[currentDepartment][mySalesID]);
                                       Wait(salesmanCV[currentDepartment][mySalesID], individualSalesmanLock[currentDepartment][mySalesID]);
                                       Acquire(shelfLock[currentDepartment][shelfNum]);
                                       Release(individualSalesmanLock[currentDepartment][mySalesID]);

					/* now i go wait on the shelf */
                                       Wait(shelfCV[currentDepartment][shelfNum], shelfLock[currentDepartment][shelfNum]);
				       if(type){
					 NPrint("DepartmentSalesman [%d] informs the Privileged Customer [%d] that [%d] is restocked.\n", sizeof("DepartmentSalesman [%d] informs the Privileged Customer [%d] that [%d] is restocked.\n"), NEncode2to1(mySalesID, myID), shelfNum);
				       }
				       else{
					 NPrint("DepartmentSalesman [%d] informs the Customer [%d] that [%d] is restocked.\n", sizeof("DepartmentSalesman [%d] informs the Customer [%d] that [%d] is restocked.\n"), NEncode2to1(mySalesID, myID), shelfNum);					 
				       }

					/* now restocked, continue looping until I have all of what I need */
				       if(type){
					 NPrint("Privileged Customer [%d] has received assistance about restocking of item [%d] from DepartmentSalesman [%d]\n", sizeof("Privileged Customer [%d] has received assistance about restocking of item [%d] from DepartmentSalesman [%d]\n"), NEncode2to1(myID, shelfNum), mySalesID);
				       }
				       else{
					 NPrint("Customer [%d] has received assistance about restocking of item [%d] from DepartmentSalesman [%d]\n", sizeof("Customer [%d] has received assistance about restocking of item [%d] from DepartmentSalesman [%d]\n"), NEncode2to1(myID, shelfNum), mySalesID);
				       }
                                       Release(shelfLock[currentDepartment][shelfNum]);
				}
			}	/* end while loop to get enough of a given item */
		}	/* end looking through shelves */
	}	/* end going through grocery list */


	/* ======================================================== */

	 
	/* --------------Begin looking for a cashier------------------------------------- */
       Acquire(cashierLinesLock);
	do{		/* loop allows us to rechoose a line if our cashier goes on break */
	  if(type){
	    NPrint("Privileged Customer [%d] is looking for the Cashier.\n", sizeof("Privileged Customer [%d] is looking for the Cashier.\n"), myID, 0);
	  }
	  else{
	    NPrint("Customer [%d] is looking for the Cashier.\n", sizeof("Customer [%d] is looking for the Cashier.\n"), myID, 0);
	  }
 
		/* Find if a cashier is free (if one is, customer doesn't need to wait in line) */
		for( i = 0; i < cashierNumber; i++ ){
			/* if I find a cashier who is free, I will: */
			if(cashierStatus[i] == CASH_NOT_BUSY){
				myCashier = i; /* remember who he is */
                               Acquire(cashierLock[i]);
				cashierStatus[i] = CASH_BUSY; /* prevent others from thinking he's free */
				if(type){
				  NPrint("Privileged Customer [%d] chose cashier [%d] with line of length [0].\n", sizeof("Privileged Customer [%d] chose cashier [%d] with line of length [0].\n"), NEncode2to1(myID, myCashier), 0);
				}
				else{
				  NPrint("Customer [%d] chose cashier [%d] with line of length [0].\n", sizeof("Customer [%d] chose cashier [%d] with line of length [0].\n"), NEncode2to1(myID, myCashier), 0);
				}
				break; /* stop searching through lines */
			}

			/* ---------------Find shortest line--------------------------- */
			else if (i == cashierNumber - 1){
			  /* set the pointers depending on which customer type i am dealing with */
				if(type){
					linesIAmLookingAt = privilegedLineCount;
					linesIAmLookingAtCV= privilegedCashierLineCV;
				}
				else{
					linesIAmLookingAt = unprivilegedLineCount;
					linesIAmLookingAtCV = unprivilegedCashierLineCV;
				}

				/* from here on, privilegedCustomers and unprivileged customers execute same code because */
				/* of the temporary variables linesIAmLookingAt (which is unprivilegedLineCount or privilegedLineCount) */
				/* and linesIAmLookingAtCV (which is un/privilegedCashierLineCV) */

				minLineValue = custNumber; /* set a default min value */
				/* find the minimum line value and remember the cashier associated with it */
				for( j = 0; j < cashierNumber; j++){
					if(linesIAmLookingAt[j] < minLineValue && cashierStatus[j] != CASH_ON_BREAK && cashierStatus[j] != 	CASH_GO_ON_BREAK){
						/* must also check if that cashier is on break */
						minLineValue = linesIAmLookingAt[j];
						minCashierID = j;
					}
				}
				myCashier = minCashierID;
				if(type){
				  NPrint("Privileged Customer [%d] chose Cashier [%d] of line length [%d].\n", sizeof("Privileged Customer [%d] chose Cashier [%d] of line length [%d].\n"), NEncode2to1(myID, myCashier), linesIAmLookingAt[minCashierID]);
				}
				else{
				  NPrint("Customer [%d] chose Cashier [%d] of line length [%d].\n", sizeof("Customer [%d] chose Cashier [%d] of line length [%d].\n"), NEncode2to1(myID, myCashier), linesIAmLookingAt[minCashierID]);
				}
				linesIAmLookingAt[minCashierID]++;
                                Wait(linesIAmLookingAtCV[minCashierID], cashierLinesLock);
				linesIAmLookingAt[myCashier]--; /* i have been woken up, remove myself from line */
			}
			/* -------------End find shortest line---------------------- */

		}
	}while(cashierStatus[myCashier] == CASH_ON_BREAK); /* customer will repeat finding a cashier algorithm if he was woken up because the cashier is going on break */

	/* ----------------End looking for cashier-------------------------------------------- */


	/* code after this means I have been woken up after getting to the front of the line */
       Acquire(cashierLock[myCashier]);
	/* allow others to view monitor variable now that I've */
       /* my claim on this cashier*/ 
       Release(cashierLinesLock);
	cashierDesk[myCashier] = myID;	/* tell cashier who I am */
	/* signal cashier I am at his desk*/ 
       Signal(cashierToCustCV[myCashier], cashierLock[myCashier]);
	/* wait for his acknowlegdment*/ 
       Wait(cashierToCustCV[myCashier], cashierLock[myCashier]);
	cashierDesk[myCashier] = type; /* now tell him that whether or not I am privileged */
	/* signal cashier I've passed this information*/ 
       Signal(cashierToCustCV[myCashier], cashierLock[myCashier]);
	/* wait for his acknowledgment*/ 
       Wait(cashierToCustCV[myCashier], cashierLock[myCashier]);

	/* When I get here, the cashier is ready for items */

	/* ---------------------------Begin passing items to cashier--------------------------- */
	/* cycle through all items in my inventory, handing one item to cashier at a Time */
	for( i = 0; i < numItemsToBuy; i++){ /* cycle through all types of items */
		for( j = 0; j < qtyItemsInCart[i]; j++){ /* be sure we report how many of each type */
			cashierDesk[myCashier] = itemsInCart[i]; /* tells the cashier what type of item */
                       Signal(cashierToCustCV[myCashier], cashierLock[myCashier]);
                       Wait(cashierToCustCV[myCashier], cashierLock[myCashier]);
		}
	}
	cashierDesk[myCashier] = -1; /* Tells cashier that I have reached the last item in my inventory */
        Signal(cashierToCustCV[myCashier], cashierLock[myCashier]);
        Wait(cashierToCustCV[myCashier], cashierLock[myCashier]);
	/* ------------------------End passing items to cashier-------------------------------- */


	/* when I get here, the cashier has loaded my total */
	/* If I don't have enough money, leave the error flag -1 on the cashier's desk */
	if(cashierDesk[myCashier] > myCash){
	  if(type){
	    NPrint("Privileged Customer [%d] cannot pay [%d]\n", sizeof("Privileged Customer [%d] cannot pay [%d]\n"), NEncode2to1(myID, cashierDesk[myCashier]), 0);
	  }
	  else{
	    NPrint("Customer [%d] cannot pay [%d]\n", sizeof("Customer [%d] cannot pay [%d]\n"), NEncode2to1(myID, cashierDesk[myCashier]), 0);
	  }
		cashierDesk[myCashier] = -1;
                Signal(cashierToCustCV[myCashier], cashierLock[myCashier]);
                Wait(cashierToCustCV[myCashier], cashierLock[myCashier]);
                Acquire(managerLock);
                Release(cashierLock[myCashier]);
		if(type){
		  NPrint("Privileged Customer [%d] is waiting for Manager negotiations\n", sizeof("Privileged Customer [%d] is waiting for Manager negotiations\n"), myID, 0);
		}
		else{
		  NPrint("Customer [%d] is waiting for Manager negotiations\n", sizeof("Customer [%d] is waiting for Manager negotiations\n"), myID, 0);
		}
                Wait(managerCV, managerLock);

		/* -----------------------Begin passing items to manager--------------------- */
		for( i = 0; i < numItemsToBuy; i++){
			while(qtyItemsInCart[i] > 0){
				if(managerDesk < myCash){
					break;
				}
				qtyItemsInCart[i] --;
				managerDesk = itemsInCart[i];
				if(type){
				  NPrint("Privileged Customer [%d] tells manager to remove [%d] from trolly.\n", sizeof("Privileged Customer [%d] tells manager to remove [%d] from trolly.\n"), NEncode2to1(myID, itemsInCart[i]), 0);
				}
				else{
				  NPrint("Customer [%d] tells manager to remove [%d] from trolly.\n", sizeof("Customer [%d] tells manager to remove [%d] from trolly.\n"), NEncode2to1(myID, itemsInCart[i]), 0);
				}
                                Signal(managerCV, managerLock);
                                Wait(managerCV, managerLock);
			}
		}
		managerDesk = -1; /* notifies the manager I'm done */
                Signal(managerCV, managerLock);
                Wait(managerCV, managerLock);
		/* --------------------End of passing items to manager--------------- */

		amountOwed = managerDesk;	/* if I still can't afford anything, amountOwed will be 0 */
		myCash -= amountOwed;	/* updating my cash amount because I am paying manager */
		managerDesk = amountOwed;	/* technically redundant, but represents me paying money */
		if(type){
		  NPrint("Privileged Customer [%d] pays [%d] to Manager after removing items and is waiting for receipt from Manager.\n", sizeof("Privileged Customer [%d] pays [%d] to Manager after removing items and is waiting for receipt from Manager.\n"), NEncode2to1(myID, amountOwed), 0);
		}
		else{
		  NPrint("Customer [%d] pays [%d] to Manager after removing items and is waiting for receipt from Manager.\n", sizeof("Customer [%d] pays [%d] to Manager after removing items and is waiting for receipt from Manager.\n"), NEncode2to1(myID, amountOwed), 0);
		}
		/* need receipt */
                Signal(managerCV, managerLock);
                Wait(managerCV, managerLock);
		/* got receipt, I can now leave */
                Release(managerLock);
		NPrint("Customer [%d] got receipt from Manager and is now leaving.\n", sizeof("Customer [%d] got receipt from Manager and is now leaving.\n"), myID, 0);
	}
	/* if I do have money, I just need to update my cash and leave the money there */
	else{
		myCash -= cashierDesk[myCashier];
		/* Now I wait for my receipt */
		/*printf("%s [%d] pays [%d] to Cashier [%d] and is now waiting for receipt.\n", type, myID, cashierDesk[myCashier], myCashier);*/
		Wait(cashierToCustCV[myCashier], cashierLock[myCashier]);
		/* now I've received my receipt and should release the cashier */
                Signal(cashierToCustCV[myCashier], cashierLock[myCashier]);
		if(type){
		  NPrint("Privileged Customer [%d] pays [%d] to Cashier [%d] and is now waiting for receipt.\n", sizeof("Privileged Customer [%d] pays [%d] to Cashier [%d] and is now waiting for receipt.\n"), NEncode2to1(myID, cashierDesk[myCashier]), myCashier);
		}
		else{
		  NPrint("Customer [%d] pays [%d] to Cashier [%d] and is now waiting for receipt.\n", sizeof("Customer [%d] pays [%d] to Cashier [%d] and is now waiting for receipt.\n"), NEncode2to1(myID, cashierDesk[myCashier]), myCashier);
		}  
                Wait(cashierToCustCV[myCashier], cashierLock[myCashier]);
		if(type){
		  NPrint("Privileged Customer [%d] got receipt from Cashier [%d] and is now leaving.\n", sizeof("Privileged Customer [%d] got receipt from Cashier [%d] and is now leaving.\n"), NEncode2to1(myID, myCashier), 0);
		}
		else{
		  NPrint("Customer [%d] got receipt from Cashier [%d] and is now leaving.\n", sizeof("Customer [%d] got receipt from Cashier [%d] and is now leaving.\n"), NEncode2to1(myID, myCashier), 0);
		}
                Signal(cashierToCustCV[myCashier], cashierLock[myCashier]);
                Release(cashierLock[myCashier]);
		myCashier = -1; /* so I can't accidentally tamper with the cashier I chose anymore */
	}

	/* ------------------------------Begin replace trolly-------------- */
	/* no need for CV since sequencing doesn't matter */
        Acquire(displacedTrollyLock);
        displacedTrollyLock++;
        Release(displacedTrollyLock);
	/* -----------------------------End replace trolly---------------------- */

	customersDone++;	/* increment the total customers done count */

	/* some cleanup */
	/*delete itemsToBuy;*/
        
}

/*gets the prices for each type of item*/
int scan(int item){
	switch(item){
	case 0: return 5;
	case 1: return 2;
	case 2: return 3;
	case 3: return 8;
	case 4: return 7;
	case 5: return 3;
	case 6: return 10;
	case 7: return 4;
	default: return 1;
	}
}


/*Manager code*/
void manager(){
	int totalRevenue = 0; /* will track the total sales of the day */
	int counter = 0;
	int i, numFullLines, numAnyLines, wakeCashier, chance, r, dept, arg,
	  wakeSalesman, targets[2], customerID, cashierID, amountOwed, custTypeID,
	  managerCustType;
	int salesmenOnBreak, cashiersOnBreak;
	salesmenOnBreak = CreateQueue();
	cashiersOnBreak = CreateQueue();

	/* initializes cashier totals, by which we keep track of how much money each cashier has had */
	/* in their registers over Time */

	NSrand(NTime(NULL));
	while(1){

		/* ------------------Check if all customers have left store---------------- */
		/* if all customers have left, print out totals and terminate simulation since manager will be the only */
		/* ready thread */
		if(customersDone == custNumber){
			for( i = 0; i < cashierNumber; i++){
                          Acquire(cashierLock[i]);
				/* -----------------------------Start empty cashier drawers------------------------------------ */
				if(cashRegister[i] > 0){
					totalRevenue += cashRegister[i];
					cashierTotals[i] += cashRegister[i];
					NPrint("Manager emptied Counter %d draw.\n", sizeof("Manager emptied Counter %d draw.\n"), i, 0);
					cashRegister[i] = 0;
					NPrint("Manager has total sale of $%d.\n", sizeof("Manager has total sale of $%d.\n"), totalRevenue, 0);
                                        Release(cashierLock[i]);
				}
			}
			for(i = 0; i < cashierNumber; i++){
			  NPrint("Total Sale from Counter [%d] is $[%d].\n", sizeof("Total Sale from Counter [%d] is $[%d].\n"), NEncode2to1(i, cashierTotals[i]), 0);
			}
			NPrint("Total Sale of the entire store is $[%d].\n", sizeof("Total Sale of the entire store is $[%d].\n"), totalRevenue, 0);
			break;
		}
		/* ------------------End check if all customers have left store------------ */

		/* -----------------Have loader check trolleys--------------------------- */
                Acquire(inactiveLoaderLock);
                Signal(inactiveLoaderCV, inactiveLoaderLock);
		Release(inactiveLoaderLock);
		if(counter != 100000000) counter ++;
		/* I don't need to acquire a lock because I never go to sleep */
		/* Therefore, it doesn't matter if a cashierFlag is changed on this pass, */
		/* I will get around to it */
		if(counter % 10 == 0){
			/* 	cout << customersDone << endl; */
			/*if(testNumber != 5 && testNumber != 6 && testNumber != 10) cout <<"-------Total Sale of the entire store until now is $" << totalRevenue <<"---------" << endl;*/
		}

                Acquire(cashierLinesLock);
        
		 numFullLines = 0; /* will be used to figure out whether to bring cashiers back from break */
		 numAnyLines = 0;
		for ( i = 0; i < cashierNumber; i++){
			if(privilegedLineCount[i]) numAnyLines++;
			if(unprivilegedLineCount[i]) numAnyLines++;
			/* a line is "full" if it has more than 3 customers (if each cashier has a line of size 3, we want to bring back a cashier */
			if(privilegedLineCount[i] >= 3) numFullLines ++;
			if(unprivilegedLineCount[i] >= 3) numFullLines ++;
		}

		/* --------------------------Begin bring cashier back from break-------------------- */
		if(numFullLines > (cashierNumber - numCashiersOnBreak) && QueueSize(cashiersOnBreak)){ /* bring back cashier if there are more lines with 3 customers than there are cashiers and if there are cashiers on break*/ 
		  wakeCashier = QueueFront(cashiersOnBreak);
		  if(cashierStatus[wakeCashier] == CASH_ON_BREAK){
		    Release(cashierLinesLock);
		    Acquire(cashierLock[wakeCashier]);
		    if (numAnyLines)/*cout << "Manager brings back Cashier " << wakeCashier << " from break." << endl;*/
		      NPrint("Manager brings back Cashier %d from break.\n", sizeof("Manager brings back Cashier %d from break.\n"), wakeCashier, 0);
		    Signal(cashierToCustCV[wakeCashier], cashierLock[wakeCashier]);
		    Release(cashierLock[wakeCashier]);
		    /* bookkeeping */
		    numCashiersOnBreak--;
		    QueuePop(cashiersOnBreak);
		  }


		}
		else
		  Release(cashierLinesLock);

		Acquire(cashierLinesLock);


		/* ---------------------------End Bring cashier back from break-------------------------- */

		/* ---------------------------Begin send cashiers on break------------------------------- */
		 chance = NRand() %10;

		if( chance == 1  && numCashiersOnBreak < cashierNumber -2){ /* .001% chance of sending cashier on break */
			/* generate cashier index */
			 r = NRand() % cashierNumber;
			if(cashierStatus[r] != CASH_ON_BREAK && cashierStatus[r] != CASH_GO_ON_BREAK){
				if(cashierStatus[r] == CASH_NOT_BUSY) {
				  Acquire(cashierLock[r]);
					cashierDesk[r] = -2;
					cashierStatus[r] = CASH_GO_ON_BREAK;
					Signal(cashierToCustCV[r], cashierLock[r]);

					Release(cashierLock[r]);
				}
				else cashierStatus[r] = CASH_GO_ON_BREAK;
				NPrint("Manager sends Cashier [%d] on break.\n", sizeof("Manager sends Cashier [%d] on break.\n"), r, 0);
				if(testNumber == 5) NPrint("Manager has iterated %d Times at this point.\n", sizeof("Manager has iterated %d Times at this point.\n"), counter, 0);
				QueuePush(cashiersOnBreak, r);
				numCashiersOnBreak++;


			}
		}

		/* -----------------------------End send cashiers on break------------------------------------- */
		Release(cashierLinesLock);

		/* __SALES_BREAK__ */
		/* -----------------------------Begin bringing salesmen back from break------------- */
		 dept = 0;
		if(QueueSize(numSalesmenOnBreak)){

		  arg = QueueFront(salesmenOnBreak);
			 targets[2];
			deconstructSalesArg(arg, targets);
			 wakeSalesman = targets[0];
			dept = targets[1];
			QueuePop(salesmenOnBreak);
			Acquire(salesLock[dept]);
			if((greetingCustWaitingLineCount[dept] + complainingCustWaitingLineCount[dept] + loaderWaitingLineCount[dept]) > 0 && currentSalesStatus[dept][wakeSalesman] == SALES_ON_BREAK){
				salesBreakBoard[dept][wakeSalesman] = 0;
				NPrint("Manager brings back Salesman [%d] from break.\n", sizeof("Manager brings back Salesman [%d] from break.\n"), wakeSalesman, 0);
				Signal(salesBreakCV[dept][wakeSalesman], salesLock[dept]);
				numSalesmenOnBreak[dept]--;
			}
			else{
			  QueuePush(salesmenOnBreak);
			}
			Release(salesLock[dept]);
		}

		/* ------------------------------end bringing salesmen back from break-------------- */

		/* ------------------------------Begin putting salesmen on break------------------ */
		dept = NRand() % numDepartments;
		Acquire(salesLock[dept]);
		if (chance == 1 && numSalesmenOnBreak[dept] < numSalesmen -1) {
			 r = NRand() % numSalesmen;
			if(!salesBreakBoard[dept][r] && currentSalesStatus[dept][r] != SALES_ON_BREAK && currentSalesStatus[dept][r] != SALES_GO_ON_BREAK) {
				salesBreakBoard[dept][r] = 1;
				NPrint("Manager sends Salesman [%d] of dept [%d] on break.\n", sizeof("Manager sends Salesman [%d] of dept [%d] on break.\n"), NEncode2to1(r, dept), 0);
				Acquire(individualSalesmanLock[dept][r]);
				if(currentSalesStatus[dept][r] == SALES_NOT_BUSY){
				  Signal(salesmanCV[dept][r], individualSalesmanLock[dept][r]);
				  currentSalesStatus[dept][r] = SALES_ON_BREAK;
				}
				Release(individualSalesmanLock[dept][r]);
				/* function that uses bit operations to store dept and salesman index*/ 
				/* in one int so I can get it from my queue later when I take a Salesman off break */

				 arg = constructSalesArg(dept, r);
				 QueuePush(salesmenOnBreak, arg);
				numSalesmenOnBreak[dept]++;
			}
		}
		Release(salesLock[dept]);
		/* -----------------------------End send salesmen on break */

		for(i = 0; i < cashierNumber; i++){

										/* only manager and cashier i ever attempt to grab this lock */
		  Acquire(cashierLock[i]);
			/* -----------------------------Start empty cashier drawers------------------------------------ */
			if(cashRegister[i] > 0){
				totalRevenue += cashRegister[i];
				cashierTotals[i] += cashRegister[i]; /* how we remember totals for each cashier */
				NPrint("Manager emptied Counter [%d] drawer.\n", sizeof("Manager emptied Counter [%d] drawer.\n"), i, 0);
				cashRegister[i] = 0;
				NPrint("Manager has total sale of $[%d].\n", sizeof("Manager has total sale of $[%d].\n"), totalRevenue, 0);
				Release(cashierLock[i]);
				break;
			}
			/* ---------------------------end empty cashier drawers--------------------------------- */

			/* ------------------------------Start deal with broke customers------------------------- */
			else if(cashierFlags[i] != -1){ /* cashier flags is [i] is change to the Customre at the i-th */
											/* cashier desk so I know who to go to and deal with */
			  /*cout <<"Manager got a call from Cashier [" << i << "]." << endl;*/
			  NPrint("Manager got a call from Cashier [%d].\n", sizeof("Manager got a call from Cashier [%d].\n"), i, 0);
				customerID = cashierFlags[i];
				cashierID = i;

				Acquire(managerLock);

				Signal(cashierToCustCV[cashierID], cashierLock[cashierID]);
				Release(cashierLock[i]);
				Wait(managerCV, managerLock);
				amountOwed = cashierFlags[i];
				cashierFlags[i] = -1; /* reset cashierFlags */
				custTypeID = managerDesk;
				managerCustType = -1;
				if(custTypeID == 0){
					managerCustType = 0;
				}
				else{
				  managerCustType = 1;
				}
				managerDesk = amountOwed;
				Signal(managerCV, managerLock);
				Wait(managerCV, managerLock);
				while(managerDesk != -1 ){ /* when managerDesk == -1, customer is out of items or can afford money */
					amountOwed -= scan(managerDesk);
					Acquire(managerItemsLock);
					QueuePush(managerItems, managerDesk);
					Release(managerItemsLock);
					if(managerCustType){
					  NPrint("Manager removes [%d] from the trolly PrivilegedCustomer [%d].\n", sizeof("Manager removes [%d] from the trolly PrivilegedCustomer [%d].\n"), NEncode2to1(managerDesk, customerID), 0);
					}
					else{
					  NPrint("Manager removes [%d] from the trolly Customer [%d].\n", sizeof("Manager removes [%d] from the trolly Customer [%d].\n"), NEncode2to1(managerDesk, customerID), 0);
					}
					managerDesk = amountOwed; /* giving customer new subtotal */
									/* customer will put back -1 if out of items */
									/*  or if they can now afford */
					Signal(managerCV, managerLock);
					Wait(managerCV, managerLock);
				}
				/* now customer has reached end of items or has enough money */
				/* I give him total */
				managerDesk = amountOwed;
				Signal(managerCV, managerLock);
				/* wait for his response, i.e. paying money*/ 
				Wait(managerCV, managerLock);
				totalRevenue += managerDesk;

				/* Now give the customer his receipt */
				if(managerCustType){
				  NPrint("Manager gives receipt to PrivilegedCustomer [%d].\n", sizeof("Manager gives receipt to PrivilegedCustomer [%d].\n"), customerID, 0);
				}
				else{
				  NPrint("Manager gives receipt to Customer [%d].\n", sizeof("Manager gives receipt to Customer [%d].\n"), customerID, 0);				  
				}
				managerDesk = -1;
				Signal(managerCV, managerLock);
				/* release manager lock */
				Release(managerLock);
				break;


			}
			/* ----------------------------End deal with broke customers-------------------- */
			else
			  Release(cashierLock[i]);
		}
	}
}




/* cashier code */

void cashier(){
  int myCounter;
	/*set the cashier's counter*/
	Acquire(cashierIndexLock);
	myCounter = nextCashierIndex;
	nextCashierIndex++;
	Release(cashierIndexLock);

	cashierStatus[myCounter] = CASH_NOT_BUSY;
	while(1){
		Acquire(cashierLinesLock);
		/* check my status to see if I've already been set to busy by a customer */
		if(cashierStatus[myCounter] == CASH_GO_ON_BREAK){
		  NPrint("Cashier [%d] is going on break.\n", sizeof("Cashier [%d] is going on break.\n"), myCounter, 0);
		cashierStatus[myCounter] = CASH_ON_BREAK;
		Broadcast(unprivilegedCashierLineCV[myCounter], cashierLinesLock);
		Broadcast(privilegedCashierLineCV[myCounter], cashierLinesLock);

		Acquire(cashierLock[myCounter]);
		Release(cashierLinesLock);

		Wait(cashierToCustCV[myCounter], cashierLock[myCounter]);
		cashierStatus[myCounter] = CASH_NOT_BUSY;
		NPrint("Cashier [%d] was called from break by Manager to work.\n", sizeof("Cashier [%d] was called from break by Manager to work.\n"), myCounter, 0);
	}
	/* check if my lines have anyone in it */
	/* set my state so approaching Customers can wait in or engage me, as apropriate */
	if(privilegedLineCount[myCounter]){
          Signal(privilegedCashierLineCV[myCounter], cashierLinesLock);
		custType[myCounter] = PRIVILEGED_CUSTOMER;
	}
	else if(unprivilegedLineCount[myCounter]){
          Signal(unprivilegedCashierLineCV[myCounter], cashierLinesLock);
		custType[myCounter] = CUSTOMER;
	}
	else{
		cashierStatus[myCounter] = CASH_NOT_BUSY; /* means I can be approached without it being necessary to wait in line */
	}
	/* whether or not I'm ready for a customer, I can get my lock and go to sleep */
	Acquire(cashierLock[myCounter]);
	Release(cashierLinesLock);
	Signal(cashierToCustCV[myCounter], cashierLock[myCounter]);
	Wait(cashierToCustCV[myCounter], cashierLock[myCounter]);
	/* the next Time I'm woken up (assuming it is by a customer, not a manager */
	/* I will be totaling items */
	/* when I get here, there will be an item to scan */
	total[myCounter] = 0;
	custID[myCounter] = cashierDesk[myCounter];
	if(custID[myCounter] == -2){
		Release(cashierLock[myCounter]);
		continue;
	}
	Signal(cashierToCustCV[myCounter], cashierLock[myCounter]);
	Wait(cashierToCustCV[myCounter], cashierLock[myCounter]);

	if(cashierDesk[myCounter] == 1){
		custType[myCounter] = PRIVILEGED_CUSTOMER;
	}
	else{
		custType[myCounter] = CUSTOMER;
	}

	Signal(cashierToCustCV[myCounter], cashierLock[myCounter]);
	Wait(cashierToCustCV[myCounter], cashierLock[myCounter]);
	while(cashierDesk[myCounter] != -1){ /* -1 means we're done scanning */


	  if(custType[myCounter] == PRIVILEGED_CUSTOMER){
	    NPrint("Cashier [%d] got [%d] from trolly of Privileged Customer [%d].\n", sizeof("Cashier [%d] got [%d] from trolly of Privileged Customer [%d].\n"), NEncode2to1(myCounter, cashierDesk[myCounter]), custID);
	    }
	  else{
	    NPrint("Cashier [%d] got [%d] from trolly of Customer [%d].\n", sizeof("Cashier [%d] got [%d] from trolly of Customer [%d].\n"), NEncode2to1(myCounter, cashierDesk[myCounter]), custID);	    
		total[myCounter] += scan(cashierDesk[myCounter]);
		Signal(cashierToCustCV[myCounter], cashierLock[myCounter]);
		Wait(cashierToCustCV[myCounter], cashierLock[myCounter]);
	}
	/* now I'm done scanning, so I tell the customer the total */
	  if(custType[myCounter] == PRIVILEGED_CUSTOMER){
	    NPrint("Cashier [%d] tells Privileged Customer [%d] total cost is $[%d].\n", sizeof("Cashier [%d] tells Privileged Customer [%d] total cost is $[%d].\n"), NEncode2to1(myCounter, custID), total);
	    }
	  else{
	    NPrint("Cashier [%d] tells Customer [%d] total cost is $[%d].\n", sizeof("Cashier [%d] tells Customer [%d] total cost is $[%d].\n"), NEncode2to1(myCounter, custID), total);
	    }
	cashierDesk[myCounter] = total[myCounter];
	Signal(cashierToCustCV[myCounter], cashierLock[myCounter]);
	Wait(cashierToCustCV[myCounter], cashierLock[myCounter]);
	if(cashierDesk[myCounter] == -1){
	  if(custType[myCounter] == PRIVILEGED_CUSTOMER){
	    NPrint("Cashier [%d] asks Privileged Customer [%d] to wait for Manager.\n", sizeof("Cashier [%d] asks Privileged Customer [%d] to wait for Manager.\n"), NEncode2to1(myCounter, custID), 0);
	    NPrint("Cashier [%d] informs the Manager that Privileged Customer [%d] does not have enough money.\n", sizeof("Cashier [%d] informs the Manager that Privileged Customer [%d] does not have enough money.\n"), NEncode2to1(myCounter, custID), 0);
	    }
	  else{
	    NPrint("Cashier [%d] asks Customer [%d] to wait for Manager.\n", sizeof("Cashier [%d] asks Customer [%d] to wait for Manager.\n"), NEncode2to1(myCounter, custID), 0);
	    NPrint("Cashier [%d] informs the Manager that Customer [%d] does not have enough money.\n", sizeof("Cashier [%d] informs the Manager that Customer [%d] does not have enough money.\n"), NEncode2to1(myCounter, custID), 0);
	    }
		cashierFlags[myCounter] = custID[myCounter];
		Wait(cashierToCustCV[myCounter], cashierLock[myCounter]);
		/* Set the manager desk to 0 or 1 to tell the manager which type of */
		/* customer he is dealing with */
		if(custType[myCounter] == PRIVILEGED_CUSTOMER){
			managerDesk = 1;
		}
		/* if(!strcmp(custType, "Customer")){ */
		if(custType[myCounter] == CUSTOMER){
			managerDesk = 0;
		}
		cashierFlags[myCounter] = total[myCounter]; /* inform manager of the total the customer owes */
		Acquire(managerLock);
		Signal(managerCV, managerLock);
		Release(managerLock);
		/* when I am woken up, the manager has taken over so I can free myself for my */
		/* next customer */
	}
	else{
		/* add value to cash register */
		cashRegister[myCounter] += cashierDesk[myCounter];
		if(custType[myCounter] == PRIVILEGED_CUSTOMER){
		  NPrint("Cashier [%d] got money $[%d] from Privileged Customer [%d].\n", sizeof("Cashier [%d] got money $[%d] from Privileged Customer [%d].\n"), NEncode2to1(myCounter, cashierDesk[myCounter]), custID);
		  NPrint("Cashier [%d] gave the receipt to Privileged Customer [%d] and tells him to leave.\n", sizeof("Cashier [%d] gave the receipt to Privileged Customer [%d] and tells him to leave.\n"), NEncode2to1(myCounter, custID), 0);
		}
		else{
		  NPrint("Cashier [%d] got money $[%d] from Customer [%d].\n", sizeof("Cashier [%d] got money $[%d] from Customer [%d].\n"), NEncode2to1(myCounter, cashierDesk[myCounter]), custID);
		  NPrint("Cashier [%d] gave the receipt to Customer [%d] and tells him to leave.\n", sizeof("Cashier [%d] gave the receipt to Customer [%d] and tells him to leave.\n"), NEncode2to1(myCounter, custID), 0);		  
		}
		/* giving the customer a receipt */
		Signal(cashierToCustCV[myCounter], cashierLock[myCounter]);
		/* wait for customer to acknowledge getting receipt and release lock */
		Wait(cashierToCustCV[myCounter], cashierLock[myCounter]);
	}
	cashierDesk[myCounter] = 0;
	Release(cashierLock[myCounter]);

	/* done, just need to loop back and check for more customers */
	custType[myCounter] = CUSTOMER;
	}
	}
	
}



/* Salesman Code		__SALESMAN__ */
void Salesman() {
  int myIndex, myCustNumber, itemOutOfStock, itemRestocked, loaderNumber, myLoaderID,
    i, myDept;
  SalesmanStatus prev;
  Acquire(salesmanIndexLock);
  myIndex = nextSalesmanIndex;
  nextSalesmanIndex++;
  Release(salesmanIndexLock);
  Acquire(departmentIndexLock);
  myDept = nextDepartmenIndex;
  nextDepartmenIndex++;
  Release(departmentIndexLock);

	while(1) {
          Acquire(salesLock[myIndex]);

		/* go on break if the manager has left me a note saying to */
		if(salesBreakBoard[myDept][myIndex] == 1) {
			prev = currentSalesStatus[myDept][myIndex];
			currentSalesStatus[myDept][myIndex] = SALES_ON_BREAK;
			salesBreakBoard[myDept][myIndex] = 0;
			Wait (salesBreakCV[myDept][myIndex],salesLock[myDept]);
			currentSalesStatus[myDept][myIndex] = prev;
		}

		/* Check if there is someone in a line and wake them up */
		if(greetingCustWaitingLineCount[myDept] > 0){	/* greeting */
			currentSalesStatus[myDept][myIndex] = SALES_READY_TO_TALK;
			Signal(greetingCustCV[myDept], salesLock[myDept]);
			greetingCustWaitingLineCount[myDept]--;
		}
		else if(loaderWaitingLineCount[myDept] > 0) {	/* loader */
			currentSalesStatus[myDept][myIndex] = SALES_READY_TO_TALK_TO_LOADER;
			Signal(loaderCV[myDept], salesLock[myDept]);
			loaderWaitingLineCount[myDept]--;
		}
		else if(complainingCustWaitingLineCount[myDept] > 0) {	/* complaining */
			currentSalesStatus[myDept][myIndex] = SALES_READY_TO_TALK;
                        Signal(complainingCustCV[myDept], salesLock[myDept]);
			complainingCustWaitingLineCount[myDept]--;
		}

		else{	/* not busy */
			currentlyTalkingTo[myDept][myIndex] = UNKNOWN;
			currentSalesStatus[myDept][myIndex] = SALES_NOT_BUSY;
		}

                Acquire(individualSalesmanLock[myDept][myIndex]);
                Release(salesLock[myDept]);
                Wait (salesmanCV[myDept][myIndex], individualSalesmanLock[myDept][myIndex]);

		if(currentlyTalkingTo[myDept][myIndex] == GREETING) {	/* a greeting customer came up to me */
			myCustNumber = salesCustNumber[myDept][myIndex];
			if(privCustomers[myCustNumber] == 1){
			  NPrint("DepartmentSalesman [%d] welcomes PrivilegedCustomer [%d] to Department [%d].\n", sizeof("DepartmentSalesman [%d] welcomes PrivilegedCustomer [%d] to Department [%d].\n"), NEncode2to1(myIndex, myCustNumber), myDept);
			}
			else{
			  NPrint("DepartmentSalesman [%d] welcomes Customer [%d] to Department [%d].\n", sizeof("DepartmentSalesman [%d] welcomes Customer [%d] to Department [%d].\n"), NEncode2to1(myIndex, myCustNumber), myDept);			  
			}
                        Signal (salesmanCV[myDept][myIndex], individualSalesmanLock[myDept][myIndex]);
                        Release(individualSalesmanLock[myDept][myIndex]);
		}
		else if(currentlyTalkingTo[myDept][myIndex] == COMPLAINING) {	/* a complaining customer came up to me */
			myCustNumber = salesCustNumber[myDept][myIndex];
			itemOutOfStock = salesDesk[myDept][myIndex];

			if(privCustomers[myCustNumber] == 1){
			  NPrint("DepartmentSalesman [%d] is informed by PrivilegedCustomer [%d] that [%d] is out of stock.\n", sizeof("DepartmentSalesman [%d] is informed by PrivilegedCustomer [%d] that [%d] is out of stock.\n"), NEncode2to1(myIndex, myCustNumber), itemOutOfStock);
			}
			else{
			  NPrint("DepartmentSalesman [%d] is informed by Customer [%d] that [%d] is out of stock.\n", sizeof("DepartmentSalesman [%d] is informed by Customer [%d] that [%d] is out of stock.\n"), NEncode2to1(myIndex, myCustNumber), itemOutOfStock);
			}
                        Acquire(individualSalesmanLock[myDept][myIndex]);
                        Signal(salesmanCV[myDept][myIndex], individualSalesmanLock[myDept][myIndex]);

			/* tell goods loader */
                        Acquire(salesLock[myDept]);
			salesDesk[myDept][myIndex] = itemOutOfStock;	/* Might not be necessary, because we never really took it off the desk */

			if(loaderWaitingLineCount[myDept] > 0) {	/* if we can, get a loader from line */
				loaderWaitingLineCount[myDept]--;
				currentSalesStatus[myDept][myIndex] = SALES_READY_TO_TALK_TO_LOADER;
                                Signal(loaderCV[myDept], salesLock[myDept]);
                                Release(salesLock[myDept]);
			}
			else {	/*  no one was in line, so go to the inactive loaders */
				currentSalesStatus[myDept][myIndex] = SALES_SIGNALLING_LOADER;
                                Release(salesLock[myDept]);

                                Acquire(inactiveLoaderLock);
                                Signal(inactiveLoaderCV, inactiveLoaderLock);
                                Release(inactiveLoaderLock);
			}

                        Wait(salesmanCV[myDept][myIndex], individualSalesmanLock[myDept][myIndex]);

			/* check to see if a loader already restocked something */
			if(salesDesk[myDept][myIndex] != -1) {		/* loader had finished stocking something and tells me about it */
				itemRestocked = salesDesk[myDept][myIndex];
                                Signal(salesmanCV[myDept][myIndex], individualSalesmanLock[myDept][myIndex]);
                                Wait(salesmanCV[myDept][myIndex], individualSalesmanLock[myDept][myIndex]);
				loaderNumber = salesDesk[myDept][myIndex];
                                Signal(salesmanCV[myDept][myIndex], individualSalesmanLock[myDept][myIndex]);
				NPrint("DepartmentSalesman [%d] is informed by the GoodsLoader [%d] that [%d] is restocked.\n", sizeof("DepartmentSalesman [%d] is informed by the GoodsLoader [%d] that [%d] is restocked.\n"), NEncode2to1(myIndex, loaderNumber), itemRestocked);
                                Acquire(shelfLock[myDept][itemRestocked]);
                                Broadcast(shelfCV[myDept][itemRestocked], shelfLock[myDept][itemRestocked]);
                                Release(shelfLock[myDept][itemRestocked]);
				/* can't know the cust number, becaue we broadcast here! */
				/* DepartmentSalesman [identifier] informs the Customer/PrivilegeCustomer [identifier] that [item] is restocked. */
			}


                        Acquire(inactiveLoaderLock);
			myLoaderID = -1;

			for(i = 0; i < numLoaders; i++) {	/* find the loader who i just sent off to restock and change his status */
				if(loaderStatus[i] == LOAD_HAS_BEEN_SIGNALLED) {
					myLoaderID = i;
					loaderStatus[i] = LOAD_STOCKING;
					break;
				}
			}

                        Release(inactiveLoaderLock);

			NPrint("DepartmentSalesman [%d] informs the GoodsLoader [%d] that [%d] is out of stock.\n", sizeof("DepartmentSalesman [%d] informs the GoodsLoader [%d] that [%d] is out of stock.\n"), NEncode2to1(myIndex, myLoaderID), itemOutOfStock);
		   
                        Signal(salesmanCV[myDept][myIndex], individualSalesmanLock[myDept][myIndex]);
                        Release(individualSalesmanLock[myDept][myIndex]);

		}
		else if(currentlyTalkingTo[myDept][myIndex] == GOODSLOADER) {	/* a loader came up to talk to me */
			itemRestocked = salesDesk[myDept][myIndex];
                        Signal(salesmanCV[myDept][myIndex], individualSalesmanLock[myDept][myIndex]);
                        Wait(salesmanCV[myDept][myIndex], individualSalesmanLock[myDept][myIndex]);
			loaderNumber = salesDesk[myDept][myIndex];
                        Signal(salesmanCV[myDept][myIndex], individualSalesmanLock[myDept][myIndex]);
			NPrint("DepartmentSalesman [%d] is informed by the GoodsLoader [%d] that [%d] is restocked.\n", sizeof("DepartmentSalesman [%d] is informed by the GoodsLoader [%d] that [%d] is restocked.\n"), NEncode2to1(myIndex, myLoaderID), itemRestocked);
                        Acquire(shelfLock[myDept][itemRestocked]);
                        Broadcast(shelfCV[myDept][itemRestocked], shelfLock[myDept][itemRestocked]);
                        Release(shelfLock[myDept][itemRestocked]);
                        Release(individualSalesmanLock[myDept][myIndex]);
			/* can't know the cust number, becaue we broadcast here! */
			/* DepartmentSalesman [identifier] informs the Customer/PrivilegeCustomer [identifier] that [item] is restocked. */
		}
		else{
                  Release(individualSalesmanLock[myDept][myIndex]);
		}
	}
	
}


/* Goods loader code			__LOADER__ */
void GoodsLoader() {
	int myID;
	int currentDept = -1;	/* the department i am dealing with */
	int mySalesID = -1;		/* the sales i am helping */
	int shelf = -1;			/* the shelf i am dealing with */
	int foundNewOrder = 0;
	int i, j, inHands, restoreTrollies, qtyInHands, newShelfToStock;
	
	Acquire(loaderIndexLock);
	myID = nextLoaderIndex;
	nextLoaderIndex++;
	Release(loaderIndexLock);


	/*//true if i go to help a salesman, and he was signalling for a loader to restock something */
	Acquire(inactiveLoaderLock);
	/* normal action loop */
	while(1) {
		if(!foundNewOrder) {	/* if i don't have a new order (from my last run) go to sleep */
			loaderStatus[myID] = LOAD_NOT_BUSY;
			if(mySalesID != -1){
				/*cout << "GoodsLoader [" << myID << "] is waiting for orders to restock." << endl;*/
			  NPrint("GoodsLoader [%d] is waiting for orders to restock.\n", sizeof("GoodsLoader [%d] is waiting for orders to restock.\n"), myID, 0);
			}
			Wait(inactiveLoaderCV, inactiveLoaderLock);
			/* at this point, I have just been woken up from the inactive loaders waiting area */
		}
		foundNewOrder = 0;	/* initialize that I have not found a new order for this run yet */

		loaderStatus[myID] = LOAD_HAS_BEEN_SIGNALLED;
		mySalesID = -50;


		/* look through all departments to find out who signalled me */
		for(j = 0; j < numDepartments; j++) {
			Acquire(salesLock[j]);
			for(i = 0; i < numSalesmen; i++) {
				if(currentSalesStatus[j][i] == SALES_SIGNALLING_LOADER) {	/* i found a person who was signalling for a loader! */
					mySalesID = i;
					currentDept = j;
					currentSalesStatus[currentDept][mySalesID] = SALES_BUSY;
					break;
				}
			}
			Release(salesLock[j]);
			if(mySalesID != -50) {	/* used to break the second level of for loop if i found a salesman who needs me */
				break;
			}
		}
		Release(inactiveLoaderLock);

		if(mySalesID == -50){ /* the loader was signaled by the manager to get trollys (signalled, but no salesmen are signalling for me) */
			Acquire(managerItemsLock);
			inHands = 0;
			i = 0;
			while(!QueueEmpty(managerItems)){
				/* simulates the manager putting the items in the stockroom */
			  inHands = QueueFront(managerItems);
			  QueuePop(managerItems);
				i++;
				Acquire(stockRoomLock);
				NPrint("Goodsloader [%d] put item [%d] back in the stock room from the manager\n", sizeof("Goodsloader [%d] put item [%d] back in the stock room from the manager\n"), NEncode2to1(myID, inHands), 0);
				inHands = 0;
				Release(stockRoomLock);
			}
			Release(managerItemsLock);

			/* moves trollies back to the front of the store */
			Acquire(displacedTrollyLock);
			restoreTrollies = 0;
			if(displacedTrollyCount > 0){
				restoreTrollies = displacedTrollyCount;
				displacedTrollyCount = 0;
			}
			Release(displacedTrollyLock);

			if(restoreTrollies != 0) {
				Acquire(trollyLock);
				trollyCount += restoreTrollies;
				Broadcast(trollyCV, trollyLock);
				Release(trollyLock);
			}

		}
		else{	/* It was not the manager who signalled me */
			Acquire(individualSalesmanLock[currentDept][mySalesID]);
			shelf = salesDesk[currentDept][mySalesID];	/* read the shelf that needs stocking in from the desk */
			salesDesk[currentDept][mySalesID] = -1;		/* write back that i was not on a previous job */

			NPrint("GoodsLoader [%d] is informed by DepartmentSalesman [%d] of Department [%d] to restock [%d]", sizeof("GoodsLoader [%d] is informed by DepartmentSalesman [%d] of Department [%d] to restock [%d]"), NEncode2to1(myID, mySalesID), NEncode2to1(currentDept, shelf));

			Signal(salesmanCV[currentDept][mySalesID], individualSalesmanLock[currentDept][mySalesID]);
			Wait(salesmanCV[currentDept][mySalesID], individualSalesmanLock[currentDept][mySalesID]);
			Release(individualSalesmanLock[currentDept][mySalesID]);


			/* Restock items */
			qtyInHands = 0;
			Acquire(shelfLock[currentDept][shelf]);
			while(shelfInventory[currentDept][shelf] < maxShelfQty) {
				Release(shelfLock[currentDept][shelf]);

				Acquire(currentLoaderInStockLock);
				if(currentLoaderInStock != -1 || waitingForStockRoomCount > 0){/* if either someone is in line or there is a loader in the stock room... */
				  NPrint("GoodsLoader [%d] is waiting for GoodsLoader[%d] to leave the StockRoom.\n", sizeof("GoodsLoader [%d] is waiting for GoodsLoader[%d] to leave the StockRoom.\n"), NEncode2to1(myID, currentLoaderInStock), 0);
					waitingForStockRoomCount++;
					Wait(currentLoaderInStockCV, currentLoaderInStockLock);
				}
				Acquire(stockRoomLock);
				if(currentLoaderInStock != -1){ /* if someone was in the stockroom, change the current loader to this loader's id, wake him up, and then wait for him to acknowledge */
					currentLoaderInStock = myID;
					Signal(stockRoomCV, stockRoomLock);
					Wait(stockRoomCV, stockRoomLock);
				}
				else{ /* if no one was in the stockroom, then just set the current loader to this loader's id */
					currentLoaderInStock = myID;
				}
				NPrint("GoodsLoader [%d] is in the StockRoom and got [%d]\n", sizeof("GoodsLoader [%d] is in the StockRoom and got [%d]\n"), NEncode2to1(myID, shelf), 0);

				Release(currentLoaderInStockLock);
				qtyInHands++; /* grab an item */
				Acquire(currentLoaderInStockLock);
				if(waitingForStockRoomCount > 0){ /* if there are people in line, then signal the next person in line and decrement the line count */
					Signal(currentLoaderInStockCV, currentLoaderInStockLock);
					waitingForStockRoomCount--;
					Release(currentLoaderInStockLock);
					Wait(stockRoomCV, stockRoomLock);
					NPrint("GoodsLoader [%d] leaves StockRoom.\n", sizeof("GoodsLoader [%d] leaves StockRoom.\n"), myID, 0);
					Signal(stockRoomCV, stockRoomLock);
				}
				else{
					currentLoaderInStock = -1; /* if no one is in line, then set the current loader to -1, which signifies that the stockroom was empty prior */
					NPrint("GoodsLoader [%d] leaves StockRoom.\n", sizeof("GoodsLoader [%d] leaves StockRoom.\n"), myID, 0);
					Release(currentLoaderInStockLock);
				}
				Release(stockRoomLock);
				for(j = 0; j < 5; j++) {
				  Yield();
				}

				/* check the shelf i am going to restock */
				Acquire(shelfLock[currentDept][shelf]);
				if(testNumber == 8){
				  NPrint("GoodsLoader [%d] is in the act of restocking shelf [%d] in Department [%d].\n", sizeof("GoodsLoader [%d] is in the act of restocking shelf [%d] in Department [%d].\n"), NEncode2to1(myID, shelf), currentDept);
				  NPrint("The shelf had [%d] but GoodsLoader [%d] has added one more.\n", sizeof("The shelf had [%d] but GoodsLoader [%d] has added one more.\n"), NEncode2to1(shelfInventory[currentDept][shelf]), currentDept);
				  }
				if(shelfInventory[currentDept][shelf] == maxShelfQty) {	/* check to see if a shelf needs more items */
				  NPrint("GoodsLoader [%d] has restocked [%d] in Department [%d].\n", sizeof("GoodsLoader [%d] has restocked [%d] in Department [%d].\n"), NEncode2to1(myID, shelf), currentDept);
					qtyInHands = 0;
					break;
				}
				shelfInventory[currentDept][shelf] += qtyInHands;	/* put more items on it */
				qtyInHands = 0;
			}
			Release(shelfLock[currentDept][shelf]);


			/* We have finished restocking.  now wait in line/inform sales */
			Acquire(salesLock[currentDept]);

			mySalesID = -50;
			/* first look for someone who is signalling for a loader, since we can multipurpose and take a new job, while also informing that we finished a previous one */
			for(i = 0; i < numSalesmen; i++) {
				if(currentSalesStatus[currentDept][i] == SALES_SIGNALLING_LOADER) {
					/* all salesmen are busy trying to get loaders, but the loaders are out and trying to tell them that a job was finished */
					mySalesID = i;

					/* Ready to go talk to sales */
					newShelfToStock = salesDesk[currentDept][mySalesID];

					currentlyTalkingTo[currentDept][mySalesID] = GOODSLOADER;
					currentSalesStatus[currentDept][mySalesID] = SALES_BUSY;
					Release(salesLock[currentDept]);
					Acquire(individualSalesmanLock[currentDept][mySalesID]);
					salesDesk[currentDept][mySalesID] = shelf;
					Signal(salesmanCV[currentDept][mySalesID], individualSalesmanLock[currentDept][mySalesID]);
					Wait(salesmanCV[currentDept][mySalesID], individualSalesmanLock[currentDept][mySalesID]);
					salesDesk[currentDept][mySalesID] = myID;
					Signal(salesmanCV[currentDept][mySalesID], individualSalesmanLock[currentDept][mySalesID]);
					Release(individualSalesmanLock[currentDept][mySalesID]);

					/* i have talked to sales and they had a new job for me */
					shelf = newShelfToStock;
					foundNewOrder = 1;
					break;
				}
			}
			if(mySalesID == -50) {
				/* no one was signalling, so look for free salesmen to go report to */
				for(i = 0; i < numSalesmen; i++) {
					if(currentSalesStatus[currentDept][i] == SALES_NOT_BUSY) {
						mySalesID = i;

						/* Ready to go talk to sales */
						currentlyTalkingTo[currentDept][mySalesID] = GOODSLOADER;
						currentSalesStatus[currentDept][mySalesID] = SALES_BUSY;
						Release(salesLock[currentDept]);
						Acquire(individualSalesmanLock[currentDept][mySalesID]);
						salesDesk[currentDept][mySalesID] = shelf;
						Signal(salesmanCV[currentDept][mySalesID], individualSalesmanLock[currentDept][mySalesID]);
						Wait(salesmanCV[currentDept][mySalesID], individualSalesmanLock[currentDept][mySalesID]);
						salesDesk[currentDept][mySalesID] = myID;
						Signal(salesmanCV[currentDept][mySalesID], individualSalesmanLock[currentDept][mySalesID]);
						Release(individualSalesmanLock[currentDept][mySalesID]);
						break;
					}
				}
			}


			if(!foundNewOrder) {	/* i have to get in line, because i didn't find a new order from someone signalling */
									/* (i would have given them my information when i took their order) */
				if(mySalesID == -50) {	/* if i have STILL not found anyone, then i do need to get in line */
					loaderWaitingLineCount[currentDept]++;

					Wait(loaderCV[currentDept], salesLock[currentDept]);

					for(i = 0; i < numSalesmen; i++) {	/* find the salesman who signalled me out of line */
						if(currentSalesStatus[currentDept][i] == SALES_READY_TO_TALK_TO_LOADER) {
							mySalesID = i;

							/* Ready to go talk to a salesman */
							currentlyTalkingTo[currentDept][mySalesID] = GOODSLOADER;
							currentSalesStatus[currentDept][mySalesID] = SALES_BUSY;

							Release(salesLock[currentDept]);
							Acquire(individualSalesmanLock[currentDept][mySalesID]);
							salesDesk[currentDept][mySalesID] = shelf;
							Signal(salesmanCV[currentDept][mySalesID], individualSalesmanLock[currentDept][mySalesID]);
							Wait(salesmanCV[currentDept][mySalesID], individualSalesmanLock[currentDept][mySalesID]);
							salesDesk[currentDept][mySalesID] = myID;
							Signal(salesmanCV[currentDept][mySalesID], individualSalesmanLock[currentDept][mySalesID]);
							Release(individualSalesmanLock[currentDept][mySalesID]);

							break;
						}
					}
				}
			}
		}

		/* Look at all depts to see if anyone else has a job that i should be aware of */
		Acquire(inactiveLoaderLock);
		for(j = 0; j < numDepartments; j++) {
			Acquire(salesLock[j]);
			for(i = 0; i < numSalesmen; i++) {
				if(currentSalesStatus[j][i] == SALES_SIGNALLING_LOADER) {
					foundNewOrder = 1;
					break;
				}
			}
			Release(salesLock[j]);
			if(foundNewOrder) {	/* break out of the outer loop if we did in fact find someone */
				break;
			}
		}
	}
}


/* ================ */
/*  Tests */
/* ================ */



void initSalesmen(){
  int i, j;
  for(i = 0; i < numDepartments; i++){
    greetingCustWaitingLineCount[i] = 0;
    complainingCustWaitingLineCount[i] = 0;
    loaderWaitingLineCount[i] = 0;
    for(j = 0; j < numSalesmen; j++){
      currentSalesStatus[i][j] = SALES_BUSY;
      currentlyTalkingTo[i][j] = UNKNOWN;
      salesCustNumber[i][j] = 0;
      salesBreakBoard[i][j] = 0;
      individualSalesmanLock[i][j] = CreateLock("Individual Salesman Lock", sizeof("Individual Salesman Lock"));
      salesmanCV[i][j] = CreateCondition("Salesman CV", sizeof("Salesman CV"));
      salesBreakCV[i][j] = CreateCondition("Sales Break CV", sizeof("Sales Break CV"));
    }
  }
}

void initLoaders(){
  int i;
  inactiveLoaderLock = CreateLock("Inactive Loader Lock", sizeof("Inactive Loader Lock"));
  inactiveLoaderCV = CreateCondition("inactive loader CV", sizeof("inactive loader CV"));
  currentLoaderInStock = -1;
  currentLoaderInStockLock = CreateLock("current loader in stock lock", sizeof("current loader in stock lock"));
  currentLoaderInStockCV = CreateCondition("Current loader in stock CV", sizeof("Current loader in stock CV"));
  stockRoomCV = CreateCondition("stock room CV", sizeof("stock room CV"));
  waitingForStockRoomCount = 0;
  for(i = 0; i < numLoaders; i++){
    loaderStatus[i] = LOAD_NOT_BUSY;
  }
}

void initCustomerCashier(){
  int i;

  for(i = 0; i < custNumber; i++){
    privCustomers[i] = 0;
  }
  customersDone = 0;
  cashierLinesLock = CreateLock("cashier line lock", sizeof("cashier line lock"));
  managerItemsLock = CreateLock("Manager Items Lock", sizeof("Manager Items Lock"));
  managerLock = CreateLock("Manager Lock", sizeof("Manager Lock"));
  managerCV = CreateCondition("Manager CV", sizeof("Manager CV"));
  inactiveLoaderCV = CreateCondition("Inactive Loader CV", sizeof("Inactive Loader CV"));
  inactiveLoaderLock = CreateLock("Inactive Loader Lock", sizeof("Inactive Loader Lock"));
  displacedTrollyCount = 0;
  displacedTrollyLock = CreateLock("Displaced Trolly Lock", sizeof("Displaced Trolly Lock"));
  trollyLock = CreateLock("trolly lock", sizeof("trolly lock"));
  trollyCV = CreateCondition("trolly CV", sizeof("trolly CV"));
  trollyCount = numTrollies;
  for(i = 0; i < cashierNumber; i++){
    privilegedCashierLineCV[i] = CreateCondition("privileged cashier line cv", sizeof("privileged cashier line cv"));
    unprivilegedCashierLineCV[i] = CreateCondition("unprivileged cashier line cv", sizeof("unprivileged cashier line cv"));
    cashierLock[i] = CreateLock("cashier lock", sizeof("cashier lock"));
    cashierToCustCV[i] = CreateCondition("cashier to cust cv", sizeof("cashier to cust cv"));
    cashierStatus[i] = CASH_BUSY;
    privilegedLineCount[i] = 0;
    unprivilegedLineCount[i] = 0;
    cashierFlags[i] = -1;
    cashierDesk[i] = -2;
    cashRegister[i] = 0;
  }
}

void initShelvesWithQty(int q){
  int i, j;
  stockRoomLock = CreateLock("Stock Room Lock", sizeof("Stock Room Lock"));
  for(i = 0; i < numDepartments; i++){
    for(j = 0; j < numItems; j++){
      shelfLock[i][j] = CreateLock("Shelf Lock", sizeof("Shelf Lock"));
      shelfCV[i][j] = CreateCondition("ShelfCV", sizeof("ShelfCV"));
      shelfInventory[i][j] = q;
    }
  }
}

void printInitialConditions() {
  int i;
  NPrint("Test %d beginning with initial conditions: \n", sizeof("Test %d beginning with initial conditions: \n"), testNumber, 0);
  NPrint("Number of Cashiers = [%d]\n", sizeof("Number of Cashiers = [%d]\n"), cashierNumber, 0);
  NPrint("Number of GoodsLoaders = [%d]\n", sizeof("Number of GoodsLoaders = [%d]\n"), numLoaders, 0);
  NPrint("Number of PrivilegedCustomers = [NRandomly generated, 30%% chance]\n", sizeof("Number of PrivilegedCustomers = [NRandomly generated, 30% chance]\n"), 0, 0);
  NPrint("Number of Customers = [%d]\n", sizeof("Number of Customers = [%d]\n"), custNumber, 0);
  NPrint("Number of Managers [1]\n", sizeof("Number of Managers [1]\n"), 0,0);
  NPrint("Number of DepartmentSalesmen = [%d]\n", sizeof("Number of DepartmentSalesmen = [%d]\n"), numSalesmen, 0);
  NPrint("Items:\n", sizeof("Items:\n"), 0,0);
  NPrint("  Number - Price\n", sizeof("  Number - Price\n"), 0,0);
  for(i = 0; i < numItems; i++) {
    NPrint("  %d - %d\n", sizeof("  %d - %d\n"), NEncode2to1(i, scan(i)), 0);
  }
  NPrint("\n", sizeof("\n"), 0, 0);

}


void Problem2(){
}


void testCustomerGettingInLine(){
  int i;
	testNumber = 1;
	customerCash = 100000;
	cashierNumber = 5;
	custNumber = 10;
	numSalesmen = 1;
	numDepartments = 1;
	numLoaders = 1;
	numTrollies = 20;
	initCustomerCashier();
	/* cout << "Test 1: please note that all cashier lines are set to be 5 except for cashier 4's line for unprivileged customers, which is set to 1 "; */
	/* cout << " and cashier 3's line for unprivileged customer, which is set to 3."; */
	/* cout << "\tThe privileged status of Customers is set NRandomly, so we will create enough "; */
	/* cout << "customer threads to likely fill up a line and start choosing other lines" << endl; */
	initShelvesWithQty(30);
	initSalesmen();
	initLoaders();
	for(i = 0; i < cashierNumber; i++){
		cashierStatus[i] = CASH_BUSY;
		privilegedLineCount[i] = 5;
		unprivilegedLineCount[i] = 5;
	}
	unprivilegedLineCount[cashierNumber-1] = 1;
	unprivilegedLineCount[cashierNumber-2] = 3;

	Fork(Salesman, "Salesman thread", sizeof("Salesman thread"));
	for(i = 0; i < custNumber; i++) {
		Fork(Customer, "Customer thread", sizeof("Customer thread"));
	}
}

int main(int argv, char** argc){

  int choice;
        NPrint("Menu: \n", sizeof("Menu: \n"), 0, 0);
	NPrint("1. Test customer choosing from cashier lines\n", sizeof("1. Test customer choosing from cashier lines\n"), 0, 0);
	NPrint("2. Test manager only interacting with one customer or cashier at a Time.\n", sizeof("2. Test manager only interacting with one customer or cashier at a Time.\n"), 0, 0);
	NPrint("3. Test cashier receipt reception and cashier waiting for cusotmer to leave\n", sizeof("3. Test cashier receipt reception and cashier waiting for cusotmer to leave\n"), 0, 0);
	NPrint("4. Test cashiers scan all items\n", sizeof("4. Test cashiers scan all items\n"), 0, 0);
	NPrint("5. Test cashiers being sent on break\n", sizeof("5. Test cashiers being sent on break\n"), 0, 0);
	NPrint("6. Test cashiers being brough back from break\n", sizeof("6. Test cashiers being brough back from break\n"), 0, 0);
	NPrint("7. Test sales never suffering a race condition\n", sizeof("7. Test sales never suffering a race condition\n"), 0, 0);
	NPrint("8. Test Goods Loaders don't restock when a Customer is trying to get an item\n", sizeof("8. Test Goods Loaders don't restock when a Customer is trying to get an item\n"), 0, 0);
	NPrint("9. Test only one Goods Loader enters the stock room at a Time\n", sizeof("9. Test only one Goods Loader enters the stock room at a Time\n"), 0, 0);
	NPrint("10. Test Customrs wait for items to be restocked\n", sizeof("10. Test Customrs wait for items to be restocked\n"), 0, 0);
	NPrint("11. Run full simulation\n", sizeof("11. Run full simulation\n"), 0, 0);
	NPrint("12. Run full simulation with some predetermined values\n", sizeof("12. Run full simulation with some predetermined values\n"), 0, 0);
	NPrint("Please input the number option you wish to take: \n", sizeof("Please input the number option you wish to take: \n"), 0, 0);

	choice = ReadInt("> ", sizeof("> "));

	switch(choice){
	  case 1:
		printInitialConditions();
	    testCustomerGettingInLine();
	    break;
	  case 2:
	    initCustomerArrays();
	    initLoaderArrays();
	    initCashierArrays(1);
	    initSalesmanArrays();
	    initManagerArrays();
	    NPrint("Customer index lock is: %d\n", sizeof("Customer index lock is: %d\n"), customerIndexLock);
	    Fork(Customer, "ACUST", sizeof("ACUST"));
	    break;
	  case 3:
	    break;
	  case 4:
	    break;
	  case 5:
	    break;
	  case 6:
	    break;
	  case 7:
	    break;
	  case 8:
	    break;
	  case 9:
	    break;
	  case 10:
	    break;
	  case 11:
	    break;
	  case 12:
	    break;
	}
	Exit(0);

}
