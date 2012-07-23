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
	Exit(0);
}
void print2() {
	NPrint("Printing %d!\n", sizeof("Printing %d!\n"), 2, 0);
	Exit(0);
}

void testForkInExec() {
	int i = 0;

	NPrint("Testing Fork and Exec: will run a smiley, and then an adding program \nwhich then executes a counter from within itself\n",
			sizeof("Testing Fork and Exec: will run a smiley, and then an adding program \nwhich then executes a counter from within itself\n"),
			0, 0);

	/*Exec("../test/NumberCounter", sizeof("../test/NumberCounter"), "NumberCounter", sizeof("NumberCounter"));*/

	Exec("../test/Smiley", sizeof("../test/Smiley"), "Smiley Main Thread", sizeof("Smiley Main Thread"));

	Exec("../test/ExecAddingThings", sizeof("../test/ExecAddingThings"), "Adding main thread", sizeof("Adding main thread"));
}

void testInfiniteExecs() {
	while(1) {
		Exec("../test/Smiley", sizeof("../test/Smiley"), "smiley main", sizeof("smiley main"));
	}
}

void permaYield() {
	NPrint("Perma Yield Running...\n", sizeof("Perma Yield Running...\n"), 0, 0);
	while(1) {
		Yield();
	}
	Exit(0);
}

void permaLoop() {
	NPrint("Perma Loop Running...\n", sizeof("Perma Loop Running...\n"), 0, 0);
	while(1) {
	}
	Exit(0);
}

void immediatelyExit() {
	Exit(0);
}

void testMaxForks() {
	int i = 0;
	while(1) {
		NPrint("Looping, Forking thread #%d\n", sizeof("Looping, Forking thread #%d\n"), i, 0);
		Fork(immediatelyExit, "test thread, max fork test", sizeof("test thread, max fork test"));
		i++;
	}
}

void testClearingLocksAndCVsOnExit() {
	Exec("../test/MaxLockCVMaker", sizeof("../test/MaxLockCVMaker"), "MaxLockCVMaker main", sizeof("MaxLockCVMaker main"));
}

int main(int argc, char** argv) {
	int choice = -1;
	int loop = 1;

	NPrint("\n\nRunning Project 2 Test Suite part 2\n", sizeof("\n\nRunning Project 2 Test Suite part 2\n"), 0, 0);

	while(loop) {
		NPrint("\nPlease select a test:\n", sizeof("Please select a test:\n"), 0, 0);
		NPrint("1. Test printing and print encoding\n", sizeof("1. Test printing and print encoding\n"), 0, 0);
		NPrint("2. Test fork\n", sizeof("2. Test fork\n"), 0, 0);
		NPrint("3. Test exec\n", sizeof("3. Test exec\n"), 0, 0);
		NPrint("4. Test more complicated fork and exec (also validates exit working for the threads and processes)\n", sizeof("4. Test more complicated fork and exec (also validates exit working for the threads and processes)\n"), 0, 0);
		NPrint("5. Test running out of memory\n", sizeof("5. Test running out of memory\n"), 0, 0);
		NPrint("6. Test clearing locks and CVs on process exit\n", sizeof("6. Test clearing locks and CVs on process exit\n"), 0, 0);
		NPrint("7. Test creating the maximum umber of threads\n", sizeof("7. Test creating the maximum umber of threads\n"), 0, 0);

		choice = ReadInt("> ", sizeof("> "));
		loop = 0;

		switch(choice) {
		case 1:
			testNEncode2to1();
			testNPrint();
			break;
		case 2:
			testFork();
			break;
		case 3:
			testExec();
			break;
		case 4:
			testForkInExec();
			break;
		case 5:
			testInfiniteExecs();
			break;
		case 6:
			testClearingLocksAndCVsOnExit();
			break;
		case 7:
			testMaxForks();
			break;
		default:
			NPrint("Invalid menu option!\n", sizeof("Invalid menu option!\n"), 0, 0);
			loop = 1;
			break;
		}
	}

	NPrint("\nProject 2 test suite part 2 main thread complete!\n", sizeof("\nProject 2 test suite part 2 main thread complete!\n"), 0, 0);

	Exit(0);
}

