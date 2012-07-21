#include "setup.h"

/*Cashier*/
void main(){
    int myCounter;
    /*set the cashier's counter*/
	
    setup();
	
    Acquire(cashierIndexLock);
    myCounter = GetMV(nextCashierIndex, 0);
    SetMV(nextCashierIndex, 0, GetMV(nextCashierIndex, 0) + 1);
    Release(cashierIndexLock);

    SetMV(cashierStatus, myCounter, CASH_NOT_BUSY);
    while(1){
	Acquire(cashierLinesLock);
	/* check my status to see if I've already been set to busy by a customer */
	if(GetMV(cashierStatus, myCounter) == CASH_GO_ON_BREAK){
	    NPrint("Cashier [%d] is going on break.\n", sizeof("Cashier [%d] is going on break.\n"), myCounter, 0);
	    SetMV(cashierStatus, myCounter, CASH_ON_BREAK);
	    Broadcast(unprivilegedCashierLineCV[myCounter], cashierLinesLock);
	    Broadcast(privilegedCashierLineCV[myCounter], cashierLinesLock);

	    Acquire(cashierLock[myCounter]);
	    Release(cashierLinesLock);

	    Wait(cashierToCustCV[myCounter], cashierLock[myCounter]);
	    Acquire(cashierLinesLock);
	    SetMV(cashierStatus, myCounter, CASH_NOT_BUSY);
	    NPrint("Cashier [%d] was called from break by Manager to work.\n", sizeof("Cashier [%d] was called from break by Manager to work.\n"), myCounter, 0);
	}
	/* check if my lines have anyone in it */
	/* set my state so approaching Customers can wait in or engage me, as apropriate */
	if(GetMV(privilegedLineCount, myCounter)){
	    Signal(privilegedCashierLineCV[myCounter], cashierLinesLock);
	    SetMV(custType, myCounter, PRIVILEGED_CUSTOMER);
	}
	else if(GetMV(unprivilegedLineCount, myCounter)){
	    Signal(unprivilegedCashierLineCV[myCounter], cashierLinesLock);
	    SetMV(custType, myCounter, CUSTOMER);
	}
	else{
	    SetMV(cashierStatus, myCounter, CASH_NOT_BUSY); /* means I can be approached without it being necessary to wait in line */
    }
    /* whether or not I'm ready for a customer, I can get my lock and go to sleep */
    Acquire(cashierLock[myCounter]);
    Release(cashierLinesLock);
    Signal(cashierToCustCV[myCounter], cashierLock[myCounter]);
    Wait(cashierToCustCV[myCounter], cashierLock[myCounter]);
    /* the next Time I'm woken up (assuming it is by a customer, not a manager */
    /* I will be totaling items */
    /* when I get here, there will be an item to scan */
    SetMV(total, myCounter, 0);
    SetMV(custID, myCounter, GetMV(cashierDesk, myCounter));
    if(GetMV(custID, myCounter) == -2){
	Release(cashierLock[myCounter]);
	continue;
    }
    Signal(cashierToCustCV[myCounter], cashierLock[myCounter]);
    Wait(cashierToCustCV[myCounter], cashierLock[myCounter]);

    NPrint("The type that was put on my counter is %d\n", sizeof("The type that was put on my counter is %d\n"), GetMV(cashierDesk, myCounter));

    if(GetMV(cashierDesk, myCounter) == 1){
	SetMV(custType, myCounter, PRIVILEGED_CUSTOMER);
    }
    else{
	SetMV(custType, myCounter, CUSTOMER);
    }

    Signal(cashierToCustCV[myCounter], cashierLock[myCounter]);
    Wait(cashierToCustCV[myCounter], cashierLock[myCounter]);

    while(GetMV(cashierDesk, myCounter) != -1){ /* -1 means we're done scanning */


	if(GetMV(custType, myCounter) == PRIVILEGED_CUSTOMER){
	    NPrint("Cashier [%d] got [%d] from trolly of Privileged Customer [%d].\n", sizeof("Cashier [%d] got [%d] from trolly of Privileged Customer [%d].\n"), NEncode2to1(myCounter, GetMV(cashierDesk, myCounter)), GetMV(custID, myCounter));
	}
	else{
	    NPrint("Cashier [%d] got [%d] from trolly of Customer [%d].\n", sizeof("Cashier [%d] got [%d] from trolly of Customer [%d].\n"), NEncode2to1(myCounter, GetMV(cashierDesk, myCounter)), GetMV(custID, myCounter));
	}
	SetMV(total, myCounter, GetMV(total, myCounter) + scan(GetMV(cashierDesk, myCounter)));
	Signal(cashierToCustCV[myCounter], cashierLock[myCounter]);
	Wait(cashierToCustCV[myCounter], cashierLock[myCounter]);
    }
    /* now I'm done scanning, so I tell the customer the total */
    if(GetMV(custType, myCounter) == PRIVILEGED_CUSTOMER){
	NPrint("Cashier [%d] tells Privileged Customer [%d] total cost is $[%d].\n", sizeof("Cashier [%d] tells Privileged Customer [%d] total cost is $[%d].\n"), NEncode2to1(myCounter, GetMV(custID, myCounter)), GetMV(total, myCounter));
    }
    else{
	NPrint("Cashier [%d] tells Customer [%d] total cost is $[%d].\n", sizeof("Cashier [%d] tells Customer [%d] total cost is $[%d].\n"), NEncode2to1(myCounter, GetMV(custID, myCounter)), GetMV(total, myCounter));
    }
    SetMV(cashierDesk, myCounter, GetMV(total, myCounter));
    Signal(cashierToCustCV[myCounter], cashierLock[myCounter]);
    Wait(cashierToCustCV[myCounter], cashierLock[myCounter]);
    if(GetMV(cashierDesk, myCounter) == -1){
	if(GetMV(custType, myCounter) == PRIVILEGED_CUSTOMER){
	    NPrint("Cashier [%d] asks Privileged Customer [%d] to wait for Manager.\n", sizeof("Cashier [%d] asks Privileged Customer [%d] to wait for Manager.\n"), NEncode2to1(myCounter, GetMV(custID, myCounter)), 0);
	    NPrint("Cashier [%d] informs the Manager that Privileged Customer [%d] does not have enough money.\n", sizeof("Cashier [%d] informs the Manager that Privileged Customer [%d] does not have enough money.\n"), NEncode2to1(myCounter, GetMV(custID, myCounter)), 0);
	}
	else{
	    NPrint("Cashier [%d] asks Customer [%d] to wait for Manager.\n", sizeof("Cashier [%d] asks Customer [%d] to wait for Manager.\n"), NEncode2to1(myCounter, GetMV(custID, myCounter)), 0);
	    NPrint("Cashier [%d] informs the Manager that Customer [%d] does not have enough money.\n", sizeof("Cashier [%d] informs the Manager that Customer [%d] does not have enough money.\n"), NEncode2to1(myCounter, GetMV(custID, myCounter)), 0);
	}
	SetMV(cashierFlags, myCounter, GetMV(custID, myCounter));
	Wait(cashierToCustCV[myCounter], cashierLock[myCounter]);
	/* Set the manager desk to 0 or 1 to tell the manager which type of */
	/* customer he is dealing with */
	if(GetMV(custType, myCounter) == PRIVILEGED_CUSTOMER){
	    SetMV(managerDesk, 0, 1);
	}
	/* if(!strcmp(custType, "Customer")){ */
	if(GetMV(custType, myCounter) == CUSTOMER){
	    SetMV(managerDesk, 0, 0);
	}
	GetMV(cashierFlags, myCounter, GetMV(total, myCounter)); /* inform manager of the total the customer owes */
	Acquire(managerLock);
	Signal(managerCV, managerLock);
	Release(managerLock);
	/* when I am woken up, the manager has taken over so I can free myself for my */
	/* next customer */
    }
    else{
	/* add value to cash register */
	SetMV(cashRegister, myCounter, GetMV(cashRegister, myCounter) + GetMV(cashierDesk, myCounter));
	if(GetMV(custType, myCounter) == PRIVILEGED_CUSTOMER){
	    NPrint("Cashier [%d] got money $[%d] from Privileged Customer [%d].\n", sizeof("Cashier [%d] got money $[%d] from Privileged Customer [%d].\n"), NEncode2to1(myCounter, GetMV(cashierDesk, myCounter)), GetMV(custID, myCounter));
	    NPrint("Cashier [%d] gave the receipt to Privileged Customer [%d] and tells him to leave.\n", sizeof("Cashier [%d] gave the receipt to Privileged Customer [%d] and tells him to leave.\n"), NEncode2to1(myCounter, GetMV(custID, myCounter)), 0);
	}
	else{
	    NPrint("Cashier [%d] got money $[%d] from Customer [%d].\n", sizeof("Cashier [%d] got money $[%d] from Customer [%d].\n"), NEncode2to1(myCounter, GetMV(cashierDesk, myCounter)), GetMV(custID, myCounter));
	    NPrint("Cashier [%d] gave the receipt to Customer [%d] and tells him to leave.\n", sizeof("Cashier [%d] gave the receipt to Customer [%d] and tells him to leave.\n"), NEncode2to1(myCounter, GetMV(custID, myCounter)), 0);
	}
	/* giving the customer a receipt */
	Signal(cashierToCustCV[myCounter], cashierLock[myCounter]);
	/* wait for customer to acknowledge getting receipt and release lock */
	Wait(cashierToCustCV[myCounter], cashierLock[myCounter]);
    }
    SetMV(cashierDesk, myCounter, 0);
    Release(cashierLock[myCounter]);

    /* done, just need to loop back and check for more customers */
    SetMV(custType, myCounter, CUSTOMER);
}
Exit(0);
}
