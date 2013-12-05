#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>
#include <defs.h>

void closedir(uint64_t dir_addr)
{

        __syscall1(15, dir_addr);

}

