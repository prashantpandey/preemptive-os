/*File containing functions to initialize IDT and registering and handling ISRs and IRQs  */
#include <stdarg.h>
#include <defs.h>
#include <stdio.h>
#include <idt.h>
#include <pic.h>
#include <timer.h>
#include <common.h>

extern void irq0();
extern void irq1();

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
   	uint32_t base;                // The address of the first element in our idt_entry_t array.
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
	__asm__ __volatile__ ("movq %%rax, %0" : "=r"(isr_num));
	printf("\n\nInside the interrupt: Interrupt Num: %d, Message: %s", isr_num, exception_messages_isr[isr_num]);
	
	while(1);
}

/* Handles the Interrupt Service Routines(ISRs) when software interrupts are triggered */
void handler_interrupt_isr0() 
{
	__asm__ (
              "cli;"
              "movq $0, %rax"
              );

	 __asm__(".global x86_64_isr_vector0 \n"\
                        "x86_64_isr_vector0:\n" \
                        "    pushq %rax;" \
                        "    pushq %rcx;" \
                        "    pushq %rdx;" \
                        "    pushq %rsi;" \
                        "    pushq %rdi;" \
                        "    pushq %r8;" \
                        "    pushq %r9;" \
                        "    pushq %r10;" \
                        "    pushq %r11;" \
                        "    call handler_print_isr;"\
                        "    popq %r11;"                 \
                        "    popq %r10;" \
                        "    popq %r9;" \
                        "    popq %r8;" \
                        "    popq %rdi;" \
                        "    popq %rsi;" \
                        "    popq %rdx;" \
                        "    popq %rcx;" \
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
                        "    pushq %rcx;" \
                        "    pushq %rdx;" \
                        "    pushq %rsi;" \
                        "    pushq %rdi;" \
                        "    pushq %r8;" \
                        "    pushq %r9;" \
                        "    pushq %r10;" \
                        "    pushq %r11;" \
                        "    call timer_callback;"\
                        "    popq %r11;"                 \
                        "    popq %r10;" \
                        "    popq %r9;" \
                        "    popq %r8;" \
                        "    popq %rdi;" \
                        "    popq %rsi;" \
                        "    popq %rdx;" \
                        "    popq %rcx;" \
                        "    popq %rax;" \
			"    sti;" \
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

        idt_entries[num].flags   = flags /* | 0x60 */;
}

// Initialize IDT
void init_idt()
{
        idt_ptr.limit = sizeof(idt_entry_t) * 256 -1;
        idt_ptr.base  = (uint64_t)&idt_entries;
	
	// Will setup the particular ISR entry in the IDT
        idt_set_gate(0,(uint64_t) &handler_interrupt_isr0, 0x08, 0x8E);
	
	// Will setup the respective IRQ entries in the IDT for PIT and Keyboard interrupt
        idt_set_gate(32,(uint64_t) &irq0, 0x08, 0x8E);
        idt_set_gate(33,(uint64_t) &irq1, 0x08, 0x8E);

        lidt();

	// To let hardware interrupts happen
	__asm__ ("sti");
}

