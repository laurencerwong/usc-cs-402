
#include "syscall.h"

int x, y;

int main()
{
	x = 2;
	y = 2;

	NPrint("Hello World!\n", sizeof("Hello World!\n"), 0, 0);

	/*NPrint("Hi, it's me, a test program.  My name is %d \n");*/

    Exit(0);
}
