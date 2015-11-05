#include "syscall.h"

void test() {
	Write("Starting test\n", 14, ConsoleOutput);
  	Yield();
  	Exit(0);   
}

int main() {
	Print("Inside tester\n", 14);
	Fork(&test);
	Exit(0);	
}