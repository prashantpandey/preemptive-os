#include <paging.h>
#include <mmu.h>
#include <stdio.h>
#include <pic.h>
#include <defs.h>
#include <common.h>

uint32_t nframes;	// Number of physical pages
uint32_t npages_basemem = 0;
uint32_t npages_extmem = 0;
uint64_t global_nextfree;

extern char kernmem;
static page *page_free_list = NULL;
uint64_t end;
static void* lphysbase;
static void* lphysfree;
pml4e* pml4e_table;
uint64_t boot_cr3;

void mem_init();

/* Will create a free list of physical pages. */
void map_physical_address(uint32_t* modulep, void* physbase, void* physfree)
{
	struct smap_t {
                uint64_t base, length;
                uint32_t type;
        }__attribute__((packed)) *smap;
	
	lphysbase = physbase;
	lphysfree = physfree;
	end = KERNBASE + (uint64_t)physfree;
	
        while(modulep[0] != 0x9001) modulep += modulep[1]+2;
        for(smap = (struct smap_t*)(modulep+2); smap < (struct smap_t*)((char*)modulep+modulep[1]+2*4); ++smap) {
                if (smap->type == 1 /* memory */ && smap->length != 0) {
                        //printf("Available Physical Memory [%x-%x]\n", smap->base, smap->base + smap->length);
			if(smap->base == 0) {
                    		npages_basemem = smap->length/PGSIZE;
                    		//printf("\nLower Memory Pages = %d\n", npages_basemem);
                	}
                	else {
                   		npages_extmem = ((smap->base + smap->length) - (uint64_t)physfree)/PGSIZE;
                    		//printf("\nHigher Memory Pages = %d\n", npages_extmem);
                	}

			//if(nbase_mem == 0) {
			//	nbase_mem = PAGE_ALIGN(smap->base + smap->length, PGSIZE) / PGSIZE;
			//}
			//else {
			//	nroof_mem = PAGE_ALIGN(smap->base, PGSIZE) / PGSIZE;
			//}
                }
		else {
                        //printf("Not Available Physical Memory [%x-%x]\n", smap->base, smap->base + smap->length);
		}
        }
		
	//printf("%p %p %p\n", physfree, physbase, &kernmem);	
	// The physical pages are mapped from physfree - limit.
	nframes = npages_basemem + npages_extmem;
	
	//printf("\nNumber of pages %d, %d, %d", nframes, npages_basemem, npages_extmem);
	//printf("\nNBase %d and NRoof %d", nbase_mem, nroof_mem);
	
	mem_init();
}

/* Will allocate pages of memory above physfree when paging is not enabled. */
static void * boot_alloc(uint32_t n) 
{
	static uint64_t nextfree;	// virtual address of next byte of free memory
	uint64_t result = 0;

	if (!nextfree) {
           nextfree = PAGE_ROUNDOFF(end, PGSIZE);
           result = nextfree;
           nextfree += n;
        }
        else
        {
        	if(n == 0) {
                	result = nextfree;
		}
           	if(n > 0)
               	{
                	nextfree = PAGE_ROUNDOFF(nextfree, PGSIZE);
                	result = nextfree;
                	nextfree += n;
               	 	nextfree = PAGE_ROUNDOFF(nextfree, PGSIZE);
		}
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
	//printf("\nPage tables: %p nframe: %d size_page: %d", ((uint64_t)pages - (uint64_t)&kernmem), nframes, sizeof(page));
	//printf("\nGlobal Next Free: %p", ((uint64_t)global_nextfree - (uint64_t)&kernmem));
	memset((uint64_t *)pages, 0, (sizeof(page) * nframes));
	
	// Will allocate the Page free List
	uint32_t i;
	uint64_t pa;
	uint32_t cnt = 0;
	for (i = 0; i < nframes; i++) {
		if (i == 0) {						// To exclude memory for the BIOS/video
			pages[i].pp_ref = 1;
			pages[i].pp_link = NULL;
		} 
		else if(i < npages_basemem) {				// Free memory
			pages[i].pp_ref = 0;
			pages[i].pp_link = page_free_list;
			page_free_list = &pages[i];
			cnt++;
		} 
		else if ((i <= (EXTPHYSMEM / PGSIZE)) || (i < (((uint64_t)boot_alloc(0) - KERNBASE) >> PGSHIFT))) {
          		pages[i].pp_ref++;
          		pages[i].pp_link = NULL;
        	} 
		else {
          		pages[i].pp_ref = 0;
          		pages[i].pp_link = page_free_list;
          		page_free_list = &pages[i];
			cnt++;
        	}

		pa = page2pa(&pages[i]);    
        	if ((pa == 0 || pa == IOPHYSMEM) && (pages[i].pp_ref==0))
        		printf("page error: i %d\n", i);	
	}
	
	//printf("\ncount %d", cnt);
        page *temp = page_free_list;	
	for(i = 0; temp != NULL; temp = (page*)temp->pp_link, i++) {}
	//printf("\nLength of Free List: %d", i); 
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
		memset(page2kva((void*)pp), 0, PGSIZE);
	return pp;
}

uint64_t kmalloc(uint32_t size)
{
	uint32_t temp_size = size;
	int pages_required = 0;
	page* pp = NULL;
	uint64_t start_address = NULL;
	int i = 0;
	// Calculating the no of pages required corresponding to the size
	if (temp_size < PGSIZE)
		pages_required = 1;
	else
		{
		pages_required = temp_size/PGSIZE;
		temp_size -= (pages_required*PGSIZE);
		if (temp_size >0)
			pages_required++;
		}
	// Getting the pages allocated	
        if (!page_free_list)
                return NULL;
        
	pp = page_free_list;
	start_address =(uint64_t) pp;

	for(i = 0; i< pages_required; i++)
	{	
		pp->pp_ref++;
		memset(page2kva((void*)pp), 0, PGSIZE);
		pp = pp->pp_link;
	}
	page_free_list = pp;
	//printf("Starting address for %d pages is %x ", pages_required, start_address); 
	return start_address;
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
	pte* pte_ptr;
        page* pp;

        if (!pde_t[PDEX(va)] & PTE_P) {
            switch (create) {
                case 0 :
                        return NULL;
                case 1 :
                        if((pp = page_alloc(ALLOC_ZERO))!=NULL) {
                        	pde_t[PDEX(va)] = (pde)page2pa(pp) | PTE_P | PTE_W | PTE_U;

                        	if(PDEX(va) >= PDEX(KERNBASE))
                             		pde_t[PDEX(va)] = (pde)page2pa(pp) | PTE_P | PTE_W | PTE_U;
                        	pp->pp_ref++;
                        }
                        else {
                         	return NULL;
                        }
                        break;
                }
          }

       pte_ptr = KADDR(PTE_ADDR(pde_t[PDEX(va)]));

       pte_ptr = &pte_ptr[PTEX(va)];

       return pte_ptr;
}

/* PDPE walk */
pte* pdpe_walk(pdpe* pdpe_t, const void* va, int create)
{
      pde* pde_ptr;
      pte* pte_ptr;
      page* pp = NULL;

      if ((!pdpe_t[PDPEX(va)]) & PTE_P ){
            switch (create) {
                case 0 :
                        return NULL;
                case 1 :
                        if((pp = page_alloc(ALLOC_ZERO)) != NULL) {
                          pdpe_t[PDPEX(va)] = (pdpe)page2pa(pp) | PTE_P | PTE_U | PTE_W;
                           pp->pp_ref++;
                        }
                        else {
                          return NULL;
                        }
                        break;
                default :
                        break;
            }
         }

        pde_ptr = KADDR(PTE_ADDR(pdpe_t[PDPEX(va)]));

        if((pte_ptr = pde_walk(pde_ptr, va, create))!=NULL)
        	return pte_ptr;
        else {
		if(pp != NULL) {
               		pdpe_t[PDPEX(va)] = 0;
               		page_decref(pp);
            	}
            	return NULL;
        }
}

/* PML4E walk */
pte* pml4e_walk(pml4e* pml4e_t, const void* va, int create) 
{
	pdpe* pdpe_ptr;
    	pte*  pte_ptr;
    	page* pp = NULL;

    	if (!pml4e_t[PML4EX(va)] & PTE_P) {
            switch (create) {
                case 0 :
                        return NULL;
                case 1 :
                        if((pp = page_alloc(ALLOC_ZERO)) != NULL) {
                            pml4e_t[PML4EX(va)] = (pml4e)(page2pa(pp)) | PTE_P | PTE_U | PTE_W;
                            pp->pp_ref++;
                        }
                        else {
                           return NULL;
                        }
                        break;

                default :
                        break;
     		}
        }

        pdpe_ptr = KADDR(PTE_ADDR(pml4e_t[PML4EX(va)]));

        if((pte_ptr = pdpe_walk(pdpe_ptr, va, create)) != NULL)
            return pte_ptr;
        else {
             if(pp != NULL) {
                pml4e_t[PML4EX(va)] = 0;
                page_decref(pp);
             }
            return NULL;
        }	
}

/* To create the mapping from VA space to PA space for a given size */
void boot_map_region(pml4e* pml4e_t, uint64_t la, uint32_t size, uint64_t pa, int perm)
{
	pte* pte;	
	uint64_t va = PAGE_ROUNDOFF(la, PGSIZE);
	//uint32_t number_pages = (size/PGSIZE);
	int i = 0;

	for(i = 0; i < PAGE_ROUNDOFF(size, PGSIZE); i += PGSIZE)
	{
		pte = pml4e_walk(pml4e_t, (void *)(va+i), 1);
		if(!pte) {
			printf(" Null Boot map segment\n");
			return;
		}
		*pte = pa + i;
		*pte = *pte | (perm | PTE_P | PTE_U);
	}

}

/* Sets up the Kernel page dir and initialize paging. */
void mem_init() 
{
	// creating the paging structures
        pml4e_table = boot_alloc(PGSIZE);
        memset(pml4e_table, 0, PGSIZE);
		
	boot_cr3 = (uint64_t)PADDR((uint64_t)pml4e_table);

	// initialize the physical pages and free list
	page_init();
	//printf("After page init(page_free_list): %p", page_free_list);
		
	// map the kernel space
	boot_map_region(pml4e_table, KERNBASE + (uint64_t)lphysbase, MEM_LIMIT, (uint64_t)lphysbase, PTE_W | PTE_P | PTE_U);
		
	// map the BIOS/Video memory region	
	boot_map_region(pml4e_table, KERNBASE + (uint64_t)0xb8000, 4096, (uint64_t)0xb8000, PTE_W | PTE_P | PTE_U);
	
	//printf("\nBoot CR3: %p, %p", boot_cr3, pml4e_table[0x1ff]);
		
	__asm__ __volatile__("mov %0, %%cr3":: "b"(boot_cr3));
	//printf("Hello Pagination done.%p", boot_cr3);	
}

void invalidate_tlb(uint64_t addr) {
	__asm__ __volatile__("invlpg (%0)" ::"r" (addr) : "memory");
}

page* page_lookup(pml4e* pml4e_t, uint64_t va, pte** pte_t) {
	 pte* pte = pml4e_walk(pml4e_t, (void*)va, 0);
         if(pte == NULL) {
                 *pte_t = NULL;
                 return NULL;
         }
         else if(*pte != 0) {
                 if (pte_t != NULL)
                         *pte_t = pte;
                 return pa2page((uint64_t)(PTE_ADDR(*pte)));
         }
         return NULL;
}

void page_remove(pml4e* pml4e_t, uint64_t va) {
	pte* pte_t;
	page* pp = page_lookup(pml4e_t, va, &pte_t);
	if(pp) {
		if(pte_t) {
			*pte_t = 0;
			invalidate_tlb(va);
		}
		page_decref(pp);
	}
}

uint32_t page_insert(pml4e* pml4e_t, page* pp, uint64_t va, int perm) {
	pte* pte_t = pml4e_walk(pml4e_t, (void*)va, 1);
	if(pte_t == NULL) {
		return -1;
	}
	pp->pp_ref++;
	if(*pte_t & PTE_P) {
		page_remove(pml4e_t, va);
	}
	*pte_t = ((uint64_t)page2pa(pp)) | perm | PTE_P | PTE_U;
	return 0;
}

