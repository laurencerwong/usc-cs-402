/*
 * testDetectDeadLock.c
 *
 *  Created on: Jul 21, 2012
 *      Author: Robert
 */

int main(){
	int lock = CreateLock("Lock", sizeof("Lock"));
	Acquire(lock);
	NPrint("Kill me!", sizeof("Kill me!"));

}

