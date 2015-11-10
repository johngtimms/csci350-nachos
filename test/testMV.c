#include "syscall.h"

int mv1, mv2, val;

int main() {
	Print("Test regular creation/use\n", 0);
	mv1 = CreateMV();
	Print("mv1: %d\n", mv1);
	val = GetMV(mv1);
	Print("val: %d\n", val);
	SetMV(mv1, 6);
	val = GetMV(mv1);
	Print("val: %d\n", val);
	DestroyMV(mv1);

	Print("Test misuse\n", 0);
	val = GetMV(100);
	Print("val: %d\n", val);
	SetMV(100, 6);
	DestroyMV(100);
	

	Exit(0);
}