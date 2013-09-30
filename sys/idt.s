
.text

.global irq0
irq0:
            pushq %rax
            pushq %rcx
            pushq %rdx
            pushq %rsi
            pushq %rdi
            pushq %r8
            pushq %r9
            pushq %r10
            pushq %r11
            movq  %rsp,%rdi
            addq  $72, %rdi
            call timer_callback
            popq %r11
            popq %r10
            popq %r9
            popq %r8
            popq %rdi
            popq %rsi
            popq %rdx
            popq %rcx
            popq %rax
            iretq
.endm

.global irq1
irq1:
            pushq %rax
            pushq %rcx
            pushq %rdx
            pushq %rsi
            pushq %rdi
            pushq %r8
            pushq %r9
            pushq %r10
            pushq %r11
            movq  %rsp,%rdi
            addq  $72, %rdi
            call keyboard_handler
            popq %r11
            popq %r10
            popq %r9
            popq %r8
            popq %rdi
            popq %rsi
            popq %rdx
            popq %rcx
            popq %rax
            iretq
.endm

