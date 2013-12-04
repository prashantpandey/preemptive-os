#include <defs.h>
#include <syscall.h>
#include <stdio.h>

void exit(int status) {
	__syscall0(5);
}
