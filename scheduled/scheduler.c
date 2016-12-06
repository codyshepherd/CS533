/*
 * Cody Shepherd
 * CS533 Course Project
 * scheduler.c
 */
#define _GNU_SOURCE

#include <sched.h>
#include "scheduler.h"

const int STACK_SIZE = 1048576;

//struct thread * current_thread;
struct queue ready_list;        // Holds runnable threads
struct queue blocked_list;
struct queue done_list;         // Holds done threads waiting to be recycled


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
}

thread * thread_fork(void(*target)(void*), void * arg){

    thread * forked_thread = NULL;
    thread * temp = NULL;

    while(done_list.count > 9){             //Trim the recycle queue
        printf("Trimming dead queue\n");   
        temp = thread_dequeue(&done_list);
        temp->stack_pointer = NULL;
        free(temp->initial_argument);
        free(temp->sp_btm);
        free(temp);
    }
    temp = NULL;

    if(!is_empty(&done_list)){      // Check on the recyclable list first

        forked_thread = thread_dequeue(&done_list);
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
    thread_enqueue(&ready_list, current_thread);

    //swap out current thread and run it
    temp = current_thread;
    set_current_thread( forked_thread );

    thread_start(temp, forked_thread);
    
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

    while(!is_empty(&ready_list)){
        yield();
    }
    
    thread * temp;
    while(!is_empty(&done_list)){
        temp = thread_dequeue(&done_list);
        temp->stack_pointer = NULL;
        free(temp->sp_btm);
        free(temp);
    }
    free(current_thread->initial_argument); //TODO: get rid of these?
    free(current_thread->sp_btm);
    free(current_thread);
}

void thread_wrap(){

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
        thread_enqueue(&done_list, current_thread);
    }

    thread * temp = current_thread;
    set_current_thread( thread_dequeue(&ready_list) );

    if(!current_thread)
        panic("ready_list returned null ptr!\n");

    current_thread->state = RUNNING;

    thread_switch(temp, current_thread);
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

    if(mtx->held){
        current_thread->state = BLOCKED;
        thread_enqueue(&(mtx->waiting_threads), current_thread);
        yield();
    }
    else
        mtx->held = 1;
}

void mutex_unlock(mutex * mtx){

    if(!is_empty(&(mtx->waiting_threads))){
        thread * temp = thread_dequeue(&(mtx->waiting_threads));
        temp->state = READY;
        thread_enqueue(&ready_list, temp);
    }
    else{
        mtx->held = 0;
    }
}

void condition_init(struct condition * cond){

    cond->waiting_threads.head = cond->waiting_threads.tail = NULL;
    cond->waiting_threads.count = 0;
}

void condition_wait(struct condition * cond, struct mutex * mtx){

    current_thread->state = BLOCKED;
    thread_enqueue(&(cond->waiting_threads), current_thread);
    mutex_unlock(mtx);
    yield();
    mutex_lock(mtx);
}

void condition_signal(struct condition * cond){

    if(!is_empty(&(cond->waiting_threads))){
        thread * temp = thread_dequeue(&(cond->waiting_threads));
        temp->state = READY;
        thread_enqueue(&ready_list, temp);
    }
}

void condition_broadcast(struct condition * cond){

    thread * temp = NULL;
    while(!is_empty(&(cond->waiting_threads))){
        temp = thread_dequeue(&(cond->waiting_threads));
        temp->state = READY;
        thread_enqueue(&ready_list, temp);
    }
}

