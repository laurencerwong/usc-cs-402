/*
 * test_server_MV.c
 *
 *  Created on: Jul 10, 2012
 *      Author: Keith
 */


void main() {
	int mv1, mv2, mv3;
	int i = 0;

	NPrint("Creating 2 MVs initialized to 0 and 5...\n", sizeof("Creating 2 MVs initialized to 0 and 5...\n"), 0, 0);
	mv1 = CreateMV("mv1", sizeof("mv1"), 1, 0);
	mv2 = CreateMV("mv2", sizeof("mv2"), 1, 0);

	NPrint("MVs are: %d, %d\n", sizeof("MVs are: %d, %d\n"), NEncode2to1(mv1, mv2), 0);
	NPrint("With values of: %d, %d\n", sizeof("With values of: %d, %d\n"), NEncode2to1(GetMV(mv1), GetMV(mv2, 0)), 0);

	NPrint("Setting 2 MVs to 3\n", sizeof("Setting 2 MVs to 3\n"), 0, 0);
	SetMV(mv1, 0, 3);
	SetMV(mv2, 0, 3);
	NPrint("MVs have values of: %d, %d\n", sizeof("MVs have values of: %d, %d\n"), NEncode2to1(GetMV(mv1, 0), GetMV(mv2, 0)), 0);

	NPrint("Destroying 2 MVs\n", sizeof("Destroying 2 MVs\n"), 0, 0);
	DestroyMV(mv1);
	DestroyMV(mv2);


	NPrint("\nCreating MV initialized to 0 with 3 entries\n", sizeof("Creating MV initialized to 0 with 3 entries\n"), 0, 0);
	mv3 = CreateMV("mv3", sizeof("mv3"), 3, 0);

	NPrint("MV is: %d\n", sizeof("MV is: %d\n"), NEncode2to1(mv3, 0), 0);
	for(i = 0; i < 3; i++) {
		NPrint("With value of MV[%d]: %d\n", sizeof("With value of MV[%d]: %d\n"), NEncode2to1(i, GetMV(mv3, i)), 0);
	}

	NPrint("Setting MV[1] to 7\n", sizeof("Setting MV[1] to 7\n"), 0, 0);
	SetMV(mv3, 1, 7);

	for(i = 0; i < 3; i++) {
		NPrint("Value of MV[%d]: %d\n", sizeof("Value of MV[%d]: %d\n"), NEncode2to1(i, GetMV(mv3, i)), 0);
	}

	NPrint("Trying to set, get, and then destroy a nonexistant MV\n", sizeof("Trying to set, get, and then destroy a nonexistant MV\n"), 0, 0);
	SetMV(12, 0, 1);
	GetMV(12, 0);
	DestroyMV(12);

	NPrint("Trying to set and get a nonexistant entry in an MV\n", sizeof("Trying to set and get a nonexistant entry in an MV\n"), 0, 0);
	SetMV(mv3, 12, 100);
	GetMV(mv3, 12);

	NPrint("Destroying the MV with 3 entries\n", sizeof("Destroying MV initialized to 0 with 3 entries\n"), 0, 0);
	DestroyMV(mv3);

	NPrint("\nServer MV test complete\n", sizeof("\nServer MV test complete\n"), 0, 0);
}
