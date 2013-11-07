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
                        printf("Available Physical Memory [%x-%x]\n", smap->base, smap->base + smap->length);
			if(smap->base == 0) {
                    		npages_basemem = smap->length/PGSIZE;
                    		printf("\nLower Memory Pages = %d\n", npages_basemem);
                	}
                	else {
                   		npages_extmem = ((smap->base + smap->length) - (uint64_t)physfree)/PGSIZE;
                    		printf("\nHigher Memory Pages = %d\n", npages_extmem);
                	}

			//if(nbase_mem == 0) {
			//	nbase_mem = PAGE_ALIGN(smap->base + smap->length, PGSIZE) / PGSIZE;
			//}
			//else {
			//	nroof_mem = PAGE_ALIGN(smap->base, PGSIZE) / PGSIZE;
			//}
                }
		else {
                        printf("Not Available Physical Memory [%x-%x]\n", smap->base, smap->base + smap->length);
		}
        }
		
	printf("%p %p %p\n", physfree, physbase, &kernmem);	
	// The physical pages are mapped from physfree - limit.
	nframes = npages_basemem + npages_extmem;
	
	printf("\nNumber of pages %d, %d, %d", nframes, npages_basemem, npages_extmem);
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
	printf("\nPage tables: %p nframe: %d size_page: %d", ((uint64_t)pages - (uint64_t)&kernmem), nframes, sizeof(page));
	printf("\nGlobal Next Free: %p", ((uint64_t)global_nextfree - (uint64_t)&kernmem));
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
	
	printf("\ncount %d", cnt);
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
		memset(page2kva((void*)pp), 0, PGSIZE);
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
                             		pde_t[PDEX(va)] = (pde)page2pa(pp) | PTE_P | PTE_W ;
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
		*pte = *pte | (perm | PTE_P);
	}

}

/* Sets up the Kernel page dir and initialize paging. */
void mem_init() 
{
	//uint64_t cr0;
	//uint32_t n;
	
	// creating the paging structures
        pml4e* pml4e_table = boot_alloc(PGSIZE);
        memset(pml4e_table, 0, PGSIZE);
		
	uint64_t boot_cr3 = (uint64_t)PADDR((uint64_t)pml4e_table);

	// initialize the physical pages and free list
	page_init();
	//printf("After page init(page_free_list): %p", page_free_list);
		
	// map the kernel space
	boot_map_region(pml4e_table, KERNBASE + (uint64_t)lphysbase, ((uint64_t)lphysfree - (uint64_t)lphysbase), (uint64_t)lphysbase, PTE_W | PTE_P);
		
	// map the BIOS/Video memory region	
	boot_map_region(pml4e_table, KERNBASE + (uint64_t)0xb8000, 4096, (uint64_t)0xb8000, PTE_W | PTE_P);
	
	printf("\nBoot CR3: %p, %p", boot_cr3, pml4e_table[0x1ff]);
		
	//lcr3(PADDR((uint64_t)pml4e_table));
	//asm volatile("mov %0, %%cr3":: "b"(boot_cr3));
//	int success = 1;
	printf("Hello Pagination done.%p", boot_cr3);	

	// entry.S set the really important flags in cr0 (including enabling
        // paging).  Here we configure the rest of the flags that we care about.
        //cr0 = rcr0();
        //cr0 |= CR0_PE|CR0_PG|CR0_AM|CR0_WP|CR0_NE|CR0_MP;
        //cr0 &= ~(CR0_TS|CR0_EM);
        //lcr0(cr0);
	
	//TODO: write code to check for installed pages
}

