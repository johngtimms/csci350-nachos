/* testfiles.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"


void test(){
	Write("Inside forked function\n", 24, ConsoleOutput);
  	Yield();
  	Exit(0);
}

int main() {
<<<<<<< HEAD
  OpenFileId fd;
  int bytesread;
  char buf[20];

    Create("testfile", 8);
    fd = Open("testfile", 8);

    Write("testing a write\n", 16, fd );
    Close(fd);


    fd = Open("testfile", 8);
    bytesread = Read( buf, 100, fd );
    Write( buf, bytesread, ConsoleOutput );
    Close(fd);
}
=======
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

>>>>>>> b2b31de... worked on Fork and Exit functions
