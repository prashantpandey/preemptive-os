#ifndef MMU_H
#define MMU_H

// +--------16----- +--------9------+-------9--------+-------9--------+-------9--------+-------12---------------+
// | SIGN BITS      | PML4E	    |    PDPE 	     | PDE	      |  PTE	       |  OFFSET WITHIN PAGE	|
// | Index          | Index 	    |    Index 	     | INDEX	      |  INDEX	       | 			| 
// +----------------+---------------+----------------+----------------------------------------------------------+
// \---          --/                 \-- PDPE(va) -/ \--- PDE(va) ---/\---- PTE(va) ----/\------PGOFF(va) --------/
// \-----------------------------------  PGNUM(va) ---------------------------------------------/

#define PAGE_TABLE_ALIGNLENT 0x1000
 
/* mask out bits under page size */
#define ALIGN_DOWN(x)   (x & ~(PAGE_TABLE_ALIGNLENT-1))

#define PPN(va)			(((uint64_t)(va) >> PGSHIFT)

#define PGSIZE                	4096                // bytes mapped by a page
#define PGSHIFT                	12                // log2(PGSIZE)

#define PGNUM(la)        	(((uint64_t) (la)) >> PTESHIFT)

// PTE Index
#define PTEX(la)                ((((uint64_t) (la)) >> PTESHIFT) & 0x1FF)

// PDPE Index
#define PDEX(la)                ((((uint64_t) (la)) >> PDESHIFT) & 0x1FF)

// PDPE Index
#define PDPEX(la)               ((((uint64_t) (la)) >> PDPESHIFT) & 0x1FF)

// PML4E Index
#define PML4EX(la)              ((((uint64_t) (la)) >> PML4ESHIFT) & 0x1FF)

#define VPD(la)         	(((uint64_t) (la)) >> PDESHIFT)                // used to index into vpd[]
#define VPDPE(la)  	 	(((uint64_t) (la)) >> PDPESHIFT)
#define VPML4E(la)  		(((uint64_t) (la)) >> PML4ESHIFT)

// offset in page
#define PGOFF(la)        	(((uint64_t) (la)) & 0xFFF)

// Address in page table or page directory entry
#define PTE_ADDR(pte)   	((uint64_t) (pte) & ~0xFFF)

#define PML4ESHIFT		39
#define PDPESHIFT		30
#define PDESHIFT		21
#define PTESHIFT		12

// Page directory and page table constants.
#define NUMBER_ENTRIES        	512                // page table entries per page table

// Page table/directory entry flags.
#define PTE_P                	0x001        // Present
#define PTE_W               	0x002        // Writeable
#define PTE_U                	0x004        // User
#define PTE_PWT                	0x008        // Write-Through
#define PTE_PCD                	0x010        // Cache-Disable
#define PTE_A                	0x020        // Accessed
#define PTE_D                	0x040        // Dirty
#define PTE_PS                	0x080        // Page Size
#define PTE_G                	0x100        // Global
#define PTE_COW                	0x800        // Avail for system programmer's use

// The PTE_AVAIL bits aren't used by the kernel or interpreted by the
// hardware, so user processes are allowed to set them arbitrarily.
#define PTE_AVAIL        0xE00        // Available for software use

// Flags in PTE_SYSCALL may be used in system calls. (Others may not.)
#define PTE_SYSCALL        (PTE_AVAIL | PTE_P | PTE_W | PTE_U)

// Control Register flags
#define CR0_PE                0x00000001        // Protection Enable
#define CR0_MP                0x00000002        // Monitor coProcessor
#define CR0_EM                0x00000004        // Emulation
#define CR0_TS                0x00000008        // Task Switched
#define CR0_ET                0x00000010        // Extension Type
#define CR0_NE                0x00000020        // Numeric Errror
#define CR0_WP                0x00010000        // Write Protect
#define CR0_AM                0x00040000        // Alignment Mask
#define CR0_NW                0x20000000        // Not Writethrough
#define CR0_CD                0x40000000        // Cache Disable
#define CR0_PG                0x80000000        // Paging

#define CR4_PCE                0x00000100        // Performance counter enable
#define CR4_MCE                0x00000040        // Machine Check Enable
#define CR4_PSE                0x00000010        // Page Size Extensions
#define CR4_DE                0x00000008        // Debugging Extensions
#define CR4_TSD                0x00000004        // Time Stamp Disable
#define CR4_PVI                0x00000002        // Protected-Mode Virtual Interrupts
#define CR4_VME                0x00000001        // V86 Mode Extensions

// Eflags register
#define FL_CF                0x00000001        // Carry Flag
#define FL_PF                0x00000004        // Parity Flag
#define FL_AF                0x00000010        // Auxiliary carry Flag
#define FL_ZF                0x00000040        // Zero Flag
#define FL_SF                0x00000080        // Sign Flag
#define FL_TF                0x00000100        // Trap Flag
#define FL_IF                0x00000200        // Interrupt Flag
#define FL_DF                0x00000400        // Direction Flag
#define FL_OF                0x00000800        // Overflow Flag
#define FL_IOPL_MASK        0x00003000        // I/O Privilege Level bitmask
#define FL_IOPL_0        0x00000000        // IOPL == 0
#define FL_IOPL_1        0x00001000        // IOPL == 1
#define FL_IOPL_2        0x00002000        // IOPL == 2
#define FL_IOPL_3        0x00003000        // IOPL == 3
#define FL_NT                0x00004000        // Nested Task
#define FL_RF                0x00010000        // Resume Flag
#define FL_VM                0x00020000        // Virtual 8086 mode
#define FL_AC                0x00040000        // Alignment Check
#define FL_VIF                0x00080000        // Virtual Interrupt Flag
#define FL_VIP                0x00100000        // Virtual Interrupt Pending
#define FL_ID                0x00200000        // ID flag

// Page fault error codes
#define FEC_PR                0x1        // Page fault caused by protection violation
#define FEC_WR                0x2        // Page fault caused by a write
#define FEC_U                0x4        // Page fault occured while in user mode

#endif
