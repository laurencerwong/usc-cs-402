/*
 * testCreateRaceConditionDriver.c
 *
 *  Created on: Jul 21, 2012
 *      Author: Robert
 */

#include "../userprog/syscall.h"

int main(){
	int i;
	for(i = 0; i < 10; i++){
		Exec("../test/testCreateRaceCondition", sizeof("../test/testCreateRaceCondition"), "main thread", sizeof("main thread"));
	}
}
