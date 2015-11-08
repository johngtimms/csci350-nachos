#include "syscall.h"

#define Dim 	50

int A[Dim][Dim];

int main() {
    int i, j;
    for (i = 0; i < Dim; i++)
		for (j = 0; j < Dim; j++)
	     	A[i][j] = 0;
	Print("A\n", 0);
    for(i = 0; i < Dim; i++)
		for(j = 0; j < Dim; j++)
		 	A[i][j] += i * j;
	Print("B\n", 0);
    Exit(A[Dim-1][Dim-1]);	
}