#include "setup.h"
/*Manager*/

void main(){
	int totalRevenue = 0; /* will track the total sales of the day */
	int counter = 0;
	int i, numFullLines, numAnyLines, wakeCashier, chance, r, dept, arg,
	wakeSalesman, targets[2], customerID, cashierID, amountOwed, custTypeID,
	    managerCustType, salesmenOnBreak, cashiersOnBreak;

	salesmenOnBreak = CreateQueue();
	cashiersOnBreak = CreateQueue();

	setup();
	
	/* initializes cashier totals, by which we keep track of how much money each cashier has had */
	/* in their registers over Time */

	NSrand(NTime(NULL));


	while(1){
		/* ------------------Check if all customers have left store---------------- */
		/* if all customers have left, print out totals and terminate simulation since manager will be the only */
		/* ready thread */
	  if(GetMV(customersDone, 0) == NUM_CUSTOMERS){
			for( i = 0; i < NUM_CASHIERS; i++){
				Acquire(cashierLock[i]);
				/* -----------------------------Start empty cashier drawers------------------------------------ */
				if(GetMV(GetMV(cashRegister, i), i) > 0){
				  totalRevenue += GetMV(GetMV(cashRegister, i), i);
				  SetMV(cashierTotals, i, GetMV(cashierTotals,i ) + GetMV(GetMV(cashRegister, i), i));
				  NPrint("Manager emptied Counter %d draw.\n", sizeof("Manager emptied Counter %d draw.\n"), i, 0);
				  SetMV(GetMV(cashRegister, i), i, 0);
				  NPrint("Manager has total sale of $%d.\n", sizeof("Manager has total sale of $%d.\n"), totalRevenue, 0);
					Release(cashierLock[i]);
				}
			}
			for(i = 0; i < NUM_CASHIERS; i++){
			    NPrint("Total Sale from Counter [%d] is $[%d].\n", sizeof("Total Sale from Counter [%d] is $[%d].\n"), NEncode2to1(i, GetMV(cashierTotals, i)), 0);
			    


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
			NPrint("-------Total Sale of the entire store until now is $%d---------\n", sizeof("-------Total Sale of the entire store until now is $%d---------\n"), totalRevenue, 0);
		}

		Acquire(cashierLinesLock);

		numFullLines = 0; /* will be used to figure out whether to bring cashiers back from break */
		numAnyLines = 0;
		for ( i = 0; i < NUM_CASHIERS; i++){
		  if(GetMV(privilegedLineCount, i)) numAnyLines++;
		  if(GetMV(unprivilegedLineCount, i)) numAnyLines++;
		  /* a line is "full" if it has more than 3 customers (if each cashier has a line of size 3, we want to bring back a cashier */
		  if(GetMV(privilegedLineCount, i) >= 3) numFullLines ++;
		  if(GetMV(unprivilegedLineCount, i) >= 3) numFullLines ++;
		}

		/* --------------------------Begin bring cashier back from break-------------------- */
		if(numFullLines > (NUM_CASHIERS - numCashiersOnBreak) && QueueSize(cashiersOnBreak)){
			/* bring back cashier if there are more lines with 3 customers than there are cashiers and if there are cashiers on break*/
			wakeCashier = QueueFront(cashiersOnBreak);
			if(GetMV(cashierStatus, wakeCashier) == CASH_ON_BREAK){
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
		if( chance == 1  && numCashiersOnBreak < NUM_CASHIERS -2){ /* .001% chance of sending cashier on break */
			/* generate cashier index */
			r = NRand() % NUM_CASHIERS;
			if(GetMV(cashierStatus, r) != CASH_ON_BREAK && GetMV(cashierStatus, r) != CASH_GO_ON_BREAK){
			  if(GetMV(cashierStatus, r) == CASH_NOT_BUSY) {
					Acquire(cashierLock[r]);
					SetMV(cashierDesk, r, -2);
					SetMV(cashierStatus, r, CASH_GO_ON_BREAK);
					Signal(cashierToCustCV[r], cashierLock[r]);

					Release(cashierLock[r]);
				}
			  else SetMV(cashierStatus, r, CASH_GO_ON_BREAK);
				NPrint("Manager sends Cashier [%d] on break.\n", sizeof("Manager sends Cashier [%d] on break.\n"), r, 0);
				QueuePush(cashiersOnBreak, r);
				numCashiersOnBreak++;


			}
		}

		/* -----------------------------End send cashiers on break------------------------------------- */
		Release(cashierLinesLock);

		/* __SALES_BREAK__ */
		/* -----------------------------Begin bringing salesmen back from break------------- */
		dept = 0;
		if(QueueSize(salesmenOnBreak)){

			arg = QueueFront(salesmenOnBreak);
			targets[2];
			deconstructSalesArg(arg, targets);
			wakeSalesman = targets[0];
			dept = targets[1];
			QueuePop(salesmenOnBreak);
			Acquire(salesLock[dept]);
			if((GetMV(greetingCustWaitingLineCount, dept) + GetMV(complainingCustWaitingLineCount, dept) + GetMV(loaderWaitingLineCount, dept)) > GetMV(0) && GetMV(currentSalesStatus[dept], wakeSalesman) == SALES_ON_BREAK){
				SetMV(salesBreakBoard[dept], wakeSalesman, 0);
				Signal(salesBreakCV[dept][wakeSalesman], salesLock[dept]);
				SetMV(numSalesmenOnBreak, dept, GetMV(numSalesmenOnBreak, dept) - 1);
			}
			else{
			  QueuePush(salesmenOnBreak, wakeSalesman);
			}
			Release(salesLock[dept]);
		}

		/* ------------------------------end bringing salesmen back from break-------------- */

		/* ------------------------------Begin putting salesmen on break------------------ */
		dept = NRand() % NUM_DEPARTMENTS;
		Acquire(salesLock[dept]);
		if (chance == 1 && GetMV(numSalesmenOnBreak, dept) < NUM_SALESMEN -1) {
			r = NRand() % NUM_SALESMEN;
			if(!GetMV(salesBreakBoard[dept], r) && GetMV(currentSalesStatus[dept], r) != SALES_ON_BREAK && GetMV(currentSalesStatus[dept], r) !=SALES_GO_ON_BREAK) {
			  SetMV(salesBreakBoard[dept], r, 1);
				NPrint("Manager sends Salesman [%d] of dept [%d] on break.\n", sizeof("Manager sends Salesman [%d] of dept [%d] on break.\n"), NEncode2to1(r, dept), 0);
				/* function that uses bit operations to store dept and salesman index*/ 
				/* in one int so I can get it from my queue later when I take a Salesman off break */

				arg = constructSalesArg(dept, r);
				QueuePush(salesmenOnBreak, arg);
				SetMV(numSalesmenOnBreak, dept, GetMV(numSalesmenOnBreak, dept) + 1);
			}
		}
		Release(salesLock[dept]);
		/* -----------------------------End send salesmen on break */

		for(i = 0; i < NUM_CASHIERS; i++){

			/* only manager and cashier i ever attempt to grab this lock */
			Acquire(cashierLock[i]);
			/* -----------------------------Start empty cashier drawers------------------------------------ */
	    if(GetMV(cashRegister, i) > 0){
				totalRevenue += GetMV(cashRegister, i);
				SetMV(cashierTotals, i, GetMV(cashierTotals, i) + GetMV(cashRegister, i)); /* how we remember totals for each cashier */
				NPrint("Manager emptied Counter [%d] drawer.\n", sizeof("Manager emptied Counter [%d] drawer.\n"), i, 0);
				SetMV(cashRegister, i, 0);
				NPrint("Manager has total sale of $[%d].\n", sizeof("Manager has total sale of $[%d].\n"), totalRevenue, 0);
				Release(cashierLock[i]);
				break;
			}
			/* ---------------------------end empty cashier drawers--------------------------------- */

			/* ------------------------------Start deal with broke customers------------------------- */
	    else if(GetMV(cashierFlags, i) != -1){ /* cashier flags is [i] is change to the Customre at the i-th */
				/* cashier desk so I know who to go to and deal with */
				/*cout <<"Manager got a call from Cashier [" << i << "]." << endl;*/
				NPrint("Manager got a call from Cashier [%d].\n", sizeof("Manager got a call from Cashier [%d].\n"), i, 0);
				customerID = GetMV(cashierFlags, i);
				cashierID = i;

				Acquire(managerLock);
				Signal(cashierToCustCV[cashierID], cashierLock[cashierID]);
				Signal(cashierToCustCV[cashierID], cashierLock[cashierID]);
				Release(cashierLock[i]);
				Wait(managerCV, managerLock);
				amountOwed = GetMV(cashierFlags, i);
				SetMV(cashierFlags, i, -1); /* reset cashierFlags */
				custTypeID = GetMV(managerDesk, 0);
				managerCustType = -1;
				if(custTypeID == 0){
					managerCustType = 0;
				}
				else{
					managerCustType = 1;
				}
				SetMV(managerDesk, 0, amountOwed);
				Signal(managerCV, managerLock);
				Wait(managerCV, managerLock);
				while(GetMV(managerDesk, 0) != -1 ){ /* when managerDesk == -1, customer is out of items or can afford money */
				  amountOwed -= scan(GetMV(managerDesk, 0));
					Acquire(managerItemsLock);
					SetMV(managerItems[getDepartmentFromItem(GetMV(managerDesk, 0))], GetMV(managerDesk, 0), GetMV(managerItems[getDepartmentFromItem(GetMV(managerDesk, 0))], GetMV(managerDesk, 0)) + 1 );
					SetMV(hasTakenItems, 0, 1);
					Release(managerItemsLock);
					if(managerCustType){
					  NPrint("Manager removes [%d] from the trolly PrivilegedCustomer [%d].\n", sizeof("Manager removes [%d] from the trolly PrivilegedCustomer [%d].\n"), NEncode2to1(GetMV(managerDesk, 0), customerID), 0);
					}
					else{
					  NPrint("Manager removes [%d] from the trolly Customer [%d].\n", sizeof("Manager removes [%d] from the trolly Customer [%d].\n"), NEncode2to1(GetMV(managerDesk, 0), customerID), 0);
					}
					SetMV(managerDesk, 0, amountOwed); /* giving customer new subtotal */
					/* customer will put back -1 if out of items */
					/*  or if they can now afford */
					Signal(managerCV, managerLock);
					Wait(managerCV, managerLock);
				}
				Acquire(inactiveLoaderLock);
				Signal(inactiveLoaderCV, inactiveLoaderLock);
				Release(inactiveLoaderLock);
				/* now customer has reached end of items or has enough money */
				/* I give him total */
				SetMV(managerDesk, 0, amountOwed);
				Signal(managerCV, managerLock);
				/* wait for his response, i.e. paying money*/ 
				Wait(managerCV, managerLock);
				totalRevenue += GetMV(managerDesk, 0);

				/* Now give the customer his receipt */
				if(managerCustType){
					NPrint("Manager gives receipt to PrivilegedCustomer [%d].\n", sizeof("Manager gives receipt to PrivilegedCustomer [%d].\n"), customerID, 0);
				}
				else{
					NPrint("Manager gives receipt to Customer [%d].\n", sizeof("Manager gives receipt to Customer [%d].\n"), customerID, 0);
				}
				SetMV(managerDesk, 0, -1);
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
	Exit(0);
}
