#ifndef PAGING_H
#define PAGING_H

#include <defs.h>
#include <isr.h>
#include <pic.h>

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
}__attribute__((packed);
typedef PML4E pml4e;

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
}__attribute__((packed);
typedef PDPE pdpe;


struct PDE
{
        uint32_t present:1;   // Page present in memory
        uint32_t rw:1;   // Read-only if clear, readwrite if set
        uint32_t us:1;   // Supervisor level only if clear
        uint32_t pwt:1;
        uint32_t pcd:1;
        uint32_t accessed:1;   // Has the page been accessed since last refresh?
        uint32_t d:1;
        uint32_t g:1;
        uint32_t avl:3;
	uint32_t pat:1;
	uint32_t mbz:8;
        uint64_t ptba:31;
        uint32_t available:10;
	uint32_t nx:1;
}__attribute__((packed);
typedef PDE pde;

struct PTE
{
        uint32_t present:1;   // Page present in memory
        uint32_t rw:1;   // Read-only if clear, readwrite if set
        uint32_t us:1;   // Supervisor level only if clear
        uint32_t pwt:1;
        uint32_t pcd:1;
        uint32_t accessed:1;   // Has the page been accessed since last refresh?
        uint32_t d:1;
        uint32_t g:1;
        uint32_t avl:3;
        uint64_t ppba:31;
        uint32_t available:10;
        uint32_t nx:1;
}__attribute__((packed);
typedef PTE pte;

// Declaration of the table structures
pml4e pml4e_table[512];
pdpe pdpe_table[512];
pde pde_table[512];
pte pte_table[512];

/**
  Sets up the environment, page directories etc and
  enables paging.
**/
void initialise_paging();

/**
  Causes the specified page directory to be loaded into the
  CR3 register.
**/
void switch_page_directory(page_directory_t *new);

/**			
  Retrieves a pointer to the page required.
  If make == 1, if the page-table in which this page should
  reside isn't created, create it!
**/
page_t *get_page(u32int address, int make, page_directory_t *dir);

/**
  Handler for page faults.
**/
void page_fault(registers_t regs);
