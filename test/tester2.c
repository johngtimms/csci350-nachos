#include "syscall.h"

void test2() {
  	Write("Starting test2\n", 15, ConsoleOutput);
  	Yield();
  	Exit(0);  
}

int main() {
	Print("Inside tester2\n", 15);
	Fork(&test2);
	Exit(0);	
}