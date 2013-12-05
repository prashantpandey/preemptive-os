#ifndef _STDIO_H
#define _STDIO_H

//#include <unistd.h>
#include <defs.h>

int printf(const char *format, ...);
int scanf(char *buffer);

void* malloc(uint32_t size);
int getpid();

uint64_t opendir(char* dir);
uint64_t readdir(char* dir);
uint64_t open(char*  file);
int read(uint64_t addr, int size, uint64_t buf);


#endif
