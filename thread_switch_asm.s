
#void thread_swap(thread * old, thread * new)
.globl thread_swap

thread_swap:
    pushq %rbx               #push callee-save registers onto stack
    pushq %rbp
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    movq %rsp, %rbp         #save current stack ptr
    movq %rsi, %rsp         #load new thread's stack ptr
    popq %rbx               #pop callee-save registers from new stack
    popq %rbp
    popq %r12
    popq %r13
    popq %r14
    popq %r15
    ret                     #return

#void thread_start(thread * old, thread * new);
.globl thread_start

thread_start:
    pushq %rbx               #push callee-save registers onto stack
    pushq %rbp
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    movq %rsp, %rbp         #save current stack ptr
    movq %rsi, %rsp         #load new thread's stack ptr
    #call *4(%rsi) 
    jmp thread_wrap
