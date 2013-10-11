#include <paging.h>
#include <stdio.h>
#include <pic.h>
#include <defs.h>
#include <common.h>

uint64_t *frames;
uint32_t nframes;

// Defined in kheap.c
extern uint64_t placement_address;

// Macros used in the bitset algorithms.
#define INDEX_FROM_BIT(a) (a/(8*8))
#define OFFSET_FROM_BIT(a) (a%(8*8))

// Static function to set a bit in the frames bitset
void set_frame(uint64_t frame_addr)
{
   	uint64_t frame = frame_addr/0x1000;
   	uint64_t idx = INDEX_FROM_BIT(frame);
   	uint32_t off = OFFSET_FROM_BIT(frame);
   	frames[idx] |= (0x1 << off);
}

// Static function to clear a bit in the frames bitset
void clear_frame(uint64_t frame_addr)
{
   	uint64_t frame = frame_addr/0x1000;
   	uint64_t idx = INDEX_FROM_BIT(frame);
   	uint32_t off = OFFSET_FROM_BIT(frame);
   	frames[idx] &= ~(0x1 << off);
}

/*
// Static function to test if a bit is set.
static uint64_t test_frame(uint64_t frame_addr)
{
	uint64_t frame = frame_addr/0x1000;
	uint64_t idx = INDEX_FROM_BIT(frame);
	uint32_t off = OFFSET_FROM_BIT(frame);
   	return (frames[idx] & (0x1 << off));
}

// Static function to find the first free frame.
static uint64_t first_frame()
{
   uint32_t i, j;
   for (i = 0; i < INDEX_FROM_BIT(nframes); i++)
   {
       if (frames[i] != 0xFFFFFFFF) // nothing free, exit early.
       {
           // at least one bit is free here.
           for (j = 0; j < 64; j++)
           {
               uint64_t toTest = 0x1 << j;
               if ( !(frames[i]&toTest) )
               {
                   return i*8*8+j;
               }
           }
       }
   }
}
*/

void map_physical_address(uint32_t* modulep, uint64_t physbase, uint64_t physfree)
{
	struct smap_t {
                uint64_t base, length;
                uint32_t type;
        }__attribute__((packed)) *smap;
        while(modulep[0] != 0x9001) modulep += modulep[1]+2;
        for(smap = (struct smap_t*)(modulep+2); smap < (struct smap_t*)((char*)modulep+modulep[1]+2*4); ++smap) {
                if (smap->type == 1 /* memory */ && smap->length != 0) {
                        printf("Available Physical Memory [%x-%x]\n", smap->base, smap->base + smap->length);
			
                }
        }
	
	int limit = 0x7ffe000;
		
	printf("%p\n", physfree);
	//rounding off the physfree to 4KB page size
	physfree = (physfree/4096);
	physfree = physfree + 4096;

	uint64_t memseg = (limit - physfree);
	nframes = (memseg/0x1000);
	memset(frames, 0, INDEX_FROM_BIT(nframes));	
	
	printf("Number of pages from %p to %p: memseg: %p  is: %d", physfree, limit, memseg, nframes);
}

/*
**
  Sets up the environment, page directories etc and
  enables paging.
**
void initialise_paging()
{
}

**
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

void switch_page_directory(page_directory_t *dir)
{
   current_directory = dir;
   asm volatile("mov %0, %%cr3":: "r"(&dir->tablesPhysical));
   uint64_t cr0;
   asm volatile("mov %%cr0, %0": "=r"(cr0));
   cr0 |= 0x80000000; // Enable paging!
   asm volatile("mov %0, %%cr0":: "r"(cr0));
}

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
