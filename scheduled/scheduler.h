/*
 * Cody Shepherd
 * CS533 Course Project
 * scheduler.h
 */
#include <stdlib.h>  
#include <stdio.h>
#include "queue.h"


typedef enum {
    RUNNING,    // The thread is currently running
    READY,      // The thread is not running, but is runnable
    BLOCKED,    // The thread is not reunning, and not runnable
    DONE        // The thread has finished
} state_t;

typedef unsigned char byte;             // for brevity
typedef struct thread                   // struct representing a thread control block (TCB)
{
    byte* stack_pointer;                // stack pointer
    void (*initial_function)(void*);    // ptr to initial function
    void* initial_argument;             // ptr to initial arg
    state_t state;
    byte* sp_btm;
} thread; 

void scheduler_begin();
void thread_fork(void(*target)(void*), void * arg);
void yield();
void scheduler_end();

/* GLOBALS */

extern struct thread * current_thread;
extern const int STACK_SIZE;
