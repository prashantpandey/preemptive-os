#include <stdio.h>
#include <defs.h>
#include <syscall.h>

uint64_t open(char* file) {
	uint64_t addr = __syscall1(12, (uint64_t)file);
	return addr;
}
