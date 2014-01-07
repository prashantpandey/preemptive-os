#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>


int scanf(char* buf) {
	__syscall1(1, (uint64_t)buf);
	return 0;
} 



