#ifndef _STDLIB_H
#define _STDLIB_H

#include <defs.h>

int main(int argc, char* argv[], char* envp[]);
void exit(int status);

void* malloc(uint32_t size);
int getpid();
int fork();
int execvpe (const char *filename, char *const argv[], char *const env[]);

uint64_t opendir(char* dir);
uint64_t readdir(char* dir);
uint64_t open(char*  file);
int read(uint64_t addr, int size, uint64_t buf);

void close(uint64_t file_addr);
void closedir(uint64_t dir_addr);

#endif
