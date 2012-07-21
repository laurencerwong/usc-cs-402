#include "setup.h"

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

int getDepartmentFromItem(int itemNum) {
    return itemNum % NUM_DEPARTMENTS;
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

void initManagerArrays(){
	managerLock = CreateLock("mLock", sizeof("mLock"));
	managerCV = CreateCondition("mCV", sizeof("mCV"));
	managerItemsLock = CreateLock("mItemsLock", sizeof("mItemsLock"));
	
	/*MVs*/
	cashierTotals = CreateMV("cTotals", sizeof("cTotals"), NUM_CASHIERS, 0); /*MV 0*/
	cashierFlags = CreateMV("cFlags", sizeof("cFlags"), NUM_CASHIERS, -1);/*MV 1*/
	numSalesmenOnBreak = CreateMV("numSOnBreak", sizeof("numSOnBreak"), NUM_DEPARTMENTS, 0);/*MV 2*/
	numCashiersOnBreak = CreateMV("numCashOnBreak", sizeof("numCashOnBreak"), 1, 0);/*MV 3*/
	managerDesk = CreateMV("managerDesk", sizeof("managerDesk"), 1, 0);/*MV 4*/
	customersDone = CreateMV("customersDone", sizeof("customersDone"), 1, 0);/*MV 5*/
	hasTakenItems = CreateMV("hasTakenItems", sizeof("hasTakenItems"), 1, 0);/*MV 6*/
	managerItems[0] = CreateMV("managerItems[0]", sizeof("managerItems[0]"), NUM_ITEMS, 0);/*MV 7*/
	managerItems[1] = CreateMV("managerItems[1]", sizeof("managerItems[1]"), NUM_ITEMS, 0);/*MV 8*/
	managerItems[2] = CreateMV("managerItems[2]", sizeof("managerItems[2]"), NUM_ITEMS, 0);/*MV 9*/
	
	NPrint("Finished initializing manager arrays\n", sizeof("Finished initializing manager arrays\n"));
}

void initCustomerArrays(){
  customerIndexLock = CreateLock("cIndexLock", sizeof("cIndexLock"));
  trollyLock  = CreateLock("trollyLock", sizeof("trollyLock"));
  trollyCV = CreateCondition("trollyCV", sizeof("trollyCV"));
  displacedTrollyLock = CreateLock("dTrollyLock", sizeof("dTrollyLock"));

  /*MVs*/
  nextCustomerIndex = CreateMV("nCustIndex", sizeof("nCustIndex"), 1, 0);/*MV 10*/
  trollyCount = CreateMV("trollyCount", sizeof("trollyCount"), 1, NUM_TROLLY);/*MV 11*/

  NPrint("finished initializing customer arrays\n", sizeof("finished initializing customer arrays\n"));

}

void initLoaderArrays(){
  inactiveLoaderLock = CreateLock("inLoaderLock", sizeof("inLoaderLock"));
  inactiveLoaderCV = CreateCondition("inLoaderCV", sizeof("inLoaderCV"));
  loaderIndexLock = CreateLock("lIndexLock", sizeof("lIndexLock"));
  currentLoaderInStockLock = CreateLock("cLInStockLock", sizeof("cLInStockLock"));
  currentLoaderInStockCV = CreateCondition("cLInStockCV", sizeof("cLInStockCV"));
  stockRoomCV = CreateCondition("stockRoomCV", sizeof("stockRoomCV"));
  stockRoomLock = CreateLock("stockRoomLock", sizeof("stockRoomLock"));

  /*MVs*/
  nextLoaderIndex = CreateMV("nextLoaderIndex", sizeof("nextLoaderIndex"), 1, 0);/*MV 12*/
  currentLoaderInStock = CreateMV("cLInStock", sizeof("cLInStock"), 1, -1);/*MV 13*/
  waitingForStockRoomCount = CreateMV("waitForStRmCnt", sizeof("waitForStRmCnt"), 1, 0);/*MV 14*/
  loaderStatus = CreateMV("LoaderStatus", sizeof("LoaderStatus"), NUM_LOADERS, LOAD_NOT_BUSY);/*MV 15*/

  /*Department 0*/
  shelfInventory[0] = CreateMV("shelfInventory[0]", sizeof("shelfInventory[0]"), 10, 10);/*MV 16*/
  shelfLock[0][0] = CreateLock("shelfLock[0][0]", sizeof("shelfLock[0][0]"));
  shelfCV[0][0] = CreateCondition("shelfCV[0][0]", sizeof("shelfCV[0][0]"));

  shelfLock[0][1] = CreateLock("shelfLock[0][1]", sizeof("shelfLock[0][1]"));
  shelfCV[0][1] = CreateCondition("shelfCV[0][1]", sizeof("shelfCV[0][1]"));
  
  shelfLock[0][2] = CreateLock("shelfLock[0][2]", sizeof("shelfLock[0][2]"));
  shelfCV[0][2] = CreateCondition("shelfCV[0][2]", sizeof("shelfCV[0][2]"));
  
  shelfLock[0][3] = CreateLock("shelfLock[0][3]", sizeof("shelfLock[0][3]"));
  shelfCV[0][3] = CreateCondition("shelfCV[0][3]", sizeof("shelfCV[0][3]"));
  
  shelfLock[0][4] = CreateLock("shelfLock[0][4]", sizeof("shelfLock[0][4]"));
  shelfCV[0][4] = CreateCondition("shelfCV[0][4]", sizeof("shelfCV[0][4]"));

  shelfLock[0][5] = CreateLock("shelfLock[0][5]", sizeof("shelfLock[0][5]"));
  shelfCV[0][5] = CreateCondition("shelfCV[0][5]", sizeof("shelfCV[0][5]"));
  
  shelfLock[0][6] = CreateLock("shelfLock[0][6]", sizeof("shelfLock[0][6]"));
  shelfCV[0][6] = CreateCondition("shelfCV[0][6]", sizeof("shelfCV[0][6]"));
  
  shelfLock[0][7] = CreateLock("shelfLock[0][7]", sizeof("shelfLock[0][7]"));
  shelfCV[0][7] = CreateCondition("shelfCV[0][7]", sizeof("shelfCV[0][7]"));
  
  shelfLock[0][8] = CreateLock("shelfLock[0][8]", sizeof("shelfLock[0][8]"));
  shelfCV[0][8] = CreateCondition("shelfCV[0][8]", sizeof("shelfCV[0][8]"));
  
  shelfLock[0][9] = CreateLock("shelfLock[0][9]", sizeof("shelfLock[0][9]"));
  shelfCV[0][9] = CreateCondition("shelfCV[0][9]", sizeof("shelfCV[0][9]"));

  /*Department 1*/
  shelfInventory[1] = CreateMV("shelfInvetory[1]", sizeof("shelfInvetory[1]"), 10, 10);/*MV 17*/
  shelfLock[1][0] = CreateLock("shelfLock[1][0]", sizeof("shelfLock[1][0]"));
  shelfCV[1][0] = CreateCondition("shelfCV[1][0]", sizeof("shelfCV[1][0]"));

  shelfLock[1][1] = CreateLock("shelfLock[1][1]", sizeof("shelfLock[1][1]"));
  shelfCV[1][1] = CreateCondition("shelfCV[1][1]", sizeof("shelfCV[1][1]"));
  
  shelfLock[1][2] = CreateLock("shelfLock[1][2]", sizeof("shelfLock[1][2]"));
  shelfCV[1][2] = CreateCondition("shelfCV[1][2]", sizeof("shelfCV[1][2]"));
  
  shelfLock[1][3] = CreateLock("shelfLock[1][3]", sizeof("shelfLock[1][3]"));
  shelfCV[1][3] = CreateCondition("shelfCV[1][3]", sizeof("shelfCV[1][3]"));
  
  shelfLock[1][4] = CreateLock("shelfLock[1][4]", sizeof("shelfLock[1][4]"));
  shelfCV[1][4] = CreateCondition("shelfCV[1][4]", sizeof("shelfCV[1][4]"));
  
  shelfLock[1][5] = CreateLock("shelfLock[1][5]", sizeof("shelfLock[1][5]"));
  shelfCV[1][5] = CreateCondition("shelfCV[1][5]", sizeof("shelfCV[1][5]"));
  
  shelfLock[1][6] = CreateLock("shelfLock[1][6]", sizeof("shelfLock[1][6]"));
  shelfCV[1][6] = CreateCondition("shelfCV[1][6]", sizeof("shelfCV[1][6]"));
  
  shelfLock[1][7] = CreateLock("shelfLock[1][7]", sizeof("shelfLock[1][7]"));
  shelfCV[1][7] = CreateCondition("shelfCV[1][7]", sizeof("shelfCV[1][7]"));
  
  shelfLock[1][8] = CreateLock("shelfLock[1][8]", sizeof("shelfLock[1][8]"));
  shelfCV[1][8] = CreateCondition("shelfCV[1][8]", sizeof("shelfCV[1][8]"));
  
  shelfLock[1][9] = CreateLock("shelfLock[1][9]", sizeof("shelfLock[1][9]"));
  shelfCV[1][9] = CreateCondition("shelfCV[1][9]", sizeof("shelfCV[1][9]"));


  /*Department 2*/
  shelfInventory[2] = CreateMV("shelfInventory[2]", sizeof("shelfInventory[2]"), 10, 10);/*MV 18*/
  shelfLock[2][0] = CreateLock("shelfLock[2][0]", sizeof("shelfLock[2][0]"));
  shelfCV[2][0] = CreateCondition("shelfCV[2][0]", sizeof("shelfCV[2][0]"));

  shelfLock[2][1] = CreateLock("shelfLock[2][1]", sizeof("shelfLock[2][1]"));
  shelfCV[2][1] = CreateCondition("shelfCV[2][1]", sizeof("shelfCV[2][1]"));
  
  shelfLock[2][2] = CreateLock("shelfLock[2][2]", sizeof("shelfLock[2][2]"));
  shelfCV[2][2] = CreateCondition("shelfCV[2][2]", sizeof("shelfCV[2][2]"));
  
  shelfLock[2][3] = CreateLock("shelfLock[2][3]", sizeof("shelfLock[2][3]"));
  shelfCV[2][3] = CreateCondition("shelfCV[2][3]", sizeof("shelfCV[2][3]"));
  
  shelfLock[2][4] = CreateLock("shelfLock[2][4]", sizeof("shelfLock[2][4]"));
  shelfCV[2][4] = CreateCondition("shelfCV[2][4]", sizeof("shelfCV[2][4]"));
  
  shelfLock[2][5] = CreateLock("shelfLock[2][5]", sizeof("shelfLock[2][5]"));
  shelfCV[2][5] = CreateCondition("shelfCV[2][5]", sizeof("shelfCV[2][5]"));
  
  shelfLock[2][6] = CreateLock("shelfLock[2][6]", sizeof("shelfLock[2][6]"));
  shelfCV[2][6] = CreateCondition("shelfCV[2][6]", sizeof("shelfCV[2][6]"));
  
  shelfLock[2][7] = CreateLock("shelfLock[2][7]", sizeof("shelfLock[2][7]"));
  shelfCV[2][7] = CreateCondition("shelfCV[2][7]", sizeof("shelfCV[2][7]"));
  
  shelfLock[2][8] = CreateLock("shelfLock[2][8]", sizeof("shelfLock[2][8]"));
  shelfCV[2][8] = CreateCondition("shelfCV[2][8]", sizeof("shelfCV[2][8]"));
  
  shelfLock[2][9] = CreateLock("shelfLock[2][9]", sizeof("shelfLock[2][9]"));
  shelfCV[2][9] = CreateCondition("shelfCV[2][9]", sizeof("shelfCV[2][9]"));
  
  NPrint("finished initializing loader arrays\n", sizeof("finished initializing loader arrays\n"));
}

void initCashierArrays(){
	cashierLinesLock  = CreateLock("cLineLock", sizeof("crLineLock"));
	cashierIndexLock = CreateLock("cashIndexLock", sizeof("cashIndexLock"));

	/*MVs */
	total = CreateMV("total", sizeof("total"), NUM_CASHIERS, 0);/*MV 19*/
	custID = CreateMV("custID", sizeof("custID"), NUM_CASHIERS, 0);/*MV 20*/
	privilegedLineCount = CreateMV("pLineCount", sizeof("pLineCount"), NUM_CASHIERS, 0);/*MV 21*/
	unprivilegedLineCount = CreateMV("unpLineCount", sizeof("unpLineCount"), NUM_CASHIERS, 0);/*MV 22*/
	privCustomers = CreateMV("privCustomers", sizeof("privCustomers"), NUM_CUSTOMERS, 0);/*MV 23*/
	cashierDesk = CreateMV("cashierDesk", sizeof("cashierDesk"), NUM_CASHIERS, 0);/*MV 24*/
	cashRegister = CreateMV("cashRegister", sizeof("cashRegister"), NUM_CASHIERS, 0);/*MV 25*/
	custType = CreateMV("custType", sizeof("custType"), NUM_CASHIERS, CUSTOMER);/*MV 26*/
	cashierStatus = CreateMV("CashierStatus", sizeof("CashierStatus"), NUM_CASHIERS, CASH_NOT_BUSY);/*MV 27*/
	nextCashierIndex = CreateMV("nCashierIndex", sizeof("nCashierIndex"), 1, 0);/*MV 28*/
  
	/*Cashier 0*/
	unprivilegedCashierLineCV[0] = CreateCondition("unpCashLineCV0", sizeof("unpCashLineCV0"));
	privilegedCashierLineCV[0] = CreateCondition("pCashLineCV0", sizeof("pCashLineCV0"));
	cashierLock[0] = CreateLock("cashierLock0", sizeof("cashierLock0"));
	cashierToCustCV[0] = CreateCondition("cashToCustCV0", sizeof("cashToCustCV0"));

	/*Cashier 1*/
	unprivilegedCashierLineCV[1] = CreateCondition("unpCashLineCV1", sizeof("unpCashLineCV1"));
	privilegedCashierLineCV[1] = CreateCondition("pCashLineCV1", sizeof("pCashLineCV1"));
	cashierLock[1] = CreateLock("cashierLock1", sizeof("cashierLock1"));
	cashierToCustCV[1] = CreateCondition("cashierToCustCV1", sizeof("cashierToCustCV1"));

	/*Cashier 2*/
	unprivilegedCashierLineCV[2] = CreateCondition("unpCashLineCV2", sizeof("unpCashLineCV2"));
	privilegedCashierLineCV[2] = CreateCondition("pCashLineCV2", sizeof("pCashLineCV2"));
	cashierLock[2] = CreateLock("cashierLock2", sizeof("cashierLock2"));
	cashierToCustCV[2] = CreateCondition("cashierToCustCV2", sizeof("cashierToCustCV2"));
	
	NPrint("Finished initializing cashier arrays\n", sizeof("Finished initializing cashier arrays\n"));
}

void initSalesmanArrays(){
  salesmanIndexLock = CreateLock("salesIndexLock", sizeof("salesIndexLock"));

  /*MVs*/
  greetingCustWaitingLineCount = CreateMV("greetingCustWLC", sizeof("greetingCustWLC"), NUM_DEPARTMENTS, 0);/*MV 29*/
  complainingCustWaitingLineCount = CreateMV("complainingCustWLC", sizeof("complainingCustWLC"), NUM_DEPARTMENTS, 0);/*MV 30*/
  loaderWaitingLineCount = CreateMV("loaderWLC", sizeof("loaderWLC"), NUM_DEPARTMENTS, 0);/*MV 31*/
  nextSalesmanIndex = CreateMV("nextSalesIndex", sizeof("nextSalesIndex"), 1, 0);/*MV 32*/
  nextDepartmentIndex = CreateMV("nextDepIndex", sizeof("nextDepIndex"), 1, 0);/*MV 33*/
  
  
  /*Department 0*/
  salesLock[0] = CreateLock("salesLock0", sizeof("salesLock0"));
  greetingCustCV[0] = CreateCondition("greetingCustCV0", sizeof("greetingCustCV0"));
  complainingCustCV[0] = CreateCondition("cCustCV0", sizeof("cCustCV0"));
  loaderCV[0] = CreateCondition("loaderCV0", sizeof("loaderCV0"));
  salesCustNumber[0] = CreateMV("salesCustNumber[0]", sizeof("salesCustNumber[0]"), NUM_SALESMEN, 0);/*MV 34*/
  salesDesk[0] = CreateMV("salesDesk[0]", sizeof("salesDesk[0]"), NUM_SALESMEN, 0);/*MV 35*/
  salesBreakBoard[0] = CreateMV("salesBreakBoard[0]", sizeof("salesBreakBoard[0]"), NUM_SALESMEN, 0);/*MV 36*/
  currentSalesStatus[0]  = CreateMV("cSalesStatus[0] ", sizeof("cSalesStatus[0] "), NUM_SALESMEN, SALES_BUSY);/*MV 37*/
  currentlyTalkingTo[0] = CreateMV("clyTalkingTo[0]", sizeof("clyTalkingTo[0]"), NUM_SALESMEN, UNKNOWN);/*MV 38*/

  /*Salesman 0*/
  salesBreakCV[0][0] = CreateCondition("salesBreakCV0-0", sizeof("salesBreakCV0-0"));
  individualSalesmanLock[0][0] = CreateLock("iSalesLock0-0", sizeof("iSalesLock0-0"));
  salesmanCV[0][0] = CreateCondition("salesmanCV0-0", sizeof("salesmanCV0-0"));

  /*Salesman 1*/
  salesBreakCV[0][1] = CreateCondition("salesBreakCV0-1", sizeof("salesBreakCV0-1"));
  individualSalesmanLock[0][1] = CreateLock("iSalesLock0-1", sizeof("iSalesLock0-1"));
  salesmanCV[0][1] = CreateCondition("salesmanCV0-1", sizeof("salesmanCV0-1"));

  /*Salesman 2*/
  salesBreakCV[0][2] = CreateCondition("salesBreakCV0-2", sizeof("salesBreakCV0-2"));
  individualSalesmanLock[0][2] = CreateLock("iSalesLock0-2", sizeof("iSalesLock0-2"));
  salesmanCV[0][2] = CreateCondition("salesmanCV0-2", sizeof("salesmanCV0-2"));

  /*Department 1*/  
  salesLock[1] = CreateLock("salesLock1", sizeof("salesLock1"));
  greetingCustCV[1] = CreateCondition("greetingCustCV1", sizeof("greetingCustCV1"));
  complainingCustCV[1] = CreateCondition("cCustCV1", sizeof("cCustCV1"));
  loaderCV[1] = CreateCondition("loaderCV1", sizeof("loaderCV1"));
  salesCustNumber[1] = CreateMV("salesCustNumber[1]", sizeof("salesCustNumber[1]"), NUM_SALESMEN, 0);/*MV 39*/
  salesDesk[1] = CreateMV("salesDesk[1]", sizeof("salesDesk[1]"), NUM_SALESMEN, 0);/*MV 40*/
  salesBreakBoard[1] = CreateMV("salesBreakBoard[1]", sizeof("salesBreakBoard[1]"), NUM_SALESMEN, 0);/*MV 41*/
  currentSalesStatus[1]  = CreateMV("cSalesStatus[1] ", sizeof("cSalesStatus[1] "), NUM_SALESMEN, SALES_BUSY);/*MV 42*/
  currentlyTalkingTo[1] = CreateMV("clyTalkingTo[1]", sizeof("clyTalkingTo[1]"), NUM_SALESMEN, UNKNOWN);/*MV 43*/
  
  /*Salesman 0*/
  salesBreakCV[1][0] = CreateCondition("salesBreakCV1-0", sizeof("salesBreakCV1-0"));
  individualSalesmanLock[1][0] = CreateLock("iSalesLock1-0", sizeof("iSalesLock1-0"));
  salesmanCV[1][0] = CreateCondition("salesmanCV1-0", sizeof("salesmanCV1-0"));

  /*Salesman 1*/
  salesBreakCV[1][1] = CreateCondition("salesBreakCV1-1", sizeof("salesBreakCV1-1"));
  individualSalesmanLock[1][1] = CreateLock("iSalesLock1-1", sizeof("iSalesLock1-1"));
  salesmanCV[1][1] = CreateCondition("salesmanCV1-1", sizeof("salesmanCV1-1"));

  /*Salesman 2*/
  salesBreakCV[1][2] = CreateCondition("salesBreakCV1-2", sizeof("salesBreakCV1-2"));
  individualSalesmanLock[1][2] = CreateLock("iSalesLock1-2", sizeof("iSalesLock1-2"));
  salesmanCV[1][2] = CreateCondition("salesmanCV1-2", sizeof("salesmanCV1-2"));

  /*Department 2*/
  salesLock[2] = CreateLock("salesLock2", sizeof("salesLock2"));
  greetingCustCV[2] = CreateCondition("greetingCustCV2", sizeof("greetingCustCV2"));
  complainingCustCV[2] = CreateCondition("cCustCV2", sizeof("cCustCV2"));
  loaderCV[2] = CreateCondition("loaderCV2", sizeof("loaderCV2"));
  salesCustNumber[2] = CreateMV("salesCustNumber[2]", sizeof("salesCustNumber[2]"), NUM_SALESMEN, 0);/*MV 44*/
  salesDesk[2] = CreateMV("salesDesk[2]", sizeof("salesDesk[2]"), NUM_SALESMEN, 0);/*MV 45*/
  salesBreakBoard[2] = CreateMV("salesBreakBoard[2]", sizeof("salesBreakBoard[2]"), NUM_SALESMEN, 0);/*MV 46*/
  currentSalesStatus[2]  = CreateMV("cSalesStatus[2] ", sizeof("cSalesStatus[2] "), NUM_SALESMEN, SALES_BUSY);/*MV 47*/
  currentlyTalkingTo[2] = CreateMV("clyTalkingTo[2]", sizeof("clyTalkingTo[2]"), NUM_SALESMEN, UNKNOWN);/*MV 48*/

  /*Salesman 0*/
  salesBreakCV[2][0] = CreateCondition("salesBreakCV2-0", sizeof("salesBreakCV2-0"));
  individualSalesmanLock[2][0] = CreateLock("iSalesLock2-0", sizeof("iSalesLock2-0"));
  salesmanCV[2][0] = CreateCondition("salesmanCV2-0", sizeof("salesmanCV2-0"));

  /*Salesman 1*/
  salesBreakCV[2][1] = CreateCondition("salesBreakCV2-1", sizeof("salesBreakCV2-1"));
  individualSalesmanLock[2][1] = CreateLock("iSalesLock2-1", sizeof("iSalesLock2-1"));
  salesmanCV[2][1] = CreateCondition("salesmanCV2-1", sizeof("salesmanCV2-1"));

  /*Salesman 2*/
  salesBreakCV[2][2] = CreateCondition("salesBreakCV2-2", sizeof("salesBreakCV2-2"));
  individualSalesmanLock[2][2] = CreateLock("iSalesLock2-2", sizeof("iSalesLock2-2"));
  salesmanCV[2][2] = CreateCondition("salesmanCV2-2", sizeof("salesmanCV2-2"));

  
  NPrint("Finished initializing salesmen arrays\n", sizeof("Finished initializing salesmen arrays\n"));
}

void setup(){
  initManagerArrays();
  initCustomerArrays();
  initLoaderArrays();
  initCashierArrays();
  initSalesmanArrays();
}

