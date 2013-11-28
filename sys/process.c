#include <process.h>
#include <defs.h>
#include <stdio.h>
#include <paging.h>
#include <sys/gdt.h>

#define yield() __asm__ __volatile__("int $0x80")

//static int counter = 0;

void * get_kva(page *pp)
{
    	uint64_t ppn_addr= page2ppn(pp) << PGSHIFT;
    	//uint32_t __m_ppn = ppn_addr >> PTXSHIFT ;

    	uint64_t kva_addr = ppn_addr + KERNBASE; 
    	return (void *)kva_addr;
}

extern pml4e *pml4e_table;


void function1();
void function2();

void initThreads() {
	page   *pp1=NULL;
        page   *pp2=NULL;

        int i=0;
        // initialize both the task structures
        // set the address of function1 to the start of the stack of thread1 and same for thread2
        // set the rsp to point to the stack	

	pp1=page_alloc(0);
        pp2=page_alloc(0);

        uint64_t *pml4a=(uint64_t *)get_kva(pp1);
        uint64_t *pml4b=(uint64_t *)get_kva(pp2);


        for(i=0;i<512;i++) {
                pml4a[i] = 0;
                pml4b[i] = 0;
        }

        pml4a[511] = pml4e_table[511]; //point to pdpe of kernel
        pml4b[511] = pml4e_table[511]; //point to pdpe of kernel

        thread1.cr3 = (uint64_t)PADDR((uint64_t)pml4a);
        thread2.cr3 = (uint64_t)PADDR((uint64_t)pml4b);

        thread1.stack[59] = (uint64_t)&function1;
        thread1.rsp = (uint64_t)&thread1.stack[45];

        thread1.stack[63] = 0x23 ;                              //  Data Segment    
        thread1.stack[62] = (uint64_t)(kmalloc(4096) + (uint64_t)4096);      //  RIP
        //thread1.stack[61] = 0x20202 ;                           //  RFlags
        thread1.stack[61] = 0x246;                           //  EFlags
        thread1.stack[60] = 0x1b ;                              // Code Segment

        thread2.stack[59] = (uint64_t)&function2;
        thread2.rsp = (uint64_t)&thread2.stack[45];

        thread2.stack[63] = 0x23 ;                              //  Data Segment    
        thread2.stack[62] = (uint64_t)(kmalloc(4096) + (uint64_t)4096);      //  RIP
        //thread1.stack[61] = 0x20202 ;                           //  RFlags
        thread2.stack[61] = 0x246;                           //  EFlags
        thread2.stack[60] = 0x1b ;                              // Code Segment

        // inilialize the ready queue with both the task structures
        readyQ[0] = thread1;
        readyQ[1] = thread2;

	// initialize flags	
	firstFlag = true;
	flag = true;
}

void first_context_switch() 
{    
	initThreads();
    	
	firstFlag = false;
	
	// load the value of rsp of thread1 into the kernel rsp
    	// this will cause context switch

   	// __asm__("cli");
    	asm volatile("movq %0, %%cr3":: "a"(thread1.cr3));
   	// __asm__("sti");
        

    	printf("I am in process virtual address space \n");

    	__asm__ __volatile__ (
            "movq %0, %%rsp;" //load next's stack in rsp
            :
            :"r"(thread1.rsp)
    	);

	__asm__ __volatile__("popq %r15");
	__asm__ __volatile__("popq %r14");
	__asm__ __volatile__("popq %r13");
	__asm__ __volatile__("popq %r12");
	__asm__ __volatile__("popq %r11");
        __asm__ __volatile__("popq %r10");
        __asm__ __volatile__("popq %r9");
        __asm__ __volatile__("popq %r8");
        __asm__ __volatile__("popq %rdi");
        __asm__ __volatile__("popq %rsi");
        __asm__ __volatile__("popq %rdx");
        __asm__ __volatile__("popq %rcx");
        __asm__ __volatile__("popq %rbx");
        __asm__ __volatile__("popq %rax");
	__asm__ __volatile__("sti");	

	tss.rsp0 = (uint64_t)&thread1.stack[63];  

    	__asm__ __volatile__("mov $0x2b,%ax");
    	__asm__ __volatile__("ltr %ax");

    	__asm__ __volatile__("iretq");
}


void switch_to(task* prev, task* next) {
    	// will save the content prev task on the stack
    	// will update the value of the current rsp to the point to the rsp of the next task
    	// this will cause the context switch from prev task to next task
    	
	// code commented because the push operation is done from the timer interrupt handler
        /*			
	__asm__ __volatile__ (
        	"pushq %rax;"\
        	"pushq %rbx;"\
        	"pushq %rcx;"\
        	"pushq %rdx;"\
        	"pushq %rsi;"\
        	"pushq %rdi;"\
        	"pushq %r8;"\
            	"pushq %r9;"
            	"pushq %r10;"\
            	"pushq %r11;"
            	"pushq %r12;"
            	"pushq %r13;"
            	"pushq %r14;"
            	"pushq %r15;"
        );
	*/
    	__asm__ __volatile__(
            	"movq %%rsp, %0;"
            	:"=m"(prev->rsp)
            	:
            	:"memory"
        );
    	
	asm volatile("movq %0, %%cr3":: "a"(next->cr3));
    	
	__asm__ __volatile__ (
            	"movq %0, %%rsp;"
            	:
            	:"m"(next->rsp)
            	:"memory"
        );

	tss.rsp0 = (uint64_t)&next->stack[63];
        
	__asm__ __volatile__("popq %r15");
	__asm__ __volatile__("popq %r14");
        __asm__ __volatile__("popq %r13");
        __asm__ __volatile__("popq %r12");
        __asm__ __volatile__("popq %r11");
        __asm__ __volatile__("popq %r10");
        __asm__ __volatile__("popq %r9");
        __asm__ __volatile__("popq %r8");
        __asm__ __volatile__("popq %rdi");
        __asm__ __volatile__("popq %rsi");
        __asm__ __volatile__("popq %rdx");
        __asm__ __volatile__("popq %rcx");
        __asm__ __volatile__("popq %rbx");
        __asm__ __volatile__("popq %rax");
        __asm__ __volatile__("sti");  
        __asm__("iretq");
}

void schedule() {
    	// will halt the currently executing thread
    	// will pop the next task from the ready queue
    	// call switch_to function with prev and next task
        //printf("In schedule%s %s", firstFlag, flag);	
	if(firstFlag) {
		firstFlag = false;
		first_context_switch();
	}
	else {
		if(flag) {
                	flag = false;
                	switch_to(&readyQ[0], &readyQ[1]);
        	}
        	else {
                	flag = true;
                	switch_to(&readyQ[1], &readyQ[0]);
        	}
	}
}

void function1() {
	// to call the synthetic interrupts
		//int arg = 14;
	// to call the hardware originated divide by zero interrupt
		//__asm__("int %0\n" : : "N"((arg)) : "cc", "memory");
		//uint64_t *a = 0x0ul; int b = *a;
		//printf("%d",b);

	printf("\nHello");    		
	while(1) {
		static int i = 0;
        	printf("\nHello inside while: %d", i++);
        	//schedule();
        	//yield();
   	}
}

void function2() {
	printf("\nWorld..!!");
    	while(1) {
		static int i = 0;
        	printf("World..!! inside while: %d", i++);
        	//schedule();
        	//yield();
   	}
}
