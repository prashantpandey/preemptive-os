#ifndef PAGING_H
#define PAGING_H

#include <defs.h>
#include <print.h>
#include <mmu.h>

extern uint32_t nframes;
extern uint64_t physfree;
extern char kernmem;

#define ALLOC_ZERO	1<<0
#define MEM_LIMIT 	0x7ffe000
#define KERNBASE	0xffffffff80000000

#define IOPHYSMEM       0x09fc00
#define EXTPHYSMEM      0x100000

#define PAGE_ROUNDOFF(physaddress, page_size) _page_roundoff(physaddress, page_size)

static inline uint64_t _page_roundoff(uint64_t physaddress, uint32_t page_size) 
{
	if(physaddress%page_size != 0) {
		physaddress += (page_size - physaddress%page_size);
	}
	return physaddress;
}

#define PAGE_ALIGN(physaddress, page_size) _page_align(physaddress, page_size) 

static inline uint64_t _page_align(uint64_t physaddress, uint32_t page_size)
{
	return physaddress -= physaddress%page_size;
}


/* page table structures */
typedef uint64_t pml4e;

typedef uint64_t pdpe;

typedef uint64_t pde;

typedef uint64_t pte;

// Declaration of the table structures
//pml4e *pml4e_table;
//pdpe *pdpe_table;
//pde *pde_table;
//pte *pte_table;

struct PAGE 
{
	struct PAGE *pp_link;
	uint16_t pp_ref;
}__attribute__((packed));
typedef struct PAGE page;

//Declaration of page free list
page *pages;


/* This macro takes a kernel virtual address -- an address that points above
 * KERNBASE, where the machine's maximum 256MB of physical memory is mapped --
 * and returns the corresponding physical address. It panics if you pass it a
 * non-kernel virtual address.
*/
#define PADDR(kva) _paddr(kva)

static inline void* _paddr(uint64_t kva)
{
	if ((uint64_t)kva < (uint64_t)KERNBASE)
		kprintf("PADDR called with invalid kva %p", kva);
	return (void*)(kva - (uint64_t)KERNBASE);
}

/* This macro takes a physical address and returns the corresponding kernel
 * virtual address. It panics if you pass an invalid physical address. 
*/
#define KADDR(pa) _kaddr(pa)

static inline void* _kaddr(uint64_t pa)
{
	if (PGNUM(pa) >= nframes)
		kprintf("KADDR called with invalid pa %p", pa);
	return (void*)(pa + (uint64_t)KERNBASE);
}


#define PAGE_OFF(la) (((uint64_t) (la)) & 0xFFF)

static inline uint32_t
page2ppn(page *pp)
{
        return pp - pages;
}

// page to memory utility functions
static inline uint64_t page2pa(page *pp)
{
        return (page2ppn(pp)) << PGSHIFT;
}

static inline page* pa2page(uint64_t pa)
{
        if (PGNUM(pa) >= nframes)
                kprintf("pa2page called with invalid pa");
        return &pages[PGNUM(pa)];
}

static inline void* page2kva(page *pp)
{
        return KADDR(page2pa(pp));
}


/**
  Sets up the environment, page directories etc and
  enables paging.
**/
void initialise_paging();

/**
  Causes the specified page directory to be loaded into the
  CR3 register.
**/
void switch_page_directory(pml4e *new);

/**
  Handler for page faults.
**/
void page_fault();

void map_physical_address(uint32_t* modulep, void* physbase, void* physfree);

page* page_alloc(int alloc_flags);
uint32_t page_insert(pml4e* pml4e_t, page* pp, uint64_t va, int perm);
uint64_t kmalloc(uint32_t size);

#endif
