#ifndef SETUP_H
#define SETUP_H
#include "../userprog/syscall.h"

/*int salesLock[dept][desk]*/
/*salesLock[0][0] = createLock(...)*/

/*Setup all the functions*/
#define NUM_CASHIERS 3
#define NUM_SALESMEN 3
#define NUM_LOADERS 5
#define NUM_DEPARTMENTS 3
#define NUM_CUSTOMERS 30
#define NUM_TROLLY 30
#define NUM_ITEMS 10
#define MAX_SHELF_QTY 15
#define MAX_ITEMS_TO_BUY 30
#define NULL 0 

/*SalesmanStatus*/
#define SALES_NOT_BUSY 10
#define SALES_BUSY 11
#define SALES_GO_ON_BREAK 12
#define SALES_ON_BREAK 13
#define SALES_READY_TO_TALK 14
#define SALES_READY_TO_TALK_TO_LOADER 15
#define SALES_SIGNALLING_LOADER 16
#define LOAD_GET_ITEMS_FROM_MANAGER 17
/*SalesmanStatus*/

int currentSalesStatus[NUM_DEPARTMENTS];

/*WhomIWantToTalkTo*/
#define GREETING 18 
#define COMPLAINING 21
#define GOODSLOADER 22
#define SALESMAN 23
#define MANAGER 24
#define UNKNOWN 25
/*WhomIWantToTalkTo*/

int currentlyTalkingTo[NUM_DEPARTMENTS];

/*CustomerType*/
#define PRIVILEGED_CUSTOMER 26
#define CUSTOMER 27
/*CustomerType*/

int custType;

/*CashierStatus*/
#define CASH_NOT_BUSY 28
#define CASH_BUSY 29
#define CASH_ON_BREAK 30
#define CASH_GO_ON_BREAK 31
#define CASH_READY_TO_TALK 32
/*CashierStatus*/

int cashierStatus;

/*LoaderStatus*/
#define LOAD_NOT_BUSY 33
#define LOAD_STOCKING 34
#define LOAD_HAS_BEEN_SIGNALLED 35
/*LoaderStatus*/

int loaderStatus;

/*Cashier Monitor Variables*/
int total;
int custID;
int privilegedLineCount;
int unprivilegedLineCount;
int privCustomers;
int cashierDesk;
int cashRegister;
int nextCashierIndex;

/*Cashier Locks and CVs*/
int cashierLinesLock;
int cashierLock[NUM_CASHIERS];
int cashierToCustCV[NUM_CASHIERS];
int privilegedCashierLineCV[NUM_CASHIERS];
int unprivilegedCashierLineCV[NUM_CASHIERS];
int cashierIndexLock;

/*Salesman Monitor Variables*/
int nextSalesmanIndex;
int nextDepartmentIndex;
int salesCustNumber[NUM_DEPARTMENTS];
int salesDesk[NUM_DEPARTMENTS];
int salesBreakBoard[NUM_DEPARTMENTS];
int greetingCustWaitingLineCount;
int complainingCustWaitingLineCount;
int loaderWaitingLineCount;

/*Salesman Locks and CVs*/
int salesmanIndexLock;
int salesBreakCV[NUM_DEPARTMENTS][NUM_SALESMEN];
int individualSalesmanLock[NUM_DEPARTMENTS][NUM_SALESMEN];
int salesLock[NUM_DEPARTMENTS];
int salesmanCV[NUM_DEPARTMENTS][NUM_SALESMEN];
int greetingCustCV[NUM_DEPARTMENTS];
int complainingCustCV[NUM_DEPARTMENTS];
int loaderCV[NUM_DEPARTMENTS];

/*Goodsloader Monitor Variables*/
int shelfInventory[NUM_DEPARTMENTS];
int nextLoaderIndex;
int currentLoaderInStock;
int waitingForStockRoomCount;

/*Goodsloader Locks and CVs*/
int inactiveLoaderLock;
int inactiveLoaderCV;
int shelfLock[NUM_DEPARTMENTS][NUM_ITEMS];
int shelfCV[NUM_DEPARTMENTS][NUM_ITEMS];
int stockRoomLock;
int stockRoomCV;
int currentLoaderInStockLock;
int currentLoaderInStockCV;
int loaderIndexLock;

/*Manager Monitor Variables*/
int cashierFlags;
int managerItems[NUM_DEPARTMENTS];
int managerDesk;
int numSalesmenOnBreak;
int customersDone;
int numCashiersOnBreak;
int hasTakenItems;

/*Manager Locks and CVs*/
int managerLock;
int managerCV;
int managerItemsLock;

/*Trolly Variables*/
int trollyCount;
int trollyLock;
int trollyCV;
int displacedTrollyCount;
int displacedTrollyLock;

/*Customer Variables*/
int nextCustomerIndex;
int customerIndexLock;

/*Init functions*/
void initCustomerArrays();
void initLoaderArrays();
void initCashierArrays();
void initSalesmanArrays();
void initManagerArrays();

/*Global functions*/
int getDepartmentFromItem(int itemNum);
int constructSalesArg(int dept, int id);
void deconstructSalesArg(int val, int target[2]);
int scan(int item);

void setup();

#endif
