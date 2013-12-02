#ifndef _PROCESS_H
#define _PROCESS_H

#include <defs.h>
#include <paging.h>

typedef struct {
	uint32_t pid;			// unique process id
	uint64_t stack[1024]; 		// process local stack
	uint64_t rsp;			// stack pointer
	uint64_t cr3;			// current value of CR3 register for process switch
	pml4e* pml4e_p;
	uint64_t entry;			// user funciton entry point	
	uint32_t state;			// 0: ready, 1: wait, 2: sleep
} task;

task thread1;
task thread2;

//task* prev;
//task* next;
//int num_process;
task* readyQ[5];

bool flag;
//bool firstFlag;

struct runQueue {
        task process;
        struct runQueue* next;
}__attribute__((packed));
typedef struct runQueue runQueue;

runQueue* runQ;


int getPid();
void initContextSwitch();
void switchProcess();
void * get_kva(page *pp);
void first_context_switch();
void addPagesMalloc(void* va, int num);

void schedule();
void switch_to(task* prev, task* next);
void initThreads();
void function1();
void function2();
#endif
