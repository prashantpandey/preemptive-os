#include <defs.h>
#include <syscall.h>


int sleep(int sec) {
	int ret = __syscall1(8, (uint64_t)sec);
	return ret;
}
