#ifndef _PROCESS_H
#define _PROCESS_H

#include <defs.h>
#include <paging.h>

typedef struct {
	uint32_t ppid;			// parent process id
	uint32_t pid;			// unique process id
	uint64_t stack[1024]; 		// process local stack
	uint64_t rsp;			// stack pointer
	uint64_t cr3;			// current value of CR3 register for process switch
	pml4e* pml4e_p;
	uint64_t entry;			// user funciton entry point	
	uint32_t state;			// 0: ready, 1: wait, 2: sleep
	uint64_t rip;			// instruction pointer
	int sleep_time;			
	
	struct vm_area_struct *heap_vma;  	// vma for heap
	uint64_t kstack[512];  
	struct mm_struct *mm; 		// a pointer to mm_struct
} task;


typedef struct vm_area_struct{
    struct mm_struct * vm_mm;       /* associated mm_struct */
    uint64_t vm_start;              /* VMA start, inclusive */
    uint64_t vm_end;                /* VMA end, exclusive */
    uint64_t vm_mmsz;               /* VMA size */
    unsigned long vm_flags;         /* flags */
    uint32_t grows_down;
    uint64_t vm_file;          /* mapped file, if any */
    struct vm_area_struct  *vm_next;/* list of VMA's */
    uint64_t vm_offset;    /* file offset */
}vma;

typedef struct mm_struct {
    int count;
    uint64_t * pt; // page table pointer
    unsigned long context;
    unsigned long start_code, end_code, start_data, end_data;
    unsigned long start_brk, brk, start_stack, start_mmap;
    unsigned long arg_start, arg_end, env_start, env_end;
    unsigned long rss, total_vm, locked_vm;
    unsigned long def_flags;
    struct vm_area_struct * mmap;
    struct vm_area_struct * mmap_avl;
}mm;

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
void returnToKernel();
void putProcessToWait();

void schedule();
void switch_to(task* prev, task* next);
void initThreads();
void function1();
void function2();
#endif
