#include "syscall.h"

int main() {
	int lock;
    lock = CreateLock();
    Acquire(lock);
    Print(lock);
    Release(lock);
    DestroyLock(lock);
}