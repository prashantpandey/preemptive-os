#include <defs.h>
#include <syscall.h>
#include <stdio.h>

void* malloc(uint32_t size) {
	void* addr = (void*)__syscall1(3, size);
	return addr;
}


