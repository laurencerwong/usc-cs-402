/*
 * project2_part2_testsuite.c
 *
 *  Created on: Jun 16, 2012
 *      Author: Robert
 */

#include "../userprog/syscall.h"

void testForkCompanion(){
	NPrint("Fork is printing\n", sizeof("Fork is printing\n"), 0, 0);
	Exit(0);
}

void testFork(){
	int i;
	for(i = 0; i < 10; i++){
		Fork(testForkCompanion);
	}
	Exit(0);
}

void testExec(){

}
