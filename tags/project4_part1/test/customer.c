#include "setup.h"

/*Customer*/

void  main(){

	int myID, r, targetDepartment, currentDepartment, mySalesIndex, someoneIsFree,
	i, j, k, shelfNum, mySalesID, myCashier, minLineValue, minCashierID,
	linesIAmLookingAt, linesIAmLookingAtCV, amountOwed;
	int numItemsToBuy = 3;
	int itemsToBuy[MAX_ITEMS_TO_BUY];
	int qtyItemsToBuy[MAX_ITEMS_TO_BUY];
	int itemsInCart[MAX_ITEMS_TO_BUY];
	int qtyItemsInCart[MAX_ITEMS_TO_BUY];
	int myCash, type;

	setup();

	Acquire(customerIndexLock);
	myID = GetMV(nextCustomerIndex, 0);
	SetMV(nextCustomerIndex, 0, GetMV(nextCustomerIndex, 0) + 1);
	Release(customerIndexLock);


	/* setup some initialization for specific tests */
	numItemsToBuy = (NRand() % NUM_ITEMS);
	myCash = NRand() % 200;

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
	for ( i = 0; i < numItemsToBuy; i++){
		itemsToBuy[i] = getDepartmentFromItem(NRand() % NUM_ITEMS);
		qtyItemsToBuy[i] = (NRand()% NUM_ITEMS);
		itemsInCart[i] = -1;
		qtyItemsInCart[i] = 0;
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
	while(GetMV(trollyCount, 0) == 0){
		if(type){
			NPrint("Privileged Customer [%d] gets in line or a trolly\n", sizeof("Privileged Customer [%d] gets in line or a trolly\n"), myID, 0);
		}
		else{
			NPrint("Customer [%d], gets in line for a trolly\n", sizeof("Customer [%d], gets in line for a trolly\n"), myID, 0);
		}
		Wait(trollyCV, trollyLock);
	}
	if(type){
		NPrint("Privileged Customer [%d] has a trolly for shopping\n", sizeof("Privileged Customer [%d] has a trolly for shopping\n"), myID, 0);
	}
	else{
		NPrint("Customer [%d] has a trolly for shopping\n", sizeof("Customer [%d] has a trolly for shopping\n"), myID, 0);
	}


	SetMV(trollyCount, 0, (GetMV(trollyCount, 0) - 1));
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
			for( j = 0; j < NUM_SALESMEN; j++) {
				if(GetMV(currentSalesStatus[targetDepartment], j) == SALES_NOT_BUSY) {
					someoneIsFree = j;
					mySalesIndex = j;
					SetMV(currentSalesStatus[targetDepartment], j, SALES_BUSY);
					SetMV(currentlyTalkingTo[targetDepartment], j, GREETING);
					break;
				}
			}

			if(someoneIsFree == -1){	/* no one is free, so wait in the greeting line */
				SetMV(greetingCustWaitingLineCount, targetDepartment, GetMV(greetingCustWaitingLineCount, targetDepartment) + 1 );
				if(type){
					NPrint("Privileged Customer [%d] gets in line for the Department [%d]\n", sizeof("Privileged Customer [%d] gets in line for the Department [%d]\n"), NEncode2to1(myID, targetDepartment), 0);
				}
				else{
					NPrint("Customer [%d] gets in line for the Department [%d]\n", sizeof("Customer [%d] gets in line for the Department [%d]\n"), NEncode2to1(myID, targetDepartment), 0);
				}
				Wait(greetingCustCV[targetDepartment], salesLock[targetDepartment]);
				for( j = 0; j < NUM_SALESMEN; j++){
					if(GetMV(currentSalesStatus[targetDepartment], j) == SALES_READY_TO_TALK){
						mySalesIndex = j;
						SetMV(currentSalesStatus[targetDepartment], j, SALES_BUSY);
						SetMV(currentlyTalkingTo[targetDepartment], j, GREETING);
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
			SetMV(salesCustNumber[targetDepartment], mySalesIndex, myID); /* Sets the customer number of the salesman to this customer's index */
			Signal(salesmanCV[targetDepartment][mySalesIndex], individualSalesmanLock[targetDepartment][mySalesIndex]);
			Wait(salesmanCV[targetDepartment][mySalesIndex], individualSalesmanLock[targetDepartment][mySalesIndex]);
			Release(individualSalesmanLock[targetDepartment][mySalesIndex]);


		}

		currentDepartment = targetDepartment;

		/* BEGINS SHOPPING */

		for( shelfNum = 0; shelfNum < NUM_ITEMS; shelfNum++) {
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

				if(GetMV(shelfInventory[currentDepartment], shelfNum) > qtyItemsToBuy[i]) {	/* if there are enough items, take what i need */
					if(type){
						NPrint("Privileged Customer [%d] has found [%d] and placed [%d] in the trolly\n", sizeof("Privileged Customer [%d] has found [%d] and placed [%d] in the trolly\n"), NEncode2to1(myID, shelfNum), qtyItemsToBuy[i]);
					}
					else{
						NPrint("Customer [%d] has found [%d] and placed [%d] in the trolly\n", sizeof("Customer [%d] has found [%d] and placed [%d] in the trolly\n"), NEncode2to1(myID, shelfNum), qtyItemsToBuy[i]);
					}

					SetMV(shelfInventory[currentDepartment], shelfNum, GetMV(shelfInventory[currentDepartment], shelfNum) - qtyItemsToBuy[i]);
					itemsInCart[i] = shelfNum;
					qtyItemsInCart[i] += qtyItemsToBuy[i];
					Release(shelfLock[currentDepartment][shelfNum]);
				}
				else {	/* We are out of this item, go tell sales! */
					if(type){
						NPrint("Privileged Customer [%d] is not able to find item [%d] and is searching for DepartmentSalesman[%d]\n", sizeof("Privileged Customer [%d] is not able to find item [%d] and is searching for DepartmentSalesman[%d]\n"), NEncode2to1(myID, shelfNum), currentDepartment);
					}
					else{
						NPrint("Customer [%d] is not able to find item [%d] and is searching for DepartmentSalesman[%d]\n", sizeof("Customer [%d] is not able to find item [%d] and is searching for DepartmentSalesman[%d]\n"), NEncode2to1(myID, shelfNum), currentDepartment);
					}
					Release(shelfLock[currentDepartment][shelfNum]);
					Acquire(salesLock[currentDepartment]);

					mySalesID = -1;

					for( j = 0; j < NUM_SALESMEN; j++) {	/* see if there is a free salesman to go to */
						/* nobody waiting, sales free */
						if(GetMV(currentSalesStatus[currentDepartment], j) == SALES_NOT_BUSY) {
							mySalesID = j;
							break;
						}
					}
					if(mySalesID == -1) {	/* no salesmen are free, I have to wait in line */
						SetMV(complainingCustWaitingLineCount, currentDepartment, (GetMV(complainingCustWaitingLineCount, currentDepartment) + 1));
						Wait(complainingCustCV[currentDepartment], salesLock[currentDepartment]);

						/* find the salesman who just signalled me */
						for( k = 0; k < NUM_SALESMEN; k++) {
							if(GetMV(currentSalesStatus[currentDepartment], k) == SALES_READY_TO_TALK) {
								mySalesID = k;
								break;
							}
						}
					}

					/* I'm now talking to a salesman */
					SetMV(currentSalesStatus[currentDepartment], mySalesID, SALES_BUSY);
					Release(salesLock[currentDepartment]);
					Acquire(individualSalesmanLock[currentDepartment][mySalesID]);

					if(type){
						NPrint("Privileged Customer [%d] is asking for assistance from DepartmentSalesmen[%d]\n", sizeof("Privileged Customer [%d] is asking for assistance from DepartmentSalesman[%d]\n"), NEncode2to1(myID, mySalesID) , 0);
					}
					else{
						NPrint("Customer [%d] is asking for assistance from DepartmentSalesmen[%d]\n", sizeof("Customer [%d] is asking for assistance from DepartmentSalesman[%d]\n"), NEncode2to1(myID, mySalesID) , 0);
					}

					SetMV(salesCustNumber[currentDepartment], mySalesID, myID);


					/* now proceed with interaction to tell sales we are out */
					SetMV(currentlyTalkingTo[currentDepartment], mySalesID, COMPLAINING);
					SetMV(salesDesk[currentDepartment], mySalesID, shelfNum);

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
		for( i = 0; i < NUM_CASHIERS; i++ ){
			/* if I find a cashier who is free, I will: */
			if(GetMV(cashierStatus, i) == CASH_NOT_BUSY){
				myCashier = i; /* remember who he is */
				Acquire(cashierLock[i]);
				SetMV(cashierStatus, i, CASH_BUSY); /* prevent others from thinking he's free */
				if(type){
					NPrint("Privileged Customer [%d] chose cashier [%d] with line of length [0].\n", sizeof("Privileged Customer [%d] chose cashier [%d] with line of length [0].\n"), NEncode2to1(myID, myCashier), 0);
				}
				else{
					NPrint("Customer [%d] chose cashier [%d] with line of length [0].\n", sizeof("Customer [%d] chose cashier [%d] with line of length [0].\n"), NEncode2to1(myID, myCashier), 0);
				}

				break; /* stop searching through lines */
			}


			/* ---------------Find shortest line--------------------------- */
			else if (i == NUM_CASHIERS - 1){
				/* set the pointers depending on which customer type i am dealing with */
				if(type){
					linesIAmLookingAt = privilegedLineCount;
					/*linesIAmLookingAtCV= privilegedCashierLineCV;*/
				}
				else{
					linesIAmLookingAt = unprivilegedLineCount;
					/*linesIAmLookingAtCV = unprivilegedCashierLineCV;*/
				}

				/* from here on, privilegedCustomers and unprivileged customers execute same code because */
				/* of the temporary variables linesIAmLookingAt (which is unprivilegedLineCount or privilegedLineCount) */
				/* and linesIAmLookingAtCV (which is un/privilegedCashierLineCV) */

				minLineValue = NUM_CUSTOMERS; /* set a default min value */
				/* find the minimum line value and remember the cashier associated with it */
				for( j = 0; j < NUM_CASHIERS; j++){
					if(GetMV(linesIAmLookingAt, j) < minLineValue && GetMV(cashierStatus, j) != CASH_ON_BREAK && GetMV(cashierStatus, j) != CASH_GO_ON_BREAK){
						/* must also check if that cashier is on break */
						minLineValue = GetMV(linesIAmLookingAt, j);
						minCashierID = j;
					}
				}
				myCashier = minCashierID;
				if(type){
					NPrint("Privileged Customer [%d] chose Cashier [%d] of line length [%d].\n", sizeof("Privileged Customer [%d] chose Cashier [%d] of line length [%d].\n"), NEncode2to1(myID, myCashier), GetMV(linesIAmLookingAt, minCashierID));
				}
				else{
					NPrint("Customer [%d] chose Cashier [%d] of line length [%d].\n", sizeof("Customer [%d] chose Cashier [%d] of line length [%d].\n"), NEncode2to1(myID, myCashier), GetMV(linesIAmLookingAt, minCashierID));
				}
				SetMV(linesIAmLookingAt, minCashierID, GetMV(linesIAmLookingAt, minCashierID) + 1);
				/*Wait(linesIAmLookingAtCV, cashierLinesLock);*/
				if(type){
					Wait(privilegedCashierLineCV[minCashierID], cashierLinesLock);
				}
				else{
					Wait(unprivilegedCashierLineCV[minCashierID], cashierLinesLock);
				}
				SetMV(linesIAmLookingAt, myCashier, GetMV(linesIAmLookingAt, myCashier) - 1); /* i have been woken up, remove myself from line */
			}
			/* -------------End find shortest line---------------------- */

		}
	}while(GetMV(cashierStatus, myCashier) == CASH_ON_BREAK); /* customer will repeat finding a cashier algorithm if he was woken up because the cashier is going on break */

	/* ----------------End looking for cashier-------------------------------------------- */


	/* code after this means I have been woken up after getting to the front of the line */
	Acquire(cashierLock[myCashier]);
	/* allow others to view monitor variable now that I've */
	/* my claim on this cashier*/
	Release(cashierLinesLock);
	SetMV(cashierDesk, myCashier, myID);	/* tell cashier who I am */
	/* signal cashier I am at his desk*/
	Signal(cashierToCustCV[myCashier], cashierLock[myCashier]);
	/* wait for his acknowlegdment*/
	Wait(cashierToCustCV[myCashier], cashierLock[myCashier]);
	SetMV(cashierDesk, myCashier, type); /* now tell him that whether or not I am privileged */
	/* signal cashier I've passed this information*/
	Signal(cashierToCustCV[myCashier], cashierLock[myCashier]);
	/* wait for his acknowledgment*/
	Wait(cashierToCustCV[myCashier], cashierLock[myCashier]);

	/* When I get here, the cashier is ready for items */

	/* ---------------------------Begin passing items to cashier--------------------------- */
	/* cycle through all items in my inventory, handing one item to cashier at a Time */
	for( i = 0; i < numItemsToBuy; i++){ /* cycle through all types of items */
		for( j = 0; j < qtyItemsInCart[i]; j++){ /* be sure we report how many of each */
			SetMV(cashierDesk, myCashier, itemsInCart[i]); /* tells the cashier what type of item */
			Signal(cashierToCustCV[myCashier], cashierLock[myCashier]);
			Wait(cashierToCustCV[myCashier], cashierLock[myCashier]);
		}
	}
	SetMV(cashierDesk, myCashier, -1); /* Tells cashier that I have reached the last item in my inventory */
	Signal(cashierToCustCV[myCashier], cashierLock[myCashier]);
	Wait(cashierToCustCV[myCashier], cashierLock[myCashier]);

	/* ------------------------End passing items to cashier-------------------------------- */


	/* when I get here, the cashier has loaded my total */
	/* If I don't have enough money, leave the error flag -1 on the cashier's desk */
	if(GetMV(cashierDesk, myCashier) > myCash){
		if(type){
			NPrint("Privileged Customer [%d] cannot pay [%d]\n", sizeof("Privileged Customer [%d] cannot pay [%d]\n"), NEncode2to1(myID, GetMV(cashierDesk, myCashier)), 0);
		}
		else{
			NPrint("Customer [%d] cannot pay [%d]\n", sizeof("Customer [%d] cannot pay [%d]\n"), NEncode2to1(myID, GetMV(cashierDesk, myCashier)), 0);
		}
		SetMV(cashierDesk, myCashier, -1);
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
				if(GetMV(managerDesk, 0) < myCash){
					break;
				}
				qtyItemsInCart[i] --;
				SetMV(managerDesk, 0, itemsInCart[i]);
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
		SetMV(managerDesk, 0, -1); /* notifies the manager I'm done */
		Signal(managerCV, managerLock);
		Wait(managerCV, managerLock);
		/* --------------------End of passing items to manager--------------- */

		amountOwed = GetMV(managerDesk, 0);	/* if I still can't afford anything, amountOwed will be 0 */
		myCash -= amountOwed;	/* updating my cash amount because I am paying manager */
		SetMV(managerDesk, 0, amountOwed);	/* technically redundant, but represents me paying money */
		if(type){
			NPrint("Privileged Customer [%d] pays $[%d] to Manager after removing items and is waiting for receipt from Manager.\n", sizeof("Privileged Customer [%d] pays $[%d] to Manager after removing items and is waiting for receipt from Manager.\n"), NEncode2to1(myID, amountOwed), 0);
		}
		else{
			NPrint("Customer [%d] pays $[%d] to Manager after removing items and is waiting for receipt from Manager.\n", sizeof("Customer [%d] pays $[%d] to Manager after removing items and is waiting for receipt from Manager.\n"), NEncode2to1(myID, amountOwed), 0);
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
		myCash -= GetMV(cashierDesk, myCashier);
		/* Now I wait for my receipt */
		/*printf("%s [%d] pays [%d] to Cashier [%d] and is now waiting for receipt.\n", type, myID, cashierDesk[myCashier], myCashier);*/
		/* now I've received my receipt and should release the cashier */
		Signal(cashierToCustCV[myCashier], cashierLock[myCashier]);
		if(type){
			NPrint("Privileged Customer [%d] pays $[%d] to Cashier [%d] and is now waiting for receipt.\n", sizeof("Privileged Customer [%d] pays $[%d] to Cashier [%d] and is now waiting for receipt.\n"), NEncode2to1(myID, GetMV(cashierDesk, myCashier)), myCashier);
		}
		else{
			NPrint("Customer [%d] pays $[%d] to Cashier [%d] and is now waiting for receipt.\n", sizeof("Customer [%d] pays $[%d] to Cashier [%d] and is now waiting for receipt.\n"), NEncode2to1(myID, GetMV(cashierDesk, myCashier)), myCashier);
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
	SetMV(displacedTrollyCount, 0, GetMV(displacedTrollyCount, 0) + 1);
	Release(displacedTrollyLock);
	/* -----------------------------End replace trolly---------------------- */

	SetMV(customersDone, 0, GetMV(customersDone, 0) + 1);	/* increment the total customers done count */

	/* some cleanup */
	/*delete itemsToBuy;*/
	Exit(0);

}
