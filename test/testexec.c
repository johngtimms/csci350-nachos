#include "syscall.h"

int main() {
	int a;
	a = Exec("../test/tester", 14);
	Exit(a);
}
