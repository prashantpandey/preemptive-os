#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>
#include <defs.h>

void close(uint64_t file_addr)
{                                
	__syscall1(14, file_addr);

}
