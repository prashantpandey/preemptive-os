#include <stdarg.h>
#include <defs.h>
#include <print.h>
#include <idt.h>
#include <pic.h>
#include <timer.h>
#include <common.h>
#include <process.h>
#include <paging.h>
#include <sys/gdt.h>
#include <sys/tarfs.h>


extern void irq0();
extern void irq1();

//extern pml4e *pml4e_table;

//bool firstSwitch = true;
//bool counter = true;

// Interrupt Descriptor Table(IDT) structure and utility functions

// A struct describing an interrupt gate.
struct idt_entry_struct
{
        uint16_t base_lo;             // The lower 16 bits of the address to jump to when this interrupt fires.
        uint16_t sel;                 // Kernel segment selector.
        uint8_t always0;             // This must always be zero.
        uint8_t flags;               // More flags. See documentation.
        uint16_t base_mid;
        uint32_t base_hi;             // The upper 16 bits of the address to jump to.
        uint32_t reserved;
} __attribute__((packed));
typedef struct idt_entry_struct idt_entry_t;


// A struct describing a pointer to an array of interrupt handlers.
// This is in a format suitable for giving to 'lidt'.
struct idt_ptr_struct
{
  	uint16_t limit;
   	uint64_t base;                // The address of the first element in our idt_entry_t array.
} __attribute__((packed));
typedef struct idt_ptr_struct idt_ptr_t;


// Global declaration of IDT and IDT_Pointer
idt_entry_t idt_entries[256];
idt_ptr_t   idt_ptr;

/*
  Global array containing the exception messages for the exceptions from 0-31
*/
char* exception_messages_isr[] = {"Division By Zero", "Debug","Non Maskable Interrupt","Reserved","Reserved"};

/**
 * Other utility functions 
 */


/* Loads the IDTR with address of IDT */
void lidt() {
	__asm__ __volatile__ ("lidt (%0)": :"r" (&idt_ptr));
}

/* Prints the software interrupt(exceptions) specific messages */
void handler_print_isr() {

	unsigned long isr_num = 0;
	uint64_t page_fault_add = 0x0;
	__asm__ __volatile__ ("movq %%rax, %0" : "=r"(isr_num));
	
	if(isr_num == 14) {
		__asm__ __volatile__ ("movq %%cr2, %0" : "=r"(page_fault_add));
		kprintf("Page fault occured at this address: %p", page_fault_add);
	}
	
	else {
		kprintf("\n\nInside the interrupt: Interrupt Num: %d, Message: %s", isr_num, exception_messages_isr[isr_num]);
	}

	while(1);
}

// handler yield
void handler_yield() {
	kprintf("\nInside Yield");
        switchProcess();                        // to handler the yield system call
}

// Handle sys call
void handler_syscall() {
		//kprint("inside isr 128\n");
	    	uint64_t syscall_no = 0;

    		__asm__ __volatile__(
	        	"movq %%rax, %0;"
        		:"=a"(syscall_no)
            		:
        	);
	    	// kprint("syscall %d\n", syscall_no);
		if(syscall_no == 1) {						// scanf
			__asm__ __volatile__ ("sti");
			char* buf;
			__asm__ __volatile__(
                                "movq %%rbx, %0;"
                                :"=b"(buf)
                                :
                        );
			kscanf(buf);
		}
	    	else if(syscall_no == 2) {					// printf
    			char* buf;
        		__asm__ __volatile__(
                		"movq %%rbx, %0;"
	                	:"=b"(buf)
        	        	:
                	);
	        	while(*buf != '\0')
        		{
            			kprintf("%c", *buf);
            			buf = buf + 1;
        		}
	      		//  __asm__("hlt"); 
    		}
		else if(syscall_no == 3) {					// malloc
			uint64_t size;
			__asm__ __volatile__(
	                        "movq %%rbx, %0;"
        	                :"=b"(size)
                	        :
                        	);
			//uint64_t returnAddr = kmalloc((uint32_t)size);
			void* returnAddr = malloc((uint32_t)size);
			/*
			int num = 0;
			if((size % 4096) != 0) {
				num = (size / 4096) + 1;
			}
			else {
				num = (size / 4096);
			}
			addPagesMalloc((void*)returnAddr, num);
			*/
			__asm__ __volatile__(
				"movq %0, %%rax;"
				:
				:"a" ((uint64_t)returnAddr)
				:"cc", "memory"
			);		
		}
		else if(syscall_no == 4) {					// getPid
			int pid = getPid();
			__asm__ __volatile__(
	                        "movq %0, %%rax;"
        	                :
                	        :"a" ((uint64_t)pid)
                        	:"cc", "memory"
	                );	
		}
		else if(syscall_no == 5) {					// exit 
			uint64_t status;
                        __asm__ __volatile__(
                                "movq %%rbx, %0;"
                                :"=b"(status)
                                :
                                );
			returnToKernel(status);			
		}
		else if(syscall_no == 6) {					// fork
			int childId = fork();
			__asm__ __volatile__(
                                "movq %0, %%rax;"
                                :
                                :"a" ((uint64_t)childId)
                                :"cc", "memory"
                        );
		}
		else if(syscall_no == 7) {					// execvpe
			char* buf;
                        __asm__ __volatile__(
                                "movq %%rbx, %0;"
                                :"=b"((char* )buf)
                                :
                        );
			int ret = execvpe(buf, NULL, NULL);
			__asm__ __volatile__(
                                "movq %0, %%rax;"
                                :
                                :"a" ((uint64_t)ret)
                                :"cc", "memory"
                        );
		}
		else if(syscall_no == 8) {					// sleep
			// TODO: sleep system call
		}
		else if(syscall_no == 9) {					// wait
			putProcessToWait();
		}
		else if(syscall_no == 10) {					// opendir
			char* dir;
                        __asm__ __volatile__(
                                "movq %%rbx, %0;"
                                :"=b"(dir)
                                :
                        );
			uint64_t addrHdr = open_dir(dir);
			__asm__ __volatile__(
                                "movq %0, %%rax;"
                                :
                                :"a" ((uint64_t)addrHdr)
                                :"cc", "memory"
                       );	
		}
		else if(syscall_no == 11) {					// read dir
			char* dir;
                        __asm__ __volatile__(
                                "movq %%rbx, %0;"
                                :"=b"((char* )dir)
                                :
                        );
			read_dir(dir);
		}
		else if(syscall_no == 12) {					// open file
			char* file;
                        __asm__ __volatile__(
                                "movq %%rbx, %0;"
                                :"=b"(file)
                                :
                        );
                        uint64_t addrHdr = open(file);
                        __asm__ __volatile__(
                                "movq %0, %%rax;"
                                :
                                :"a" ((uint64_t)addrHdr)
                                :"cc", "memory"
                       );
		}
		else if(syscall_no == 13) {					// read file
			uint64_t file;
			uint64_t  buf;
			uint64_t size;
                        __asm__ __volatile__(
                                "movq %%rbx, %0;"
                                :"=b"(file)
                                :
                        );
			__asm__ __volatile__(
                                "movq %%rcx, %0;"
                                :"=c"(size)
                                :
                        );
                        __asm__ __volatile__(
                                "movq %%rdx, %0;"
                                :"=d"(buf)
                                :
                        );
			int size_read = read_file(file, (uint32_t)size, buf);
		        __asm__ __volatile__(
                                "movq %0, %%rax;"
                                :
                                :"a" ((uint64_t)size_read)
                                :"cc", "memory"
                        );
		}
		else if(syscall_no == 14) {					///Close file
                        uint64_t file_addr;
                        __asm__ __volatile__(
                                "movq %%rbx, %0;"
                                :"=b"(file_addr)
                                :
                        );
			close(file_addr);
                }

		else if(syscall_no == 15) {					///Close Directory
                        uint64_t dir_addr;
                        __asm__ __volatile__(
                                "movq %%rbx, %0;"
                                :"=b"(dir_addr)
                                :
                        );
			closedir(dir_addr);
                }
}

/* Handles the Interrupt Service Routines(ISRs) when software interrupts are triggered */
void handler_interrupt_isr0() 
{
	__asm__ (
              "movq $0, %rax"
              );

	 __asm__(".global x86_64_isr_vector0 \n"\
                        "x86_64_isr_vector0:\n" \
          		"    pushq %rax;" \
                        "    pushq %rbx;" \
                        "    pushq %rcx;" \
                        "    pushq %rdx;" \
                        "    pushq %rsi;" \
                        "    pushq %rdi;" \
                        "    pushq %r8;" \
                        "    pushq %r9;" \
                        "    pushq %r10;" \
                        "    pushq %r11;" \
                        "    pushq %r12;" \
                        "    pushq %r13;" \
                        "    pushq %r14;" \
                        "    pushq %r15;" \
                        "    call handler_print_isr;"\
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
                        "    popq %rax;" \
		"iretq;");
}


void handler_interrupt_isr13()
{
        __asm__ (
               "movq $13, %rax"
              );
	
	__asm__(".global x86_64_isr_vector13 \n"\
                        "x86_64_isr_vector13:\n" \
          		"    pushq %rax;" \
                        "    pushq %rbx;" \
                        "    pushq %rcx;" \
                        "    pushq %rdx;" \
                        "    pushq %rsi;" \
                        "    pushq %rdi;" \
                        "    pushq %r8;" \
                        "    pushq %r9;" \
                        "    pushq %r10;" \
                        "    pushq %r11;" \
                        "    pushq %r12;" \
                        "    pushq %r13;" \
                        "    pushq %r14;" \
                        "    pushq %r15;" \
                        "    call handler_print_isr;"\
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
                        "    popq %rax;" \
		"iretq;");	
}

void handler_interrupt_isr14()
{
        __asm__ (
              "movq $14, %rax"
              );

         __asm__(".global x86_64_isr_vector14 \n"\
                        "x86_64_isr_vector14:\n" \
                        "    pushq %rax;" \
                        "    pushq %rbx;" \
                        "    pushq %rcx;" \
                        "    pushq %rdx;" \
                        "    pushq %rsi;" \
                        "    pushq %rdi;" \
                        "    pushq %r8;" \
                        "    pushq %r9;" \
                        "    pushq %r10;" \
                        "    pushq %r11;" \
                        "    pushq %r12;" \
                        "    pushq %r13;" \
                        "    pushq %r14;" \
                        "    pushq %r15;" \
                        "    call handler_print_isr;"\
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
                        "    popq %rax;" \
          "iretq;");
}

// To handle the ring 3 system calls
void handler_interrupt_isr128()
{
/*
        __asm__ (
              "cli;"
              "movq $128, %rax"
              );
*/
         __asm__(".global x86_64_isr_vector128 \n"\
                        "x86_64_isr_vector128:\n" \
                        "    pushq %rbx;" \
                        "    pushq %rcx;" \
                        "    pushq %rdx;" \
                        "    pushq %rsi;" \
                        "    pushq %rdi;" \
                        "    pushq %r8;" \
                        "    pushq %r9;" \
                        "    pushq %r10;" \
                        "    pushq %r11;" \
                        "    pushq %r12;" \
                        "    pushq %r13;" \
                        "    pushq %r14;" \
                        "    pushq %r15;" \
                        "    call handler_syscall;"\
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
		"iretq;");
}

/* Handles the Interrupt Service Routines(ISRs) when software interrupts are triggered */
void handler_interrupt_isr144()
{
         __asm__(".global x86_64_isr_vector144 \n"\
                        "x86_64_isr_vector144:\n" \
                        "    pushq %rax;" \
                        "    pushq %rbx;" \
                        "    pushq %rcx;" \
                        "    pushq %rdx;" \
                        "    pushq %rsi;" \
                        "    pushq %rdi;" \
                        "    pushq %r8;" \
                        "    pushq %r9;" \
                        "    pushq %r10;" \
                        "    pushq %r11;" \
                        "    pushq %r12;" \
                        "    pushq %r13;" \
                        "    pushq %r14;" \
                        "    pushq %r15;" \
                        "    call handler_yield;"\
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
                        "    popq %rax;" \
                "iretq;");
}

/* Prints the IRQ specific message */
void handler_print_irq() {
        unsigned long irq_num = 0;
        __asm__ __volatile__ ("movq %%rax, %0" : "=r"(irq_num));
	
}

/* Handles the timer interrupt corresponding to IRQ0 */
void handler_interrupt_irq0()
{
       /* __asm__ (
              "cli;"
              "movq $32, %rax"
              );
	*/
         __asm__(".global x86_64_irq_vector0 \n"\
                        "x86_64_irq_vector0:\n" \
		        "    pushq %rax;" \
                        "    pushq %rbx;" \
                        "    pushq %rcx;" \
                        "    pushq %rdx;" \
                        "    pushq %rsi;" \
                        "    pushq %rdi;" \
                        "    pushq %r8;" \
                        "    pushq %r9;" \
                        "    pushq %r10;" \
                        "    pushq %r11;" \
                        "    call timer_callback;"	\
			"    call schedule;"	\
                        "    popq %r11;"                 \
                        "    popq %r10;" \
                        "    popq %r9;" \
                        "    popq %r8;" \
                        "    popq %rdi;" \
                        "    popq %rsi;" \
                        "    popq %rdx;" \
                        "    popq %rcx;" \
                        "    popq %rbx;" \
                        "    popq %rax;" \
          "iretq;");

	pic_send_eoi(0);
}

/* Handles the keyboard interrupt corresponding to IRQ1 */
void handler_interrupt_irq1()
{
      /*  __asm__ (
              "cli;"
              "movq $33, %rax"
              );
	*/
         __asm__(".global x86_64_irq_vector1 \n"\
                        "x86_64_irq_vector1:\n" \
                        "    pushq %rax;" \
                        "    pushq %rbx;" \
                        "    pushq %rcx;" \
                        "    pushq %rdx;" \
                        "    pushq %rsi;" \
                        "    pushq %rdi;" \
                        "    pushq %r8;" \
                        "    pushq %r9;" \
                        "    pushq %r10;" \
                        "    pushq %r11;" \
                        "    call handler_print_irq;"\
                        "    popq %r11;"                 \
                        "    popq %r10;" \
                        "    popq %r9;" \
                        "    popq %r8;" \
                        "    popq %rdi;" \
                        "    popq %rsi;" \
                        "    popq %rdx;" \
                        "    popq %rcx;" \
                        "    popq %rbx;" \
                        "    popq %rax;" \

          "iretq;");
}

// Setting up an interrupt inside IDT at given index 'num' 
void idt_set_gate(int num, uint64_t base, uint16_t sel, uint8_t flags)
{
        idt_entries[num].base_lo = base & 0xFFFF;
        idt_entries[num].base_mid = (base >> 16) & 0xFFFF;
        idt_entries[num].base_hi = (base >> 32) & 0xFFFFFFFF;

        idt_entries[num].sel     = sel;
        idt_entries[num].always0 = 0;

        idt_entries[num].flags   = flags;
}

// Initialize IDT
void init_idt()
{
        idt_ptr.limit = sizeof(idt_entry_t) * 256 -1;
        idt_ptr.base  = (uint64_t)&idt_entries;
	
	// Will setup the particular ISR entry in the IDT
        idt_set_gate(0, (uint64_t) &handler_interrupt_isr0, 0x08, 0x8E);
        idt_set_gate(13, (uint64_t) &handler_interrupt_isr13, 0x08, 0x8E);
        idt_set_gate(14, (uint64_t) &handler_interrupt_isr14, 0x08, 0x8E);
	
        idt_set_gate(128, (uint64_t) &handler_interrupt_isr128, 0x08, 0xEE);		// DPL 3
        idt_set_gate(144, (uint64_t) &handler_interrupt_isr144, 0x08, 0xEE);		// DPL 3
	
	// Will setup the respective IRQ entries in the IDT for PIT and Keyboard interrupt
        idt_set_gate(32, (uint64_t) &irq0, 0x08, 0x8E);
        idt_set_gate(33, (uint64_t) &irq1, 0x08, 0x8E);

        lidt();

	// To let hardware interrupts happen
	// __asm__ ("sti");
}
