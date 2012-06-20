/*
 * NumberCounter.c
 *
 *	tests making threads within a process
 *
 *  Created on: Jun 20, 2012
 *      Author: Keith
 */



#include "syscall.h"

void print1() {
	NPrint("%d\n", sizeof("%d\n"), 1, 0);
	Exit(0);
}
void print2() {
	NPrint("%d\n", sizeof("%d\n"), 2, 0);
	Exit(0);
}
void print3() {
	NPrint("%d\n", sizeof("%d\n"), 3, 0);
	Exit(0);
}


int main()
{
	Fork(print1, "printer 1", sizeof("printer 1"));
	Fork(print2, "printer 2", sizeof("printer 2"));
	Fork(print3, "printer 3", sizeof("printer 3"));

	NPrint("Done forking counting threads\n", sizeof("Done forking counting threads\n"), 0, 0);

    Exit(0);
}
