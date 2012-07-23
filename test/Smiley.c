/*
 * Smiley.c
 *
 *	tests a lot of uninitialized data, and printing things in a sequence
 *
 *  Created on: Jun 20, 2012
 *      Author: Keith
 */



#include "syscall.h"


char a[21] = "     *********     \n";
char b[21] = "    *         *    \n";
char c[21] = "   *   O   O   *   \n";
char d[21] = "  *             *  \n";
char e[21] = "  *   _     _   *  \n";
char f[21] = "   *   \\___/   *   \n";	/*Note that the extra slash here is intentional!  escape sequence!*/
char g[21] = "    *         *    \n";
char h[21] = "      *******      \n";

int main()
{
	int i = 0;

	NPrint("Hello!\n", sizeof("Hello!\n"), 0, 0);
	NPrint(a, sizeof(a), 0, 0);
	NPrint(b, sizeof(b), 0, 0);
	NPrint(c, sizeof(c), 0, 0);
	NPrint(d, sizeof(d), 0, 0);
	NPrint(e, sizeof(e), 0, 0);
	NPrint(f, sizeof(f), 0, 0);
	NPrint(g, sizeof(g), 0, 0);
	NPrint(h, sizeof(h), 0, 0);

	NPrint("\nThe following should be a 9x9 'square':\n", sizeof("\nThe following should be a 9x9 'square':\n"), 0, 0);
	for(i = 0; i < 9; i++) {
		NPrint(a, sizeof(a), 0, 0);
	}

	NPrint("Userprog Smiley done\n\n", sizeof("Userprog Smiley done\n\n"), 0, 0);

    Exit(0);
}
