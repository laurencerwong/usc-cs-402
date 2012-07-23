#include "../userprog/syscall.h"



int A[20][20];
int B[20][20];
int C[20][20];
int i, j, k;

int D[20][20];
int E[20][20];
int F[20][20];
int l, m, n;


int matMultA()
{

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

int matMultB()
{

	for (l = 0; l < 20; l++)		/* flrst lnltlallze the matrlces */
		for (m = 0; m < 20; m++) {
			D[l][m] = l;
			E[l][m] = m;
			F[l][m] = 0;
		}

	for (l = 0; l < 20; l++)		/* then multlply them together */
		for (m = 0; m < 20; m++)
			for (n = 0; n < 20; n++)
				F[l][m] += D[l][n] * E[n][m];

	Exit(F[20-1][20-1]);		/* and then we're done */
}

int main(){
	NPrint("Testing fork 2 matmults\n", sizeof("Testing fork 2 matmults\n"));
	Fork(matMultA, "test matmult A", sizeof("test matmult A"));
	Fork(matMultB, "test matmult B", sizeof("test matmult B"));
	Exit(0);
}
