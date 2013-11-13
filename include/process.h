#ifndef _PROCESS_H
#define _PROCESS_H

#include <defs.h>

typedef struct {
	uint64_t stack[1024]; 		// process local stack
	uint64_t rsp;			// stack pointer
	uint64_t cr3;			// current value of CR3 register for process switch
} task;


void first_context_switch();
#endif
