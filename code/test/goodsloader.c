#include "setup.h"

/*Goodsloader*/
void main(){
	int myID;
	int currentDept = -1;	/* the department i am dealing with */
	int mySalesID = -1;		/* the sales i am helping */
	int shelf = -1;			/* the shelf i am dealing with */
	int foundNewOrder = 0;
	int i, j, inHands, restoreTrollies, qtyInHands, newShelfToStock;

	setup();
	
	Acquire(loaderIndexLock);
	myID = GetMV(nextLoaderIndex, 0);
        SetMV(nextLoaderIndex, 0, GetMV(nextLoaderIndex, 0) + 1);
	Release(loaderIndexLock);


	/*//true if i go to help a salesman, and he was signalling for a loader to restock something */
	Acquire(inactiveLoaderLock);
	/* normal action loop */
	while(1) {
		if(!foundNewOrder) {	/* if i don't have a new order (from my last run) go to sleep */
		    SetMV(loaderStatus, myID, LOAD_NOT_BUSY);
			if(mySalesID != -1){
				/*cout << "GoodsLoader [" << myID << "] is waiting for orders to restock." << endl;*/
				NPrint("GoodsLoader [%d] is waiting for orders to restock.\n", sizeof("GoodsLoader [%d] is waiting for orders to restock.\n"), myID, 0);
			}
			Wait(inactiveLoaderCV, inactiveLoaderLock);
			/* at this point, I have just been woken up from the inactive loaders waiting area */
		}
		foundNewOrder = 0;	/* initialize that I have not found a new order for this run yet */

		SetMV(loaderStatus, myID, LOAD_HAS_BEEN_SIGNALLED);
		mySalesID = -50;


		/* look through all departments to find out who signalled me */
		for(j = 0; j < NUM_DEPARTMENTS; j++) {
			Acquire(salesLock[j]);
			for(i = 0; i < NUM_SALESMEN; i++) {
			    if(GetMV(currentSalesStatus[j], i) == SALES_SIGNALLING_LOADER) {	/* i found a person who was signalling for a loader! */
					mySalesID = i;
					currentDept = j;
					SetMV(currentSalesStatus[currentDept], mySalesID, SALES_BUSY);
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
			if(GetMV(hasTakenItems, 0)){
			    for (i = 0; i < NUM_DEPARTMENTS; i++){
				for (j = 0; j < NUM_ITEMS; j++){
				    if (GetMV(managerItems[i], j) == 0){
					continue;
				    }
				    else{
					inHands = j;
					while(GetMV(managerItems[i], j)){ /*Delete ALL THE ITEMS*/
					    SetMV(managerItems[i], j, GetMV(managerItems[i], j) - 1);
					    Acquire(stockRoomLock);
					    NPrint("Goodsloader [%d] put item [%d] back in the stock room from the manager\n", sizeof("Goodsloader [%d] put item [%d] back in the stock room from the manager\n"), NEncode2to1(myID, inHands), 0);
					    Release(stockRoomLock);
					}
				    }
				}
			    }
			    SetMV(hasTakenItems, 0, 0); /*Tells everyone that the managerItems is empty*/
			}
			Release(managerItemsLock);

			/* moves trollies back to the front of the store */
			Acquire(displacedTrollyLock);
			restoreTrollies = 0;
			if(GetMV(displacedTrollyCount, 0) > 0){
			    restoreTrollies = GetMV(displacedTrollyCount, 0);
			    SetMV(displacedTrollyCount, 0, 0);
			}
			Release(displacedTrollyLock);

			if(restoreTrollies != 0) {
				Acquire(trollyLock);
				SetMV(trollyCount, 0, GetMV(trollyCount, 0) + restoreTrollies);
				Broadcast(trollyCV, trollyLock);
				Release(trollyLock);
			}

		}
		else{	/* It was not the manager who signalled me */
			Acquire(individualSalesmanLock[currentDept][mySalesID]);
			SetMV(shelf, 0, GetMV(salesDesk[currentDept], mySalesID));	/* read the shelf that needs stocking in from the desk */
			SetMV(salesDesk[currentDept], mySalesID, -1);		/* write back that i was not on a previous job */

			NPrint("GoodsLoader [%d] is informed by DepartmentSalesman [%d] of Department [%d] to restock [%d]\n", sizeof("GoodsLoader [%d] is informed by DepartmentSalesman [%d] of Department [%d] to restock [%d]\n"), NEncode2to1(myID, mySalesID), NEncode2to1(currentDept, shelf));

			Signal(salesmanCV[currentDept][mySalesID], individualSalesmanLock[currentDept][mySalesID]);
			Wait(salesmanCV[currentDept][mySalesID], individualSalesmanLock[currentDept][mySalesID]);
			Release(individualSalesmanLock[currentDept][mySalesID]);


			/* Restock items */
			qtyInHands = 0;
			Acquire(shelfLock[currentDept][shelf]);
			while(GetMV(shelfInventory[currentDept], shelf) < MAX_SHELF_QTY) {
				Release(shelfLock[currentDept][shelf]);

				Acquire(currentLoaderInStockLock);
				if(GetMV(currentLoaderInStock, 0) != -1 || GetMV(waitingForStockRoomCount, 0) > 0){/* if either someone is in line or there is a loader in the stock room... */
				    NPrint("GoodsLoader [%d] is waiting for GoodsLoader[%d] to leave the StockRoom.\n", sizeof("GoodsLoader [%d] is waiting for GoodsLoader[%d] to leave the StockRoom.\n"), NEncode2to1(myID, GetMV(currentLoaderInStock, 0)), 0);
				    SetMV(waitingForStockRoomCount, 0, GetMV(waitingForStockRoomCount, 0) + 1);
				    Wait(currentLoaderInStockCV, currentLoaderInStockLock);
				}
				Acquire(stockRoomLock);
				if(GetMV(currentLoaderInStock, 0) != -1){ /* if someone was in the stockroom, change the current loader to this loader's id, wake him up, and then wait for him to acknowledge */
					SetMV(currentLoaderInStock, 0, myID);
					Signal(stockRoomCV, stockRoomLock);
					Wait(stockRoomCV, stockRoomLock);
				}
				else{ /* if no one was in the stockroom, then just set the current loader to this loader's id */
					SetMV(currentLoaderInStock, 0, myID);
				}
				NPrint("GoodsLoader [%d] is in the StockRoom and got [%d]\n", sizeof("GoodsLoader [%d] is in the StockRoom and got [%d]\n"), NEncode2to1(myID, shelf), 0);

				Release(currentLoaderInStockLock);
				qtyInHands++; /* grab an item */
				Acquire(currentLoaderInStockLock);
				if(GetMV(waitingForStockRoomCount, 0) > 0){ /* if there are people in line, then signal the next person in line and decrement the line count */
					Signal(currentLoaderInStockCV, currentLoaderInStockLock);
					SetMV(waitingForStockRoomCount, 0, GetMV(waitingForStockRoomCount, 0) - 1);
					Release(currentLoaderInStockLock);
					Wait(stockRoomCV, stockRoomLock);
					NPrint("GoodsLoader [%d] leaves StockRoom.\n", sizeof("GoodsLoader [%d] leaves StockRoom.\n"), myID, 0);
					Signal(stockRoomCV, stockRoomLock);
				}
				else{
				    SetMV(currentLoaderInStock, 0, -1); /* if no one is in line, then set the current loader to -1, which signifies that the stockroom was empty prior */
					NPrint("GoodsLoader [%d] leaves StockRoom.\n", sizeof("GoodsLoader [%d] leaves StockRoom.\n"), myID, 0);
					Release(currentLoaderInStockLock);
				}
				Release(stockRoomLock);
				for(j = 0; j < 5; j++) {
					Yield();
				}

				/* check the shelf i am going to restock */
				Acquire(shelfLock[currentDept][shelf]);
				if(GetMV(shelfInventory[currentDept], shelf) == MAX_SHELF_QTY) {	/* check to see if a shelf needs more items */
					NPrint("GoodsLoader [%d] has restocked [%d] in Department [%d].\n", sizeof("GoodsLoader [%d] has restocked [%d] in Department [%d].\n"), NEncode2to1(myID, shelf), currentDept);
					qtyInHands = 0;
					break;
				}
				SetMV(shelfInventory[currentDept], shelf, GetMV(shelfInventory[currentDept], shelf) + qtyInHands);	/* put more items on it */
				qtyInHands = 0;
			}
			Release(shelfLock[currentDept][shelf]);


			/* We have finished restocking.  now wait in line/inform sales */
			Acquire(salesLock[currentDept]);

			mySalesID = -50;
			/* first look for someone who is signalling for a loader, since we can multipurpose and take a new job, while also informing that we finished a previous one */
			for(i = 0; i < NUM_SALESMEN; i++) {
			    if(GetMV(currentSalesStatus[currentDept], i) == SALES_SIGNALLING_LOADER) {
					/* all salesmen are busy trying to get loaders, but the loaders are out and trying to tell them that a job was finished */
					mySalesID = i;

					/* Ready to go talk to sales */
					newShelfToStock = GetMV(salesDesk[currentDept], mySalesID);

					SetMV(currentlyTalkingTo[currentDept], mySalesID, GOODSLOADER);
					SetMV(currentSalesStatus[currentDept], mySalesID, SALES_BUSY);
					Release(salesLock[currentDept]);
					Acquire(individualSalesmanLock[currentDept][mySalesID]);
					SetMV(salesDesk[currentDept], mySalesID, shelf);
					Signal(salesmanCV[currentDept][mySalesID], individualSalesmanLock[currentDept][mySalesID]);
					Wait(salesmanCV[currentDept][mySalesID], individualSalesmanLock[currentDept][mySalesID]);
					SetMV(salesDesk[currentDept], mySalesID, myID);
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
				for(i = 0; i < NUM_SALESMEN; i++) {
				    if(GetMV(currentSalesStatus[currentDept], i) == SALES_NOT_BUSY) {
						mySalesID = i;

						/* Ready to go talk to sales */
						SetMV(currentlyTalkingTo[currentDept], mySalesID, GOODSLOADER);
						SetMV(currentSalesStatus[currentDept], mySalesID, SALES_BUSY);
						Release(salesLock[currentDept]);
						Acquire(individualSalesmanLock[currentDept][mySalesID]);
						SetMV(salesDesk[currentDept], mySalesID, shelf);
						Signal(salesmanCV[currentDept][mySalesID], individualSalesmanLock[currentDept][mySalesID]);
						Wait(salesmanCV[currentDept][mySalesID], individualSalesmanLock[currentDept][mySalesID]);
						SetMV(salesDesk[currentDept], mySalesID, myID);
						Signal(salesmanCV[currentDept][mySalesID], individualSalesmanLock[currentDept][mySalesID]);
						Release(individualSalesmanLock[currentDept][mySalesID]);
						break;
					}
				}
			}


			if(!foundNewOrder) {	/* i have to get in line, because i didn't find a new order someone signalling */
				/* (i would have given them my information when i took their order) */
				if(mySalesID == -50) {	/* if i have STILL not found anyone, then i do need to get in line */
				    SetMV(loaderWaitingLineCount, currentDept, GetMV(loaderWaitingLineCount, currentDept) + 1);
					Wait(loaderCV[currentDept], salesLock[currentDept]);

					for(i = 0; i < NUM_SALESMEN; i++) {	/* find the salesman who signalled me out of line */
					    if(GetMV(currentSalesStatus[currentDept], i) == SALES_READY_TO_TALK_TO_LOADER) {
							mySalesID = i;

							/* Ready to go talk to a salesman */
							SetMV(currentlyTalkingTo[currentDept], mySalesID, GOODSLOADER);
							SetMV(currentSalesStatus[currentDept], mySalesID, SALES_BUSY);
							Release(salesLock[currentDept]);
							Acquire(individualSalesmanLock[currentDept][mySalesID]);
							SetMV(salesDesk[currentDept], mySalesID, shelf);
							Signal(salesmanCV[currentDept][mySalesID], individualSalesmanLock[currentDept][mySalesID]);
							Wait(salesmanCV[currentDept][mySalesID], individualSalesmanLock[currentDept][mySalesID]);
							SetMV(salesDesk[currentDept], mySalesID, myID);
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
		for(j = 0; j < NUM_DEPARTMENTS; j++) {
			Acquire(salesLock[j]);
			for(i = 0; i < NUM_SALESMEN; i++) {
			    if(GetMV(currentSalesStatus[j], i) == SALES_SIGNALLING_LOADER) {
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
	Exit(0);
}
