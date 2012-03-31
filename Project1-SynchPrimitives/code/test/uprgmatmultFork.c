#include "syscall.h"

#define Dim 	20	/* sum total of the arrays doesn't fit in 
			 * physical memory 
			 */
int A[Dim][Dim];
int B[Dim][Dim];
int C[Dim][Dim];			 

int D[Dim][Dim];
int E[Dim][Dim];
int F[Dim][Dim];			 

void
mult()
{
	int i, j, k;
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
	WriteToConsole("\nFirst Fork : Result = %d\n",C[Dim-1][Dim-1],-1,-1);
    Exit(C[Dim-1][Dim-1]);		/* and then we're done */
}

void
mult1()
{	
	int l, m, n;
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
	WriteToConsole("\nSecond Fork : Result = %d\n",F[Dim-1][Dim-1],-1,-1);
    Exit(F[Dim-1][Dim-1]);		/* and then we're done */
}


int main(){
	WriteToConsole("\nForking 2 matmults\n\0",-1,-1,-1);
	Fork(mult);
	Fork(mult1);
	Exit(0);
}