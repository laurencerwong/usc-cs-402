/* test_fork_sort.c
 *    Test program to sort a large number of integers, using the fork command to run it twice.
 *
 *    Intention is to stress virtual memory system.
 *
 *    Ideally, we could read the unsorted array off of the file system,
 *	and store the result back to the file system!
 */

#include "../userprog/syscall.h"
#define SIZE 1024
int A[SIZE];	/* size of physical memory; with code, we'll run out of space!*/
int B[SIZE];	/* size of physical memory; with code, we'll run out of space!*/

int sortA()
{
	int i, j, tmp;

	/* first initialize the array, in reverse sorted order */
	for (i = 0; i < SIZE; i++)
		A[i] = SIZE - i;

	/* then sort! */
	for (i = 0; i < SIZE - 1; i++)
		for (j = i; j < (SIZE - 1 - i); j++)
			if (A[j] > A[j + 1]) {	/* out of order -> need to swap ! */
				tmp = A[j];
				A[j] = A[j + 1];
				A[j + 1] = tmp;
			}
	Exit(A[0]);		/* and then we're done -- should be 0! */
}

int sortB()
{
	int i, j, tmp;

	/* first initialize the array, in reverse sorted order */
	for (i = 0; i < SIZE; i++)
		B[i] = SIZE - i;

	/* then sort! */
	for (i = 0; i < SIZE - 1; i++)
		for (j = i; j < (SIZE - 1 - i); j++)
			if (B[j] > B[j + 1]) {	/* out of order -> need to swap ! */
				tmp = B[j];
				B[j] = B[j + 1];
				B[j + 1] = tmp;
			}
	Exit(B[0]);		/* and then we're done -- should be 0! */
}


int main() {
	Fork(sortA, "sort_1", sizeof("sort_1"));
	Fork(sortB, "sort_2", sizeof("sort_2"));
	Exit(0);
}

