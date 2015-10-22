#include "syscall.h"

void test() {
	Write("Inside forked function #1\n", 27, ConsoleOutput);
  Yield();
  Exit(0);
    
}

void test2(){
  Write("Inside forked function #2\n", 27, ConsoleOutput);
  Yield();
  Exit(0);
}

int main() {
	Write("Before Fork\n", 14, ConsoleOutput);
  Fork(&test);
  Fork(&test2);
}

