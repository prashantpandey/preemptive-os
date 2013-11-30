#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <defs.h>

#define SYSCALL_PROTO(n) static __inline uint64_t __syscall##n
#define T_SYSCALL   128


static __inline uint64_t __syscall0(uint64_t n) {
	uint64_t res;
   	__asm__ volatile ("int %1"\
        	:"=a"(res)\
                :"i"(T_SYSCALL),"0"(n)\
                :"cc","memory");
  	return res;
}

static __inline uint64_t __syscall1(uint64_t n, uint64_t a1) {
    	uint64_t res;
     	__asm__ volatile ("int %1"\
                   :"=a"(res)\
                   :"i"(T_SYSCALL),"0"(n),"b"((uint64_t)(a1))\
                   :"cc","memory");
  	return res;
}

static __inline uint64_t __syscall2(uint64_t n, uint64_t a1, uint64_t a2) {
    	uint64_t res;
    	__asm__ volatile ("int %1"\
                    :"=a"(res)\
                    :"i"(T_SYSCALL),"0"(n) ,"b"((uint64_t)(a1)),"c"((uint64_t)(a2))\
                    :"cc","memory");
    	return res;
}

static __inline uint64_t __syscall3(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3) {
    	uint64_t res;
    	__asm__ volatile ("int %1"\
                    :"=a"(res)\
                    :"i"(T_SYSCALL),"0"(n),"b"((uint64_t)(a1)),"c"((uint64_t)(a2)),"d"((uint64_t)(a3))\
                    :"cc","memory");
    	return res;
}

static __inline uint64_t __syscall4(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    	uint64_t res;
    	__asm__ volatile ("int %1"\
                    :"=a"(res)\
                    :"i"(T_SYSCALL),"0" (n),"b"((uint64_t)(a1)),"c"((uint64_t)(a2)),"d"((uint64_t)(a3)),"D"((uint64_t)(a4))\
                    :"cc","memory");
    	return res;
}

/*
SYSCALL_PROTO(0)(uint64_t n) {
	uint64_t res;
    	__asm__ volatile ("int %1"\
                :"=a"(res)\
                :"i"(T_SYSCALL),"0"(n)\
                :"cc","memory");
  	return res;
}

SYSCALL_PROTO(1)(uint64_t n, uint64_t a1) {
	    uint64_t res;
     __asm__ volatile ("int %1"\
                   :"=a"(res)\
                   :"i"(T_SYSCALL),"0"(n),"b"((uint64_t)(a1))\
                   :"cc","memory");
  return res;
}

SYSCALL_PROTO(2)(uint64_t n, uint64_t a1, uint64_t a2) {
	    uint64_t res;
    __asm__ volatile ("int %1"\
                    :"=a"(res)\
                    :"i"(T_SYSCALL),"0"(n) ,"b"((uint64_t)(a1)),"c"((uint64_t)(a2))\
                    :"cc","memory");
    return res;
}

SYSCALL_PROTO(3)(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3) {
	    uint64_t res;
    __asm__ volatile ("int %1"\
                    :"=a"(res)\
                    :"i"(T_SYSCALL),"0"(n),"b"((uint64_t)(a1)),"c"((uint64_t)(a2)),"d"((uint64_t)(a3))\
                    :"cc","memory");
    return res;
}

SYSCALL_PROTO(4)(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
	    uint64_t res;
    __asm__ volatile ("int %1"\
                    :"=a"(res)\
                    :"i"(T_SYSCALL),"0" (n),"b"((uint64_t)(a1)),"c"((uint64_t)(a2)),"d"((uint64_t)(a3)),"D"((uint64_t)(a4))\
                    :"cc","memory");
    return res;
}
*/
#endif
