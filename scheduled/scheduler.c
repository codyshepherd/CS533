/*
 * Cody Shepherd
 * CS533 Course Project
 * scheduler.c
 */
#define _GNU_SOURCE

#include <sched.h>
#include <atomic_ops.h>
#include <stdio.h>
#include <stdlib.h>
#include "scheduler.h"

#undef malloc
#undef free
void * safe_mem(int op, void * arg) {
  static AO_TS_t spinlock = AO_TS_INITIALIZER;
  void * result = 0;

  spinlock_lock(&spinlock);
  if(op == 0) {
    result = malloc((size_t)arg);
  } else {
    free(arg);
  }
  spinlock_unlock(&spinlock);
  return result;
}
#define malloc(arg) safe_mem(0, ((void*)(arg)))
#define free(arg) safe_mem(1, arg)

const int STACK_SIZE = 1048576;

//struct thread * current_thread;
struct queue ready_list;        // Holds runnable threads
AO_TS_t ready_list_lock;

struct queue blocked_list;
AO_TS_t blocked_list_lock;

struct queue done_list;         // Holds done threads waiting to be recycled
AO_TS_t done_list_lock;

AO_TS_t print_lock;

void print_thread(thread * thrd){
    switch(thrd->state){
        case BLOCKED:
            printf("BLOCKED\n");
            break;
        case READY:
            printf("READY\n");
            break;
        case RUNNING:
            printf("RUNNING\n");
            break;
        case DONE:
            printf("DONE\n");
            break;
        default:
            printf("????\n");
            break;
    }
}

void scheduler_begin(){

    //allocate a struct for when the main thread gets switched out
    set_current_thread( (thread*)malloc(sizeof(thread)) );
    current_thread->state = RUNNING;

    //null-initialize the queue pointers, because they're empty
    ready_list.head = ready_list.tail = NULL;
    done_list.head = done_list.tail = NULL;
    blocked_list.head = blocked_list.tail = NULL;
    ready_list.count = done_list.count = blocked_list.count = 0;

    mutex_init(&(current_thread->mtx));
    condition_init(&(current_thread->cond));

    byte* sp = malloc(STACK_SIZE);
    sp = sp + STACK_SIZE;

    clone( kernel_thread_begin , sp, CLONE_THREAD | CLONE_VM | CLONE_SIGHAND | CLONE_FILES | CLONE_FS | CLONE_IO, NULL ); 
}

void kernel_thread_begin(){
    thread* empty_thread = (thread*)malloc(sizeof(thread));
    empty_thread->state = RUNNING;

    set_current_thread( empty_thread );

    while(1)
        yield();
}

thread * thread_fork(void(*target)(void*), void * arg){

    thread * forked_thread = NULL;
    thread * temp = NULL;

    spinlock_lock(&done_list_lock);
    while(done_list.count > 9){             //Trim the recycle queue
        spinlock_lock(&print_lock);
        printf("Trimming dead queue\n");   
        spinlock_unlock(&print_lock);
        temp = thread_dequeue(&done_list);
        temp->stack_pointer = NULL;
        free(temp->initial_argument);
        free(temp->sp_btm);
        free(temp);
    }
    spinlock_unlock(&done_list_lock);
    temp = NULL;

    spinlock_lock(&done_list_lock);
    if(!is_empty(&done_list)){      // Check on the recyclable list first

        forked_thread = thread_dequeue(&done_list);
        spinlock_unlock(&done_list_lock);
        forked_thread->initial_function = target;

    }
    else {

        forked_thread = (thread*)malloc(sizeof(thread));
        forked_thread->initial_function = target;
        forked_thread->sp_btm = malloc(STACK_SIZE);

        mutex_init(&(forked_thread->mtx));
        condition_init(&(forked_thread->cond));
    }

    forked_thread->initial_argument = arg;

    forked_thread->stack_pointer = forked_thread->sp_btm + STACK_SIZE;
    forked_thread->state = RUNNING;

    //sleep currently running thread
    current_thread->state = READY;
    
    //enqueue currently running thread
    spinlock_lock(&ready_list_lock);
    thread_enqueue(&ready_list, current_thread);

    //swap out current thread and run it
    temp = current_thread;
    set_current_thread( forked_thread );

    thread_start(temp, forked_thread);
    
    spinlock_unlock(&ready_list_lock);
    return forked_thread;
}

void thread_join(struct thread * thrd){

    mutex_lock(&(thrd->mtx));
    while(thrd->state != DONE){
        condition_wait(&(thrd->cond), &(thrd->mtx));
    }
    mutex_unlock(&(thrd->mtx));
}

void scheduler_end(){

    spinlock_lock(&ready_list_lock);
    while(!is_empty(&ready_list)){
        spinlock_unlock(&ready_list_lock);
        yield();
        spinlock_lock(&ready_list_lock);
    }
    spinlock_unlock(&ready_list_lock);
    
    thread * temp;
    spinlock_lock(&done_list_lock);
    while(!is_empty(&done_list)){
        temp = thread_dequeue(&done_list);
        temp->stack_pointer = NULL;
        free(temp->sp_btm);
        free(temp);
    }
    spinlock_unlock(&done_list_lock);
    free(current_thread->initial_argument); //TODO: get rid of these?
    free(current_thread->sp_btm);
    free(current_thread);
}

void thread_wrap(){

    spinlock_unlock(&ready_list_lock);

    current_thread->initial_function(current_thread->initial_argument);
    current_thread->state = DONE;       //required for queue functionality
    condition_broadcast(&(current_thread->cond));
    yield();
}

void yield(){

    if(current_thread->state == RUNNING || current_thread->state == READY) {
        current_thread->state = READY;
        thread_enqueue(&ready_list, current_thread);
    }
    else if(current_thread->state == BLOCKED && is_empty(&ready_list)){
        panic("Blocking on empty ready list!\n");
    }
    else if (current_thread->state == DONE){
        spinlock_lock(&done_list_lock);
        thread_enqueue(&done_list, current_thread);
        spinlock_unlock(&done_list_lock);
    }

    thread * temp = current_thread;
    set_current_thread( thread_dequeue(&ready_list) );

    if(!current_thread)
        panic("ready_list returned null ptr!\n");

    current_thread->state = RUNNING;

    spinlock_lock(&ready_list_lock);
    thread_switch(temp, current_thread);
    spinlock_unlock(&ready_list_lock);
}


void panic(char arg[]){
    printf("PANIC: %s", arg);
    exit(1);
}


void mutex_init(mutex * mtx){

    mtx->held = 0;
    mtx->waiting_threads.head = mtx->waiting_threads.tail = NULL;
    mtx->waiting_threads.count = 0;
}

void mutex_lock(mutex * mtx){

    spinlock_lock(&(mtx->s));
    if(mtx->held){
        current_thread->state = BLOCKED;
        thread_enqueue(&(mtx->waiting_threads), current_thread);
        //yield();
        block(&(mtx->s));
    }
    else
        mtx->held = 1;
}

void mutex_unlock(mutex * mtx){

    spinlock_lock(&(mtx->s));
    if(!is_empty(&(mtx->waiting_threads))){
        thread * temp = thread_dequeue(&(mtx->waiting_threads));
        temp->state = READY;
        spinlock_lock(&ready_list_lock);
        thread_enqueue(&ready_list, temp);
        spinlock_unlock(&ready_list_lock);
    }
    else{
        mtx->held = 0;
    }
    spinlock_unlock(&(mtx->s));
}

void block(AO_TS_t * spinlock){

    spinlock_lock(&ready_list_lock);
    spinlock_unlock(spinlock);
    if(current_thread->state == RUNNING || current_thread->state == READY) {
        current_thread->state = READY;
        thread_enqueue(&ready_list, current_thread);
    }
    else if(current_thread->state == BLOCKED && is_empty(&ready_list)){
        spinlock_unlock(&ready_list_lock);
        panic("Blocking on empty ready list!\n");
    }
    else if (current_thread->state == DONE){
        spinlock_lock(&done_list_lock);
        thread_enqueue(&done_list, current_thread);
        spinlock_unlock(&done_list_lock);
    }

    thread * temp = current_thread;
    set_current_thread( thread_dequeue(&ready_list) );

    if(!current_thread)
        panic("ready_list returned null ptr!\n");

    current_thread->state = RUNNING;

    thread_switch(temp, current_thread);
    spinlock_unlock(&ready_list_lock);

}

void condition_init(struct condition * cond){

    cond->waiting_threads.head = cond->waiting_threads.tail = NULL;
    cond->waiting_threads.count = 0;
}

void condition_wait(struct condition * cond, struct mutex * mtx){

    spinlock_lock(&(cond->s));

    current_thread->state = BLOCKED;
    thread_enqueue(&(cond->waiting_threads), current_thread);
    mutex_unlock(mtx);
    //yield();
    block(&(cond->s));
    mutex_lock(mtx);

    spinlock_unlock(&(cond->s));
}

void condition_signal(struct condition * cond){
    spinlock_lock(&(cond->s));

    if(!is_empty(&(cond->waiting_threads))){
        thread * temp = thread_dequeue(&(cond->waiting_threads));
        temp->state = READY;
        spinlock_lock(&ready_list_lock);
        thread_enqueue(&ready_list, temp);
        spinlock_unlock(&ready_list_lock);
    }

    spinlock_unlock(&(cond->s));
}

void condition_broadcast(struct condition * cond){
    spinlock_lock(&(cond->s));

    thread * temp = NULL;
    while(!is_empty(&(cond->waiting_threads))){
        temp = thread_dequeue(&(cond->waiting_threads));
        temp->state = READY;
        spinlock_lock(&ready_list_lock);
        thread_enqueue(&ready_list, temp);
        spinlock_unlock(&ready_list_lock);
    }
    spinlock_unlock(&(cond->s));
}

void spinlock_lock(AO_TS_t * lock){
    while(AO_test_and_set_acquire(lock) != AO_TS_SET);
}

void spinlock_unlock(AO_TS_t * lock){
    AO_CLEAR(lock);
}

