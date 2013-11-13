#include <process.h>
#include <defs.h>
#include <stdio.h>
#include <paging.h>


task readyQ[10];

task thread1;
task thread2;

int num_process = 2;

void function1();
void function2();

void first_context_switch() {
	// initialize both the task structures
	// set the address of function1 to the start of the stack of thread1 and same for thread2
	// set the rsp to point to the stack 
	thread1.stack[0] = (uint64_t)&function1;	
	thread1.rsp = (uint64_t)&thread1.stack;		
	
	thread2.stack[0] = (uint64_t)&function2;	
	thread2.rsp = (uint64_t)&thread2.stack;		
	
	// inilialize the ready queue with both the task structures
	readyQ[0] = thread1;
	readyQ[1] = thread2;	

	uint64_t dummy_prev = 0;
	
	// load the value of rsp of thread1 into the kernel rsp
	// this will cause context switch
	
	__asm__ __volatile__( 
        	"movq %%rsp, %0;" //save rsp in prev's stack 
        	"movq %1, %%rsp;" //load next's stack in rsp
        	:"=g"(dummy_prev) 
        	:"d"(thread1.rsp)
        	: "memory" 
    	); 
 
    	//__asm__ __volatile__( "popq %rdx"); 
    	//__asm__ __volatile__( "popq %rcx"); 
    	//__asm__ __volatile__( "popq %rbx");
    	//__asm__ __volatile__( "popq %rax");
    	asm volatile("retq");
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
			"movq %1, %%rsp;"
			:"=g"(prev->rsp)
			:"d"(next->rsp)
			:"memory"
		);
	__asm__ __volatile__(
			"popq %rdx;"\
			"popq %rcx;"\
			"popq %rbx;"\
			"popq %rax;"
	);
}

void schedule() {
	// will halt the currently executing thread
	// will pop the next task from the ready queue
	// call switch_to function with prev and next task
	task prev;
	task next;

	static int i = 0;
	prev = readyQ[i];
	i = (i + 1) % num_process;
	next = readyQ[i];

	switch_to(&prev, &next);
}

void function1() {
	printf("Hello ");
	schedule();
}

void function2() {
	printf("World..!!\n");
	schedule();
}
