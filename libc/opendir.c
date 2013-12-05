#include <defs.h>
#include <syscall.h>
#include <stdio.h>



uint64_t opendir(char* dir) {
	uint64_t addr = __syscall1(10, (uint64_t)dir);
	return addr;
}


