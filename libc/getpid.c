#include <syscall.h>


int getpid() {
	int pid = (int)__syscall0(4);
	return pid;
}
