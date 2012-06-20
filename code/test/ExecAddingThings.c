/*
 * ExecAddingThings.c
 *
 * Does simple addition in threads and executes another program that counts
 *
 *  Created on: Jun 20, 2012
 *      Author: Keith
 */


#include "syscall.h"

void addRand() {
	int res = (RandInt() % 10) + (RandInt() % 10);
	NPrint("%d\n", sizeof("%d\n"), res, 0);
	Exit(0);
}

int main()
{
	Fork(addRand, "random adder", sizeof("random adder"));
	Fork(addRand, "random adder", sizeof("random adder"));
	Fork(addRand, "random adder", sizeof("random adder"));
	Fork(addRand, "random adder", sizeof("random adder"));
	Fork(addRand, "random adder", sizeof("random adder"));

	NPrint("Done forking random adding threads\n", sizeof("Done forking random adding threads\n"), 0, 0);

	Exec("../test/NumberCounter", sizeof("../test/NumberCounter"), "NumberCounter main thread", sizeof("NumberCounter main thread"));

    Exit(0);
}



