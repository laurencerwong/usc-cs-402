#include "../userprog/syscall.h"

/*
#define MAX_SHELF_QTY 20
#define MAX_TROLLY 40
#define MAX_SALESMEN 3
#define MAX_LOADERS 10
#define MAX_CUSTOMERS 30
#define NUM_ITEMS 10
*/

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
/* Lock* cashierLinesLock; */


/* array of CVs for privileged customers waiting in line */
/* one for each cashier */
/* Condition** privilegedCashierLineCV; */
int privilegedCashierLineCV[MAX_CASHIERS];

/* array of CVs for unprivileged customers waiting in line */
/* one for each cashier */
/* Condition** unprivilegedCashierLineCV; */
int unprivilegedCashierLineCV[MAX_CASHIERS];
 
/* Lock associated with cashToCustCV */
/* controls access to each cashier */
/* Lock** cashierLock; */
int cashierLock[MAX_CASHIERS];
/* array of CVs, one for each cashier, that the cashier and */
/* customer use to communicate during checkout */
/* Condition** cashierToCustCV; */
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
/* Condition ***salesBreakCV; */
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

/* WhomIWantToTalkTo *loaderCurrentlyTalkingTo = new WhomIWantToTalkTo[numLoaders]; */


typedef enum {LOAD_NOT_BUSY, LOAD_STOCKING, LOAD_HAS_BEEN_SIGNALLED} LoaderStatus;
LoaderStatus loaderStatus[MAX_LOADERS];

/* Lock *inactiveLoaderLock; */
int inactiveLoaderLock;

/* Condition *inactiveLoaderCV; */
int inactiveLoaderCV;

/* Lock ***shelfLock; */
int shelfLock [MAX_DEPARTMENTS][MAX_ITEMS];

/* Condition ***shelfCV; */
int shelfCV [MAX_DEPARTMENTS][MAX_ITEMS];

int shelfInventory [MAX_DEPARTMENTS][MAX_ITEMS];

/* Lock *stockRoomLock; */
int stockRoomLock;

int nextLoaderIndex = 0;
int loaderIndexLock;

/* Manager data */
int cashierFlags[MAX_CASHIERS]; /* will be set to a customer ID in the slot corresponding to the */
/* index of the cashier who sets the flag. */

/* Lock* managerLock; //will be the lock behind the manager CV */
int managerLock;
/* Condition* managerCV; //will be the CV the manager and customer use to communicate */
int managerCV;
int managerDesk = 0; /* will be place customer puts items to show to manager */

/* manager stores items taken from customer so they can be taken by Loader */
/* queue<int> managerItems; */
/* lock to protect managerItems; */
/* Lock* managerItemsLock; */
int managerItems;
int managerItemsLock;

int cashierTotals[MAX_CASHIERS];
int numSalesmenOnBreak[MAX_DEPARTMENTS];

/* Trolleys */
int trollyCount;

/* Lock controlling access to trollyCount; */
/* Lock* trollyLock; */
int trollyLock;

/* CV for customers to wait on if no trollies are there */
/* associated with trollyLock */
/* Condition* trollyCV; */
int trollyCV;

/* Discarded trollies, waiting to be replaced by Goods Loader */
int displacedTrollyCount = 0;

/* Lock controlling access to displaced trolly count */
/* Lock* displacedTrollyLock; */
int displacedTrollyLock;

/* which goodsloader is currently in the stockroom */
int currentLoaderInStock = -1;

/* Lock for currentLockInStock */
/* Lock *currentLoaderInStockLock; */
int currentLoaderInStockLock;

/* Control variable for current loader in stock */
/* Condition *currentLoaderInStockCV; */
int currentLoaderInStockCV;

/* Control variable for waiting for the stock room */
/* Condition *stockRoomCV; */
int stockRoomCV;

/* Number of goodsloaders waiting for the stockroom */
int waitingForStockRoomCount = 0;

/* Which customers are privileged or not */

int customersDone = 0;

int testNumber = -1;
/* queue<int> cashiersOnBreak; //allows manager to remember which cashiers he's sent on break */

int numCashiersOnBreak = 0;

/* Function prototypes if needed */

/*Statically allocate customer stuff*/
/*int numItemsToBuy [MAX_CUSTOMERS];
int itemsToBuy [MAX_CUSTOMERS][MAX_ITEMS];
int qtyItemsToBuy [MAX_CUSTOMERS][MAX_ITEMS];
int itemsInCart [MAX_CUSTOMERS][MAX_ITEMS];
int qtyItemsInCart [MAX_CUSTOMERS][MAX_ITEMS];*/
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
}

void initCustomerArrays(){
  int i, j;
  customerIndexLock = CreateLock("customerIndexLock", sizeof("customerIndexLock"));
  trollyLock  = CreateLock("trollyLock", sizeof("trollyLock"));
  trollyCount = numTrollies;
  trollyCV = CreateCondition("trollyCV", sizeof("trollyCV"));
  displacedTrollyLock = CreateLock("displacedTrollyLock", sizeof("displacedTrollyLock"));
}

/* void initShelves() { */
/* 	char* name; */
/* 	stockRoomLock = new Lock("Stock room lock"); */
/*  */
/* 	shelfLock = new Lock**[numDepartments]; */
/* 	shelfCV = new Condition**[numDepartments]; */
/* 	shelfInventory = new int*[numDepartments]; */
/*  */
/* 	for(int i = 0; i < numDepartments; i++) { */
/* 		shelfLock[i] = new Lock*[numItems]; */
/* 		shelfCV[i] = new Condition*[numItems]; */
/* 		shelfInventory[i] = new int[numItems]; */
/* 	} */
/*  */
/* 	for(int i = 0; i < numDepartments; i++) { */
/* 		for(int j = 0; j < numItems; j++) { */
/* 			name = new char[20]; */
/* 			sprintf(name, "shelfLock dept%d item%d", i, j); */
/* 			shelfLock[i][j] = new Lock("name"); */
/* 			name = new char[20]; */
/* 			sprintf(name, "shelCV dept%d item%d", i, j); */
/* 			shelfCV[i][j] = new Condition("name"); */
/* 			shelfInventory[i][j] = 0; */
/* 		} */
/* 	} */
/* } */
/*  */
/* void initShelvesWithQty(int q) { */
/* 	//cout << "Initializing shelves with qty: " << q << endl; */
/*  */
/* 	stockRoomLock = new Lock("Stock room lock"); */
/*  */
/* 	shelfLock = new Lock**[numDepartments]; */
/* 	shelfCV = new Condition**[numDepartments]; */
/* 	shelfInventory = new int*[numDepartments]; */
/*  */
/* 	for(int i = 0; i < numDepartments; i++) { */
/* 		shelfLock[i] = new Lock*[numItems]; */
/* 		shelfCV[i] = new Condition*[numItems]; */
/* 		shelfInventory[i] = new int[numItems]; */
/* 	} */
/*  */
/* 	for(int i = 0; i < numDepartments; i++) { */
/* 		for(int j = 0; j < numItems; j++) { */
/* 			shelfLock[i][j] = new Lock("shelfLock"); */
/* 			shelfCV[i][j] = new Condition("shelfCV"); */
/* 			shelfInventory[i][j] = q; */
/* 		} */
/* 	} */
/* } */
/*  */
/* void initSalesmen(){ */
/* 	//cout << "Sales init level 1..." << endl; */
/* 	currentSalesStatus = new SalesmanStatus*[numDepartments]; */
/* 	currentlyTalkingTo = new WhomIWantToTalkTo*[numDepartments]; */
/* 	salesCustNumber = new int*[numDepartments]; */
/* 	salesDesk = new int*[numDepartments]; */
/* 	salesBreakBoard = new int*[numDepartments]; */
/* 	salesBreakCV = new Condition**[numDepartments]; */
/* 	greetingCustWaitingLineCount = new int[numDepartments]; //How many customers need to be greeted */
/* 	complainingCustWaitingLineCount = new int[numDepartments]; //How many customers need to be helped with empty shelves */
/* 	loaderWaitingLineCount = new int[numDepartments];	//How many loaders are waiting on salesmen */
/*  */
/* 	individualSalesmanLock = new Lock**[numDepartments]; //The lock that each salesman uses for his own "desk" */
/* 	salesLock = new Lock*[numDepartments]; //The lock that protects the salesmen statuses and the line for all three */
/*  */
/* 	salesmanCV = new Condition**[numDepartments]; //The condition variable for one on one interactions with the Salesman */
/*  */
/* 	greetingCustCV = new Condition*[numDepartments]; */
/* 	complainingCustCV = new Condition*[numDepartments]; */
/* 	loaderCV = new Condition*[numDepartments]; */
/*  */
/* 	//cout << "Sales init level 2..." << endl; */
/* 	for(int i = 0; i < numDepartments; i++) { */
/* 		currentSalesStatus[i] = new SalesmanStatus[numSalesmen]; */
/* 		currentlyTalkingTo[i] = new WhomIWantToTalkTo[numSalesmen]; */
/* 		salesCustNumber[i] = new int[numSalesmen]; */
/* 		salesDesk[i] = new int[numSalesmen]; */
/* 		salesBreakBoard[i] = new int[numSalesmen]; */
/* 		salesBreakCV[i] = new Condition*[numSalesmen]; */
/*  */
/* 		individualSalesmanLock[i] = new Lock*[numSalesmen]; */
/* 		salesLock[i] = new Lock("Overall salesman lock"); */
/*  */
/* 		salesmanCV[i] = new Condition*[numSalesmen]; //The condition variable for one on one interactions with the Salesman */
/*  */
/* 		greetingCustCV[i] = new Condition ("Greeting Customer Condition Variable"); //The condition var that represents the line all the greeters stay in */
/* 		complainingCustCV[i] = new Condition("Complaining Customer Condition Variable"); //Represents the line that the complainers stay in */
/* 		loaderCV[i] = new Condition("GoodsLoader Condition Variable");	//Represents the line that loaders wait in */
/*  */
/* 		greetingCustWaitingLineCount[i] = 0; */
/* 		complainingCustWaitingLineCount[i] = 0; */
/* 		loaderWaitingLineCount[i] = 0; */
/* 	} */
/*  */
/* 	//cout << "Sales init level 3..." << endl; */
/* 	for(int i = 0; i < numDepartments; i++) { */
/* 		for(int j = 0; j < numSalesmen; j++){ */
/* 			currentSalesStatus[i][j] = SALES_BUSY; */
/* 			currentlyTalkingTo[i][j] = UNKNOWN; */
/* 			salesCustNumber[i][j] = 0; */
/* 			salesBreakBoard[i][j] = 0; */
/* 			individualSalesmanLock[i][j] = new Lock(("Indivisdual Salesman Lock")); */
/* 			salesmanCV[i][j] = new Condition(("Salesman Condition Variable")); */
/* 			salesBreakCV[i][j] = new Condition("Salesman Break CV"); */
/* 		} */
/* 	} */
/* 	//cout << "Done initializing sales" << endl; */
/* } */

/* void initLoaders() { */
/* 	//cout << "Initializing loaders..." << endl; */
/* 	inactiveLoaderLock = new Lock("Lock for loaders waiting to be called on"); */
/* 	inactiveLoaderCV = new Condition("CV for loaders waiting to be called on"); */
/* 	currentLoaderInStock = -1; */
/* 	currentLoaderInStockLock = new Lock("cur loader in stock lock"); */
/* 	currentLoaderInStockCV = new Condition("Current Loader in Stock CV"); */
/* 	stockRoomCV = new Condition("Stock Room CV"); */
/* 	waitingForStockRoomCount = 0; */
/* 	loaderStatus = new LoaderStatus[numLoaders]; */
/*  */
/* 	for(int i = 0; i < numLoaders; i++){ */
/* 		loaderStatus[i] = LOAD_NOT_BUSY; */
/* 	} */
/* } */
/*  */
/* void initTrolly() { */
/* 	//cout << "Initializing trollies..." << endl; */
/*  */
/* 	char* name; */
/*  */
/* 	trollyCount = numTrollies; */
/* 	displacedTrollyCount = 0; */
/* 	name = new char[20]; */
/* 	name = "trolly lock"; */
/* 	trollyLock = new Lock(name); */
/* 	name = new char[20]; */
/* 	name = "trolly CV"; */
/* 	trollyCV = new Condition(name); */
/* 	name = new char[20]; */
/* 	name = "displaced trolly lock"; */
/* 	displacedTrollyLock = new Lock(name); */
/* } */
/*  */
/* void initCustomerCashier(){ */
/* 	//cout << "Initializing CustomerCashier..." << endl; */
/*  */
/* 	for(int i = 0; i < maxCustomers; i++){ */
/* 		privCustomers[i] = 0; //sets all the customers to unprivileged */
/* 	} */
/* 	char* name; */
/* 	name = new char[20]; */
/* 	name = "cashier line lock"; */
/* 	customersDone =0; */
/* 	//	cashierLinesLock = new Lock(name); */
/* 	name = new char[20]; */
/* 	//name = "manaager items lock"; */
/* 	managerItemsLock = new Lock(name); */
/* 	name = new char[20]; */
/* 	name = "inactive loader cv"; */
/* 	inactiveLoaderCV = new Condition(name); */
/* 	name = new char[20]; */
/* 	name = "inactive loader lock"; */
/* 	inactiveLoaderLock = new Lock(name); */
/* 	//privilegedCashierLineCV = new Condition*[cashierNumber]; */
/* 	//unprivilegedCashierLineCV = new Condition*[cashierNumber]; */
/* 	//cashierLock = new Lock*[cashierNumber]; */
/* 	//cashierToCustCV = new Condition*[cashierNumber]; */
/* 	//cashierStatus = new CashierStatus[cashierNumber]; */
/* 	//privilegedLineCount = new int[cashierNumber]; */
/* 	//unprivilegedLineCount = new int[cashierNumber]; */
/* 	//cashierDesk = new int[cashierNumber]; */
/* 	//cashierFlags = new int[cashierNumber]; */
/* 	//cashRegister = new int[cashierNumber]; */
/* 	cashierBreakBoard = new int[cashierNumber]; */
/* 	displacedTrollyCount = 0; */
/* 	name = new char[20]; */
/* 	name = "trolly lock"; */
/* 	trollyLock = new Lock(name); */
/* 	name = new char[20]; */
/* 	name = "trolly CV"; */
/* 	trollyCV = new Condition(name); */
/* 	name = new char[20]; */
/* 	name = "displaced trolly lock"; */
/* 	displacedTrollyLock = new Lock(name); */
/*  */
/* 	initTrolly(); */
/*  */
/* 	for(int i = 0; i < cashierNumber; i++){ */
/* 		name = new char [20]; */
/* 		sprintf(name,"priv cashier line CV %d",i); */
/* 		//privilegedCashierLineCV[i] = new Condition(name); */
/* 	} */
/* 	for(int i = 0; i < cashierNumber; i++){ */
/* 		name = new char[20]; */
/* 		sprintf(name, "unpriv cashier line CV %d", i); */
/* 		//unprivilegedCashierLineCV[i] = new Condition(name); */
/* 	} */
/* 	for(int i = 0; i < cashierNumber; i++){ */
/* 		name = new char[20]; */
/* 		sprintf(name, "cashier lock %d", i); */
/* 		//cashierLock[i] = new Lock(name); */
/* 	} */
/* 	for(int i = 0; i < cashierNumber; i++){ */
/* 		name = new char[20]; */
/* 		sprintf(name, "cashier cust CV %d", i); */
/* 		//cashierToCustCV[i] = new Condition(name); */
/* 	} */
/* 	for(int i = 0; i < cashierNumber; i++){ */
/* 		cashierStatus[i] = CASH_BUSY; */
/* 		privilegedLineCount[i] = 0; */
/* 		unprivilegedLineCount[i] = 0; */
/* 		cashierFlags[i] = -1; */
/* 		cashierDesk[i] = -2; */
/* 		cashRegister[i] = 0; */
/* 		cashierBreakBoard[i] = 0; */
/* 	} */
/*  */
/* 	name = new char[20]; */
/* 	name = "manager lock"; */
/* 	//managerLock = new Lock(name); */
/* 	name = new char[20]; */
/* 	name = "manager CV"; */
/* 	//managerCV = new Condition(name); */
/* } */

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
		  Fork(Salesman);
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
		/* numItemsToBuy = (Rand() % (numItems - 1)) + 1; */
		numItemsToBuy = (Rand() % numItems);
		myCash = Rand() % 200;
	}

	/*itemsToBuy = new int[numItemsToBuy];
	qtyItemsToBuy = new int[numItemsToBuy];
	itemsInCart = new int[numItemsToBuy];
	qtyItemsInCart = new int[numItemsToBuy];*/

	/*char* type = new char[20];
         int privileged;*/

	/* ---------Randomly generate whether this customer is privileged-------------- */
	Srand(myID + Time(NULL));
	r = Rand() % 10; /* Random value to set Customer either as privileged or unprivileged */
	if(r < 2){				/* 30% chance customer is privileged */
           /*privileged = 1;*/
           type = 1;
	}
	else /*privileged = 0;	/* 70% chance this customer is unprivileged*/
           type = 0;

	/* set char array for I/O purposes */
	/*if(privileged){
		type = "PrivilegedCustomer";
	}
	else type = "Customer";*/

	/* privileged = 0; */
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
			/* itemsToBuy[i] = Rand() % numItems;//getDepartmentFromItem(Rand() % numItems); */
                  itemsToBuy[i] = getDepartmentFromItem(Rand() % numItems);
			qtyItemsToBuy[i] = (Rand()% numItems);
			itemsInCart[i] = -1;
			qtyItemsInCart[i] = 0;
		}
	}
        /*
	cout << type << " [" << myID << "] will be buying:" << endl;
	cout << "Item - Qty" << endl;
	for(int j = 0; j < numItemsToBuy[myID]; j++) {
		cout << "  " << itemsToBuy[j] << " - " << qtyItemsToBuy[j] << endl;
                }*/
	if(type){
	  NPrint("Privileged Customer [%d] will be buying:\n", sizeof("Privileged Customer [%d] will be buying:\n"), myID, 0);
	}
	else{
	  NPrint("Customer [%d] will be buying:\n", sizeof("Customer [%d] will be buying:\n"), myID, 0);
	}
	NPrint("Item - Qty\n", sizeof("Item - Qty\n"), 0, 0);
	for(j = 0; j < numItemsToBuy; j++){
	  NPrint("  %d - %d\n", sizeof("  %d - %d\n"), NEncode2to1(itemsToBuy, qtyItemsToBuy[j]), 0);
	}

	/* ENTERS STORE */

	/*cout << type << " [" << myID << "] enters the SuperMarket" << endl;
	  cout << type << " [" << myID << "] wants to buy [" << numItemsToBuy << "] no.of items" << endl;*/

	if(type){
	  NPrint("Privileged Customer [%d] enters the SuperMarket\n", sizeof("Privileged Customer [%d] enters the SuperMarket\n"), myID, 0);
	  NPrint("Privileged Customer [%d] wants to buy [%d] no. of items\n", sizeof("Privileged Customer [%d] wants to buy [%d] no. of items\n"), NEncode2to1(myID, numItemsToBuy), 0);
	}
	else{
	  NPrint("Customer [%d] enters the SuperMarket\n", sizeof("Customer [%d] enters the SuperMarket\n"), myID, 0);
	  NPrint("Customer [%d] wants to buy [%d] no. of items\n", sizeof("Customer [%d] wants to buy [%d] no. of items\n"), NEncode2to1(myID, numItemsToBuy), 0);
	}
	  
	
	/*trollyLock->Acquire();
	while(trollyCount == 0) {
		cout << type << " [" << myID << "] gets in line for a trolly" << endl;
		trollyCV->Wait(trollyLock);
	}
	trollyCount--;
	cout << type << " [" << myID << "] has a trolly for shopping" << endl;
	trollyLock->Release();*/


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
                  /*	cout << type << " [" << myID << "] has finished shopping in department [" << currentDepartment << "]" << endl;
			cout << type << " [" << myID << "] wants to shop in department [" << targetDepartment << "]" << endl;*/
			/*salesLock[targetDepartment]->Acquire();*/
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

			/* printf("cust %d is about to select salesman, greeting line is: %d\n", myID,  greetingCustWaitingLineCount[targetDepartment]); */

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
				/*cout << type << " [" << myID << "] gets in line for the Department [" << targetDepartment << "]" << endl;*/
				if(type){
				  NPrint("Privileged Customer [%d] gets in line for the Department [%d]\n", sizeof("Privileged Customer [%d] gets in line for the Department [%d]\n"), NEncode2to1(myID, targetDepartment), 0);
				}
				else{				
				  NPrint("Customer [%d] gets in line for the Department [%d]\n", sizeof("Customer [%d] gets in line for the Department [%d]\n"), NEncode2to1(myID, targetDepartment), 0);
				}
				/*greetingCustCV[targetDepartment]->Wait(salesLock[targetDepartment]);*/
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

			/*individualSalesmanLock[targetDepartment][mySalesIndex]->Acquire(); /* Acquire the salesman's "desk" lock*/ 
                       Acquire(individualSalesmanLock[targetDepartment][mySalesIndex]);
		       /*cout << type << " [" << myID << "] is interacting with DepartmentSalesman[" << mySalesIndex << "] of Department[" << targetDepartment << "]" << endl;*/
		       if(type){
			 NPrint("Privileged Customer [%d] is interacting with DepartmentSalesmen[%d] of Department[%d]\n", sizeof("Privileged Customer [%d] is interacting with DepartmentSalesmen[%d] of Department[%d]\n"), NEncode2to1(myID, mySalesIndex), targetDepartment);
		       }
		       else{
			 NPrint("Customer [%d] is interacting with DepartmentSalesmen[%d] of Department[%d]\n", sizeof("Customer [%d] is interacting with DepartmentSalesmen[%d] of Department[%d]\n"), NEncode2to1(myID, mySalesIndex), targetDepartment);
		       }
			/*salesLock[targetDepartment]->Release();*/
                       Release(salesLock[targetDepartment]);
			salesCustNumber[targetDepartment][mySalesIndex] = myID; /* Sets the customer number of the salesman to this customer's index */

			/*salesmanCV[targetDepartment][mySalesIndex]->Signal(individualSalesmanLock[targetDepartment][mySalesIndex]);*/
                       Signal(salesmanCV[targetDepartment][mySalesIndex], individualSalesmanLock[targetDepartment][mySalesIndex]);
			/*salesmanCV[targetDepartment][mySalesIndex]->Wait (individualSalesmanLock[targetDepartment][mySalesIndex]);*/
                       Wait(salesmanCV[targetDepartment][mySalesIndex], individualSalesmanLock[targetDepartment][mySalesIndex]);
                       /*individualSalesmanLock[targetDepartment][mySalesIndex]->Release();*/
                       Release(individualSalesmanLock[targetDepartment][mySalesIndex]);


		}

		currentDepartment = targetDepartment;

		/* BEGINS SHOPPING */

		for( shelfNum = 0; shelfNum < numItems; shelfNum++) {
			if(shelfNum != itemsToBuy[i]) {
				continue;
			}

			/*cout << type << " ["<< myID << "] wants to buy [" << shelfNum << "]-[" << qtyItemsToBuy << "]" << endl;*/
			if(type){
			  NPrint("Privileged Customer [%d] wants to buy [%d]-[%d]\n", sizeof("Privileged Customer [%d] wants to buy [%d]-[%d]\n"), NEncode2to1(myID, shelfNum), qtyItemsToBuy[i]);
			}
			else{
			  NPrint("Customer [%d] wants to buy [%d]-[%d]\n", sizeof("Customer [%d] wants to buy [%d]-[%d]\n"), NEncode2to1(myID, shelfNum), qtyItemsToBuy[i]);
			}
			while(qtyItemsInCart[i] < qtyItemsToBuy[i]) {
                           /*shelfLock[currentDepartment][shelfNum]->Acquire();*/
                           Acquire(shelfLock[currentDepartment][shelfNum]);

				if(shelfInventory[currentDepartment][shelfNum] > qtyItemsToBuy[i]) {	/* if there are enough items, take what i need */
				  /*cout << type << " ["<< myID << "] has found ["
				    << shelfNum << "] and placed [" << qtyItemsToBuy << "] in the trolly" << endl;*/
				  if(type){
				    NPrint("Privileged Customer [%d] has found [%d] and placed [%d] in the trolly\n", sizeof("Privileged Customer [%d] has found [%d] and placed [%d] in the trolly\n"), NEncode2to1(myID, shelfNum), qtyItemsToBuy[i]);
				  }
				  else{
				    NPrint("Customer [%d] has found [%d] and placed [%d] in the trolly\n", sizeof("Customer [%d] has found [%d] and placed [%d] in the trolly\n"), NEncode2to1(myID, shelfNum), qtyItemsToBuy[i]);
				  }
				  if(testNumber == 8) /*cout << "There were " << shelfInventory[currentDepartment][shelfNum] << " before Customer " << myID << " took the item(s)." << endl;*/
				    NPrint("There were %d before Customer %d took the item(s).\n", sizeof("There were %d before Customer %d took the item(s).\n"), NEncode2to1(shelfInventory[currentDepartment][shelfNum], myID), 0);

					shelfInventory[currentDepartment][shelfNum] -= qtyItemsToBuy[i];
					itemsInCart[i] = shelfNum;
					qtyItemsInCart[i] += qtyItemsToBuy[i];
					/*shelfLock[currentDepartment][shelfNum]->Release();*/
                                       Release(shelfLock[currentDepartment][shelfNum]);
				}
				else {	/* We are out of this item, go tell sales! */
					/*cout << type << " [" << myID << "] was not able to find item " << shelfNum <<
					  " and is searching for department salesman " << currentDepartment << endl;*/
				  if(type){
				    NPrint("Privileged Customer [%d] was not able to find item %d and is searching for department salesman %d\n", sizeof("Privileged Customer [%d] was not able to find item %d and is searching for department salesman %d\n"), NEncode2to1(myID, shelfNum), currentDepartment);
				  }
				  else{
				    NPrint("Customer [%d] was not able to find item %d and is searching for department salesman %d\n", sizeof("Customer [%d] was not able to find item %d and is searching for department salesman %d\n"), NEncode2to1(myID, shelfNum), currentDepartment);
				  }
					/*shelfLock[currentDepartment][shelfNum]->Release();*/
                                       Release(shelfLock[currentDepartment][shelfNum]);
					/*salesLock[currentDepartment]->Acquire();*/
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
						/*complainingCustCV[currentDepartment]->Wait(salesLock[currentDepartment]);*/
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
					/*salesLock[currentDepartment]->Release();*/
                                       Release(salesLock[currentDepartment]);
					/*individualSalesmanLock[currentDepartment][mySalesID]->Acquire();*/
                                       Acquire(individualSalesmanLock[currentDepartment][mySalesID]);
				       /*cout << type << " [" << myID << "] is asking for assistance "
					 "from DepartmentSalesman [" << mySalesID << "]" << endl;*/

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

					/*salesmanCV[currentDepartment][mySalesID]->Signal(individualSalesmanLock[currentDepartment][mySalesID]);*/
                                       Signal(salesmanCV[currentDepartment][mySalesID], individualSalesmanLock[currentDepartment][mySalesID]);
					/*salesmanCV[currentDepartment][mySalesID]->Wait(individualSalesmanLock[currentDepartment][mySalesID]);	/* wait for sales to tell me to wait on shelf*/ 
                                       Wait(salesmanCV[currentDepartment][mySalesID], individualSalesmanLock[currentDepartment][mySalesID]);
					/*shelfLock[currentDepartment][shelfNum]->Acquire();*/
                                       Acquire(shelfLock[currentDepartment][shelfNum]);
					/*individualSalesmanLock[currentDepartment][mySalesID]->Release();*/
                                       Release(individualSalesmanLock[currentDepartment][mySalesID]);

					/* now i go wait on the shelf */
					/*shelfCV[currentDepartment][shelfNum]->Wait(shelfLock[currentDepartment][shelfNum]);*/
                                       Wait(shelfCV[currentDepartment][shelfNum], shelfLock[currentDepartment][shelfNum]);
				       /*cout << "DepartmentSalesman [" << mySalesID << "] informs the " << type << " [" << myID << "] that [" << shelfNum << "] is restocked." << endl;*/
				       if(type){
					 NPrint("DepartmentSalesman [%d] informs the Privileged Customer [%d] that [%d] is restocked.\n", sizeof("DepartmentSalesman [%d] informs the Privileged Customer [%d] that [%d] is restocked.\n"), NEncode2to1(mySalesID, myID), shelfNum);
				       }
				       else{
					 NPrint("DepartmentSalesman [%d] informs the Customer [%d] that [%d] is restocked.\n", sizeof("DepartmentSalesman [%d] informs the Customer [%d] that [%d] is restocked.\n"), NEncode2to1(mySalesID, myID), shelfNum);					 
				       }

					/* now restocked, continue looping until I have all of what I need */
					/*cout << type << " [" <<  myID << "] has received assistance about restocking of item [" <<
					  shelfNum << "] from DepartmentSalesman [" << mySalesID << "]" << endl;*/
				       if(type){
					 NPrint("Privileged Customer [%d] has received assistance about restocking of item [%d] from DepartmentSalesman [%d]\n", sizeof("Privileged Customer [%d] has received assistance about restocking of item [%d] from DepartmentSalesman [%d]\n"), NEncode2to1(myID, shelfNum), mySalesID);
				       }
				       else{
					 NPrint("Customer [%d] has received assistance about restocking of item [%d] from DepartmentSalesman [%d]\n", sizeof("Customer [%d] has received assistance about restocking of item [%d] from DepartmentSalesman [%d]\n"), NEncode2to1(myID, shelfNum), mySalesID);
				       }
					/*shelfLock[currentDepartment][shelfNum]->Release();*/
                                       Release(shelfLock[currentDepartment][shelfNum]);
				}
			}	/* end while loop to get enough of a given item */
		}	/* end looking through shelves */
	}	/* end going through grocery list */


	/* ======================================================== */

	 
	/* --------------Begin looking for a cashier------------------------------------- */
	/*cashierLinesLock->Acquire(); /* acquire locks to view line counts and cashier statuses*/ 
       Acquire(cashierLinesLock);
	do{		/* loop allows us to rechoose a line if our cashier goes on break */
          /*printf("%s [%d] is looking for the Cashier.\n", type, myID );*/
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
				/*cashierLock[i]->Acquire(); /* get his lock before I wake him up*/ 
                               Acquire(cashierLock[i]);
				cashierStatus[i] = CASH_BUSY; /* prevent others from thinking he's free */
				/*printf("%s [%d] chose Cashier [%d] with line of length [0].\n", type, myID, myCashier);*/
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
				/*printf("%s [%d] chose Cashier [%d] of line length [%d].\n", type, myID, myCashier, linesIAmLookingAt[minCashierID]);*/
				if(type){
				  NPrint("Privileged Customer [%d] chose Cashier [%d] of line length [%d].\n", sizeof("Privileged Customer [%d] chose Cashier [%d] of line length [%d].\n"), NEncode2to1(myID, myCashier), linesIAmLookingAt[minCashierID]);
				}
				else{
				  NPrint("Customer [%d] chose Cashier [%d] of line length [%d].\n", sizeof("Customer [%d] chose Cashier [%d] of line length [%d].\n"), NEncode2to1(myID, myCashier), linesIAmLookingAt[minCashierID]);
				}
				linesIAmLookingAt[minCashierID]++;
				/* linesIAmLookingAtCV[minCashierID]->Wait(cashierLinesLock); //wait in line */
                                Wait(linesIAmLookingAtCV[minCashierID], cashierLinesLock);
				linesIAmLookingAt[myCashier]--; /* i have been woken up, remove myself from line */
			}
			/* -------------End find shortest line---------------------- */

		}
	}while(cashierStatus[myCashier] == CASH_ON_BREAK); /* customer will repeat finding a cashier algorithm if he was woken up because the cashier is going on break */

	/* ----------------End looking for cashier-------------------------------------------- */


	/* code after this means I have been woken up after getting to the front of the line */
	/*cashierLock[myCashier]->Acquire(); /* disallow others from getting access to my cashier*/ 
       Acquire(cashierLock[myCashier]);
	/*cashierLinesLock->Release();	/* allow others to view monitor variable now that I've */
       /* my claim on this cashier*/ 
       Release(cashierLinesLock);
	cashierDesk[myCashier] = myID;	/* tell cashier who I am */
	/*cashierToCustCV[myCashier]->Signal(cashierLock[myCashier]);	/* signal cashier I am at his desk*/ 
       Signal(cashierToCustCV[myCashier], cashierLock[myCashier]);
	/*cashierToCustCV[myCashier]->Wait(cashierLock[myCashier]);	/* wait for his acknowlegdment*/ 
       Wait(cashierToCustCV[myCashier], cashierLock[myCashier]);
	cashierDesk[myCashier] = type; /* now tell him that whether or not I am privileged */
	/*cashierToCustCV[myCashier]->Signal(cashierLock[myCashier]);	/* signal cashier I've passed this information*/ 
       Signal(cashierToCustCV[myCashier], cashierLock[myCashier]);
	/*cashierToCustCV[myCashier]->Wait(cashierLock[myCashier]);	/* wait for his acknowledgment*/ 
       Wait(cashierToCustCV[myCashier], cashierLock[myCashier]);

	/* When I get here, the cashier is ready for items */

	/* ---------------------------Begin passing items to cashier--------------------------- */
	/* cycle through all items in my inventory, handing one item to cashier at a Time */
	for( i = 0; i < numItemsToBuy; i++){ /* cycle through all types of items */
		for( j = 0; j < qtyItemsInCart[i]; j++){ /* be sure we report how many of each type */
			cashierDesk[myCashier] = itemsInCart[i]; /* tells the cashier what type of item */
			/*cashierToCustCV[myCashier]->Signal(cashierLock[myCashier]); /* signal cashier item is on his desk*/ 
                       Signal(cashierToCustCV[myCashier], cashierLock[myCashier]);
			/*cashierToCustCV[myCashier]->Wait(cashierLock[myCashier]); /* wait for his acknowledgement*/ 
                       Wait(cashierToCustCV[myCashier], cashierLock[myCashier]);
		}
	}
	cashierDesk[myCashier] = -1; /* Tells cashier that I have reached the last item in my inventory */
        /*	cashierToCustCV[myCashier]->Signal(cashierLock[myCashier]); /* tell cashier */
                /* cashierToCustCV[myCashier]->Wait(cashierLock[myCashier]); */ /* wait for him to put the amount on desk*/ 
        Signal(cashierToCustCV[myCashier], cashierLock[myCashier]);
        Wait(cashierToCustCV[myCashier], cashierLock[myCashier]);
	/* ------------------------End passing items to cashier-------------------------------- */


	/* when I get here, the cashier has loaded my total */
	/* If I don't have enough money, leave the error flag -1 on the cashier's desk */
	if(cashierDesk[myCashier] > myCash){
          /*printf("%s [%d] cannot pay [%d]\n", type, myID, cashierDesk[myCashier]);*/
	  if(type){
	    NPrint("Privileged Customer [%d] cannot pay [%d]\n", sizeof("Privileged Customer [%d] cannot pay [%d]\n"), NEncode2to1(myID, cashierDesk[myCashier]), 0);
	  }
	  else{
	    NPrint("Customer [%d] cannot pay [%d]\n", sizeof("Customer [%d] cannot pay [%d]\n"), NEncode2to1(myID, cashierDesk[myCashier]), 0);
	  }
		cashierDesk[myCashier] = -1;
		/*cashierToCustCV[myCashier]->Signal(cashierLock[myCashier]);
                  cashierToCustCV[myCashier]->Wait(cashierLock[myCashier]);*/
                Signal(cashierToCustCV[myCashier], cashierLock[myCashier]);
                Wait(cashierToCustCV[myCashier], cashierLock[myCashier]);
		/* manager-customer interaction */
		/*managerLock->Acquire();
                  cashierLock[myCashier]->Release();*/
                Acquire(managerLock);
                Release(cashierLock[myCashier]);
		/*printf("%s [%d] is waiting for Manager for negotiations\n", type, myID);*/
		if(type){
		  NPrint("Privileged Customer [%d] is waiting for Manager negotiations\n", sizeof("Privileged Customer [%d] is waiting for Manager negotiations\n"), myID, 0);
		}
		else{
		  NPrint("Customer [%d] is waiting for Manager negotiations\n", sizeof("Customer [%d] is waiting for Manager negotiations\n"), myID, 0);
		}
		/*managerCV->Wait(managerLock);*/
                Wait(managerCV, managerLock);

		/* -----------------------Begin passing items to manager--------------------- */
		for( i = 0; i < numItemsToBuy; i++){
			while(qtyItemsInCart[i] > 0){
				if(managerDesk < myCash){
					break;
				}
				qtyItemsInCart[i] --;
				managerDesk = itemsInCart[i];
				/*printf("%s [%d] tells Manager to remove [%d] from trolly.\n", type, myID, itemsInCart[i]);*/
				if(type){
				  NPrint("Privileged Customer [%d] tells manager to remove [%d] from trolly.\n", sizeof("Privileged Customer [%d] tells manager to remove [%d] from trolly.\n"), NEncode2to1(myID, itemsInCart[i]), 0);
				}
				else{
				  NPrint("Customer [%d] tells manager to remove [%d] from trolly.\n", sizeof("Customer [%d] tells manager to remove [%d] from trolly.\n"), NEncode2to1(myID, itemsInCart[i]), 0);
				}
				/*managerCV->Signal(managerLock);
				managerCV->Wait(managerLock);*/
                                Signal(managerCV, managerLock);
                                Wait(managerCV, managerLock);
			}
		}
		managerDesk = -1; /* notifies the manager I'm done */
		/*managerCV->Signal(managerLock);
                  managerCV->Wait(managerLock);*/
                Signal(managerCV, managerLock);
                Wait(managerCV, managerLock);
		/* --------------------End of passing items to manager--------------- */

		amountOwed = managerDesk;	/* if I still can't afford anything, amountOwed will be 0 */
		myCash -= amountOwed;	/* updating my cash amount because I am paying manager */
		managerDesk = amountOwed;	/* technically redundant, but represents me paying money */
		/*printf("%s [%d] pays [%d] to Manager after removing items and is waiting for receipt from Manager.\n", type, myID, amountOwed);*/
		if(type){
		  NPrint("Privileged Customer [%d] pays [%d] to Manager after removing items and is waiting for receipt from Manager.\n", sizeof("Privileged Customer [%d] pays [%d] to Manager after removing items and is waiting for receipt from Manager.\n"), NEncode2to1(myID, amountOwed), 0);
		}
		else{
		  NPrint("Customer [%d] pays [%d] to Manager after removing items and is waiting for receipt from Manager.\n", sizeof("Customer [%d] pays [%d] to Manager after removing items and is waiting for receipt from Manager.\n"), NEncode2to1(myID, amountOwed), 0);
		}
		/* need receipt */
		/*managerCV->Signal(managerLock);
                  managerCV->Wait(managerLock);*/
                Signal(managerCV, managerLock);
                Wait(managerCV, managerLock);
		/* got receipt, I can now leave */
		/*managerLock->Release();*/
                Release(managerLock);
		/*cout << "Customer [" << myID << "] got receipt from Manager and is now leaving." << endl;*/
		NPrint("Customer [%d] got receipt from Manager and is now leaving.\n", sizeof("Customer [%d] got receipt from Manager and is now leaving.\n"), myID, 0);
	}
	/* if I do have money, I just need to update my cash and leave the money there */
	else{
		myCash -= cashierDesk[myCashier];
		/* Now I wait for my receipt */
		/*cashierToCustCV[myCashier]->Signal(cashierLock[myCashier]);
		printf("%s [%d] pays [%d] to Cashier [%d] and is now waiting for receipt.\n", type, myID, cashierDesk[myCashier], myCashier);
		cashierToCustCV[myCashier]->Wait(cashierLock[myCashier]);
		/* now I've received my receipt and should release the cashier */
		/* cout << type << " [" << myID << "] got receipt from Cashier [" << myCashier << "] and is now leaving." << endl; */
		/* cashierToCustCV[myCashier]->Signal(cashierLock[myCashier]); */
		/* cashierLock[myCashier]->Release(); */
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
	/*displacedTrollyLock->Acquire();
	displacedTrollyCount++;
	displacedTrollyLock->Release();*/
        Acquire(displacedTrollyLock);
        displacedTrollyLock++;
        Release(displacedTrollyLock);
	/* -----------------------------End replace trolly---------------------- */

	customersDone++;	/* increment the total customers done count */

	/* some cleanup */
	/*delete itemsToBuy;
	delete qtyItemsToBuy;
	delete itemsInCart;
	delete qtyItemsInCart;*/
        
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
	/*int *cashierTotals = new int[cashierNumber];*/

	/* initializes cashier totals, by which we keep track of how much money each cashier has had */
	/* in their registers over Time */
	/*for(int i = 0; i < cashierNumber; i++){
		cashierTotals[i] = 0;
                }*/


	/*queue<int> Break;
          int* numSalesmenOnBreak = new int[numDepartments];*/
	
	Srand(Time(NULL));
	while(1){

		/* ------------------Check if all customers have left store---------------- */
		/* if all customers have left, print out totals and terminate simulation since manager will be the only */
		/* ready thread */
		if(customersDone == custNumber){
			for( i = 0; i < cashierNumber; i++){
                          /*cashierLock[i]->Acquire();*/
                          Acquire(cashierLock[i]);
				/* -----------------------------Start empty cashier drawers------------------------------------ */
				if(cashRegister[i] > 0){
					totalRevenue += cashRegister[i];
					cashierTotals[i] += cashRegister[i];
					/*cout << "Manager emptied Counter " << i << " drawer." << endl;*/
					NPrint("Manager emptied Counter %d draw.\n", sizeof("Manager emptied Counter %d draw.\n"), i, 0);
					cashRegister[i] = 0;
					/*cout << "Manager has total sale of $" << totalRevenue << "." << endl;
                                          cashierLock[i]->Release();*/
					NPrint("Manager has total sale of $%d.\n", sizeof("Manager has total sale of $%d.\n"), totalRevenue, 0);
                                        Release(cashierLock[i]);
				}
			}
			/*for(int i = 0; i < cashierNumber; i++){
				cout << "Total Sale from Counter [" << i << "] is $[" << cashierTotals[i] << "]." << endl;
			}
			cout << "Total Sale of the entire store is $[" << totalRevenue << "]." << endl;*/
			for(i = 0; i < cashierNumber; i++){
			  NPrint("Total Sale from Counter [%d] is $[%d].\n", sizeof("Total Sale from Counter [%d] is $[%d].\n"), NEncode2to1(i, cashierTotals[i]), 0);
			}
			NPrint("Total Sale of the entire store is $[%d].\n", sizeof("Total Sale of the entire store is $[%d].\n"), totalRevenue, 0);
			break;
		}
		/* ------------------End check if all customers have left store------------ */

		/* -----------------Have loader check trolleys--------------------------- */
		/*inactiveLoaderLock->Acquire();
		inactiveLoaderCV->Signal(inactiveLoaderLock); /* wake up goods loader to do a regular check of trolley and manager items */
		/*inactiveLoaderLock->Release();*/
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

		/*cashierLinesLock->Acquire(); /* going to be checking line counts and statuses, so need this lock*/ 
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
		    /*cashierLinesLock->Release();*/
		    Release(cashierLinesLock);
		    /*cashierLock[wakeCashier]->Acquire();*/
		    Acquire(cashierLock[wakeCashier]);
		    if (numAnyLines)/*cout << "Manager brings back Cashier " << wakeCashier << " from break." << endl;*/
		      NPrint("Manager brings back Cashier %d from break.\n", sizeof("Manager brings back Cashier %d from break.\n"), wakeCashier, 0);
		    /*cashierToCustCV[wakeCashier]->Signal(cashierLock[wakeCashier]); /* this is the actual act of bring a cashier back from break*/ 
		    Signal(cashierToCustCV[wakeCashier], cashierLock[wakeCashier]);
		    /*cashierLock[wakeCashier]->Release();*/
		    Release(cashierLock[wakeCashier]);
		    /* bookkeeping */
		    numCashiersOnBreak--;
		    /*cashiersOnBreak.pop();*/
		    QueuePop(cashiersOnBreak);
		  }


		}
		else /*cashierLinesLock->Release();*/
		  Release(cashierLinesLock);

		/*cashierLinesLock->Acquire();*/
		Acquire(cashierLinesLock);


		/* ---------------------------End Bring cashier back from break-------------------------- */

		/* ---------------------------Begin send cashiers on break------------------------------- */
		 chance = Rand() %10;

		if( chance == 1  && numCashiersOnBreak < cashierNumber -2){ /* .001% chance of sending cashier on break */
			/* generate cashier index */
			 r = Rand() % cashierNumber;
			if(cashierStatus[r] != CASH_ON_BREAK && cashierStatus[r] != CASH_GO_ON_BREAK){
				if(cashierStatus[r] == CASH_NOT_BUSY) {
				  /*cashierLock[r]->Acquire();*/
				  Acquire(cashierLock[r]);
					cashierDesk[r] = -2;
					cashierStatus[r] = CASH_GO_ON_BREAK;
					/*cashierToCustCV[r]->Signal(cashierLock[r]);*/
					Signal(cashierToCustCV[r], cashierLock[r]);

					/*cashierLock[r]->Release();*/
					Release(cashierLock[r]);
				}
				else cashierStatus[r] = CASH_GO_ON_BREAK;
				/*cout << "Manager sends Cashier [" << r << "] on break." << endl;*/
				NPrint("Manager sends Cashier [%d] on break.\n", sizeof("Manager sends Cashier [%d] on break.\n"), r, 0);
				if(testNumber == 5) NPrint("Manager has iterated %d Times at this point.\n", sizeof("Manager has iterated %d Times at this point.\n"), counter, 0);
				/* cout << "Manager has iterated " << counter << " Times at this point." << endl; */
				/*cashiersOnBreak.push(r);*/
				QueuePush(cashiersOnBreak, r);
				numCashiersOnBreak++;


			}
		}

		/* -----------------------------End send cashiers on break------------------------------------- */
		/*cashierLinesLock->Release();*/
		Release(cashierLinesLock);

		/* __SALES_BREAK__ */
		/* -----------------------------Begin bringing salesmen back from break------------- */
		 dept = 0;
		/*if(salesmenOnBreak.size()){*/
		if(QueueSize(numSalesmenOnBreak)){

		  /*int arg = salesmenOnBreak.front();*/
		  arg = QueueFront(salesmenOnBreak);
			 targets[2];
			deconstructSalesArg(arg, targets);
			 wakeSalesman = targets[0];
			dept = targets[1];
			/*salesmenOnBreak.pop();*/
			QueuePop(salesmenOnBreak);
			/*salesLock[dept]->Acquire();*/
			Acquire(salesLock[dept]);
			if((greetingCustWaitingLineCount[dept] + complainingCustWaitingLineCount[dept] + loaderWaitingLineCount[dept]) > 0 && currentSalesStatus[dept][wakeSalesman] == SALES_ON_BREAK){
				salesBreakBoard[dept][wakeSalesman] = 0;
				/*cout << "Manager brings back Salesman [" << wakeSalesman << "] from break." << endl;*/
				NPrint("Manager brings back Salesman [%d] from break.\n", sizeof("Manager brings back Salesman [%d] from break.\n"), wakeSalesman, 0);
				/*	salesBreakCV[dept][wakeSalesman]->Signal(salesLock[dept]);*/
				Signal(salesBreakCV[dept][wakeSalesman], salesLock[dept]);
				numSalesmenOnBreak[dept]--;
			}
			else{
			  /*salesmenOnBreak.push(arg);*/
			  QueuePush(salesmenOnBreak);
			}
			/*salesLock[dept]->Release();*/
			Release(salesLock[dept]);
		}

		/* ------------------------------end bringing salesmen back from break-------------- */

		/* ------------------------------Begin putting salesmen on break------------------ */
		dept = Rand() % numDepartments;
		/*salesLock[dept]->Acquire();*/
		Acquire(salesLock[dept]);
		if (chance == 1 && numSalesmenOnBreak[dept] < numSalesmen -1) {
			 r = Rand() % numSalesmen;
			if(!salesBreakBoard[dept][r] && currentSalesStatus[dept][r] != SALES_ON_BREAK && currentSalesStatus[dept][r] != SALES_GO_ON_BREAK) {
				salesBreakBoard[dept][r] = 1;
				/*cout << "Manager sends Salesman [" << r << "] of dept " << dept << " on break." << endl;*/
				NPrint("Manager sends Salesman [%d] of dept [%d] on break.\n", sizeof("Manager sends Salesman [%d] of dept [%d] on break.\n"), NEncode2to1(r, dept), 0);
				/*individualSalesmanLock[dept][r]->Acquire();
				if(currentSalesStatus[dept][r] == SALES_NOT_BUSY) {
					salesmanCV[dept][r]->Signal(individualSalesmanLock[dept][r]);
					currentSalesStatus[dept][r] = SALES_ON_BREAK;
				}
				individualSalesmanLock[dept][r]->Release();*/
				Acquire(individualSalesmanLock[dept][r]);
				if(currentSalesStatus[dept][r] == SALES_NOT_BUSY){
				  Signal(salesmanCV[dept][r], individualSalesmanLock[dept][r]);
				  currentSalesStatus[dept][r] = SALES_ON_BREAK;
				}
				Release(individualSalesmanLock[dept][r]);
				/*salesmenOnBreak.push(constructSalesArg(dept, r)); /* function that uses bit operations to store dept and salesman index*/ 
				/* in one int so I can get it from my queue later when I take a Salesman off break */

				 arg = constructSalesArg(dept, r);
				 QueuePush(salesmenOnBreak, arg);
				numSalesmenOnBreak[dept]++;
			}
		}
		/*salesLock[dept]->Release();*/
		Release(salesLock[dept]);
		/* -----------------------------End send salesmen on break */

		for(i = 0; i < cashierNumber; i++){

		  /*cashierLock[i]->Acquire(); /* acquiring this lock prevents race conditions*/ 
										/* only manager and cashier i ever attempt to grab this lock */
		  Acquire(cashierLock[i]);
			/* -----------------------------Start empty cashier drawers------------------------------------ */
			if(cashRegister[i] > 0){
				totalRevenue += cashRegister[i];
				cashierTotals[i] += cashRegister[i]; /* how we remember totals for each cashier */
				/*cout << "Manager emptied Counter [" << i << "] drawer." << endl;*/
				NPrint("Manager emptied Counter [%d] drawer.\n", sizeof("Manager emptied Counter [%d] drawer.\n"), i, 0);
				cashRegister[i] = 0;
				/*cout << "Manager has total sale of $[" << totalRevenue << "]." << endl;*/
				NPrint("Manager has total sale of $[%d].\n", sizeof("Manager has total sale of $[%d].\n"), totalRevenue, 0);
				/*cashierLock[i]->Release();*/
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

				/*managerLock->Acquire(); /* manager lock protects the manager's interactions*/ 
				Acquire(managerLock);

				/*cashierToCustCV[cashierID]->Signal(cashierLock[cashierID]); /* wakes up customer, who also waits first in this interaction */
				/* cashierToCustCV[cashierID]->Signal(cashierLock[cashierID]); wakes up cashier  */
				/* cashierLock[i]->Release(); */
				/* managerCV->Wait(managerLock); */
				Signal(cashierToCustCV[cashierID], cashierLock[cashierID]);
				Release(cashierLock[i]);
				Wait(managerCV, managerLock);
				amountOwed = cashierFlags[i];
				cashierFlags[i] = -1; /* reset cashierFlags */
				custTypeID = managerDesk;
				/*char* custType = new char[20];*/
				managerCustType = -1;
				if(custTypeID == 0){
					managerCustType = 0;
				}
				else{
				  managerCustType = 1;
				}
				managerDesk = amountOwed;
				/*managerCV->Signal(managerLock);*/
				Signal(managerCV, managerLock);
				/*managerCV->Wait(managerLock);*/
				Wait(managerCV, managerLock);
				while(managerDesk != -1 ){ /* when managerDesk == -1, customer is out of items or can afford money */
					amountOwed -= scan(managerDesk);
					/*managerItemsLock->Acquire();*/
					Acquire(managerItemsLock);
					/* managerItems.push(managerDesk); */
					QueuePush(managerItems, managerDesk);
					/*managerItemsLock->Release();*/
					Release(managerItemsLock);
					/*cout << "Manager removes [" << managerDesk << "] from the trolly of " << custType << " [" << customerID << "]."<<endl;*/
					if(managerCustType){
					  NPrint("Manager removes [%d] from the trolly PrivilegedCustomer [%d].\n", sizeof("Manager removes [%d] from the trolly PrivilegedCustomer [%d].\n"), NEncode2to1(managerDesk, customerID), 0);
					}
					else{
					  NPrint("Manager removes [%d] from the trolly Customer [%d].\n", sizeof("Manager removes [%d] from the trolly Customer [%d].\n"), NEncode2to1(managerDesk, customerID), 0);
					}
					managerDesk = amountOwed; /* giving customer new subtotal */
									/* customer will put back -1 if out of items */
									/*  or if they can now afford */
					/*managerCV->Signal(managerLock);*/
					Signal(managerCV, managerLock);
					/*managerCV->Wait(managerLock);*/
					Wait(managerCV, managerLock);
				}
				/* inactiveLoaderLock->Acquire(); */
				/* inactiveLoaderCV->Signal(inactiveLoaderLock); */
				/* inactiveLoaderLock->Release(); */
				/* now customer has reached end of items or has enough money */
				/* I give him total */
				managerDesk = amountOwed;
				/*managerCV->Signal(managerLock);*/
				Signal(managerCV, managerLock);
				/*managerCV->Wait(managerLock); /* wait for his response, i.e. paying money*/ 
				Wait(managerCV, managerLock);
				totalRevenue += managerDesk;

				/* Now give the customer his receipt */
				/*cout << "Manager gives receipt to " << custType << " [" << customerID << "]." << endl;*/
				if(managerCustType){
				  NPrint("Manager gives receipt to PrivilegedCustomer [%d].\n", sizeof("Manager gives receipt to PrivilegedCustomer [%d].\n"), customerID, 0);
				}
				else{
				  NPrint("Manager gives receipt to Customer [%d].\n", sizeof("Manager gives receipt to Customer [%d].\n"), customerID, 0);				  
				}
				managerDesk = -1;
				/*managerCV->Signal(managerLock);*/
				Signal(managerCV, managerLock);
				/* release manager lock */
				/*managerLock->Release();*/
				Release(managerLock);
				break;


			}
			/* ----------------------------End deal with broke customers-------------------- */
			/*else cashierLock[i]->Release();*/
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
		/* char* custType = new char[20]; */
		/* cashierLinesLock->Acquire(); */
		Acquire(cashierLinesLock);
		/* check my status to see if I've already been set to busy by a customer */
		if(cashierStatus[myCounter] == CASH_GO_ON_BREAK){
			/* cout << "Cashier [" << myCounter << " acknowledges he will go on break" << endl; */


		/*cout << "Cashier [" << myCounter << "] is going on break." << endl;*/
		  NPrint("Cashier [%d] is going on break.\n", sizeof("Cashier [%d] is going on break.\n"), myCounter, 0);
		cashierStatus[myCounter] = CASH_ON_BREAK;
		/* unprivilegedCashierLineCV[myCounter]->Broadcast(cashierLinesLock); */
		Broadcast(unprivilegedCashierLineCV[myCounter], cashierLinesLock);
		/* privilegedCashierLineCV[myCounter]->Broadcast(cashierLinesLock); */
		Broadcast(privilegedCashierLineCV[myCounter], cashierLinesLock);

		/* cashierLock[myCounter]->Acquire(); */
		Acquire(cashierLock[myCounter]);
		/* cashierLinesLock->Release(); */
		Release(cashierLinesLock);

		/* cashierToCustCV[myCounter]->Wait(cashierLock[myCounter]); */
		Wait(cashierToCustCV[myCounter], cashierLock[myCounter]);
		/* cashierLinesLock->Acquire(); */
		Acquire(cashierLinesLock);
		cashierStatus[myCounter] = CASH_NOT_BUSY;
		/*cout << "Cashier [" << myCounter << "] was called from break by Manager to work." << endl;*/
		NPrint("Cashier [%d] was called from break by Manager to work.\n", sizeof("Cashier [%d] was called from break by Manager to work.\n"), myCounter, 0);
	}
	/* check if my lines have anyone in it */
	/* set my state so approaching Customers can wait in or engage me, as apropriate */
	if(privilegedLineCount[myCounter]){
		/* privilegedCashierLineCV[myCounter]->Signal(cashierLinesLock); */
          Signal(privilegedCashierLineCV[myCounter], cashierLinesLock);
		/* custType = "Privileged Customer"; */
		custType[myCounter] = PRIVILEGED_CUSTOMER;
	}
	else if(unprivilegedLineCount[myCounter]){
		/* cout << "Signalling customer in line" << endl; */
		/* unprivilegedCashierLineCV[myCounter]->Signal(cashierLinesLock); */
          Signal(unprivilegedCashierLineCV[myCounter], cashierLinesLock);
		/* custType = "Customer"; */
		custType[myCounter] = CUSTOMER;
	}
	else{
		cashierStatus[myCounter] = CASH_NOT_BUSY; /* means I can be approached without it being necessary to wait in line */
	}
	/* whether or not I'm ready for a customer, I can get my lock and go to sleep */
	/* cashierLock[myCounter]->Acquire(); */
	Acquire(cashierLock[myCounter]);
	/* cashierLinesLock->Release(); */
	Release(cashierLinesLock);
	/* cashierToCustCV[myCounter]->Signal(cashierLock[myCounter]); */
	Signal(cashierToCustCV[myCounter], cashierLock[myCounter]);
	/* cashierToCustCV[myCounter]->Wait(cashierLock[myCounter]); */
	Wait(cashierToCustCV[myCounter], cashierLock[myCounter]);
	/* the next Time I'm woken up (assuming it is by a customer, not a manager */
	/* I will be totaling items */
	/* when I get here, there will be an item to scan */
	/* int total = 0; */
	total[myCounter] = 0;
	/* int custID = cashierDesk[myCounter]; */
	custID[myCounter] = cashierDesk[myCounter];
	if(custID[myCounter] == -2){
		/* cashierLock[myCounter]->Release(); */
		Release(cashierLock[myCounter]);
		continue;
	}
	/* cashierToCustCV[myCounter]->Signal(cashierLock[myCounter]); */
	Signal(cashierToCustCV[myCounter], cashierLock[myCounter]);
	/* cashierToCustCV[myCounter]->Wait(cashierLock[myCounter]); */
	Wait(cashierToCustCV[myCounter], cashierLock[myCounter]);

	if(cashierDesk[myCounter] == 1){
		/* custType = "PrivilegedCustomer"; */
		custType[myCounter] = PRIVILEGED_CUSTOMER;
	}
	else{
		/* custType = "Customer"; */
		custType[myCounter] = CUSTOMER;
	}

	/* cashierToCustCV[myCounter]->Signal(cashierLock[myCounter]); */
	Signal(cashierToCustCV[myCounter], cashierLock[myCounter]);
	/* cashierToCustCV[myCounter]->Wait(cashierLock[myCounter]); */
	Wait(cashierToCustCV[myCounter], cashierLock[myCounter]);
	while(cashierDesk[myCounter] != -1){ /* -1 means we're done scanning */


		/*cout << "Cashier [" << myCounter << "] got [" << cashierDesk[myCounter] << "] from trolly of " << custType << " [" << custID << "]." << endl;*/
	  if(custType[myCounter] == PRIVILEGED_CUSTOMER){
	    NPrint("Cashier [%d] got [%d] from trolly of Privileged Customer [%d].\n", sizeof("Cashier [%d] got [%d] from trolly of Privileged Customer [%d].\n"), NEncode2to1(myCounter, cashierDesk[myCounter]), custID);
	    }
	  else{
	    NPrint("Cashier [%d] got [%d] from trolly of Customer [%d].\n", sizeof("Cashier [%d] got [%d] from trolly of Customer [%d].\n"), NEncode2to1(myCounter, cashierDesk[myCounter]), custID);	    
		total[myCounter] += scan(cashierDesk[myCounter]);
		/* cashierToCustCV[myCounter]->Signal(cashierLock[myCounter]); */
		Signal(cashierToCustCV[myCounter], cashierLock[myCounter]);
		/* cashierToCustCV[myCounter]->Wait(cashierLock[myCounter]); */
		Wait(cashierToCustCV[myCounter], cashierLock[myCounter]);
	}
	/* now I'm done scanning, so I tell the customer the total */
	/*cout << "Cashier [" << myCounter << "] tells " << custType << " [" << custID << "] total cost is $[" << total << "]." << endl;*/
	  if(custType[myCounter] == PRIVILEGED_CUSTOMER){
	    NPrint("Cashier [%d] tells Privileged Customer [%d] total cost is $[%d].\n", sizeof("Cashier [%d] tells Privileged Customer [%d] total cost is $[%d].\n"), NEncode2to1(myCounter, custID), total);
	    }
	  else{
	    NPrint("Cashier [%d] tells Customer [%d] total cost is $[%d].\n", sizeof("Cashier [%d] tells Customer [%d] total cost is $[%d].\n"), NEncode2to1(myCounter, custID), total);
	    }
	cashierDesk[myCounter] = total[myCounter];
	/* cashierToCustCV[myCounter]->Signal(cashierLock[myCounter]); */
	Signal(cashierToCustCV[myCounter], cashierLock[myCounter]);
	/* cashierToCustCV[myCounter]->Wait(cashierLock[myCounter]); */
	Wait(cashierToCustCV[myCounter], cashierLock[myCounter]);
	if(cashierDesk[myCounter] == -1){
          /*		cout << "Cashier [" << myCounter << "] asks " << custType << " [" << custID << "] to wait for Manager." << endl;
                        cout << "Cashier [" << myCounter << "] informs the Manager that " << custType << " [" << custID << "] does not have enough money." << endl;*/
	  if(custType[myCounter] == PRIVILEGED_CUSTOMER){
	    NPrint("Cashier [%d] asks Privileged Customer [%d] to wait for Manager.\n", sizeof("Cashier [%d] asks Privileged Customer [%d] to wait for Manager.\n"), NEncode2to1(myCounter, custID), 0);
	    NPrint("Cashier [%d] informs the Manager that Privileged Customer [%d] does not have enough money.\n", sizeof("Cashier [%d] informs the Manager that Privileged Customer [%d] does not have enough money.\n"), NEncode2to1(myCounter, custID), 0);
	    }
	  else{
	    NPrint("Cashier [%d] asks Customer [%d] to wait for Manager.\n", sizeof("Cashier [%d] asks Customer [%d] to wait for Manager.\n"), NEncode2to1(myCounter, custID), 0);
	    NPrint("Cashier [%d] informs the Manager that Customer [%d] does not have enough money.\n", sizeof("Cashier [%d] informs the Manager that Customer [%d] does not have enough money.\n"), NEncode2to1(myCounter, custID), 0);
	    }
		cashierFlags[myCounter] = custID[myCounter];
		/* cout << "cashier is goign to sleep" << endl; */
		/* cashierToCustCV[myCounter]->Wait(cashierLock[myCounter]); */
		Wait(cashierToCustCV[myCounter], cashierLock[myCounter]);
		/* Set the manager desk to 0 or 1 to tell the manager which type of */
		/* customer he is dealing with */
		/* if(!strcmp(custType, "Privileged Customer")){ */
		if(custType[myCounter] == PRIVILEGED_CUSTOMER){
			managerDesk = 1;
		}
		/* if(!strcmp(custType, "Customer")){ */
		if(custType[myCounter] == CUSTOMER){
			managerDesk = 0;
		}
		/*cout << " total " << total << endl;*/
		cashierFlags[myCounter] = total[myCounter]; /* inform manager of the total the customer owes */
		/* managerLock->Acquire(); */
		Acquire(managerLock);
		/* managerCV->Signal(managerLock); //wake up manager, who was waiting for this information */
		Signal(managerCV, managerLock);
		/* managerLock->Release(); */
		Release(managerLock);
		/* when I am woken up, the manager has taken over so I can free myself for my */
		/* next customer */
	}
	else{
		/* add value to cash register */
		cashRegister[myCounter] += cashierDesk[myCounter];
		/*cout << "Cashier [" << myCounter << "] got money $[" << cashierDesk[myCounter] << "] from " << custType << " [" << custID << "]." << endl;*/
		if(custType[myCounter] == PRIVILEGED_CUSTOMER){
		  NPrint("Cashier [%d] got money $[%d] from Privileged Customer [%d].\n", sizeof("Cashier [%d] got money $[%d] from Privileged Customer [%d].\n"), NEncode2to1(myCounter, cashierDesk[myCounter]), custID);
		  NPrint("Cashier [%d] gave the receipt to Privileged Customer [%d] and tells him to leave.\n", sizeof("Cashier [%d] gave the receipt to Privileged Customer [%d] and tells him to leave.\n"), NEncode2to1(myCounter, custID), 0);
		}
		else{
		  NPrint("Cashier [%d] got money $[%d] from Customer [%d].\n", sizeof("Cashier [%d] got money $[%d] from Customer [%d].\n"), NEncode2to1(myCounter, cashierDesk[myCounter]), custID);
		  NPrint("Cashier [%d] gave the receipt to Customer [%d] and tells him to leave.\n", sizeof("Cashier [%d] gave the receipt to Customer [%d] and tells him to leave.\n"), NEncode2to1(myCounter, custID), 0);		  
		}
		/* giving the customer a receipt */
		/* cout << "Cashier [" << myCounter << "] gave the receipt to " << custType << " [" << custID << "] and tells him to leave" << endl; */
		/* cashierToCustCV[myCounter]->Signal(cashierLock[myCounter]); */
		Signal(cashierToCustCV[myCounter], cashierLock[myCounter]);
		/* wait for customer to acknowledge getting receipt and release lock */
		/* cashierToCustCV[myCounter]->Wait(cashierLock[myCounter]); */
		Wait(cashierToCustCV[myCounter], cashierLock[myCounter]);
	}
	cashierDesk[myCounter] = 0;
	/* cashierLock[myCounter]->Release(); */
	Release(cashierLock[myCounter]);

	/* done, just need to loop back and check for more customers */
	/* delete custType; */
	custType[myCounter] = CUSTOMER;
	}
	}
	
}



/* Salesman Code		__SALESMAN__ */
/* void Salesman (int myIndex){ */
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
/* 	int argTargets[2]; */
/* 	deconstructSalesArg(arg, argTargets);	//gets the index and department from the number passed in */
/* 	int myIndex = argTargets[0]; */
/* 	int myDept = argTargets[1]; */

	while(1) {
          /* salesLock[myDept]->Acquire(); */
          Acquire(salesLock[myIndex]);

		/* go on break if the manager has left me a note saying to */
		if(salesBreakBoard[myDept][myIndex] == 1) {
			/*cout << "Sales " << myIndex << " in department " << myDept << " going on break" << endl;*/
			prev = currentSalesStatus[myDept][myIndex];
			currentSalesStatus[myDept][myIndex] = SALES_ON_BREAK;
			salesBreakBoard[myDept][myIndex] = 0;
			/* salesBreakCV[myDept][myIndex]->Wait(salesLock[myDept]); */
			Wait (salesBreakCV[myDept][myIndex],salesLock[myDept]);
			currentSalesStatus[myDept][myIndex] = prev;
			/*cout << "Sales " << myIndex << " in department " << myDept << " back from break" << endl;*/
		}

		/* Check if there is someone in a line and wake them up */
		if(greetingCustWaitingLineCount[myDept] > 0){	/* greeting */
			currentSalesStatus[myDept][myIndex] = SALES_READY_TO_TALK;
			/* greetingCustCV[myDept]->Signal(salesLock[myDept]); */
			Signal(greetingCustCV[myDept], salesLock[myDept]);
			greetingCustWaitingLineCount[myDept]--;
		}
		else if(loaderWaitingLineCount[myDept] > 0) {	/* loader */
			currentSalesStatus[myDept][myIndex] = SALES_READY_TO_TALK_TO_LOADER;
			/* loaderCV[myDept]->Signal(salesLock[myDept]); */
			Signal(loaderCV[myDept], salesLock[myDept]);
			loaderWaitingLineCount[myDept]--;
		}
		else if(complainingCustWaitingLineCount[myDept] > 0) {	/* complaining */
			currentSalesStatus[myDept][myIndex] = SALES_READY_TO_TALK;
			/* complainingCustCV[myDept]->Signal(salesLock[myDept]); */
                        Signal(complainingCustCV[myDept], salesLock[myDept]);
			complainingCustWaitingLineCount[myDept]--;
		}

		else{	/* not busy */
			currentlyTalkingTo[myDept][myIndex] = UNKNOWN;
			currentSalesStatus[myDept][myIndex] = SALES_NOT_BUSY;
		}

		/* individualSalesmanLock[myDept][myIndex]->Acquire(); */
                Acquire(individualSalesmanLock[myDept][myIndex]);
		/* salesLock[myDept]->Release(); */
                Release(salesLock[myDept]);
		/* salesmanCV[myDept][myIndex]->Wait(individualSalesmanLock[myDept][myIndex]);	//Wait for cust/loader to walk up to me */
                Wait (salesmanCV[myDept][myIndex], individualSalesmanLock[myDept][myIndex]);

		if(currentlyTalkingTo[myDept][myIndex] == GREETING) {	/* a greeting customer came up to me */
			myCustNumber = salesCustNumber[myDept][myIndex];
			if(privCustomers[myCustNumber] == 1){
				/*cout << "DepartmentSalesman [" << myIndex << "] welcomes PrivilegeCustomer [" << myCustNumber << "] to Department [" << myDept << "]." << endl;*/
			  NPrint("DepartmentSalesman [%d] welcomes PrivilegedCustomer [%d] to Department [%d].\n", sizeof("DepartmentSalesman [%d] welcomes PrivilegedCustomer [%d] to Department [%d].\n"), NEncode2to1(myIndex, myCustNumber), myDept);
			}
			else{
				/*cout << "DepartmentSalesman [" << myIndex << "] welcomes Customer [" << myCustNumber << "] to Department [" << myDept << "]." << endl;*/
			  NPrint("DepartmentSalesman [%d] welcomes Customer [%d] to Department [%d].\n", sizeof("DepartmentSalesman [%d] welcomes Customer [%d] to Department [%d].\n"), NEncode2to1(myIndex, myCustNumber), myDept);			  
			}
			/* salesmanCV[myDept][myIndex]->Signal(individualSalesmanLock[myDept][myIndex]); */
                        Signal (salesmanCV[myDept][myIndex], individualSalesmanLock[myDept][myIndex]);
			/* individualSalesmanLock[myDept][myIndex]->Release(); */
                        Release(individualSalesmanLock[myDept][myIndex]);
		}
		else if(currentlyTalkingTo[myDept][myIndex] == COMPLAINING) {	/* a complaining customer came up to me */
			myCustNumber = salesCustNumber[myDept][myIndex];
			itemOutOfStock = salesDesk[myDept][myIndex];

			if(privCustomers[myCustNumber] == 1){
				/*cout << "DepartmentSalesman [" << myIndex << "] is informed by PrivilegeCustomer [" << myCustNumber << "] that [" << itemOutOfStock << "] is out of stock." << endl;*/
			  NPrint("DepartmentSalesman [%d] is informed by PrivilegedCustomer [%d] that [%d] is out of stock.\n", sizeof("DepartmentSalesman [%d] is informed by PrivilegedCustomer [%d] that [%d] is out of stock.\n"), NEncode2to1(myIndex, myCustNumber), itemOutOfStock);
			}
			else{
				/* cout << "DepartmentSalesman [" << myIndex << "] is informed by Customer [" << myCustNumber << "] that [" << itemOutOfStock << "] is out of stock." << endl; */
			  NPrint("DepartmentSalesman [%d] is informed by Customer [%d] that [%d] is out of stock.\n", sizeof("DepartmentSalesman [%d] is informed by Customer [%d] that [%d] is out of stock.\n"), NEncode2to1(myIndex, myCustNumber), itemOutOfStock);
			}
			/* individualSalesmanLock[myDept][myIndex]->Acquire(); */
                        Acquire(individualSalesmanLock[myDept][myIndex]);
			/* salesmanCV[myDept][myIndex]->Signal(individualSalesmanLock[myDept][myIndex]);	//tell cust to wait for the item */
                        Signal(salesmanCV[myDept][myIndex], individualSalesmanLock[myDept][myIndex]);

			/* tell goods loader */
			/* salesLock[myDept]->Acquire(); */
                        Acquire(salesLock[myDept]);
			salesDesk[myDept][myIndex] = itemOutOfStock;	/* Might not be necessary, because we never really took it off the desk */

			if(loaderWaitingLineCount[myDept] > 0) {	/* if we can, get a loader from line */
				loaderWaitingLineCount[myDept]--;
				currentSalesStatus[myDept][myIndex] = SALES_READY_TO_TALK_TO_LOADER;
				/* loaderCV[myDept]->Signal(salesLock[myDept]);	//get a loader from line */
                                Signal(loaderCV[myDept], salesLock[myDept]);
				/* salesLock[myDept]->Release(); */
                                Release(salesLock[myDept]);
			}
			else {	/*  no one was in line, so go to the inactive loaders */
				currentSalesStatus[myDept][myIndex] = SALES_SIGNALLING_LOADER;
				/* salesLock[myDept]->Release(); */
                                Release(salesLock[myDept]);

				/* inactiveLoaderLock->Acquire(); */
                                Acquire(inactiveLoaderLock);
				/* inactiveLoaderCV->Signal(inactiveLoaderLock);	//call a loader over */
                                Signal(inactiveLoaderCV, inactiveLoaderLock);
				/* inactiveLoaderLock->Release(); */
                                Release(inactiveLoaderLock);
			}

			/* salesmanCV[myDept][myIndex]->Wait(individualSalesmanLock[myDept][myIndex]);	//wait for the OK from loader (no matter where he came from) */
                        Wait(salesmanCV[myDept][myIndex], individualSalesmanLock[myDept][myIndex]);

			/* check to see if a loader already restocked something */
			if(salesDesk[myDept][myIndex] != -1) {		/* loader had finished stocking something and tells me about it */
				itemRestocked = salesDesk[myDept][myIndex];
				/* salesmanCV[myDept][myIndex]->Signal(individualSalesmanLock[myDept][myIndex]); */
                                Signal(salesmanCV[myDept][myIndex], individualSalesmanLock[myDept][myIndex]);
				/* salesmanCV[myDept][myIndex]->Wait(individualSalesmanLock[myDept][myIndex]); */
                                Wait(salesmanCV[myDept][myIndex], individualSalesmanLock[myDept][myIndex]);
				loaderNumber = salesDesk[myDept][myIndex];
				/* salesmanCV[myDept][myIndex]->Signal(individualSalesmanLock[myDept][myIndex]); */
                                Signal(salesmanCV[myDept][myIndex], individualSalesmanLock[myDept][myIndex]);
				/* cout << "DepartmentSalesman [" << myIndex << "] is informed by the GoodsLoader [" << loaderNumber << "] that [" << itemRestocked << "] is restocked." << endl; */
				NPrint("DepartmentSalesman [%d] is informed by the GoodsLoader [%d] that [%d] is restocked.\n", sizeof("DepartmentSalesman [%d] is informed by the GoodsLoader [%d] that [%d] is restocked.\n"), NEncode2to1(myIndex, loaderNumber), itemRestocked);
				/* shelfLock[myDept][itemRestocked]->Acquire(); */
                                Acquire(shelfLock[myDept][itemRestocked]);
				/* shelfCV[myDept][itemRestocked]->Broadcast(shelfLock[myDept][itemRestocked]); */
                                Broadcast(shelfCV[myDept][itemRestocked], shelfLock[myDept][itemRestocked]);
				/* shelfLock[myDept][itemRestocked]->Release(); */
                                Release(shelfLock[myDept][itemRestocked]);
				/* can't know the cust number, becaue we broadcast here! */
				/* DepartmentSalesman [identifier] informs the Customer/PrivilegeCustomer [identifier] that [item] is restocked. */
			}


			/* inactiveLoaderLock->Acquire(); */
                        Acquire(inactiveLoaderLock);
			myLoaderID = -1;

			for(i = 0; i < numLoaders; i++) {	/* find the loader who i just sent off to restock and change his status */
				if(loaderStatus[i] == LOAD_HAS_BEEN_SIGNALLED) {
					myLoaderID = i;
					loaderStatus[i] = LOAD_STOCKING;
					break;
				}
			}

			/* inactiveLoaderLock->Release(); */
                        Release(inactiveLoaderLock);

			/* cout << "DepartmentSalesman [" << myIndex << "] informs the GoodsLoader [" << myLoaderID << "] that [" << itemOutOfStock << "] is out of stock." << endl; */;
			NPrint("DepartmentSalesman [%d] informs the GoodsLoader [%d] that [%d] is out of stock.\n", sizeof("DepartmentSalesman [%d] informs the GoodsLoader [%d] that [%d] is out of stock.\n"), NEncode2to1(myIndex, myLoaderID), itemOutOfStock);
		   
			/* salesmanCV[myDept][myIndex]->Signal(individualSalesmanLock[myDept][myIndex]); */
                        Signal(salesmanCV[myDept][myIndex], individualSalesmanLock[myDept][myIndex]);
			/* individualSalesmanLock[myDept][myIndex]->Release(); */
                        Release(individualSalesmanLock[myDept][myIndex]);

		}
		else if(currentlyTalkingTo[myDept][myIndex] == GOODSLOADER) {	/* a loader came up to talk to me */
			itemRestocked = salesDesk[myDept][myIndex];
			/* salesmanCV[myDept][myIndex]->Signal(individualSalesmanLock[myDept][myIndex]); */
                        Signal(salesmanCV[myDept][myIndex], individualSalesmanLock[myDept][myIndex]);
			/* salesmanCV[myDept][myIndex]->Wait(individualSalesmanLock[myDept][myIndex]); */
                        Wait(salesmanCV[myDept][myIndex], individualSalesmanLock[myDept][myIndex]);
			loaderNumber = salesDesk[myDept][myIndex];
			/* salesmanCV[myDept][myIndex]->Signal(individualSalesmanLock[myDept][myIndex]); */
                        Signal(salesmanCV[myDept][myIndex], individualSalesmanLock[myDept][myIndex]);
			/* cout << "DepartmentSalesman [" << myIndex << "] is informed by the GoodsLoader [" << loaderNumber << "] that [" << itemRestocked << "] is restocked." << endl; */
			NPrint("DepartmentSalesman [%d] is informed by the GoodsLoader [%d] that [%d] is restocked.\n", sizeof("DepartmentSalesman [%d] is informed by the GoodsLoader [%d] that [%d] is restocked.\n"), NEncode2to1(myIndex, myLoaderID), itemRestocked);
			/* shelfLock[myDept][itemRestocked]->Acquire(); */
                        Acquire(shelfLock[myDept][itemRestocked]);
			/* shelfCV[myDept][itemRestocked]->Broadcast(shelfLock[myDept][itemRestocked]);	//tell customers who were waiting on an item that it was restocked */
                        Broadcast(shelfCV[myDept][itemRestocked], shelfLock[myDept][itemRestocked]);
			/* shelfLock[myDept][itemRestocked]->Release(); */
                        Release(shelfLock[myDept][itemRestocked]);
			/* individualSalesmanLock[myDept][myIndex]->Release(); */
                        Release(individualSalesmanLock[myDept][myIndex]);
			/* can't know the cust number, becaue we broadcast here! */
			/* DepartmentSalesman [identifier] informs the Customer/PrivilegeCustomer [identifier] that [item] is restocked. */
		}
		else{
                  /* individualSalesmanLock[myDept][myIndex]->Release(); */
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


	/* bool foundNewOrder = false;	//true if i go to help a salesman, and he was signalling for a loader to restock something */
	/* inactiveLoaderLock->Acquire(); */
	Acquire(inactiveLoaderLock);
	/* normal action loop */
	while(1) {
		if(!foundNewOrder) {	/* if i don't have a new order (from my last run) go to sleep */
			loaderStatus[myID] = LOAD_NOT_BUSY;
			if(mySalesID != -1){
				/*cout << "GoodsLoader [" << myID << "] is waiting for orders to restock." << endl;*/
			  NPrint("GoodsLoader [%d] is waiting for orders to restock.\n", sizeof("GoodsLoader [%d] is waiting for orders to restock.\n"), myID, 0);
			}
			/* inactiveLoaderCV->Wait(inactiveLoaderLock); */
			Wait(inactiveLoaderCV, inactiveLoaderLock);
			/* at this point, I have just been woken up from the inactive loaders waiting area */
		}
		foundNewOrder = 0;	/* initialize that I have not found a new order for this run yet */

		loaderStatus[myID] = LOAD_HAS_BEEN_SIGNALLED;
		mySalesID = -50;


		/* look through all departments to find out who signalled me */
		for(j = 0; j < numDepartments; j++) {
			/* salesLock[j]->Acquire(); */
			Acquire(salesLock[j]);
			for(i = 0; i < numSalesmen; i++) {
				if(currentSalesStatus[j][i] == SALES_SIGNALLING_LOADER) {	/* i found a person who was signalling for a loader! */
					mySalesID = i;
					currentDept = j;
					currentSalesStatus[currentDept][mySalesID] = SALES_BUSY;
					break;
				}
			}
			/* salesLock[j]->Release(); */
			Release(salesLock[j]);
			if(mySalesID != -50) {	/* used to break the second level of for loop if i found a salesman who needs me */
				break;
			}
		}
		/* inactiveLoaderLock->Release(); */
		Release(inactiveLoaderLock);

		if(mySalesID == -50){ /* the loader was signaled by the manager to get trollys (signalled, but no salesmen are signalling for me) */
			/* managerItemsLock->Acquire(); */
			Acquire(managerItemsLock);
			inHands = 0;
			i = 0;
			while(!QueueEmpty(managerItems)){
				/* simulates the manager putting the items in the stockroom */
				/* inHands = managerItems.front(); */
			  inHands = QueueFront(managerItems);
				/* managerItems.pop(); //remove the item from the manager's item list */
			  QueuePop(managerItems);
				i++;
				/* stockRoomLock->Acquire(); */
				Acquire(stockRoomLock);
				/* cout << "Goodsloader [" << myID << "] put item [" << inHands << "] back in the stock room from the manager" << endl; */
				NPrint("Goodsloader [%d] put item [%d] back in the stock room from the manager\n", sizeof("Goodsloader [%d] put item [%d] back in the stock room from the manager\n"), NEncode2to1(myID, inHands), 0);
				inHands = 0;
				/* stockRoomLock->Release(); */
				Release(stockRoomLock);
			}
			/* managerItemsLock->Release(); */
			Release(managerItemsLock);

			/* moves trollies back to the front of the store */
			/* displacedTrollyLock->Acquire(); */
			Acquire(displacedTrollyLock);
			restoreTrollies = 0;
			if(displacedTrollyCount > 0){
				restoreTrollies = displacedTrollyCount;
				displacedTrollyCount = 0;
			}
			/* displacedTrollyLock->Release(); */
			Release(displacedTrollyLock);

			if(restoreTrollies != 0) {
				/* trollyLock->Acquire(); */
				Acquire(trollyLock);
				trollyCount += restoreTrollies;
				/* trollyCV->Broadcast(trollyLock);	//inform everyone that there are now more trollies in the front */
				Broadcast(trollyCV, trollyLock);
				/* trollyLock->Release(); */
				Release(trollyLock);
			}

		}
		else{	/* It was not the manager who signalled me */
			/* individualSalesmanLock[currentDept][mySalesID]->Acquire(); */
			Acquire(individualSalesmanLock[currentDept][mySalesID]);
			shelf = salesDesk[currentDept][mySalesID];	/* read the shelf that needs stocking in from the desk */
			salesDesk[currentDept][mySalesID] = -1;		/* write back that i was not on a previous job */

			/* cout << "GoodsLoader [" << myID << "] is informed by DepartmentSalesman [" << mySalesID << */
			/* 		"] of Department [" << currentDept << "] to restock [" << shelf << "]" << endl; */
			NPrint("GoodsLoader [%d] is informed by DepartmentSalesman [%d] of Department [%d] to restock [%d]", sizeof("GoodsLoader [%d] is informed by DepartmentSalesman [%d] of Department [%d] to restock [%d]"), NEncode2to1(myID, mySalesID), NEncode2to1(currentDept, shelf));

			/* salesmanCV[currentDept][mySalesID]->Signal(individualSalesmanLock[currentDept][mySalesID]);	//tell him i'm here */
			Signal(salesmanCV[currentDept][mySalesID], individualSalesmanLock[currentDept][mySalesID]);
			/* salesmanCV[currentDept][mySalesID]->Wait(individualSalesmanLock[currentDept][mySalesID]); */
			Wait(salesmanCV[currentDept][mySalesID], individualSalesmanLock[currentDept][mySalesID]);
			/* individualSalesmanLock[currentDept][mySalesID]->Release(); */
			Release(individualSalesmanLock[currentDept][mySalesID]);


			/* Restock items */
			qtyInHands = 0;
			/* shelfLock[currentDept][shelf]->Acquire(); */
			Acquire(shelfLock[currentDept][shelf]);
			while(shelfInventory[currentDept][shelf] < maxShelfQty) {
				/* shelfLock[currentDept][shelf]->Release(); */
				Release(shelfLock[currentDept][shelf]);

				/* currentLoaderInStockLock->Acquire(); //acquires the lock in order to access the number of waiting loaders and the id of the current loader in the stock room */
				Acquire(currentLoaderInStockLock);
				if(currentLoaderInStock != -1 || waitingForStockRoomCount > 0){/* if either someone is in line or there is a loader in the stock room... */
					/* cout << "GoodsLoader [" << myID << "] is waiting for GoodsLoader [" << currentLoaderInStock << "] to leave the StockRoom." << endl; */
				  NPrint("GoodsLoader [%d] is waiting for GoodsLoader[%d] to leave the StockRoom.\n", sizeof("GoodsLoader [%d] is waiting for GoodsLoader[%d] to leave the StockRoom.\n"), NEncode2to1(myID, currentLoaderInStock), 0);
					waitingForStockRoomCount++;
					/* currentLoaderInStockCV->Wait(currentLoaderInStockLock); //wait to be let into the stock room */
					Wait(currentLoaderInStockCV, currentLoaderInStockLock);
				}
				/* stockRoomLock->Acquire(); //acquire the lock to the actual stockroom */
				Acquire(stockRoomLock);
				if(currentLoaderInStock != -1){ /* if someone was in the stockroom, change the current loader to this loader's id, wake him up, and then wait for him to acknowledge */
					currentLoaderInStock = myID;
					/* stockRoomCV->Signal(stockRoomLock); */
					Signal(stockRoomCV, stockRoomLock);
					/* stockRoomCV->Wait(stockRoomLock); */
					Wait(stockRoomCV, stockRoomLock);
				}
				else{ /* if no one was in the stockroom, then just set the current loader to this loader's id */
					currentLoaderInStock = myID;
				}
				/* cout << "GoodsLoader [" << myID << "] is in the StockRoom and got [" << shelf << "]" << endl; */
				NPrint("GoodsLoader [%d] is in the StockRoom and got [%d]\n", sizeof("GoodsLoader [%d] is in the StockRoom and got [%d]\n"), NEncode2to1(myID, shelf), 0);

				/* currentLoaderInStockLock->Release(); //releasing this allows others to check the current loader and get in line */
				Release(currentLoaderInStockLock);
				qtyInHands++; /* grab an item */
				/* currentLoaderInStockLock->Acquire(); */
				Acquire(currentLoaderInStockLock);
				if(waitingForStockRoomCount > 0){ /* if there are people in line, then signal the next person in line and decrement the line count */
					/* currentLoaderInStockCV->Signal(currentLoaderInStockLock); */
					Signal(currentLoaderInStockCV, currentLoaderInStockLock);
					waitingForStockRoomCount--;
					/* currentLoaderInStockLock->Release(); */
					Release(currentLoaderInStockLock);
					/* stockRoomCV->Wait(stockRoomLock); //wait for the loader that was woke up to put their id into current loader and then print that this loader is leaving */
					Wait(stockRoomCV, stockRoomLock);
					/* cout << "GoodsLoader [" << myID << "] leaves StockRoom." << endl; */
					NPrint("GoodsLoader [%d] leaves StockRoom.\n", sizeof("GoodsLoader [%d] leaves StockRoom.\n"), myID, 0);
					/* stockRoomCV->Signal(stockRoomLock); //wake up the last loader so they can get into the stockroom */
					Signal(stockRoomCV, stockRoomLock);
				}
				else{
					currentLoaderInStock = -1; /* if no one is in line, then set the current loader to -1, which signifies that the stockroom was empty prior */
					/* cout << "GoodsLoader [" << myID << "] leaves StockRoom." << endl; */
					NPrint("GoodsLoader [%d] leaves StockRoom.\n", sizeof("GoodsLoader [%d] leaves StockRoom.\n"), myID, 0);
					/* currentLoaderInStockLock->Release(); */
					Release(currentLoaderInStockLock);
				}
				/* stockRoomLock->Release(); */
				Release(stockRoomLock);
				for(j = 0; j < 5; j++) {
					/* currentThread->Yield(); */
				  Yield();
				}

				/* check the shelf i am going to restock */
				/* shelfLock[currentDept][shelf]->Acquire(); */
				Acquire(shelfLock[currentDept][shelf]);
				if(testNumber == 8){/*  cout << "GoodsLoader [" << myID << "] is in the act of restocking shelf [" << shelf << "] in Department [" << currentDept << "]." << endl; */
				/* if(testNumber == 8) cout << "The shelf had " << shelfInventory[currentDept][shelf] << "  but GoodsLoader " << myID <<" has added one more." << endl; */
				  NPrint("GoodsLoader [%d] is in the act of restocking shelf [%d] in Department [%d].\n", sizeof("GoodsLoader [%d] is in the act of restocking shelf [%d] in Department [%d].\n"), NEncode2to1(myID, shelf), currentDept);
				  NPrint("The shelf had [%d] but GoodsLoader [%d] has added one more.\n", sizeof("The shelf had [%d] but GoodsLoader [%d] has added one more.\n"), NEncode2to1(shelfInventory[currentDept][shelf]), currentDept);
				  }
				if(shelfInventory[currentDept][shelf] == maxShelfQty) {	/* check to see if a shelf needs more items */
					/* cout << "GoodsLoader [" << myID << "] has restocked [" << shelf << "] in Department [" << currentDept << "]." << endl; */
				  NPrint("GoodsLoader [%d] has restocked [%d] in Department [%d].\n", sizeof("GoodsLoader [%d] has restocked [%d] in Department [%d].\n"), NEncode2to1(myID, shelf), currentDept);
					qtyInHands = 0;
					break;
				}
				shelfInventory[currentDept][shelf] += qtyInHands;	/* put more items on it */
				qtyInHands = 0;
			}
			/* shelfLock[currentDept][shelf]->Release(); */
			Release(shelfLock[currentDept][shelf]);


			/* We have finished restocking.  now wait in line/inform sales */
			/* salesLock[currentDept]->Acquire(); */
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
					/* salesLock[currentDept]->Release(); */
					Release(salesLock[currentDept]);
					/* individualSalesmanLock[currentDept][mySalesID]->Acquire(); */
					Acquire(individualSalesmanLock[currentDept][mySalesID]);
					salesDesk[currentDept][mySalesID] = shelf;
					/* salesmanCV[currentDept][mySalesID]->Signal(individualSalesmanLock[currentDept][mySalesID]); */
					Signal(salesmanCV[currentDept][mySalesID], individualSalesmanLock[currentDept][mySalesID]);
					/* salesmanCV[currentDept][mySalesID]->Wait(individualSalesmanLock[currentDept][mySalesID]); */
					Wait(salesmanCV[currentDept][mySalesID], individualSalesmanLock[currentDept][mySalesID]);
					salesDesk[currentDept][mySalesID] = myID;
					/* salesmanCV[currentDept][mySalesID]->Signal(individualSalesmanLock[currentDept][mySalesID]); */
					Signal(salesmanCV[currentDept][mySalesID], individualSalesmanLock[currentDept][mySalesID]);
					/* individualSalesmanLock[currentDept][mySalesID]->Release(); */
					Release(individualSalesmanLock[currentDept][mySalesID]);

					/* i have talked to sales and they had a new job for me */
					shelf = newShelfToStock;
					/* foundNewOrder = true; */
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
						/* salesLock[currentDept]->Release(); */
						Release(salesLock[currentDept]);
						/* individualSalesmanLock[currentDept][mySalesID]->Acquire(); */
						Acquire(individualSalesmanLock[currentDept][mySalesID]);
						salesDesk[currentDept][mySalesID] = shelf;
						/* salesmanCV[currentDept][mySalesID]->Signal(individualSalesmanLock[currentDept][mySalesID]); */
						Signal(salesmanCV[currentDept][mySalesID], individualSalesmanLock[currentDept][mySalesID]);
						/* salesmanCV[currentDept][mySalesID]->Wait(individualSalesmanLock[currentDept][mySalesID]); */
						Wait(salesmanCV[currentDept][mySalesID], individualSalesmanLock[currentDept][mySalesID]);
						salesDesk[currentDept][mySalesID] = myID;
						/* salesmanCV[currentDept][mySalesID]->Signal(individualSalesmanLock[currentDept][mySalesID]); */
						Signal(salesmanCV[currentDept][mySalesID], individualSalesmanLock[currentDept][mySalesID]);
						/* individualSalesmanLock[currentDept][mySalesID]->Release(); */
						Release(individualSalesmanLock[currentDept][mySalesID]);
						break;
					}
				}
			}


			if(!foundNewOrder) {	/* i have to get in line, because i didn't find a new order from someone signalling */
									/* (i would have given them my information when i took their order) */
				if(mySalesID == -50) {	/* if i have STILL not found anyone, then i do need to get in line */
					loaderWaitingLineCount[currentDept]++;

					/* loaderCV[currentDept]->Wait(salesLock[currentDept]);//get in line		//STUCK HERE */
					Wait(loaderCV[currentDept], salesLock[currentDept]);

					for(i = 0; i < numSalesmen; i++) {	/* find the salesman who signalled me out of line */
						if(currentSalesStatus[currentDept][i] == SALES_READY_TO_TALK_TO_LOADER) {
							mySalesID = i;

							/* Ready to go talk to a salesman */
							currentlyTalkingTo[currentDept][mySalesID] = GOODSLOADER;
							currentSalesStatus[currentDept][mySalesID] = SALES_BUSY;

							/* salesLock[currentDept]->Release(); */
							Release(salesLock[currentDept]);
							/* individualSalesmanLock[currentDept][mySalesID]->Acquire(); */
							Acquire(individualSalesmanLock[currentDept][mySalesID]);
							salesDesk[currentDept][mySalesID] = shelf;
							/* salesmanCV[currentDept][mySalesID]->Signal(individualSalesmanLock[currentDept][mySalesID]); */
							Signal(salesmanCV[currentDept][mySalesID], individualSalesmanLock[currentDept][mySalesID]);
							/* salesmanCV[currentDept][mySalesID]->Wait(individualSalesmanLock[currentDept][mySalesID]); */
							Wait(salesmanCV[currentDept][mySalesID], individualSalesmanLock[currentDept][mySalesID]);
							salesDesk[currentDept][mySalesID] = myID;
							/* salesmanCV[currentDept][mySalesID]->Signal(individualSalesmanLock[currentDept][mySalesID]); */
							Signal(salesmanCV[currentDept][mySalesID], individualSalesmanLock[currentDept][mySalesID]);
							/* individualSalesmanLock[currentDept][mySalesID]->Release(); */
							Release(individualSalesmanLock[currentDept][mySalesID]);

							break;
						}
					}
				}
			}
		}

		/* Look at all depts to see if anyone else has a job that i should be aware of */
		/* inactiveLoaderLock->Acquire(); */
		Acquire(inactiveLoaderLock);
		for(j = 0; j < numDepartments; j++) {
			/* salesLock[j]->Acquire(); */
			Acquire(salesLock[j]);
			for(i = 0; i < numSalesmen; i++) {
				if(currentSalesStatus[j][i] == SALES_SIGNALLING_LOADER) {
					/* foundNewOrder = true; */
					foundNewOrder = 1;
					break;
				}
			}
			/* salesLock[j]->Release(); */
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

/* Test for greeting the customer */
/*
void TestGreetingCustomer(int NUM_CUSTOMERS, int NUM_SALESMEN){
	Thread *t;
	char *name;

	printf ("Initializing Variables for 'Greeting Customer Test'\n");

	initTrolly();
	initSalesmen();
	initShelvesWithQty(16);

	printf ("Starting 'Greeting Customer Test'\n");

	/* Fork Salesmen Threads */
/*
	for(int i = 0; i < NUM_SALESMEN; i++){
		name = new char [20];
		sprintf(name,"salesman_thread_%d",i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr)Salesman,i);
	}
*/
	/* Fork Customer Threads */
/*
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
	initShelvesWithQty(10);
	initLoaders();
	initCustomerCashier();
	initTrolly();

	char* name;
	Thread * t;

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
	numLoaders = 1;
	numTrollies = 20;
	initCustomerCashier();
	char* name;
	cout << "Test 1: please note that all cashier lines are set to be 5 except for cashier 4's line for unprivileged customers, which is set to 1 ";
	cout << " and cashier 3's line for unprivileged customer, which is set to 3.";
	cout << "\tThe privileged status of Customers is set Randomly, so we will create enough ";
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

	for(int i = 0; i < cashierNumber; i++){
		name = new char[20];
		sprintf(name, "cashier%d", i);
		t = new Thread(name);
		t->Fork((VoidFunctionPtr)cashier, i);
	}
	name = new char[20];
	name = "manager thread";
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)manager, 0);

}

void testBringCashiersBackFromBreak(){
	testNumber = 6;
	customerCash = 10000;
	cashierNumber = 4;
	custNumber = 30;
	numSalesmen = 5;
	numDepartments = 1;
	numTrollies = 20;
	numLoaders = 1;
	initShelvesWithQty(100);
	initSalesmen();
	initLoaders();
	initTrolly();
	char* name;
	Thread* t;
	initCustomerCashier();

	for(int i = 0; i < cashierNumber; i++){
			unprivilegedLineCount[i] = 3;
			privilegedLineCount[i] = 3;

	}

	name = new char[20];
	name = "manager thread";
	t = new Thread(name);
	t->Fork((VoidFunctionPtr)manager, 0);
	for(int i = 0; i < cashierNumber; i++){
		name = new char[20];
		sprintf(name, "cashier%d", i);
		cashierStatus[i] = CASH_NOT_BUSY;
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
	maxShelfQty = 5;
	customerCash = 10000;
	cashierNumber = 5;
	custNumber = 10;
	numSalesmen = 1;
	numDepartments = 1;
	numItems = 1;
	numTrollies = 1;
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
	maxShelfQty = 3;
	customerCash = 10000;
	cashierNumber = 1;
	custNumber = 1;
	numSalesmen = 1;
	numDepartments = 1;
	numTrollies = 1;
	numLoaders = 1;
	numItems = 1;
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

void printInitialConditions() {
	cout << "Test " << testNumber << " beginning with initial conditions:" << endl;
	cout << "Number of Cashiers = [" << cashierNumber << "]" << endl;
	cout << "Number of Goods Loaders = [" << numLoaders << "]" << endl;
	cout << "Number of PrivilegedCustomers = [" << "Randomly generated, 30% chance" << "]" << endl;
	cout << "Number of Customers = [" << custNumber << "]" << endl;
	cout << "Number of Managers = [" << "1" << "]" << endl;
	cout << "Number of DepartmentSalesmen = [" << numSalesmen << "]" << endl;

    cout << "Items:" << endl;
    cout << "  Number - Price" << endl;
    for(int i = 0; i < numItems; i++) {
    	cout << "  " << i << " - " << scan(i) << endl;
    }
    cout << endl;

}
*/



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
  NPrint("Number of PrivilegedCustomers = [Randomly generated, 30%% chance]\n", sizeof("Number of PrivilegedCustomers = [Randomly generated, 30% chance]\n"), 0, 0);
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
	/* cout << "\tThe privileged status of Customers is set Randomly, so we will create enough "; */
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
	/* t = new Thread(name); */
	/* t->Fork((VoidFunctionPtr)Salesman, 0); */
	/* for(int i = 0; i < custNumber; i++){ */
	/* 	name = new char [20]; */
	/* 	sprintf(name,"cust%d",i); */
	/* 	t = new Thread(name); */
	/* 	t->Fork((VoidFunctionPtr)Customer, i); */
	/* } */
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


	printInitialConditions();
	choice = 1;
	switch(choice){
	  case 1:
	    testCustomerGettingInLine();
	    break;
	  case 2:
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
}
  /*
	cout << "Menu: " << endl;
	cout << "1. Test customer choosing from cashier lines" << endl;
	cout << "2. Test manager only interacting with one customer or cashier at a Time." << endl;
	cout << "3. Test cashier receipt reception and cashier waiting for cusotmer to leave" << endl;
	cout << "4. Test cashiers scan all items" <<endl;
	cout << "5. Test cashiers being sent on break" << endl;
	cout << "6. Test cashiers being brough back from break" << endl;
	cout << "7. Test sales never suffering a race condition" << endl;
	cout << "8. Test Goods Loaders don't restock when a Customer is trying to get an item" << endl;
	cout << "9. Test only one Goods Loader enters the stock room at a Time" << endl;
	cout << "10. Test Customrs wait for items to be restocked" << endl;
	cout << "11. Run full simulation" << endl;
	cout << "12. Run full simulation with some predetermined values" << endl;
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
		else if(choice > 12 || choice < 1){
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

	default: break;
	}
}
*/
