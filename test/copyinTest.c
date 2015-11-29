#include "syscall.h"

int main() {
	Print("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABC\n");
	Print("newline\n");

	/* 

	This test is to experiment with how Nachos handles char arguments passed
	to syscalls. It has no relevance to the project at large. 

	First test: change Print() to only take one argument
		changed ExceptionHandler and Print_Syscall in /userprog/exception.cc 
		changed /userprog/syscall.h

		At the time, Print_Syscall used copyin(text, 100, buf)
		I tried Print("AAAAAAAAAAAAAAAA\n"); and it worked (16 chars) 
		I tried Print("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"); and it worked (32 chars) 
		Print("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAB\n"); worked (64 chars)
		Print("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAB\n"); worked (96 chars)
		Print("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAB\n"); worked (99 chars)

		Next I tried (100 chars):
		Print("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABC\n");
		Print("newline\n");

		The first \n wasn't respected, it got cut off. This is expected, because Print_Syscall is only reading 100 chars.
		So things work exactly as they should. 

	Conclusion: I have demonstrated that syscalls in user programs can take arguments of at least 100 bytes, and can probably take more

	*/

	Exit(0);
}
