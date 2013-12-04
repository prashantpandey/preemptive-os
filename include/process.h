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

typedef struct mm_struct {
        struct vm_area_struct * mmap;           /* list of VMAs */
        struct vm_area_struct * mmap_cache;     /* last find_vma result */
        uint64_t free_area_cache                /* 1st address space hole*/

        pml4e* pml4e;
        int mm_users;                           /* Address space users */
        int mm_count;                           /* How many references to "struct mm_struct" (users count as 1) */
        int map_count;                          /* Number of VMAs */

        // struct list_head mmlist;                /* list of all mm_structs */

        uint64_t start_code;                    /* start address of code */
        uint64_t end_code;                      /* final address of code */
        uint64_t start_data;                    /* start address of data */
        uint64_t end_data;                      /* final address of data */
        uint64_t start_brk;                     /* start address of heap */
        uint64_t brk;                           /* final address of heap */
        uint64_t start_stack;                   /* start address of stack */
        uint64_t arg_start;                     /* start of arguments */
        uint64_t arg_end;                       /* end of arguments */
        uint64_t total_vm;                      /* Total pages mapped */

        #ifdef CONFIG_MM_OWNER
        /*
        * "owner" points to a task that is regarded as the canonical
        * user/owner of this mm. All of the following must be true in
        * order for it to be changed:
        *
        * current == mm->owner
        * current->mm != mm
        * new_owner->mm == mm
        * new_owner->alloc_lock is held
        */
       struct task* owner;
} mm_struct;

typedef struct vm_area_struct{
        struct mm_struct *vm_mm;                // Associated
        uint64_t vm_start;
        uint64_t vm_end;
        struct vm_area_struct *vm_next;
} vma;


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
