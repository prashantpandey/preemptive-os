
.text

.global irq0
irq0:
	    #cli
            pushq %rax
	    pushq %rbx
            pushq %rcx
            pushq %rdx
            pushq %rsi
            pushq %rdi
            pushq %r8
            pushq %r9
            pushq %r10
            pushq %r11
            pushq %r12
            pushq %r13
            pushq %r14
            pushq %r15
            movq  %rsp,%rdi
            addq  $72, %rdi
            call timer_callback
	    #call schedule
            popq %r15
            popq %r14
            popq %r13
            popq %r12
            popq %r11
            popq %r10
            popq %r9
            popq %r8
            popq %rdi
            popq %rsi
            popq %rdx
            popq %rcx
	    popq %rbx
            popq %rax
	    #sti
            iretq
.endm

.global irq1
irq1:
            pushq %rax
	    pushq %rbx
            pushq %rcx
            pushq %rdx
            pushq %rsi
            pushq %rdi
            pushq %r8
            pushq %r9
            pushq %r10
            pushq %r11
            pushq %r12
            pushq %r13
            pushq %r14
            pushq %r15
            movq  %rsp,%rdi
            addq  $72, %rdi
            call keyboard_handler
            popq %r15
            popq %r14
            popq %r13
            popq %r12
            popq %r11
            popq %r10
            popq %r9
            popq %r8
            popq %rdi
            popq %rsi
            popq %rdx
            popq %rcx
	    popq %rbx
            popq %rax
            iretq
.endm

