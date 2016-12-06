
#void thread_switch(thread * old, thread * new)
.globl thread_switch

thread_switch:               #ready list lock must be held when entering this routine
    pushq %rbx               #push callee-save registers onto stack
    pushq %rbp
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    movq %rsp, (%rdi)         #save current stack ptr
    movq (%rsi), %rsp         #load new thread's stack ptr
    popq %r15                 #pop callee-save registers from new stack
    popq %r14
    popq %r13
    popq %r12
    popq %rbp
    popq %rbx
    ret                       #return

#void thread_start(thread * old, thread * new);
.globl thread_start

thread_start:                #ready list lock must be held when entering this routine
    pushq %rbx               #push callee-save registers onto stack
    pushq %rbp
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    movq %rsp, (%rdi)         #save current stack ptr
    movq (%rsi), %rsp         #load new thread's stack ptr
    jmp thread_wrap
