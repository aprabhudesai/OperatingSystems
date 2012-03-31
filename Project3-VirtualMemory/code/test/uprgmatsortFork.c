#include "syscall.h"

#define Dim 	20	/* sum total of the arrays doesn't fit in 
			 * physical memory 
			 */


int A[Dim][Dim];
int B[Dim][Dim];
int C[Dim][Dim];

int D[1024];	/* size of physical memory; with code, we'll run out of space!*/

void
matmultFun()
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
	
	WriteToConsole("\nMatMult - Result = %d\n",C[Dim-1][Dim-1],-1,-1);
    Exit(C[Dim-1][Dim-1]);		/* and then we're done */
}

void
sortFun()
{
    int i, j, tmp;

    /* first initialize the array, in reverse sorted order */
    for (i = 0; i < 1024; i++)		
        D[i] = 1024 - i;

    /* then sort! */
    for (i = 0; i < 1023; i++)
        for (j = i; j < (1023 - i); j++)
	   if (D[j] > D[j + 1]) {	/* out of order -> need to swap ! */
	      tmp = D[j];
	      D[j] = D[j + 1];
	      D[j + 1] = tmp;
    	   }
	WriteToConsole("\nSort - Result = %d\n",D[0],-1,-1);
    Exit(D[0]);		/* and then we're done -- should be 0! */
}

int main()
{
	WriteToConsole("\nForking matmult\n\0",-1,-1,-1);
	Fork(matmultFun);
	WriteToConsole("\nForking sort\n\0",-1,-1,-1);
	Fork(sortFun);
	Exit(0);
}