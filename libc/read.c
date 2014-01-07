#include <stdio.h>
#include <defs.h>
#include <syscall.h>


int read(uint64_t file_addr, int size, uint64_t buf) {
	int size_read = (int)__syscall3(13, file_addr, (uint64_t) size, buf);
	return size_read;
}
