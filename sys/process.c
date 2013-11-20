#include <process.h>
#include <defs.h>
#include <stdio.h>
#include <paging.h>
#include <mmu.h>
#include <sys/gdt.h>

task thread1;
task thread2;

int num_process = 2;
task readyQ[5]; 

extern pml4e* pml4e_table;

bool flag = true;

void function1();
void function2();

void* get_kva(page *pp)
{
    uint64_t ppn_addr= page2ppn(pp) << PGSHIFT;
    //uint32_t __m_ppn = ppn_addr >> PTXSHIFT ;
    uint64_t kva_addr = ppn_addr + KERNBASE; 
    return (void *)kva_addr;
}

void first_context_switch() {
     	page   *pp1=NULL;
     	page   *pp2=NULL;

     	int i=0;
    	// initialize both the task structures
    	// set the address of function1 to the start of the stack of thread1 and same for thread2
    	// set the rsp to point to the stack

    	//int i = 0;
   	//  thread2.stack[i] = 0;
    	//}

    	pp1 = page_alloc(0);
    	pp2 = page_alloc(0);
   
  	//  print("\n%x", page2pa(pp1)); 
    	uint64_t *pml4a=(uint64_t *)get_kva(pp1);
    	uint64_t *pml4b=(uint64_t *)get_kva(pp2);
   	// uint64_t *pml4a=(uint64_t *)page2kva(pp1);
   	// uint64_t *pml4b=(uint64_t *)page2kva(pp2);


    	for(i=0;i<512;i++) {
        	pml4a[i] = 0;
        	pml4b[i] = 0;
    	}

    	pml4a[511] = pml4e_table[511]; //point to pdpe of kerne
    	pml4b[511] = pml4e_table[511]; //point to pdpe of kerne

    	thread1.cr3 = (uint64_t)PADDR((uint64_t)pml4a);
    	thread2.cr3 = (uint64_t)PADDR((uint64_t)pml4b);
	
	thread1.stack[59] = (uint64_t)&function1;	
	thread1.rsp = (uint64_t)&thread1.stack[55];		
		
	// code to enter ring3
	thread1.stack[63] = 0x23;     
        thread1.stack[62] = (uint64_t)&thread1.stack[63];
        thread1.stack[61] = 0x20202; 
 	thread1.stack[60] = 0x1B;   
	
	thread2.stack[59] = (uint64_t)&function2;	
	thread2.rsp = (uint64_t)&thread2.stack[55];		
	
	// inilialize the ready queue with both the task structures
	readyQ[0] = thread1;
	readyQ[1] = thread2;	

	// load the value of rsp of thread1 into the kernel rsp
	// this will cause context switch
    	__asm__("cli");
    	asm volatile("movq %0, %%cr3":: "a"(thread1.cr3));
   	__asm__("sti");
        

    	printf("I am in process virtual address space \n");	
	
	__asm__ __volatile__ (
        	"movq %0, %%rsp;" //load next's stack in rsp
        	:
		:"d"(thread1.rsp)
	);

    	__asm__ __volatile__( "popq %rdx"); 
    	__asm__ __volatile__( "popq %rcx"); 
    	__asm__ __volatile__( "popq %rbx");
    	__asm__ __volatile__( "popq %rax");
    	asm volatile("iretq");
}


void switch_to(task* prev, task* next) {
	// will save the content prev task on the stack
	// will update the value of the current rsp to the point to the rsp of the next task
	// this will cause the context switch from prev task to next task
	// TODO: write the assembly language code
	__asm__ __volatile__ (
			"pushq %rax;"\
			"pushq %rbx;"\
			"pushq %rcx;"\
			"pushq %rdx;"
		);
	__asm__ __volatile__(
			"movq %%rsp, %0;"
			:"=g"(prev->rsp)
		);
	__asm__ __volatile__ (
			"movq %0, %%rsp;"
			:
			:"r"(next->rsp)
		);
	__asm__ __volatile__(
			"popq %rdx;"\
			"popq %rcx;"\
			"popq %rbx;"\
			"popq %rax;"
	);	
    	asm volatile("iretq");
}

void schedule() {
	// will halt the currently executing thread
	// will pop the next task from the ready queue
	// call switch_to function with prev and next task
	
	if(flag) {
		flag = false;
		switch_to(&readyQ[0], &readyQ[1]);
	}
	else {
		flag = true;
		switch_to(&readyQ[1], &readyQ[0]);
	}
}

void function1() {
	while(1) {
		printf("\nHello ");
		//int a= 10; int b = 0;
		//int c = a/b;
		//printf("%d", c);
		schedule();
	}
}

void function2() {
	while(1) {
		printf("World..!!");
		schedule();
	}
}
