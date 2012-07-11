/*
 * test_server_MV.c
 *
 *  Created on: Jul 10, 2012
 *      Author: Keith
 */


void main() {
	int mv1, mv2;

	NPrint("Creating 2 MVs...\n", sizeof("Creating 2 MVs...\n"), 0, 0);
	mv1 = CreateMV("mv1", sizeof("mv1"), 1, 0);
	mv2 = CreateMV("mv2", sizeof("mv2"), 1, 0);

	NPrint("Values are: %d, %d\n", sizeof("Values are: %d, %d\n"), NEncode2to1(mv1, mv2), 0);
	NPrint("Creating 2 MVs...\n", sizeof("Creating 2 MVs...\n"), 0, 0);
}
