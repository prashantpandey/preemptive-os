#ifndef _COMMON_H
#define _COMMON_H

//#include <unistd.h>
#include <defs.h>

void outb( unsigned short port, unsigned char val );
unsigned char inb( unsigned short port );
void memcpy(uint16_t *dest, const uint16_t *src, uint32_t len);
void memset(uint64_t *dest, uint32_t val, uint32_t len);

#endif

