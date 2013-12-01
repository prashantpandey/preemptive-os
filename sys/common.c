#include <defs.h>
#include <common.h>
#include <print.h>


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
void memcpy(void *dest, void *src, uint32_t n)
{
	const char *s;
        char *d;

        s = src;
        d = dest;
        if (s < d && s + n > d) {
                s += n;
                d += n;
                while (n-- > 0)
                        *--d = *--s;
        } else
                while (n-- > 0)
                        *d++ = *s++;

        //return dest;
}

void memset(void *dest, uint8_t val, uint32_t len)
{
    uint8_t *temp = (uint8_t *)dest;
    for ( ; len != 0; len--) *temp++ = val;
}
