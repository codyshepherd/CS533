/*
 * Cody Shepherd
 * CS533 Course Project
 * scheduler.h
 */

typedef unsigned char byte;
//struct representing a thread control block (TCB)
struct thread
{
    //stack pointer
    byte* stack_pointer;

    //ptr to initial function
    void (*initial_function)(void*);

    //ptr to initial arg
    void* initial_argument;
}; 
typedef struct thread thread;

void scheduler_begin();
void thread_fork(void(*target)(void*), void * arg);
void yield();
void scheduler_end();

extern struct thread * current_thread;
extern struct thread * inactive_thread;
