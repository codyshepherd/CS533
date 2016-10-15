/*
 * Cody Shepherd
 * CS533 Course Project
 * scheduler.c
 */
#include "scheduler.h"

const int STACK_SIZE = 1048576;

struct thread * current_thread;
struct queue ready_list;        // Holds runnable threads
struct queue done_list;         // Holds done threads waiting to be recycled


void scheduler_begin(){
    printf("Allocating first TCB\n");

    //allocate a struct for when the main thread gets switched out
    current_thread = (thread*)malloc(sizeof(thread));
    current_thread->state = RUNNING;

    //null-initialize the queue pointers, because they're empty
    ready_list.head = ready_list.tail = NULL;
    done_list.head = done_list.tail = NULL;
    ready_list.count = done_list.count = 0;
}

void thread_fork(void(*target)(void*), void * arg){
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

        printf("Allocating new TCB\n");

        forked_thread = (thread*)malloc(sizeof(thread));
        forked_thread->initial_function = target;
        forked_thread->initial_argument = (int*)malloc(sizeof(int));
        forked_thread->sp_btm = malloc(STACK_SIZE);

    }

    *(int*)(forked_thread->initial_argument) = *(int*)arg;

    forked_thread->stack_pointer = forked_thread->sp_btm + STACK_SIZE;
    forked_thread->state = RUNNING;

    //sleep currently running thread
    current_thread->state = READY;
    
    //enqueue currently running thread
    thread_enqueue(&ready_list, current_thread);

    //swap out current thread and run it
    temp = current_thread;
    current_thread = forked_thread;

    thread_start(temp, forked_thread);
}

void scheduler_end(){

    while(!is_empty(&ready_list)){
        yield();
    }
    
    thread * temp;
    while(!is_empty(&done_list)){
        temp = thread_dequeue(&done_list);
        temp->stack_pointer = NULL;
        free(temp->initial_argument);
        free(temp->sp_btm);
        free(temp);
    }
    free(current_thread->initial_argument);
    free(current_thread->sp_btm);
    free(current_thread);
}

void thread_wrap(){
    current_thread->initial_function(current_thread->initial_argument);
    current_thread->state = DONE;       //required for queue functionality
    yield();
}

void yield(){
    if(current_thread->state != DONE){
        current_thread->state = READY;
        thread_enqueue(&ready_list, current_thread);
    }
    else{
        thread_enqueue(&done_list, current_thread);
    }

    thread * temp = current_thread;
    current_thread = thread_dequeue(&ready_list);
    current_thread->state = RUNNING;

    thread_switch(temp, current_thread);
}



