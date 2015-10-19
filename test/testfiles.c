/* testfiles.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"


void test(){
	Write("Inside forked function\n", 24, ConsoleOutput);
}

int main() {

	Write("Before Fork\n", 14, ConsoleOutput);
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


