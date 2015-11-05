#include "syscall.h"

int main() {
	int a, b;
	a = Exec("../test/tester", 14);
	Print("Between\n", 9);
	b = Exec("../test/tester2", 15);
	Exit(0);
}
