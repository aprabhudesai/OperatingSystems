#include "syscall.h"

int A[1024];	/* size of physical memory; with code, we'll run out of space!*/

int B[1024];	/* size of physical memory; with code, we'll run out of space!*/

void
sortfun()
{
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
	WriteToConsole("\nFirst Fork : Result = %d\n",A[0],-1,-1);
    Exit(A[0]);		/* and then we're done -- should be 0! */
}

void
sortfun1()
{
	int k, l, tmp1;
    /* first initialize the array, in reverse sorted order */
    for (k = 0; k < 1024; k++)		
        B[k] = 1024 - k;

    /* then sort! */
    for (k = 0; k < 1023; k++)
        for (l = k; l < (1023 - k); l++)
	   if (B[l] > B[l + 1]) {	/* out of order -> need to swap ! */
	      tmp1 = B[l];
	      B[l] = B[l + 1];
	      B[l + 1] = tmp1;
    	   }
	WriteToConsole("\nSecond Fork : Result = %d\n",B[0],-1,-1);
    Exit(B[0]);		/* and then we're done -- should be 0! */
}

int main(){
	WriteToConsole("\nForking 2 sorts\n\0",-1,-1,-1);
	Fork(sortfun);
	Fork(sortfun1);
	Exit(0);
}