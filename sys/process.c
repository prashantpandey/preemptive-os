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

	int i = 0;
	for(i = 0; i < 1024; i++) {
		thread1.stack[i] = 0;
		thread2.stack[i] = 0;
	}
 
	thread1.stack[63] = (uint64_t)&function1;	
	thread1.rsp = (uint64_t)&thread1.stack[59];		
	
	thread2.stack[63] = (uint64_t)&function2;	
	thread2.rsp = (uint64_t)&thread2.stack[59];		
	
	// inilialize the ready queue with both the task structures
	readyQ[0] = thread1;
	readyQ[1] = thread2;	

	// load the value of rsp of thread1 into the kernel rsp
	// this will cause context switch
	
	__asm__ __volatile__ (
        	"movq %0, %%rsp;" //load next's stack in rsp
        	:
		:"d"(thread1.rsp)
	);

    	__asm__ __volatile__( "popq %rdx"); 
    	__asm__ __volatile__( "popq %rcx"); 
    	__asm__ __volatile__( "popq %rbx");
    	__asm__ __volatile__( "popq %rax");
    	asm volatile("retq");
}


void switch_to(task* prev, task* next) {
	// will save the content prev task on the stack
	// will update the value of the current rsp to the point to the rsp of the next task
	// this will cause the context switch from prev task to next task
	// TODO: write the assembly language code
	do {
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
    	asm volatile("retq");
	} while(0);
}

void schedule(int i) {
	// will halt the currently executing thread
	// will pop the next task from the ready queue
	// call switch_to function with prev and next task
	if(i == 0) {	
		switch_to(&readyQ[0], &readyQ[1]);
	}
	else if(i == 1) {
		switch_to(&readyQ[1], &readyQ[0]);
	}
}

void function1() {
	printf("Hello ");
	schedule(0);
}

void function2() {
	printf("World..!!\n");
	schedule(1);
}
