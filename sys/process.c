#include <process.h>
#include <defs.h>
#include <print.h>
#include <paging.h>
#include <sys/gdt.h>
#include <sys/tarfs.h>
#include <common.h>

#define yield() __asm__ __volatile__("int $0x90")

static void load_icode(task* t, char* filename, uint32_t size);

static uint32_t ids = 0;
extern pml4e *pml4e_table;
extern uint64_t boot_cr3;

static task kernelProcess;
runQueue* currProcess;
bool firstSwitch = true;
bool stackAdj = false;

void function1();
void function2();

// Will return the pid of the process
int getPid() {
	return currProcess->process.pid;
}

// remove the process from runQ
void removeProcess(runQueue* currPr) {
	runQueue* temp = runQ;
	while(temp->next != currPr) {
		temp = temp->next;
	}
	temp->next = temp->next->next;
}

runQueue* fetchNextRunProcess() {
	runQueue* temp = currProcess;
	temp = temp->next;
	while(temp->process.state != 0) {
		temp = temp->next;
	}
	return temp;
}

void putProcessToWait() {
	currProcess->process.state = 1;
	switchProcess();
}

// Will return load the kernel process after the exit call
void returnToKernel() {
	__asm__ __volatile__(                             // save the context of curr Process
        	"movq %%rsp, %0;"
                :"=m"(kernelProcess.rsp)
                :
                :"memory"
        );

	runQueue* temp = currProcess;
        currProcess = fetchNextRunProcess();                        // move the curr Process to the next process in runQ
	
	removeProcess(temp);					// will remove the process from the runQueue	
	
        //task next = currProcess->process;
        asm volatile("movq %0, %%cr3":: "a"(kernelProcess.cr3));

       	__asm__ __volatile__ (
                "movq %0, %%rsp;"
                :
                :"m"(kernelProcess.rsp)
       		:"memory"
       );

       tss.rsp0 = (uint64_t)kernelProcess.stack[63];

       // stack adjustment
       if(stackAdj) {
                asm volatile("addq $0x08,%rsp");
                asm volatile("popq %rbx");
                asm volatile("popq %rbx");
                asm volatile("popq %rbp");
                asm volatile("popq %r12");
        	asm volatile("popq %r13");
        }
        stackAdj = true;

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
//        __asm__ __volatile__("sti");
	__asm__ __volatile__("iretq");
}

// To fetch the kernel virtual address
void* get_kva(page *pp)
{
    	uint64_t ppn_addr= page2ppn(pp) << PGSHIFT;
    	//uint32_t __m_ppn = ppn_addr >> PTXSHIFT ;

    	uint64_t kva_addr = ppn_addr + KERNBASE; 
    	return (void *)kva_addr;
}


// Will create a circular linked list for process in the run queue
void createProcessLc(void* function) {
        runQueue* newPr = (runQueue *)get_kva(page_alloc(0));

        // create the process structure
        page* pp = page_alloc(0);
        uint64_t* pml4e_p = (uint64_t* )get_kva(pp);                    // initiaize the memory str for process
        pml4e_p[511] = pml4e_table[511];                        // copy the kernel address space in process VM

        newPr->process.pid = ids++;
	newPr->process.state = 0;
        newPr->process.cr3 = (uint64_t)PADDR((uint64_t)pml4e_p);                // init cr3
        newPr->process.pml4e_p = pml4e_p;

        newPr->process.stack[59] = (uint64_t)function;
        newPr->process.rsp = (uint64_t)&newPr->process.stack[45];

        newPr->process.stack[63] = 0x23 ;                              //  Data Segment
        newPr->process.stack[62] = (uint64_t)(kmalloc(4096) + (uint64_t)4096);      //  RIP
        newPr->process.stack[61] = 0x246;                           //  EFlags
        newPr->process.stack[60] = 0x1b ;                              // Code Segment

        if(ids == 2) {
                runQ = newPr;
                runQ->next = runQ;
        }
        else {
                runQueue* temp = runQ;
                int i = 1;
                while(i < ids - 1) {
                        temp = temp->next;
                        i++;
                }
                temp->next = newPr;
                newPr->next = runQ;
        }
}

// Will create a circular linked list for process in the run queue
void createProcess(char* file) {
	runQueue* newPr = (runQueue *)get_kva(page_alloc(0));

	// create the process structure
	page* pp = page_alloc(0);
	uint64_t* pml4e_p = (uint64_t* )get_kva(pp);			// initiaize the memory str for process
	pml4e_p[511] = pml4e_table[511];			// copy the kernel address space in process VM
	
	newPr->process.pid = ids++;
	newPr->process.state = 0;
	newPr->process.cr3 = (uint64_t)PADDR((uint64_t)pml4e_p);		// init cr3
	newPr->process.pml4e_p = pml4e_p;

        load_icode(&(newPr->process), file, 0);	
	
	newPr->process.stack[59] = (uint64_t)newPr->process.entry;
        newPr->process.rsp = (uint64_t)&newPr->process.stack[45];

        newPr->process.stack[63] = 0x23 ;                              //  Data Segment    
        newPr->process.stack[62] = (uint64_t)(kmalloc(4096) + (uint64_t)4096);      //  RIP
        newPr->process.stack[61] = 0x246;                           //  EFlags
        newPr->process.stack[60] = 0x1b ;                              // Code Segment
		
	if(ids == 2) {
		runQ = newPr;
		runQ->next = runQ;
	}	
	else {
		runQueue* temp = runQ;
		int i = 1;
		while(i < ids - 1) {
			temp = temp->next;
			i++;
		}
		temp->next = newPr;
		newPr->next = runQ;
	}
}

// Will switch to first process in the run Queue
void initContextSwitch() {
	
	// create kernal
	kernelProcess.pid = ids++;
	kernelProcess.state = 0;
	kernelProcess.cr3 = boot_cr3;
	kernelProcess.pml4e_p = pml4e_table;
		
	// create two user space process
	// createProcess("bin/hello");
	// createProcess("bin/hello1");
	// createProcess("bin/hello2");
	// createProcess("bin/hello3");
	
	createProcessLc(&function1);
	createProcessLc(&function2);

	firstSwitch = false;		// unset the first context switch flag
	
	currProcess = runQ;		// init the current Process with head of runQ
	task pr = currProcess->process;

	__asm__ __volatile__("movq %0, %%cr3":: "a"(pr.cr3));		// load the cr3 reg with the cr3 of process
	// kprintf("I am in process virtual address space \n");
	
	__asm__ __volatile__ (						
            "movq %0, %%rsp;" //load next's stack in rsp
            :
            :"r"(pr.rsp)
    	);								
		
	// pop all the general purpose registers
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
//	__asm__ __volatile__("sti");	

	tss.rsp0 = (uint64_t)&pr.stack[63];  

    	__asm__ __volatile__("mov $0x2b,%ax");
    	__asm__ __volatile__("ltr %ax");

    	__asm__ __volatile__("iretq");
}

// Will switch the process the processes in round robin manner using the circular
// run Queue
void switchProcess() {
	if(firstSwitch) {
		initContextSwitch();		
	}
	else {
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
		__asm__ __volatile__(					// save the context of curr Process
            		"movq %%rsp, %0;"
            		:"=m"(currProcess->process.rsp)
            		:
            		:"memory"
        	);
    			
		currProcess = fetchNextRunProcess();			// move the curr Process to the next process in runQ
		
		//task next = currProcess->process;
		asm volatile("movq %0, %%cr3":: "a"(currProcess->process.cr3));	
    	
		__asm__ __volatile__ (
            		"movq %0, %%rsp;"
            		:
            		:"m"(currProcess->process.rsp)
            		:"memory"
        	);
		
		tss.rsp0 = (uint64_t)currProcess->process.stack[63];
        
		// stack adjustment
        	if(stackAdj) {
            		asm volatile("addq $0x08,%rsp");
          	  	asm volatile("popq %rbx");
            		asm volatile("popq %rbx");
            		asm volatile("popq %rbp");
            		asm volatile("popq %r12");
            		asm volatile("popq %r13");
        	}
        	stackAdj = true;
			
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
//        	__asm__ __volatile__("sti");  
        	__asm__ __volatile__("iretq");		
	}
}

void initThreads() {
	page* pp1 = NULL;
        page* pp2 = NULL;

        // initialize both the task structures
        // set the address of function1 to the start of the stack of thread1 and same for thread2
        // set the rsp to point to the stack	

	pp1 = page_alloc(0);
        pp2 = page_alloc(0);

        uint64_t *pml4a = (uint64_t *)get_kva(pp1);
        uint64_t *pml4b = (uint64_t *)get_kva(pp2);


        pml4a[511] = pml4e_table[511]; //point to pdpe of kernel
        pml4b[511] = pml4e_table[511]; //point to pdpe of kernel

        // inilialize 
	thread1.cr3 = (uint64_t)PADDR((uint64_t)pml4a);
	thread1.pml4e_p = (pml4e*)pml4a;
        thread2.cr3 = (uint64_t)PADDR((uint64_t)pml4b);	
	thread2.pml4e_p = (pml4e*)pml4b;

	load_icode(&thread1, "bin/hello", 0);
    	load_icode(&thread2, "bin/hello1", 0);
	
        thread1.stack[59] = (uint64_t)thread1.entry;
        thread1.rsp = (uint64_t)&thread1.stack[45];

        thread1.stack[63] = 0x23 ;                              //  Data Segment    
        thread1.stack[62] = (uint64_t)(kmalloc(4096) + (uint64_t)4096);      //  RIP
        //thread1.stack[61] = 0x20202 ;                           //  RFlags
        thread1.stack[61] = 0x246;                           //  EFlags
        thread1.stack[60] = 0x1b ;                              // Code Segment

        thread2.stack[59] = (uint64_t)thread2.entry;
        thread2.rsp = (uint64_t)&thread2.stack[45];

        thread2.stack[63] = 0x23 ;                              //  Data Segment    
        thread2.stack[62] = (uint64_t)(kmalloc(4096) + (uint64_t)4096);      //  RIP
        //thread1.stack[61] = 0x20202 ;                           //  RFlags
        thread2.stack[61] = 0x246;                           //  EFlags
        thread2.stack[60] = 0x1b ;                              // Code Segment

	// inilialize the ready queue with both the task structures
        readyQ[0] = &thread1;
        readyQ[1] = &thread2;

	// initialize flags	
	firstSwitch = true;
	flag = true;
}

void first_context_switch() 
{    
	initThreads();
    	
	firstSwitch = false;
	
	// load the value of rsp of thread1 into the kernel rsp
    	// this will cause context switch

   	// __asm__("cli");
    	asm volatile("movq %0, %%cr3":: "a"(thread1.cr3));
   	// __asm__("sti");
        

    	// kprintf("I am in process virtual address space \n");

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
	//__asm__ __volatile__("sti");	

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
//        __asm__ __volatile__("sti");  
        __asm__("iretq");
}

void schedule() {
    	// will halt the currently executing thread
    	// will pop the next task from the ready queue
    	// call switch_to function with prev and next task
        //kprintf("In schedule%s %s", firstSwitch, flag);	
	if(firstSwitch) {
		firstSwitch = false;
		first_context_switch();
	}
	else {
		if(flag) {
                	flag = false;
                	switch_to(readyQ[0], readyQ[1]);
        	}
        	else {
                	flag = true;
                	switch_to(readyQ[1], readyQ[0]);
        	}
	}
}

void function1() {
	// to call the synthetic interrupts
		//int arg = 14;
	// to call the hardware originated divide by zero interrupt
		//__asm__("int %0\n" : : "N"((arg)) : "cc", "memory");
		//uint64_t *a = 0x0ul; int b = *a;
		//kprintf("%d",b);

	kprintf("\nHello");    		
	while(1) {
		static int i = 0;
        	kprintf("\nfunction 1: %d", i++);
        	//switchProcess();
		//schedule();
        	yield();
   	}
}

void function2() {
	kprintf("\nWorld..!!");
    	while(1) {
		static int i = 0;
        	kprintf("\nfunction 2: %d", i++);
        	//switchProcess();
		//schedule();
        	yield();
   	}
}

// Add the given number of pages from va to the pml4e of the curr process 
void addPagesMalloc(void* va, int num) {
        //uint64_t v;
        //kprintf("Starting va %p\n Ending va %p",va_start,va_end);
        int i = 0;
	for(;i < num; i++, va += PGSIZE) {
        	//if (page_insert((pml4e *) currProcess->process.pml4e_p, (uint64_t)PADDR((uint64_t)va) ,(uint64_t) va, PTE_P | PTE_U | PTE_W) != 0) {
                // 	kprintf("Running out of memory\n");
                //}
        }
}

// Will allocate a region for the user program
static void region_alloc(task* t, void* va, uint32_t len) {
	uint64_t va_start = PAGE_ROUNDOFF((uint64_t)va, PGSIZE);
    	uint64_t va_end = PAGE_ROUNDOFF((uint64_t)va+len, PGSIZE);
    	page* p;
    	uint64_t v;
    	//kprintf("Starting va %p\n Ending va %p",va_start,va_end);
	for(v = va_start; v < va_end; v += PGSIZE) {
        	if (!(p = page_alloc(ALLOC_ZERO)))
            		kprintf("Running out of memory\n");
        	else {
            		if (page_insert((pml4e *) t->pml4e_p, p,(uint64_t) v, PTE_P | PTE_U | PTE_W) != 0)
                	kprintf("Running out of memory\n");
        	}
    	}
}

// Will load the user program header through tarfs
static void load_icode(task* t, char* filename, uint32_t size) {
	uint64_t offset = is_file_exists(filename);
 
    	if(offset == 0 || offset == 999) {
        	kprintf("\n Error. File not found in tarfs.");
        	offset = 0;
    	} 
	else {
        	Elf_hdr *elf = (Elf_hdr *) (&_binary_tarfs_start + offset);
        	Elf64_Phdr *ph, *eph;
        	ph = (Elf64_Phdr *) ((uint8_t *) elf + elf->e_phoff);
        	eph = ph + elf->e_phnum;
 
        	for(; ph < eph; ph++) {
            		if(ph->p_type == ELF_PROG_LOAD) {
                		if(ph->p_filesz > ph->p_memsz) {
                    			// kprintf("Wrong size in elf binary\n");
				}
        	        	// kprintf("\n vaddr : %x size %x", ph->p_vaddr, ph->p_memsz);
 	
        	        	region_alloc(t, (void*) ph->p_vaddr, ph->p_memsz);
				// kprintf("Allocated region for load_icode va");
                		// Switch to env's address space so that we can use memcpy
                		lcr3(t->cr3);
	                	memcpy((char*) ph->p_vaddr, (void *) elf + ph->p_offset, ph->p_filesz);
 	
        	        	if(ph->p_filesz < ph->p_memsz) {
                	    		memset((char*) ph->p_vaddr + ph->p_filesz, 0, ph->p_memsz - ph->p_filesz);
				}
 
                		// Switch back to kernel's address space
                		lcr3(boot_cr3);
            		}
        	}
 
        // Now map one page for the program's initial stack
        // at virtual address USTACKTOP - PGSIZE.
        //region_alloc(e, (void*) (USTACKTOP - PGSIZE), PGSIZE);
 
        // Magic to start executing environment at its address.
        	t->entry = elf->e_entry;
    	}
}

