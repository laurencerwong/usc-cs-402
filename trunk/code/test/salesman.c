#include "setup.h"

/*Salesman*/

void main(){
	int myIndex, myCustNumber, itemOutOfStock, itemRestocked, loaderNumber, myLoaderID,
	i, myDept;
	int prev;
	setup();
	
	Acquire(departmentIndexLock);

	Acquire(salesmanIndexLock);
	myIndex = GetMV(nextSalesmanIndex, 0);
	SetMV(nextSalesmanIndex, 0, (GetMV(nextSalesmanIndex, 0) + 1));
	myDept = GetMV(nextDepartmentIndex, 0);
	if(GetMV(nextSalesmanIndex, 0) == NUM_SALESMEN){
		SetMV(nextSalesmanIndex, 0, 0);
		SetMV(nextDepartmentIndex, 0, GetMV(nextDepartmentIndex, 0) + 1);
		if(GetMV(nextDepartmentIndex, 0) == NUM_DEPARTMENTS){
			SetMV(nextDepartmentIndex, 0, 0);
		}
	}
	Release(salesmanIndexLock);

	Release(departmentIndexLock);
	NPrint("Salesman created with index: %d, in department: %d\n", sizeof("Salesman created with index: %d, in department: %d\n"), NEncode2to1(myIndex, myDept));
	while(1) {
		Acquire(salesLock[myDept]);

		/* go on break if the manager has left me a note saying to */
		if(GetMV(salesBreakBoard[myDept], myIndex) == 1) {
		    prev = GetMV(currentSalesStatus[myDept], myIndex);
		    SetMV(currentSalesStatus[myDept], myIndex, SALES_ON_BREAK);
		    SetMV(salesBreakBoard[myDept], myIndex, 0);
			Wait (salesBreakCV[myDept][myIndex],salesLock[myDept]);
			SetMV(currentSalesStatus[myDept], myIndex, prev);
		}

		/* Check if there is someone in a line and wake them up */
		if(GetMV(greetingCustWaitingLineCount, myDept) > 0){	/* greeting */
		    SetMV(currentSalesStatus[myDept], myIndex, SALES_READY_TO_TALK);
			Signal(greetingCustCV[myDept], salesLock[myDept]);
			SetMV(greetingCustWaitingLineCount, myDept, GetMV(greetingCustWaitingLineCount, myDept) - 1);
		}
		else if(GetMV(loaderWaitingLineCount, myDept) > 0) {	/* loader */
		    SetMV(currentSalesStatus[myDept], myIndex, SALES_READY_TO_TALK_TO_LOADER);
			Signal(loaderCV[myDept], salesLock[myDept]);
			SetMV(loaderWaitingLineCount, myDept, GetMV(loaderWaitingLineCount, myDept) - 1);
		}
		else if(GetMV(complainingCustWaitingLineCount, myDept) > 0) {	/* complaining */
		    SetMV(currentSalesStatus[myDept], myIndex,  SALES_READY_TO_TALK);
			Signal(complainingCustCV[myDept], salesLock[myDept]);
			SetMV(complainingCustWaitingLineCount, myDept, GetMV(complainingCustWaitingLineCount, myDept) - 1);
		}

		else{	/* not busy */
		    SetMV(currentlyTalkingTo[myDept], myIndex, UNKNOWN);
		    SetMV(currentSalesStatus[myDept], myIndex, SALES_NOT_BUSY);
		}

		Acquire(individualSalesmanLock[myDept][myIndex]);
		Release(salesLock[myDept]);
		Wait (salesmanCV[myDept][myIndex], individualSalesmanLock[myDept][myIndex]);

		if(GetMV(currentlyTalkingTo[myDept], myIndex) == GREETING) {	/* a greeting customer came up to me */
		    myCustNumber = GetMV(salesCustNumber[myDept], myIndex);
		    if(GetMV(privCustomers, myCustNumber) == 1){
				NPrint("DepartmentSalesman [%d] welcomes PrivilegedCustomer [%d] to Department [%d].\n", sizeof("DepartmentSalesman [%d] welcomes PrivilegedCustomer [%d] to Department [%d].\n"), NEncode2to1(myIndex, myCustNumber), myDept);
			}
			else{
				NPrint("DepartmentSalesman [%d] welcomes Customer [%d] to Department [%d].\n", sizeof("DepartmentSalesman [%d] welcomes Customer [%d] to Department [%d].\n"), NEncode2to1(myIndex, myCustNumber), myDept);
			}
			Signal (salesmanCV[myDept][myIndex], individualSalesmanLock[myDept][myIndex]);
			Release(individualSalesmanLock[myDept][myIndex]);
		}
		else if(GetMV(currentlyTalkingTo[myDept], myIndex) == COMPLAINING) {	/* a complaining customer came up to me */
		    myCustNumber = GetMV(salesCustNumber[myDept], myIndex);
		    itemOutOfStock = GetMV(salesDesk[myDept], myIndex);

		    if(GetMV(privCustomers, myCustNumber) == 1){
				NPrint("DepartmentSalesman [%d] is informed by PrivilegedCustomer [%d] that [%d] is out of stock.\n", sizeof("DepartmentSalesman [%d] is informed by PrivilegedCustomer [%d] that [%d] is out of stock.\n"), NEncode2to1(myIndex, myCustNumber), itemOutOfStock);
			}
			else{
				NPrint("DepartmentSalesman [%d] is informed by Customer [%d] that [%d] is out of stock.\n", sizeof("DepartmentSalesman [%d] is informed by Customer [%d] that [%d] is out of stock.\n"), NEncode2to1(myIndex, myCustNumber), itemOutOfStock);
			}
			Acquire(individualSalesmanLock[myDept][myIndex]);
			Signal(salesmanCV[myDept][myIndex], individualSalesmanLock[myDept][myIndex]);

			/* tell goods loader */
			Acquire(salesLock[myDept]);
			SetMV(salesDesk[myDept], myIndex, itemOutOfStock);	/* Might not be necessary, because we never really took it off the desk */

			if(GetMV(loaderWaitingLineCount, myDept) > 0) {	/* if we can, get a loader from line */
			    SetMV(loaderWaitingLineCount, myDept, GetMV(loaderWaitingLineCount, myDept) - 1);
			    SetMV(currentSalesStatus[myDept], myIndex, SALES_READY_TO_TALK_TO_LOADER);
				Signal(loaderCV[myDept], salesLock[myDept]);
				Release(salesLock[myDept]);
			}
			else {	/*  no one was in line, so go to the inactive loaders */
			    SetMV(currentSalesStatus[myDept], myIndex, SALES_SIGNALLING_LOADER);
				Release(salesLock[myDept]);

				Acquire(inactiveLoaderLock);
				Signal(inactiveLoaderCV, inactiveLoaderLock);
				Release(inactiveLoaderLock);
			}

			Wait(salesmanCV[myDept][myIndex], individualSalesmanLock[myDept][myIndex]);

			/* check to see if a loader already restocked something */
			if(GetMV(salesDesk[myDept], myIndex) != -1) {		/* loader had finished stocking something and tells me about it */
			    itemRestocked = GetMV(salesDesk[myDept], myIndex);
				Signal(salesmanCV[myDept][myIndex], individualSalesmanLock[myDept][myIndex]);
				Wait(salesmanCV[myDept][myIndex], individualSalesmanLock[myDept][myIndex]);
				loaderNumber = GetMV(salesDesk[myDept], myIndex);
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

			for(i = 0; i < NUM_LOADERS; i++) {	/* find the loader who i just sent off to restock and change his status */
			    if(GetMV(loaderStatus, i) == LOAD_HAS_BEEN_SIGNALLED) {
					myLoaderID = i;
					SetMV(loaderStatus, i, LOAD_STOCKING);
					break;
				}
			}

			Release(inactiveLoaderLock);

			NPrint("DepartmentSalesman [%d] informs the GoodsLoader [%d] that [%d] is out of stock.\n", sizeof("DepartmentSalesman [%d] informs the GoodsLoader [%d] that [%d] is out of stock.\n"), NEncode2to1(myIndex, myLoaderID), itemOutOfStock);

			Signal(salesmanCV[myDept][myIndex], individualSalesmanLock[myDept][myIndex]);
			Release(individualSalesmanLock[myDept][myIndex]);

		}
		else if(GetMV(currentlyTalkingTo[myDept], myIndex) == GOODSLOADER) {	/* a loader came up to talk to me */
		    itemRestocked = GetMV(salesDesk[myDept], myIndex);
			Signal(salesmanCV[myDept][myIndex], individualSalesmanLock[myDept][myIndex]);
			Wait(salesmanCV[myDept][myIndex], individualSalesmanLock[myDept][myIndex]);
			loaderNumber = GetMV(salesDesk[myDept], myIndex);
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
	Exit(0);
}
