#include "syscall.h"

int main() {
	int a;
	a = Exec("../test/customer",16);
	Exit(0);
}