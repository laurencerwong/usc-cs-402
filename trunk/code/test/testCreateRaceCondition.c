/*
 * testCreateRaceCondition.c
 *
 *  Created on: Jul 21, 2012
 *      Author: Robert
 */


#include "../userprog/syscall.h"

int main(){
	int lock = CreateLock("lock");
	NPrint("Lock ID is %d", sizeof("Lock ID is %d"), lock, 0);
}
