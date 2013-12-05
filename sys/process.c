#include <process.h>
#include <defs.h>
#include <print.h>
#include <paging.h>
#include <sys/gdt.h>
#include <sys/tarfs.h>
#include <common.h>
#include <string.h>

#define yield() __asm__ __volatile__("int $0x90")

static void load_icode(task* t, char* filename, uint32_t size);

static int processCnt = 0;
static int ltick = 0;
static int timertick = 100;
static uint32_t ids = 0;
extern pml4e *pml4e_table;
extern uint64_t boot_cr3;
extern void irq0();

uint64_t reg[14]; // set of registers
uint64_t child_ret_addr;
uint64_t child_stack_addr;
uint64_t child_pid=0;

static task kernelProcess;
runQueue* currProcess;
volatile bool firstSwitch = true;
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
	processCnt--;
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
void returnToKernel(int status) {
	__asm__ __volatile__(                             // save the context of curr Process
        	"movq %%rsp, %0;"
                :"=m"(kernelProcess.rsp)
                :
                :"memory"
        );

	currProcess->process.state = status;
	runQueue* temp = currProcess;
        currProcess = fetchNextRunProcess();                        // move the curr Process to the next process in runQ
	removeProcess(temp);					// will remove the process from the runQueue	
	
        asm volatile("movq %0, %%cr3":: "a"(kernelProcess.cr3));

	firstSwitch = true;
	__asm__ __volatile__ (
		"popq %rbx;"\
		"popq %rax;"\
		"jmp *%rax;"
	);
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
        int i = 0;
        for(i = 0; i < 512; i++) {
                pml4e_p[i] = 0;
        }
        pml4e_p[511] = pml4e_table[511];                        // copy the kernel address space in process VM

        newPr->process.pid = ids++;
        newPr->process.ppid = 0;
        newPr->process.state = 0;
        newPr->process.sleep_time = 0;
        newPr->process.cr3 = (uint64_t)PADDR((uint64_t)pml4e_p);                // init cr3
        newPr->process.pml4e_p = pml4e_p;

        newPr->process.mm = (struct mm_struct *)((char *)(&newPr->process + 1));
        newPr->process.mm->count = 0;
        newPr->process.mm->mmap = NULL;

        newPr->process.stack = kmalloc_user((pml4e *)newPr->process.pml4e_p,512);

        // uint64_t* pte = pml4e_walk((pml4e *)newPr->process.pml4e_p, (char*)newPr->process.stack, 1);
        // kprintf("Page adress=%x-%x" ,pte ,*pte);

        __asm__ __volatile__("movq %0, %%cr3":: "a"(newPr->process.cr3));

        newPr->process.kstack[506] = 1; newPr->process.kstack[505] = 2;  newPr->process.kstack[504] = 3;  newPr->process.kstack[503] = 4;
        newPr->process.kstack[502] = 5; newPr->process.kstack[501] = 6;  newPr->process.kstack[500] = 7;  newPr->process.kstack[499] = 8;
        newPr->process.kstack[498] = 9; newPr->process.kstack[497] = 10; newPr->process.kstack[496] = 11; newPr->process.kstack[495] = 12;
        newPr->process.kstack[494] = 13; newPr->process.kstack[493] = 14; newPr->process.kstack[492] = 15;

        newPr->process.kstack[491] = (uint64_t)(&irq0+34);
        newPr->process.rsp = (uint64_t)&newPr->process.kstack[490];

        newPr->process.kstack[511] = 0x23 ;                              //  Data Segment
        newPr->process.kstack[510] = (uint64_t)(kmalloc(4096) + (uint64_t)4096);      //  RIP
	newPr->process.kstack[509] = 0x246;                           //  EFlags
        newPr->process.kstack[508] = 0x1b ;                              // Code Segment

        // load_icode(&(newPr->process), file, 0);
        newPr->process.kstack[507] = (uint64_t)function;

        lcr3(boot_cr3);

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
        processCnt++;
	
}

// Will create a circular linked list for process in the run queue
runQueue* createProcess(char* file) {
	runQueue* newPr = (runQueue *)get_kva(page_alloc(0));

	// create the process structure
	page* pp = page_alloc(0);
	uint64_t* pml4e_p = (uint64_t* )get_kva(pp);			// initiaize the memory str for process
	int i = 0;
	for(i = 0; i < 512; i++) {
		pml4e_p[i] = 0;	
	}
	pml4e_p[511] = pml4e_table[511];			// copy the kernel address space in process VM
	
	newPr->process.pid = ids++;
	newPr->process.ppid = 0;
	newPr->process.state = 0;
	newPr->process.sleep_time = 0;
	newPr->process.cr3 = (uint64_t)PADDR((uint64_t)pml4e_p);		// init cr3
	newPr->process.pml4e_p = pml4e_p;

	newPr->process.mm = (struct mm_struct *)((char *)(&newPr->process + 1));
	newPr->process.mm->count = 0;
    	newPr->process.mm->mmap = NULL;

    	newPr->process.stack = kmalloc_user((pml4e *)newPr->process.pml4e_p,512);

	// uint64_t* pte = pml4e_walk((pml4e *)newPr->process.pml4e_p, (char*)newPr->process.stack, 1);
    	// kprintf("Page adress=%x-%x" ,pte ,*pte);
	
	__asm__ __volatile__("movq %0, %%cr3":: "a"(newPr->process.cr3));

        newPr->process.kstack[506] = 1; newPr->process.kstack[505] = 2;  newPr->process.kstack[504] = 3;  newPr->process.kstack[503] = 4;
        newPr->process.kstack[502] = 5; newPr->process.kstack[501] = 6;  newPr->process.kstack[500] = 7;  newPr->process.kstack[499] = 8;
        newPr->process.kstack[498] = 9; newPr->process.kstack[497] = 10; newPr->process.kstack[496] = 11; newPr->process.kstack[495] = 12;
        newPr->process.kstack[494] = 13; newPr->process.kstack[493] = 14; newPr->process.kstack[492] = 15;

        newPr->process.kstack[491] = (uint64_t)(&irq0+34);
        newPr->process.rsp = (uint64_t)&newPr->process.kstack[490];

	newPr->process.kstack[511] = 0x23 ;                              //  Data Segment    
        newPr->process.kstack[510] = (uint64_t)(&newPr->process.stack[511]);      //  RIP
        newPr->process.kstack[509] = 0x246;                           //  EFlags
        newPr->process.kstack[508] = 0x1b ;                              // Code Segment
        
	load_icode(&(newPr->process), file, 0);	
        newPr->process.heap_vma->vm_end = newPr->process.heap_vma->vm_start;
	newPr->process.kstack[507] = (uint64_t)newPr->process.entry;

	lcr3(boot_cr3);	
	
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
	processCnt++;
	return newPr;
}

// Will switch to first process in the run Queue
void initContextSwitch(uint64_t* stack) {
	
	// create kernal
	kernelProcess.pid = ids++;
	kernelProcess.state = 0;
	kernelProcess.stack = (uint64_t *) stack;
	kernelProcess.cr3 = boot_cr3;
	kernelProcess.pml4e_p = pml4e_table;
		
	// create two user space process
	createProcess("bin/shell");
	// createProcess("bin/hello1");
	
	// createProcess("bin/hello2");
	// createProcess("bin/hello3");
	
	//createProcessLc(&function1);
	//createProcessLc(&function2);

	currProcess = runQ;		// init the current Process with head of runQ

	__asm__ __volatile__ ("mov $0x2b,%ax");
  	__asm__ __volatile__ ("ltr %ax");
	
	/*
	task pr = currProcess->process;
	__asm__ __volatile__("movq %0, %%cr3":: "a"(pr.cr3));		// load the cr3 reg with the cr3 of process
	// kprintf("I am in process virtual address space \n");
	__asm__ __volatile__ (						
            "movq %0, %%rsp;" //load next's stack in rsp
            :
            :"r"(pr.rsp)
    	);								
	// pop all the general purpose registers
        __asm__ __volatile__ (
                        "    popq %r15;"  \
                        "    popq %r14;"  \
                        "    popq %r13;"  \
                        "    popq %r12;"  \
                        "    popq %r11;"  \
                        "    popq %r10;" \
                        "    popq %r9;" \
                        "    popq %r8;" \
                        "    popq %rdi;" \
                        "    popq %rsi;" \
                        "    popq %rdx;" \
                        "    popq %rcx;" \
                        "    popq %rbx;" \
                        "    popq %rax;" 
              );
	// __asm__ __volatile__("sti");	

	tss.rsp0 = (uint64_t)&pr.stack[63];  
    	__asm__ __volatile__("mov $0x2b,%ax");
    	__asm__ __volatile__("ltr %ax");
    	__asm__ __volatile__("iretq");
	*/
}

// Will switch the process the processes in round robin manner using the circular
// run Queue
void switchProcess() {
	ltick++;
	if(ltick == timertick) {
		int i = 0;
		runQueue* temp = currProcess;
		for(i = 0; i < processCnt; i++) {
			if(temp->process.sleep_time > 0) {
				temp->process.sleep_time--;
			}
			temp = temp->next;
		}	
		if(firstSwitch) {
		        __asm__ __volatile__(
		                "movq %%rsp, %0;"
                		:"=m"(kernelProcess.rsp)
		                :   
               	 		:"memory"
		        );
			
		        __asm__ __volatile__ ("movq %0, %%cr3":: "a"(currProcess->process.cr3));
		        // kprintf("\nFirst context switch");

			__asm__ __volatile__ (
				"movq %0, %%rsp;"
			        :   
			        :"m"(currProcess->process.rsp)
                		:"memory"
        		);
			
			firstSwitch = false;
			tss.rsp0 =(uint64_t)(&(currProcess->process.kstack[511]));
			ltick = 0;
		}			
		else {
			volatile runQueue* prev = currProcess;
			// int old = currProcess->process.pid;
	                currProcess = fetchNextRunProcess();
			__asm__ __volatile__(
				"movq %%rsp, %0;"
                		:"=m"(prev->process.rsp)
		                :   
	               	 	:"memory"
			);
		        __asm__ __volatile__ ("movq %0, %%cr3":: "a"(currProcess->process.cr3));
			
			//kprintf("\nSecond context switch: %d", currProcess->process.pid);
			__asm__ __volatile__ (
				"movq %0, %%rsp;"
			        :   
			        :"m"(currProcess->process.rsp)
                		:"memory"
			);
			//kprintf("\nprev: %d next: %d", old, currProcess->process.pid);
                        tss.rsp0 =(uint64_t)(&(currProcess->process.kstack[511]));
			ltick = 0;			
		}
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
        	//yield();
   	}
}

void function2() {
	kprintf("\nWorld..!!");
    	while(1) {
		static int i = 0;
        	kprintf("\nfunction 2: %d", i++);
        	//switchProcess();
		//schedule();
        	// yield();
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
	volatile uint64_t va_start = ROUNDDOWN((uint64_t)va, PGSIZE);
    	volatile uint64_t va_end = ROUNDUP((uint64_t)va+len, PGSIZE);
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

struct vm_area_struct* malloc_vma(struct mm_struct* mm)
{
        struct vm_area_struct* vm_tail;
        char *tmp;
        if(mm->mmap == NULL)  // first vma allocate one page for it
        {
                tmp=(char *)kmalloc(1); // this will allocate one page so just pass size>0
                vm_tail = (struct vm_area_struct *)tmp;
                mm->mmap = vm_tail; 
                mm->count += 1;
                return (struct vm_area_struct *)tmp;
        }
        else
        {
                vm_tail = mm->mmap;
                while(vm_tail->vm_next != NULL)
                        vm_tail = vm_tail->vm_next;

                tmp = (char *)vm_tail + sizeof(struct vm_area_struct);  // go to the next vma in the same page (base +size)
                vm_tail->vm_next = (struct vm_area_struct *)tmp;
                mm->count += 1;
                return (struct vm_area_struct *)tmp;
   }
}

// Will load the user program header through tarfs
static void load_icode(task* t, char* filename, uint32_t size) {
	uint64_t offset = is_file_exists(filename);
	uint64_t start = 0;
	uint64_t end = 0;
	 
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
				
				struct vm_area_struct* vm = malloc_vma(t->mm);
                                vm->vm_start = ph->p_vaddr;
                                vm->vm_end = vm->vm_start + ph->p_memsz;
                                vm->vm_mmsz = ph->p_memsz;
                                vm->vm_next = NULL;
                                vm->vm_file =(uint64_t)elf;
                                vm->vm_flags = ph->p_flags;
                                vm->vm_offset = ph->p_offset; 
           			
				start = vm->vm_start;
				end = vm->vm_end;	
			}
        	}
 
        // Now map one page for the program's initial stack
        // at virtual address USTACKTOP - PGSIZE.
        //region_alloc(e, (void*) (USTACKTOP - PGSIZE), PGSIZE);
 
        // Magic to start executing environment at its address.
        	t->entry = elf->e_entry;

		t->heap_vma = (struct vm_area_struct *)kmalloc(1); // allocate a separate page for heap vma, however this is waste
                struct vm_area_struct* tmp = t->mm->mmap;
                while(tmp->vm_next != NULL)  {
                        tmp = tmp->vm_next;  // go to the last vma
                }

		uint64_t val = ALIGN_DOWN((uint64_t)(tmp->vm_end + 0x1000));
                t->heap_vma->vm_start = val;
		t->heap_vma->vm_end = val;  // start from next page (keep the distance)
                t->heap_vma->vm_mmsz = 0x1000;
		

                region_alloc(t, (void *)t->heap_vma->vm_start, t->heap_vma->vm_mmsz);
		t->mm->mmap->vm_start = start;
		t->mm->mmap->vm_end = end;
	}
}

int fork() {
	runQueue* parent = currProcess;
	runQueue* childPr = (runQueue *)get_kva(page_alloc(0));	
	if(!childPr) 
		return 0;

    	page  *pp1 = page_alloc(0);
        uint64_t* pml4e_p = (uint64_t* )get_kva(pp1);
	int i = 0;
        for(i = 0; i < 512; i++) {
                pml4e_p[i] = 0;
        }	
	pml4e_p[511] = pml4e_table[511];

    	page  *pp2 = page_alloc(0);
	
        childPr->process.mm = (struct mm_struct *)((char *)(&childPr->process + 1));
        childPr->process.mm->count = 0;
        childPr->process.mm->mmap = NULL;
	
        childPr->process.pid = ids++;
        childPr->process.ppid = parent->process.pid;
        childPr->process.state = 0;
        childPr->process.sleep_time = 0;
        childPr->process.cr3 = (uint64_t)PADDR((uint64_t)pml4e_p);                // init cr3
        childPr->process.pml4e_p = pml4e_p;
	
	child_pid = childPr->process.pid;

	uint64_t* tmpStack = (uint64_t *)get_kva(pp2);
	int j = 0;
	for(j = 0; j < 512; j++) {
		tmpStack[j] = parent->process.stack[j];
		// kprintf(" %x %x ",tmpStack[j], parent->process.stack[j]);
	}
	
	__asm__ __volatile__ ("movq %0, %%cr3":: "a"(childPr->process.cr3));
	
	childPr->process.stack = parent->process.stack;
        boot_map_region( (pml4e *)childPr->process.pml4e_p, (uint64_t)childPr->process.stack, 4096, (uint64_t)PADDR((uint64_t)tmpStack), PTE_W | PTE_P | PTE_U);
	
        childPr->process.kstack[511] = 0x23 ;                              //  Data Segment
        childPr->process.kstack[510] = parent->process.kstack[508];      //  RIP
        childPr->process.kstack[509] = 0x200286;                           //  EFlags
        childPr->process.kstack[508] = 0x1b ;                              // Code Segment
	childPr->process.kstack[507] = parent->process.kstack[505];

	childPr->process.kstack[491] = (uint64_t)(&irq0+34);
	childPr->process.kstack[490] = 16;
	
	childPr->process.kstack[506] = 0; //rax return to child           

    	childPr->process.kstack[505] = parent->process.kstack[503]; 
    	childPr->process.kstack[504] = parent->process.kstack[502]; 
    	childPr->process.kstack[503] = parent->process.kstack[501];  
    	childPr->process.kstack[502] = parent->process.kstack[500];  
    	childPr->process.kstack[501] = parent->process.kstack[499];   
    	childPr->process.kstack[500] = parent->process.kstack[498];   
    	childPr->process.kstack[499] = parent->process.kstack[497];  
    	childPr->process.kstack[498] = parent->process.kstack[496];  
    	childPr->process.kstack[497] = parent->process.kstack[495];  
    	childPr->process.kstack[496] = parent->process.kstack[494];  
    	childPr->process.kstack[495] = parent->process.kstack[493];  
    	childPr->process.kstack[494] = parent->process.kstack[492];  
    	childPr->process.kstack[493] = parent->process.kstack[491];  
    	childPr->process.kstack[492] = parent->process.kstack[490];

	childPr->process.rsp = (uint64_t)(&childPr->process.kstack[490]);
	
        struct vm_area_struct* parent_vm = parent->process.mm->mmap;
        struct vm_area_struct* child_vm;
	
	while(parent_vm != NULL) {
		child_vm = malloc_vma(childPr->process.mm);
	        child_vm->vm_start = parent_vm->vm_start;
        	child_vm->vm_end = parent_vm->vm_end;
        	child_vm->vm_mmsz = parent_vm->vm_mmsz;
        	child_vm->vm_next = NULL;
	        child_vm->vm_file = parent_vm->vm_file;
        	child_vm->vm_flags = parent_vm->vm_flags;
	        child_vm->vm_offset = parent_vm->vm_offset;
		
		region_alloc(&childPr->process, (void*) parent_vm->vm_start, parent_vm->vm_mmsz);
		int j = 0;
	        for(j=0; j<parent_vm->vm_mmsz; j++)
	        {
            		*((uint64_t *)(parent_vm->vm_start + j)) = *((uint64_t *)((uint64_t)parent_vm->vm_file + parent_vm->vm_offset + j));
        	}
        	parent_vm = parent_vm->vm_next;
	}
     	childPr->process.heap_vma = (struct vm_area_struct *)kmalloc(1);
    	struct vm_area_struct *tmp = childPr->process.mm->mmap;
	while(tmp->vm_next != NULL)  {
		tmp = tmp->vm_next;  // go to the last vma
	}
    	childPr->process.heap_vma->vm_start = childPr->process.heap_vma->vm_end = ALIGN_DOWN((uint64_t)(tmp->vm_end + 0x1000));  // start from next page (keep the distance)
    	childPr->process.heap_vma->vm_mmsz = 0x1000;
	region_alloc(&childPr->process, (void *)childPr->process.heap_vma->vm_start, childPr->process.heap_vma->vm_mmsz);
	asm volatile("movq %0, %%cr3":: "b"(parent->process.cr3));

	return child_pid;
}	

uint64_t execvpe(char* arg1, uint64_t arg2, uint64_t arg3) {
	
    	char* tmp_pathname = (char *)arg1;
    	kprintf("filename=%s\n", tmp_pathname);
   
    	char pathname[1024];
    	strcpy(pathname,tmp_pathname); 
	
	runQueue* pr = createProcess(pathname);

	pr->process.pid = currProcess->process.pid;
	pr->process.ppid = currProcess->process.pid;
	
	returnToKernel(0);
	return 0;
}

void displayProcess() {

	runQueue* temp = currProcess;
	runQueue* temp1 = temp->next;
	kprintf("\n PID: %d  PPID: %d", temp->process.pid, temp->process.ppid);
	while(temp1 != temp) {	
		kprintf("\n PID: %d  PPID: %d", temp->process.pid, temp->process.ppid);
		temp1 = temp1->next;
	}
}

void* malloc(uint32_t size) {
	uint64_t old = currProcess->process.heap_vma->vm_end;
    	currProcess->process.heap_vma->vm_end = currProcess->process.heap_vma->vm_end + size;   
    	// kprintf("Malloc %p\n",old);
	if(currProcess->process.heap_vma->vm_end - currProcess->process.heap_vma->vm_start > 0x1000)
    	{
        	kprintf("Can't allocate memory\n");
        	return NULL;
    	}
    	return (void *)old;
}

