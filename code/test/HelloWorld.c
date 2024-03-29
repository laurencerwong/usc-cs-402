
#include "syscall.h"

int x, y;

char g[19] = "George Washington\n";

int main()
{
	x = 2;
	y = 2;

	NPrint("Hello World!\n", sizeof("Hello World!\n"), 0, 0);

	NPrint("Hi, it's me, a test program.  My name is:\n", sizeof("Hi, it's me, a test program.  My name is:\n"), 0, 0);
	NPrint(g, sizeof(g), 0, 0);
	NPrint("\n", sizeof("\n"), 0, 0);

	NPrint("This is a number: %d\n", sizeof("This is a number: %d\n"), 5, 0);
	NPrint("%d + %d = %d\n", sizeof("%d + %d = %d\n"), NEncode2to1(x, y), x + y);

	NPrint("Userprog HelloWorld done\n\n", sizeof("Userprog HelloWorld done\n\n"), 0, 0);

    Exit(0);
}
