#ifndef _DEFS_H
#define _DEFS_H

#define NULL 0

typedef unsigned char __uint8_t;
typedef __uint8_t uint8_t;
typedef unsigned long __uint64_t;
typedef __uint64_t uint64_t;
typedef unsigned int __uint32_t;
typedef __uint32_t uint32_t;
typedef int __int32_t;
typedef __int32_t int32_t;
typedef unsigned short __uint16_t;
typedef __uint16_t uint16_t;

// Represents true-or-false values
typedef _Bool bool;
enum { false, true };

// Explicitly-sized versions of integer types

// Pointers and addresses are 64 bits long.
// We use pointer types to represent virtual addresses,
// uintptr_t to represent the numerical values of virtual addresses,
// and physaddr_t to represent physical addresses.

typedef uint64_t intptr_t;
typedef uint64_t uintptr_t;
typedef uint64_t physaddr_t;

// Page numbers are 64 bits long.

typedef uint64_t ppn_t;

// size_t is used for memory object sizes.
typedef uint64_t size_t;
// ssize_t is a signed version of ssize_t, used in case there might be an
// error return.
typedef uint64_t ssize_t;

// off_t is used for file offsets and lengths.
typedef uint32_t off_t;

// Efficient min and max operations
#define MIN(_a, _b)                                             \
({                                                              \
        typeof(_a) __a = (_a);                                  \
        typeof(_b) __b = (_b);                                  \
        __a <= __b ? __a : __b;                                 \
})
#define MAX(_a, _b)                                             \
({                                                              \
        typeof(_a) __a = (_a);                                  \
        typeof(_b) __b = (_b);                                  \
        __a >= __b ? __a : __b;                                 \
})

// Rounding operations (efficient when n is a power of 2)
// Round down to the nearest multiple of n
#define ROUNDDOWN(a, n)                                         \
({                                                              \
        uint64_t __a = (uint64_t) (a);                          \
        (typeof(a)) (__a - __a % (n));                          \
})
// Round up to the nearest multiple of n
#define ROUNDUP(a, n)                                           \
({                                                              \
        uint64_t __n = (uint64_t) (n);                          \
        (typeof(a)) (ROUNDDOWN((uint64_t) (a) + __n - 1, __n)); \
})

// Return the offset of 'member' relative to the beginning of a struct type
#define offsetof(type, member)  ((size_t) (&((type*)0)->member))

#endif
