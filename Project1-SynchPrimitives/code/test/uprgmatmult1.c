#include "syscall.h"

#define Dim 	20	/* sum total of the arrays doesn't fit in 
			 * physical memory 
			 */
int A[Dim][Dim];
int B[Dim][Dim];
int C[Dim][Dim];			 
int i, j, k;

int D[Dim][Dim];
int E[Dim][Dim];
int F[Dim][Dim];			 
int l, m, n;

void
mult()
{
    for (i = 0; i < Dim; i++)		/* first initialize the matrices */
	for (j = 0; j < Dim; j++) {
	     A[i][j] = i;
	     B[i][j] = j;
	     C[i][j] = 0;
	}

    for (i = 0; i < Dim; i++)		/* then multiply them together */
	for (j = 0; j < Dim; j++)
            for (k = 0; k < Dim; k++)
		 C[i][j] += A[i][k] * B[k][j];
    Exit(C[Dim-1][Dim-1]);		/* and then we're done */
}

void
mult1()
{	
    for (l = 0; l < Dim; l++)		/* first initialize the matrices */
	for (m = 0; m < Dim; m++) {
	     D[l][m] = l;
	     E[l][m] = m;
	     F[l][m] = 0;
	}

    for (l = 0; l < Dim; l++)		/* then multiply them together */
	for (m = 0; m < Dim; m++)
            for (n = 0; n < Dim; n++)
		 F[l][m] += D[l][n] * E[n][m];
    Exit(F[Dim-1][Dim-1]);		/* and then we're done */
}


int main(){
	Fork(mult);
	Fork(mult1);
	Exit(0);
}