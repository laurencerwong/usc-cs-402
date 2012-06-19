
#include "syscall.h"

int x, y;

char g[7] = "George ";
char w[11] = "Washington";

int main()
{
	x = 2;
	y = 2;

	NPrint("Hello World!\n", sizeof("Hello World!\n"), 0, 0);

	NPrint("Hi, it's me, a test program.  My name is ", sizeof("Hi, it's me, a test program.  My name is "), 0, 0);
	NPrint(g, sizeof(g), 0, 0);
	NPrint(w, sizeof(w), 0, 0);
	NPrint("\n", sizeof("\n"), 0, 0);

	NPrint("This is a number: %d\n", sizeof("This is a number: %d\n"), 5, 0);
	NPrint("%d + %d = %d\n", sizeof("%d + %d = %d\n"), NEncode2to1(x, y), x + y);

	NPrint("Userprog HelloWorld done", sizeof("Userprog HelloWorld done"), 0, 0);

    Exit(0);
}
