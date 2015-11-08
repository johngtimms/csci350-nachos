#include "syscall.h"

int main() {
	Exec("../test/matmult", 15);
	Print("Between\n", 9);
	Exec("../test/matmult", 15);
	Exit(0);
}