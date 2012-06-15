/*
 * project2_part1_testsuite.cc
 *
 *  Created on: Jun 9, 2012
 *      Author: Laurence, Rob
 */

/*#include "syscall.h"*/
#include "../userprog/syscall.h"

void forkTestPrint()
{
	/*char message[14] = "Print message";
	NPrint(message, 19, 0, 0);*/
	NPrint("A thread printing...\n", sizeof("Thread printing...\n"), 0, 0);
	Exit(0);
}

void execTestPrint1() {
	NPrint("Exec 1 printing!\n", sizeof("Exec 1 printing!\n"), 0, 0);
}
void execTestPrint2() {
	NPrint("Exec 2 printing!\n", sizeof("Exec 2 printing!\n"), 0, 0);
}
void execTestPrint3() {
	NPrint("Exec 3 printing!\n", sizeof("Exec 3 printing!\n"), 0, 0);
}

void testExec() {
	Exec("../test/matmult", sizeof("../test/matmult"));
	Exec("../test/sort", sizeof("../test/sort"));
	/*Exec("halt", 4);*/
}

void testExecAndFork()
{
	int i;
	/*Exec(forkTestPrint)*/
	for(i = 0; i < 5; i++) {
		Fork(forkTestPrint);
	}
}

void testNEncode2to1() {
	int e = 0;

	e = NEncode2to1(5, 10);


}

void testNPrint()
{
	int enc1 = 0;
	int enc2 = 0;
	NPrint("NPrint test starting\n", sizeof("NPrint test starting\n"), 0, 0);
	NPrint("Testing printing...\n", sizeof("Testing printing...\n"), 0, 0);
	NPrint("Testing printing the number 7: %d\n", sizeof("Testing printing the number 7: %d\n"), 7, 0);
	NPrint("Testing printing the first 4 numbers constants: %d, %d, %d, %d\n", sizeof("Testing printing the first 4 numbers constants: %d, %d, %d, %d\n"),
			65536, 196612);
	NPrint("Testing printing the first 4 numbers encoded: %d, %d, %d, %d\n", sizeof("Testing printing the first 4 numbers encoded: %d, %d, %d, %d\n"),
			NEncode2to1(0, 1), NEncode2to1(3, 4));
	NPrint("Values should have been equal", sizeof("Values should have been \n\n"), 0, 0);

	Exit(0);

	/*NPrint("Printing...\n");
	NPrint("Printing one number: %d\n", 1);
	NPrint("Printing two numbers: %d, %d\n", 1, 2);
	NPrint("Printing three numbers: %d, %d, %d\n", 1, 2, 3);
	NPrint("Printing four numbers: %d, %d, %d, %d\n", 1, 2, 3, 4);
	NPrint("NPrint test complete\n\n");*/
}

int main(int argc, char** argv) {

	/*char *buffer = "something";
	Write(buffer, sizeof(buffer[]), 1);	// are these arguments in the right place? */

	testNPrint();
	testExec();

	return 0;
}
