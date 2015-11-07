#include "syscall.h"

int main() {
	Print("Network Tests\n", 14);
	NetPrint("First Message\n", 14);
	NetPrint("Second Message\n", 15);
	NetHalt();
	Exit(0);
}

