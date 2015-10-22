#include "syscall.h"

int main() {
	int a, b, c;
	a = Exec("../test/tester", 14);
	b = Exec("../test/tester2", 15);
}
