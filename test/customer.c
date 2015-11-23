#include "syscall.h"
#include "setup.h"


int main() {
	int testMV;
	initGlobalVariables();
	testMV = CreateMV();
	SetMV(testMV, 5);
	Print("Customer.c ran \n",0);
	Exit(0);
}










