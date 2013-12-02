#ifndef _STDIO_H
#define _STDIO_H

//#include <unistd.h>
#include <defs.h>

int printf(const char *format, ...);
int scanf(const char *format, ...);

void* malloc(uint32_t size);

#endif
