/*
 * Cody Shepherd
 * CS533 Course Project
 * scheduler.h
 */
#include <stdlib.h>  
#include <atomic_ops.h>
#include <stdio.h>
#include "queue.h"

extern void * safe_mem(int, void*);
#define malloc(arg) safe_mem(0, ((void*)(arg)))
#define free(arg) safe_mem(1, arg)

#define current_thread (get_current_thread())


typedef enum {
    RUNNING,    // The thread is currently running
    READY,      // The thread is not running, but is runnable
    BLOCKED,    // The thread is not running, and not runnable
    DONE        // The thread has finished
} state_t;

typedef struct mutex{
    int held;
    AO_TS_t s;
    struct queue waiting_threads;
} mutex;

typedef struct condition{
    AO_TS_t s;
    struct queue waiting_threads;
} condition;

typedef unsigned char byte;             // for brevity
typedef struct thread                   // struct representing a thread control block (TCB)
{
    byte* stack_pointer;                // stack pointer
    void (*initial_function)(void*);    // ptr to initial function
    void* initial_argument;             // ptr to initial arg
    state_t state;
    byte* sp_btm;
    mutex mtx;
    condition cond;
} thread; 

void print_thread(thread * thrd);

void scheduler_begin();
void kernel_thread_begin();
thread * thread_fork(void(*target)(void*), void * arg);
void thread_join(struct thread*);
void yield();
void scheduler_end();

void panic(char arg[]);

/*  MUTEX   */

void mutex_init(mutex * mtx);
void mutex_lock(mutex * mtx);
void mutex_unlock(mutex * mtx);

void block(AO_TS_t * spinlock);

/*  CONDITION VARIABLE  */

void condition_init(struct condition * cond);
void condition_wait(struct condition * cond, struct mutex * mtx);
void condition_signal(struct condition * cond);
void condition_broadcast(struct condition * cond);

/*  ATOMIC LOCKS  */

void spinlock_lock(AO_TS_t * ts);
void spinlock_unlock(AO_TS_t * ts);

/* GLOBALS */

extern struct thread * current_thread;
extern const int STACK_SIZE;
