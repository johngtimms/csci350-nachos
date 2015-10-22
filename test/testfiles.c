#include "syscall.h"

void test() {
	Write("Starting test\n", 14, ConsoleOutput);
  Yield();
  Exit(0);   
}

void test2() {
  Write("Starting test2\n", 15, ConsoleOutput);
  Yield();
  Exit(0);  
}

int main() {
	Write("Starting main\n", 14, ConsoleOutput);
  Fork(&test);
  Fork(&test2);
  Exit(0);
}

