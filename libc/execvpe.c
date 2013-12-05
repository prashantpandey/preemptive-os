#include <stdlib.h>
#include <syscall.h>
#include <defs.h>



int execvpe (const char *filename, char *const argv[], char *const env[]) {
	int ret = __syscall3(7, (uint64_t)filename, (uint64_t)argv, (uint64_t)env);
	return ret;
}




