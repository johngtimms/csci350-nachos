#include "syscall.h"

int mv1, mv2, val;

int main() {
	Print("Test regular creation & use\n", 0);
	mv1 = CreateMV();
	mv2 = CreateMV();
	Print("Created two MVs: mv1 & mv2\n", 0);


	val = GetMV(mv1);
	Print("mv1->val: %d\n", val);

	val = GetMV(mv2);
	Print("mv2->val: %d\n", val);


	Print("Testing SetMV\n", 0);
	Print("Setting mv1->val to 3\n", 0);
	SetMV(mv1, 3);
	Print("Setting mv2->val to 9\n", 0);
	SetMV(mv2, 9);

	val = GetMV(mv1);
	Print("mv1->val: %d\n", val);

	val = GetMV(mv2);
	Print("mv2->val: %d\n", val);
	Print("SetMV successful\n", 0);


	Print("Destroying mv1\n", 0);
	DestroyMV(mv1);

	Print("Destroying mv2\n", 0);
	DestroyMV(mv2);

	Print("Test misuse\n", 0);
	Print("Try setting mv2 - Look for WARN: 'GetMV failed. No such MV.'\n", 0);
	SetMV(mv2, 100);

	Exit(0);
}