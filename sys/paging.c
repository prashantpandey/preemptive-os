#include <paging.h>
#include <mmu.h>
#include <stdio.h>
#include <pic.h>
#include <defs.h>
#include <common.h>

uint32_t nframes;	// Number of physical pages
uint32_t nbase_mem = 0;
uint32_t nroof_mem = 0;

uint64_t physfree;
extern char kernmem, physbase;
static page *page_free_list;


/* Will create a free list of physical pages. */
void map_physical_address(uint32_t* modulep, uint64_t physfree_limit)
{
	struct smap_t {
                uint64_t base, length;
                uint32_t type;
        }__attribute__((packed)) *smap;
        while(modulep[0] != 0x9001) modulep += modulep[1]+2;
        for(smap = (struct smap_t*)(modulep+2); smap < (struct smap_t*)((char*)modulep+modulep[1]+2*4); ++smap) {
                if (smap->type == 1 /* memory */ && smap->length != 0) {
                        printf("Available Physical Memory [%x-%x]\n", smap->base, smap->base + smap->length);
			if(nbase_mem == 0) {
				nbase_mem = PAGE_ALIGN(smap->base + smap->length, PGSIZE) / PGSIZE;
			}
			else {
				nroof_mem = PAGE_ALIGN(smap->base, PGSIZE) / PGSIZE;
			}
                }
		else {
                        printf("Not Available Physical Memory [%x-%x]\n", smap->base, smap->base + smap->length);
		}
        }
		
	physfree = physfree_limit;
	printf("%p %p %p\n", physfree, &physbase, &kernmem);

	// The physical pages are mapped from physfree - limit.
	uint64_t memseg = (MEM_LIMIT - 0x0);
	nframes = (memseg/0x1000);
	
	printf("\nNumber of pages from %p to %p: memseg: %p  is: %d", 0x0, MEM_LIMIT, memseg, nframes);
	printf("\nNBase %d and NRoof %d", nbase_mem, nroof_mem);
	initialise_paging();
}

/* Will allocate pages of memory above physfree when paging is not enabled. */
static void * boot_alloc(uint32_t n) 
{
	static uint64_t nextfree;	// virtual address of next byte of free memory
	uint64_t result;

	if (!nextfree) {
		result = nextfree = PAGE_ROUNDOFF((uint64_t)(physfree + &kernmem), PGSIZE);
	} else {
		result = nextfree;
	}
	nextfree = (PAGE_ROUNDOFF(nextfree + n, PGSIZE));
	
	return (void *)result;			
}

/*
  Sets up the environment, page directories etc and
  enables paging.
*/
void initialise_paging()
{
	// creating the paging structures
	pml4e_table = (pml4e*) boot_alloc(sizeof(pml4e));
	printf("\nKernel page directory level 1: %p", pml4e_table);
	memset((uint64_t *)pml4e_table, 0, (sizeof(pml4e)));	
/*
	pdpe_table = (pdpe*) boot_alloc(sizeof(pdpe));
	printf("\nKernel page directory level 2: %p", pdpe_table);
	memset((uint64_t *)pdpe_table, 0, (sizeof(pdpe)));	

	pde_table = (pde*) boot_alloc(sizeof(pde));
	printf("\nKernel page directory level 3: %p", pde_table);
	memset((uint64_t *)pde_table, 0, (sizeof(pde)));

	pte_table = (pte*) boot_alloc(sizeof(pte));
	printf("\nKernel page directory level 4: %p", pte_table);
	memset((uint64_t *)pte_table, 0, (sizeof(pte)));	
		
	pml4e_table[0].entry = PAGE_ALIGN((uint64_t)&pdpe_table, PGSIZE);
	pdpe_table[0].entry = PAGE_ALIGN((uint64_t)&pde_table, PGSIZE);
	pde_table[0].entry = PAGE_ALIGN((uint64_t)&pte_table, PGSIZE);
*/	
	pages = (page*) boot_alloc(sizeof(page) * nframes);
	printf("\nPage tables: %p nframe: %d size_page: %d", pages, nframes, sizeof(page));
	memset((uint64_t *)pages, 0, (sizeof(page) * nframes));
	
	uint32_t i;
	for (i = 0; i < nframes; i++) {
		if (i==0) {						// To exclude memory for the BIOS/video
			pages[i].pp_ref = 1;
			pages[i].pp_link = NULL;
		} else if(i < nbase_mem) {				// Free memory
			pages[i].pp_ref = 0;
			pages[i].pp_link = page_free_list;
			page_free_list = &pages[i];
		} else if ((i < nroof_mem)				// To exclude the hole, kernel space and the memory already allocated using bot_alloc
			|| (i < (PAGE_ALIGN((uint64_t)boot_alloc(0) - kernmem, PGSIZE)))){
			pages[i].pp_ref++;
			pages[i].pp_link = NULL;
		} else {						// Rest all the memory above the physfree + boot_alloc(nextfree)
			pages[i].pp_ref = 0;
			pages[i].pp_link = page_free_list;
			page_free_list = &pages[i];
		}
	} 
}

// Will allocate a page from the free list
page* page_alloc(int alloc_flags)
{
	page* pp = NULL;
	if (!page_free_list)
		return NULL;
	pp = page_free_list;
	page_free_list = page_free_list->pp_link;

	if (alloc_flags & ALLOC_ZERO)
		memset(page2kva(pp), 0, PGSIZE);
	return pp;
}

// Return a page to the free list.
// (This function should only be called when pp->pp_ref reaches 0.)
//
void page_free(page *pp)
{
        if(pp->pp_ref == 0) {
		printf("The page is already free..!!!");
		while(1);
	}
        pp->pp_link = page_free_list;
        page_free_list = pp;
}

// Decrement the reference count on a page,
// freeing it if there are no more refs.
//
void page_decref(struct Page* pp)
{
        if (--pp->pp_ref == 0)
                page_free(pp);
}

/**
  Causes the specified page directory to be loaded into the
  CR3 register.
**
void switch_page_directory(pml4e *new_page_dir)
{
}

**                     
  Retrieves a pointer to the page required.
  If make == 1, if the page-table in which this page should
  reside isn't created, create it!
**
pte *get_page(uint64_t address, int make, pml4e *dir)
{
}

**
  Handler for page faults.
**
void page_fault()
{
}
*/

/*  
void initialise_paging()
{
   // The size of physical memory. For the moment we
   // assume it is 16MB big.
   uint64_t mem_end_page = 0x1000000;

   nframes = mem_end_page / 0x1000;
   frames = (u32int*)kmalloc(INDEX_FROM_BIT(nframes));
   memset(frames, 0, INDEX_FROM_BIT(nframes));

   // Let's make a page directory.
   kernel_directory = (page_directory_t*)kmalloc_a(sizeof(page_directory_t));
   memset(kernel_directory, 0, sizeof(page_directory_t));
   current_directory = kernel_directory;

   // We need to identity map (phys addr = virt addr) from
   // 0x0 to the end of used memory, so we can access this
   // transparently, as if paging wasn't enabled.
   // NOTE that we use a while loop here deliberately.
   // inside the loop body we actually change placement_address
   // by calling kmalloc(). A while loop causes this to be
   // computed on-the-fly rather than once at the start.
   int i = 0;
   while (i < placement_address)
   {
       // Kernel code is readable but not writeable from userspace.
       alloc_frame( get_page(i, 1, kernel_directory), 0, 0);
       i += 0x1000;
   }
   // Before we enable paging, we must register our page fault handler.
   register_interrupt_handler(14, page_fault);

   // Now, enable paging!
   switch_page_directory(kernel_directory);
}

*
void switch_page_directory(pml4e *dir)
{
   current_directory = dir;
   asm volatile("mov %0, %%cr3":: "r"(&dir->tablesPhysical));
   uint64_t cr0;
   asm volatile("mov %%cr0, %0": "=r"(cr0));
   cr0 |= 0x80000000; // Enable paging!
   asm volatile("mov %0, %%cr0":: "r"(cr0));
}

*
page_t *get_page(u32int address, int make, page_directory_t *dir)
{
   // Turn the address into an index.
   address /= 0x1000;
   // Find the page table containing this address.
   u32int table_idx = address / 1024;
   if (dir->tables[table_idx]) // If this table is already assigned
   {
       return &dir->tables[table_idx]->pages[address%1024];
   }
   else if(make)
   {
       u32int tmp;
       dir->tables[table_idx] = (page_table_t*)kmalloc_ap(sizeof(page_table_t), &tmp);
       memset(dir->tables[table_idx], 0, 0x1000);
       dir->tablesPhysical[table_idx] = tmp | 0x7; // PRESENT, RW, US.
       return &dir->tables[table_idx]->pages[address%1024];
   }
   else
   {
       return 0;
   }
} */
