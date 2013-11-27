/* File containing the functions to handle timer interrupt*/

#include <timer.h>
#include <idt.h>
#include <stdio.h>
#include <pic.h>
#include <common.h>
#include <process.h>
#include <paging.h>
#include <sys/gdt.h>

extern pml4e *pml4e_table;

int tick = 0;
int total_time = 0;

//bool firstSwitch = true;


// Will handle the timer interrupt when the interrupt is fired.
// Also will print the time elasped since the last reboot on the lower right corner of the screen
void timer_callback()
{
//	int len = 0;
	int hour = 0;
	int minute = 0;
	int second = 60;
   	tick++;
	outb(0x20, 0x20);	

	if(tick % 100 == 0) {
    		total_time++;
		int cursor_p_x = 30;
		int cursor_p_y = 24;
		
		second = total_time;			
		hour = (int)(second/3600);
		second = second - (hour*3600);
		minute = (int)(second/60);
		second = second - (minute*60);
	
		clear_line(cursor_p_y, cursor_p_x);	
		cursor_p_x = cursor_p_x + printf_string("TIME  ", cursor_p_y, cursor_p_x);
		cursor_p_x = cursor_p_x + printf_int(hour, cursor_p_y, cursor_p_x);
		cursor_p_x = cursor_p_x + printf_string(":",cursor_p_y, cursor_p_x);
		cursor_p_x = cursor_p_x + printf_int(minute, cursor_p_y, cursor_p_x);
		cursor_p_x = cursor_p_x + printf_string(":", cursor_p_y, cursor_p_x);
		cursor_p_x = cursor_p_x + printf_int(second, cursor_p_y, cursor_p_x);

	/*		
		if(firstSwitch) {
				firstSwitch = false;
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

        pml4a[511] = pml4e_table[511]; //point to pdpe of kerne
        pml4b[511] = pml4e_table[511]; //point to pdpe of kerne

        thread1.cr3 = (uint64_t)PADDR((uint64_t)pml4a);
        thread2.cr3 = (uint64_t)PADDR((uint64_t)pml4b);

        thread1.stack[59] = (uint64_t)&function1;
        thread1.rsp = (uint64_t)&thread1.stack[49];

        thread1.stack[63] = 0x23 ;                              //  Data Segment    
        thread1.stack[62] = (uint64_t)&thread1.stack[63] ;      //  RIP
        //thread1.stack[61] = 0x20202 ;                           //  RFlags
        thread1.stack[61] = 0x246;                           //  EFlags
        thread1.stack[60] = 0x1b ;                              // Code Segment

        thread2.stack[59] = (uint64_t)&function2;
        thread2.rsp = (uint64_t)&thread2.stack[49];

        thread2.stack[63] = 0x23 ;                              //  Data Segment    
        thread2.stack[62] = (uint64_t)&thread2.stack[63] ;      //  RIP
        //thread1.stack[61] = 0x20202 ;                           //  RFlags
        thread2.stack[61] = 0x246;                           //  EFlags
        thread2.stack[60] = 0x1b ;                              // Code Segment

        // inilialize the ready queue with both the task structures
        readyQ[0] = thread1;
        readyQ[1] = thread2;
	        // __asm__("cli");
        asm volatile("movq %0, %%cr3":: "a"(thread1.cr3));
        // __asm__("sti");


        printf("\n I am in process virtual address space \n");

        __asm__ __volatile__ (
            "movq %0, %%rsp;" //load next's stack in rsp
            :
            :"r"(thread1.rsp)
        );

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

        //__asm__ __volatile__(
        //      "movq %0, %%r15;"
        //      :   
        //      :"r"(&thread1.stack[63])
        //);  

        //__asm__ __volatile__(
        //      "movq %%r15, %0;"
        //      :"=r"(tss.rsp0)
        //);
        tss.rsp0 = (uint64_t)&thread1.stack[63];

        __asm__ __volatile__("mov $0x2b,%ax");
        __asm__ __volatile__("ltr %ax");

        __asm__ __volatile__("iretq");	

		}
	else {
	printf("Inside second context switch..!!");
	int i = 0;
		prev = (task *)&readyQ[i];
            i = (i + 1) % num_process;
            next = (task *)&readyQ[i];
		
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

        __asm__ __volatile__(
                "popq %r11;" \
                "popq %r10;" \
                "popq %r9;" \
                "popq %r8;" \
                "popq %rdi;" \
                "popq %rsi;" \
                "popq %rdx;" \
                "popq %rcx;" \
                "popq %rbx;" \
                "popq %rax;" \
                "sti;"
        );
        __asm__("iretq");
	}
	*/
		
 	}
}

// Will initialize the timer interrupt 
void init_timer(uint32_t frequency)
{
	// The timer interrupt is registered in idt.c when the IDT is inilialized at position 32 mapped to 0 for PIC.

	// The value we send to the PIT is the value to divide it's input clock
   	// (1193180 Hz) by, to get our required frequency. Important to note is
   	// that the divisor must be small enough to fit into 16-bits.
   	uint32_t divisor = 1193180 / frequency;

   	// Send the command byte.
   	outb(0x43, 0x36);

   	// Divisor has to be sent byte-wise, so split here into upper/lower bytes.
   	uint8_t l = (uint8_t)(divisor & 0xFF);
   	uint8_t h = (uint8_t)( (divisor>>8) & 0xFF );

   	// Send the frequency divisor.
   	outb(0x40, l);
   	outb(0x40, h);
}
