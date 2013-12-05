#include <stdlib.h>
#include <stdio.h>
#include <defs.h>
#include <syscall.h>


int fork() {
	int ret = __syscall0(6);
	return ret;
}
