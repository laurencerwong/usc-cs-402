/* test_fork_sort.c
 *    Test program to sort a large number of integers, using the fork command to run it twice.
 *
 *    Intention is to stress virtual memory system.
 *
 *    Ideally, we could read the unsorted array off of the file system,
 *	and store the result back to the file system!
 */

#include "syscall.h"


int main() {
	Fork(sort, "Sort 1", sizeof("Sort 1"));
	Fork(sort, "Sort 2", sizeof("Sort 2"));

	Exit(0);
}


int sort()
{
	int A[1024];	/* size of physical memory; with code, we'll run out of space!*/
    int i, j, tmp;

    /* first initialize the array, in reverse sorted order */
    for (i = 0; i < 1024; i++)
        A[i] = 1024 - i;

    /* then sort! */
    for (i = 0; i < 1023; i++)
        for (j = i; j < (1023 - i); j++)
	   if (A[j] > A[j + 1]) {	/* out of order -> need to swap ! */
	      tmp = A[j];
	      A[j] = A[j + 1];
	      A[j + 1] = tmp;
    	   }
    Exit(A[0]);		/* and then we're done -- should be 0! */
}
