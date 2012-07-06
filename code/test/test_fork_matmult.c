#include "../userprog/syscall.h"





int matMult()
{
	int A[20][20];
	int B[20][20];
	int C[20][20];
	int i, j, k;

	for (i = 0; i < 20; i++)		/* first initialize the matrices */
		for (j = 0; j < 20; j++) {
			A[i][j] = i;
			B[i][j] = j;
			C[i][j] = 0;
		}

	for (i = 0; i < 20; i++)		/* then multiply them together */
		for (j = 0; j < 20; j++)
			for (k = 0; k < 20; k++)
				C[i][j] += A[i][k] * B[k][j];

	Exit(C[20-1][20-1]);		/* and then we're done */
}

int main(){
	NPrint("Testing fork 2 matmults\n", sizeof("Testing fork 2 matmults\n"));
	Fork(matMult, "test matmult 1", sizeof("test matmult 1"));
	Fork(matMult, "test matmult 1", sizeof("test matmult 1"));
	Exit(0);
}
