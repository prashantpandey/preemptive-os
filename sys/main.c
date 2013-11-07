/* Main file */
/* With boot and start functions */

#include <defs.h>
#include <stdarg.h>
#include <sys/gdt.h>
#include <stdio.h>
#include <idt.h>
#include <pic.h>
#include <timer.h>
#include <common.h>
#include <paging.h>

void start(uint32_t* modulep, void* physbase, void* physfree)
{
	/*
	struct smap_t {
		uint64_t base, length;
		uint32_t type;
	}__attribute__((packed)) *smap;
	while(modulep[0] != 0x9001) modulep += modulep[1]+2;
	for(smap = (struct smap_t*)(modulep+2); smap < (struct smap_t*)((char*)modulep+modulep[1]+2*4); ++smap) {
		if (smap->type == 1 * memory * && smap->length != 0) {
			printf("Available Physical Memory [%x-%x]\n", smap->base, smap->base + smap->length);
		}
	}
	*/
	

	map_physical_address(modulep, physbase, physfree);
	// kernel starts here
	while(1);
}

#define INITIAL_STACK_SIZE 4096
char stack[INITIAL_STACK_SIZE];
uint32_t* loader_stack;
extern char kernmem, physbase;

void boot(void)
{
	// note: function changes rsp, local stack variables can't be practically used
	//register char *temp1, *temp2;
	__asm__(
		"movq %%rsp, %0;"
		"movq %1, %%rsp;"
		:"=g"(loader_stack)
		:"r"(&stack[INITIAL_STACK_SIZE])
	);

// Will reload the gdt
	reload_gdt();

	setup_tss();

// Will initialize the IDT
//        init_idt();

// Will initialize the PIC and remap the interrupt number 0-15 to 32-47
//        pic_remap(0x20, 0x28);

// Will initialize the timer interrupt
//        init_timer(100);

// code snippet to invoke an interrupt based upon arg number which is the interrupt number
//      int arg = 0;
//        __asm__("int %0\n" : : "N"((arg)) : "cc", "memory");

       	clear_screen();
       	start(
        	(uint32_t*)((char*)(uint64_t)loader_stack[3] + (uint64_t)&kernmem - (uint64_t)&physbase),
               	&physbase,
               	(void*)(uint64_t)loader_stack[4]
       	);

// code snippet to invoke the divide-by-zero interrupt
//      int a = 5;
//      int b = 5;
//      printf("%d", (a/(a-b)));

//      int a = 32;
//      printf("%p ", &a);
//      int *b = &a;
//      printf(" %d", *b);

        while(1);
}
