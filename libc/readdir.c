#include <stdio.h>
#include <defs.h>
#include <syscall.h>



uint64_t readdir(char* dir) {
	uint64_t ret = __syscall1(11, (uint64_t)(dir));
	return ret;
}
