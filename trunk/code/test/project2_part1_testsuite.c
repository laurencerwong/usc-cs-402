/*
 * project2_part1_testsuite.cc
 *
 *  Created on: Jun 9, 2012
 *      Author: Laurence, Rob
 */

#include "syscall.h"

int main(){
	char *buffer = "something";
	Write(buffer, sizeof(buffer[]), 1);
	return 0;
}
