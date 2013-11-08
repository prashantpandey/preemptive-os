#include <paging.h>
#include <mmu.h>
#include <stdio.h>
#include <pic.h>
#include <defs.h>
#include <common.h>

// rax: pointer to me
// rbx: pointer to last
// rcx: pointer to next


// from mike's slide
/*
switch_to()
{
__asm_volatile 
push rax // ptr to me on my stack //
push rbx // ptr to local last (&last) //
mov rsp,rax(10) // save my stack ptr //
mov rcx(10),rsp // switch to next stack //
pop rbx // get next’s ptr to &last //
mov rax,(rbx) // store rax in &last //

}





extern struct task_struct * FASTCALL(__switch_to(struct task_struct *prev,
struct task_struct *next));


// from linux reference modifyfor 64 bit
#define switch_to(prev,next,last) do { \
	unsigned long esi,edi; \
	asm volatile(
		"pushfl\n\t" \
		"pushl %%ebp\n\t" \
		"movl %%esp,%0\n\t" // save ESP // \
		"movl %5,%%esp\n\t" // restore ESP // \
		"movl $1f,%1\n\t" // save EIP // \
		"pushl %6\n\t" // restore EIP // \
		"jmp __switch_to\n" \
		"1:\t" \
		"popl %%ebp\n\t" \
		"popfl" \
		:"=m" (prev->thread.esp),"=m" (prev->thread.eip), \
		"=a" (last),"=S" (esi),"=D" (edi) \
		:"m" (next->thread.esp),"m" (next->thread.eip), \
		"2" (prev), "d" (next)); \
} while (0)


// context_switch - switch to the new MM and the new
// thread's register state.



static inline
task_t * context_switch(runqueue_t *rq, task_t *prev, task_t *next)
{
struct mm_struct *mm = next->mm;
struct mm_struct *oldmm = prev->active_mm;
switch_mm(oldmm, mm, next);
switch_to(prev, next, prev);

return prev;
}

*/


