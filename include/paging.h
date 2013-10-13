#ifndef PAGING_H
#define PAGING_H

#include <defs.h>
#include <stdio.h>

extern uint32_t k_npages;
extern uint32_t u_npages;
extern uint64_t physfree;

#define PAGESIZE 4096
#define MEM_LIMIT 0x7ffe000

#define PAGE_ROUNDOFF(physaddress, page_size) _page_roundoff(physaddress, page_size)

static inline uint64_t _page_roundoff(uint64_t physaddress, uint32_t page_size) 
{
	physaddress += (page_size - physaddress%page_size);
	return physaddress;
}

#define PAGE_ALIGN(physaddress, page_size) _page_align(physaddress, page_size) 

static inline uint64_t _page_align(uint64_t physaddress, uint32_t page_size)
{
	return physaddress -= physaddress%page_size;
}

#define K_PHYS_ADDRESS(_kva, k_lower_limit) _k_phys_address(_kva, k_lower_limit)

static inline uint64_t _k_phys_address(uint64_t kva, uint64_t k_lower_limit) 
{
	if(kva < k_lower_limit) 
	{
		printf("Invalid kernel virtual address: %p", kva);
	}
	return  (kva - k_lower_limit);
}

#define K_V_ADDRESS(kpa, k_lower_limit) _k_v_address(kpa, k_lower_limit)

static inline uint64_t _k_v_address(uint64_t kpa, uint64_t k_lower_limit)
{
	if(kpa > physfree)
	{
		printf("Invalid kernel physical address: %p", kpa);
	}
	return (kpa + k_lower_limit);
}

#define PAGE_OFF(la) (((uint64_t) (la)) & 0xFFF)

struct PML4E
{
        uint32_t present:1;   // Page present in memory
        uint32_t rw:1;   // Read-only if clear, readwrite if set
        uint32_t us:1;   // Supervisor level only if clear
        uint32_t pwt:1;
        uint32_t pcd:1;
        uint32_t accessed:1;   // Has the page been accessed since last refresh?
        uint32_t ign:1;
        uint32_t mbz:2;
        uint32_t avl:3;
        uint64_t pdpba:40;
        uint32_t available:10;
        uint32_t nx:1;
}__attribute__((packed));
typedef struct PML4E pml4e;

struct PDPE
{
        uint32_t present:1;   // Page present in memory
        uint32_t rw:1;   // Read-only if clear, readwrite if set
        uint32_t us:1;   // Supervisor level only if clear
        uint32_t pwt:1;
        uint32_t pcd:1;
        uint32_t accessed:1;   // Has the page been accessed since last refresh?
        uint32_t ign:1;
        uint32_t mbz:1;
        uint32_t avl:3;
        uint64_t pdba:40;
        uint32_t available:10;
        uint32_t nx:1;
}__attribute__((packed));
typedef struct PDPE pdpe;

struct PDE
{
        uint32_t present:1;   // Page present in memory
        uint32_t rw:1;   // Read-only if clear, readwrite if set
        uint32_t us:1;   // Supervisor level only if clear
        uint32_t pwt:1;
        uint32_t pcd:1;
        uint32_t accessed:1;   // Has the page been accessed since last refresh?
        uint32_t ign_1:1;
        uint32_t ign_2:1;
        uint32_t avl:3;
        uint64_t ptba:40;
        uint32_t available:10;
        uint32_t nx:1;
}__attribute__((packed));
typedef struct PDE pde;

struct PTE
{
        uint32_t present:1;   // Page present in memory
        uint32_t rw:1;   // Read-only if clear, readwrite if set
        uint32_t us:1;   // Supervisor level only if clear
        uint32_t pwt:1;
        uint32_t pcd:1;
        uint32_t accessed:1;   // Has the page been accessed since last refresh?
        uint32_t d:1;
	uint32_t pat:1;
        uint32_t g:1;
        uint32_t avl:3;
        uint64_t ppba:40;
        uint32_t available:10;
        uint32_t nx:1;
}__attribute__((packed));
typedef struct PTE pte;

// Declaration of the table structures
pml4e *pml4e_table;
pdpe *pdpe_table;
pde *pde_table;
pte *pte_table;

struct PAGE 
{
	struct PAGE *next_page;
	uint32_t pp_ref_count;
}__attribute__((packed));
typedef struct PAGE page;

//Declaration of page free list
page *pages;
static page *page_free_list;

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
  Retrieves a pointer to the page required.
  If make == 1, if the page-table in which this page should
  reside isn't created, create it!
**/
pte *get_page(uint64_t address, int make, pml4e *dir);

/**
  Handler for page faults.
**/
void page_fault();

void map_physical_address(uint32_t* modulep, uint64_t physfree);

#endif
