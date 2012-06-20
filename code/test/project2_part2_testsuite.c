/*
 * project2_part2_testsuite.c
 *
 *  Created on: Jun 16, 2012
 *      Author: Keith
 */

#include "../userprog/syscall.h"

void testNEncode2to1() {
	NPrint("NEncode2to1 test starting...\n", sizeof("NEncode2to1 test starting...\n"), 0, 0);
	NPrint("Encoded value for v1 (20) and v2 (100): 0x%.4x%.4x\n", sizeof("Encoded value for v1 (20) and v2 (100): 0x%.4x%.4x\n"), NEncode2to1(100, 20), 0);
	NPrint("NEncode2to1 test complete!\n\n", sizeof("NEncode2to1 test complete!\n\n"), 0, 0);
}

void testNPrint()
{
	int enc1 = 0;
	int enc2 = 0;

	NPrint("NPrint test starting\n", sizeof("NPrint test starting\n"), 0, 0);

	NPrint("Testing printing...\n", sizeof("Testing printing...\n"), 0, 0);
	NPrint("Testing printing the number 7: %d\n", sizeof("Testing printing the number 7: %d\n"), 7, 0);
	NPrint("Testing printing the first 4 numbers constants: %d, %d, %d, %d\n", sizeof("Testing printing the first 4 numbers constants: %d, %d, %d, %d\n"),
			65536, 262147);
	NPrint("Testing printing the first 4 numbers encoded: %d, %d, %d, %d\n", sizeof("Testing printing the first 4 numbers encoded: %d, %d, %d, %d\n"),
			NEncode2to1(0, 1), NEncode2to1(3, 4));
	NPrint("Values should have been equal\n", sizeof("Values should have been equal\n"), 0, 0);

	NPrint("NPrint test complete!\n\n", sizeof("NPrint test complete!\n\n"), 0, 0);
	/*Exit(0);*/
}

void testExec() {
	NPrint("Exec test starting...\n", sizeof("Exec test starting...\n"), 0, 0);

	/*Exec("../test/matmult", sizeof("../test/matmult"));
	NPrint("Exec test: matmult executed\n", sizeof("Exec test: matmult complete\n"), 0, 0);*/
	Exec("../test/HelloWorld", sizeof("../test/HelloWorld"), "HelloWorld main thread", sizeof("HelloWorld main thread"));
	NPrint("Exec test: HelloWorld executed\n", sizeof("Exec test: HelloWorld complete\n"), 0, 0);

	Exec("../test/HelloWorld", sizeof("../test/HelloWorld"), "HelloWorld2 main thread", sizeof("HelloWorld2 main thread"));
	NPrint("Exec test: HelloWorld 2 executed\n", sizeof("Exec test: HelloWorld 2 complete\n"), 0, 0);

	Exec("../test/HelloWorld", sizeof("../test/HelloWorld"), "HelloWorld3 main thread", sizeof("HelloWorld3 main thread"));
	NPrint("Exec test: HelloWorld 3 executed\n", sizeof("Exec test: HelloWorld 3 complete\n"), 0, 0);

	NPrint("Exec test complete!\n\n", sizeof("Exec test complete!\n\n"), 0, 0);
	/*Exec("halt", 4);*/
}

void forkTestPrint()
{
	/*char message[14] = "Print message";
	NPrint(message, 19, 0, 0);*/
	NPrint("A forked thread is printing...\n", sizeof("A forked thread is printing...\n"), 0, 0);
	Exit(0);
}

void testFork()
{
	int i;
	/*char n[10] = "Thread X\n";*/
	NPrint("Fork test starting...\n", sizeof("Fork test starting...\n"), 0, 0);
	for(i = 0; i < 5; i++) {
		NPrint("Forking thread %d\n", sizeof("Forking thread %d\n"), i, 0);
		Fork(forkTestPrint, "Thread", sizeof("Thread"));
	}
	NPrint("Fork test complete!\n\n", sizeof("Fork test complete!\n\n"), 0, 0);
}

void print1() {
	NPrint("Printing %d!\n", sizeof("Printing %d!\n"), 1, 0);
}
void print2() {
	NPrint("Printing %d!\n", sizeof("Printing %d!\n"), 2, 0);
}

void testForkInExec() {
	int i = 0;

	/*Exec("../test/NumberCounter", sizeof("../test/NumberCounter"), "NumberCounter", sizeof("NumberCounter"));*/

	Exec("../test/Smily", sizeof("../test/Smily"), "Smily Main Thread", sizeof("Smily Main Thread"));

	Exec("../test/ExecAddingThings", sizeof("../test/ExecAddingThings"), "Adding main thread", sizeof("Adding main thread"));
}

void testInfiniteExecs() {
	while(1) {
		Exec("../test/Smily", sizeof("../test/Smily"), "smily main", sizeof("smily main"));
	}
}

int main(int argc, char** argv) {
	NPrint("\n\nRunning Project 2 Test Suite part 2\n", sizeof("\n\nRunning Project 2 Test Suite part 2\n"), 0, 0);

	testNEncode2to1();
	testNPrint();
	/*testExec();
	testFork();*/

	NPrint("Testing Fork and Exec: will run a smily, and then an adding program \nwhich then executes a counter from within itself\n",
			sizeof("Testing Fork and Exec: will run a smily, and then an adding program \nwhich then executes a counter from within itself\n"),
			0, 0);
	testForkInExec();

	NPrint("\nProject 2 test suite part 2 main thread complete!\n", sizeof("\nProject 2 test suite part 2 main thread complete!\n"), 0, 0);

	Exit(0);

	/*NPrint("Testing infinite Execs...", sizeof("Testing infinite Execs..."), 0, 0);
	testInfiniteExecs();*/
}

