#include <stdlib.h>
#include <syscall.h>

void ps() {
	__syscall0(16);
}


