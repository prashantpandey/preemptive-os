#include <paging.h>
#include <mmu.h>
#include <stdio.h>
#include <pic.h>
#include <defs.h>
#include <common.h>

uint32_t nframes;	// Number of physical pages
uint32_t nbase_mem = 0;
uint32_t nroof_mem = 0;
uint64_t global_nextfree;

uint64_t physfree;
extern char kernmem, physbase;
static page *page_free_list = NULL;

void mem_init();

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
	
	mem_init();
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
	if(n > 0) {
		global_nextfree = nextfree = (PAGE_ROUNDOFF(nextfree + n, PGSIZE));
	}
	return (void *)result;			
}

/*
  Sets up the environment, page directories etc and
  enables paging.
*/
void page_init()
{
	// Creating the page list for all the pages in 128 MB RAM
	pages = (page*) boot_alloc(sizeof(page) * nframes);
	printf("\nPage tables: %p nframe: %d size_page: %d", ((uint64_t)pages - (uint64_t)&kernmem), nframes, sizeof(page));
	printf("\nGlobal Next Free: %p", ((uint64_t)global_nextfree - (uint64_t)&kernmem));
	memset((uint64_t *)pages, 0, (sizeof(page) * nframes));
	
	// Will allocate the Page free List
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
			|| (i < (PAGE_ALIGN((uint64_t)boot_alloc(0) - (uint64_t)&kernmem, PGSIZE) / PGSIZE))){
			pages[i].pp_ref++;
			pages[i].pp_link = NULL;
		} else {						// Rest all the memory above the physfree + boot_alloc(nextfree)
			pages[i].pp_ref = 0;
			pages[i].pp_link = page_free_list;
			page_free_list = &pages[i];
		}
	}
	
	//printf("\npage_free_list: %p", page_free_list);
        page *temp = page_free_list;	
	for(i = 0; temp != NULL; temp = (page*)temp->pp_link, i++) {}
	printf("\nLength of Free List: %d", i); 
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
		memset(PADDR((uint64_t)pp), 0, PGSIZE);
	return pp;
}

// Return a page to the free list.
// (This function should only be called when pp->pp_ref reaches 0.)
//
void page_free(page *pp)
{
        if(pp->pp_ref == 0) {
		printf("\nThe page is already free..!!!");
		while(1);
	}
        pp->pp_link = page_free_list;
        page_free_list = pp;
}

// Decrement the reference count on a page,
// freeing it if there are no more refs.
//
void page_decref(page* pp)
{
        if (--pp->pp_ref == 0)
                page_free(pp);
}

/* PDE walk */
pte* pde_walk(pde* pde_t, const void* va, int create)
{
        pde* pde_e;
        pte* pte_t;
        page* pp;

        pde_e = &pde_t[PDEX(va)];
	*pde_e = 0x0;
	//printf("\npde index: %p", PDEX(va));
        if(*pde_e & PTE_P)
        {
                pte_t = (PADDR(PTE_ADDR(*pde_e)));
        }
        else
        {
                if(!create ||
                   !(pp = page_alloc(ALLOC_ZERO)) ||
                   !(pte_t = (pte*)pp))
                        return NULL;

                pp->pp_ref++;
                *pde_e = (uint64_t)PADDR((uint64_t)pte_t) | PTE_P | PTE_W | PTE_U;
        }

        return &pte_t[PTEX(va)];
}

/* PDPE walk */
pte* pdpe_walk(pdpe* pdpe_t, const void* va, int create)
{
        pdpe* pdpe_e;
        pde* pde_t;
        page* pp;

        pdpe_e = &pdpe_t[PDPEX(va)];
	//printf("\npdpe index: %p", PDPEX(va));
        if(*pdpe_e & PTE_P)
        {
                pde_t = (PADDR(PTE_ADDR(*pdpe_e)));
        }
        else
        {
                if(!create ||
                   !(pp = page_alloc(ALLOC_ZERO)) ||
                   !(pde_t = (pde*)pp))
                        return NULL;

                pp->pp_ref++;
                *pdpe_e = (uint64_t)PADDR((uint64_t)pde_t) | PTE_P | PTE_W | PTE_U;
        }

        return pde_walk(pde_t, va, create);
}

/* PML4E walk */
pte* pml4e_walk(pml4e* pml4e_t, const void* va, int create) 
{
	pml4e* pml4e_e;
	pdpe* pdpe_t;
	page* pp;

	pml4e_e = &pml4e_t[PML4EX(va)];
	//printf("\npml4e index: %p, page_free_list: %p", PML4EX(va), page_free_list);
	if(*pml4e_e & PTE_P) 
	{
		pdpe_t = (PADDR(PTE_ADDR(*pml4e_e)));
	}
	else
	{
		if(!create ||
                   !(pp = page_alloc(ALLOC_ZERO)) ||
                   !(pdpe_t = (pdpe*)pp))
                        return NULL;
                    
                pp->pp_ref++;
                *pml4e_e = (uint64_t)PADDR((uint64_t)pdpe_t) | PTE_P | PTE_W | PTE_U;
        }

        return pdpe_walk(pdpe_t, va, create);	
}

/* To create the mapping from VA space to PA space for a given size */
void boot_map_region(pml4e* pml4e_t, uint64_t va, uint32_t size, uint64_t pa, int perm)
{
	uint64_t va_next = va;
	uint64_t pa_next = pa;
	pte* pte;
	//uint64_t pa_check;
	
	uint32_t number_pages = size/PGSIZE;
	int i = 0;
	for(i = 0; i < number_pages; i++)
	{
		pte = pml4e_walk(pml4e_t, (void *)va_next, 1);
		if(!pte) {
			return;
		}
		*pte = (PTE_ADDR(pa_next) | perm | PTE_P);
                va_next += PGSIZE;
                pa_next += PGSIZE;
	}
}

/* Sets up the Kernel page dir and initialize paging. */
void mem_init() 
{
	//uint64_t cr0;
	//uint32_t n;
	
	// creating the paging structures
        pml4e_table = (pml4e*) boot_alloc(sizeof(pml4e));
        printf("\nKernel page directory level 1: %p", ((uint64_t)pml4e_table - (uint64_t)&kernmem));
        printf("\nGlobal Next Free: %p", ((uint64_t)global_nextfree - (uint64_t)&kernmem));
        memset((uint64_t *)pml4e_table, 0, (sizeof(pml4e)));
	
	// initialize the physical pages and free list
	page_init();
	printf("After page init(page_free_list): %p", page_free_list);
		
	// map the kernel space
	boot_map_region(pml4e_table, ((uint64_t)&kernmem), ((uint64_t)physfree - (uint64_t)&physbase), (uint64_t)&physbase, PTE_W);
	
	// map the BIOS/Video memory region	
	//boot_map_region(kern_pgdir, ((uint64_t)&kernmem + (uint64_t)&physbase), (physfree - (((uint64_t)&kernmem + (uint64_t)&physbase))), 0x0, PTE_W);
	
	//printf("\nDirectory Add: %d", pml4e_table[0x1ff]);	
	//lcr3((uint64_t)pml4e_table);
	
	// entry.S set the really important flags in cr0 (including enabling
        // paging).  Here we configure the rest of the flags that we care about.
        //cr0 = rcr0();
        //cr0 |= CR0_PE|CR0_PG|CR0_AM|CR0_WP|CR0_NE|CR0_MP;
        //cr0 &= ~(CR0_TS|CR0_EM);
        //lcr0(cr0);
	
	//TODO: write code to check for installed pages
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
