#include "syscall.h"


void test(){
	/*Write("Inside forked function\n", 24, ConsoleOutput);*/
  /*
  	Yield();
  	Exit(0);
    */
}

int main() {
	/*Write("Before Fork\n", 14, ConsoleOutput);*/
    Fork(&test);
    /*
	int lock; 
	lock = CreateLock();
    Acquire(lock);
    Print(lock);
    Release(lock);
    DestroyLock(lock);
    */
    
}

