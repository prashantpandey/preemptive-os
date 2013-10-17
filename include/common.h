#ifndef _COMMON_H
#define _COMMON_H

//#include <unistd.h>
#include <defs.h>

void outb( unsigned short port, unsigned char val );
unsigned char inb( unsigned short port );
void memcpy(void* dest, void *src, uint32_t len);
void memset(void *dest, uint32_t val, uint32_t len);

static __inline void lcr0(uint64_t val) __attribute__((always_inline));
static __inline uint64_t rcr0(void) __attribute__((always_inline));
static __inline uint64_t rcr2(void) __attribute__((always_inline));
static __inline void lcr3(uint64_t val) __attribute__((always_inline));
static __inline uint64_t rcr3(void) __attribute__((always_inline));
static __inline void lcr4(uint64_t val) __attribute__((always_inline));
static __inline uint64_t rcr4(void) __attribute__((always_inline));

static __inline void
lcr0(uint64_t val)
{
	__asm __volatile("movl %0,%%cr0" : : "r" (val));
}

static __inline uint64_t
rcr0(void)
{
	uint64_t val;
	__asm __volatile("movl %%cr0,%0" : "=r" (val));
return val;
}

static __inline uint64_t
rcr2(void)
{
	uint64_t val;
	__asm __volatile("movl %%cr2,%0" : "=r" (val));
return val;
}

static __inline void
lcr3(uint64_t val)
{
	__asm __volatile("movl %0,%%cr3" : : "r" (val));
}

static __inline uint64_t
rcr3(void)
{
	uint64_t val;
	__asm __volatile("movl %%cr3,%0" : "=r" (val));
return val;
}

static __inline void
lcr4(uint64_t val)
{
	__asm __volatile("movl %0,%%cr4" : : "r" (val));
}

static __inline uint64_t
rcr4(void)
{
	uint64_t cr4;
	__asm __volatile("movl %%cr4,%0" : "=r" (cr4));
	return cr4;
}


#endif

