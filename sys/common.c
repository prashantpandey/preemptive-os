#include <defs.h>
#include <common.h>


// Writing to the port
void outb( unsigned short port, unsigned char val )
{
    __asm__ __volatile__ ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

// Reading from the port
unsigned char inb( unsigned short port )
{
    unsigned char ret;
    __asm__ __volatile__ ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

// Copy len bytes from src to dest.
void memcpy(uint16_t *dest, const uint16_t *src, uint32_t len)
{
        const uint16_t *sp = (const uint16_t *)src;
        uint16_t *dp = (uint16_t *)dest;
        for(; len != 0; len--) *dp++ = *sp++;
}

// Write len copies of val into dest.
void memset(uint64_t *dest, uint32_t val, uint32_t len)
{
        uint16_t *temp = (uint16_t *)dest;
        for ( ; len != 0; len--) *temp++ = val;
}

