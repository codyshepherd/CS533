//Cody Shepherd
//CS 533

#include <stdlib.h>
#include <stdio.h>

//stack size = 1 MB (1024 ^ 2 bytes)
const int STACK_SIZE = 1048576;

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

//global thread pointer
thread * current_thread;

//global ptr for inactive thread
thread * inactive_thread;

//function that does some work
void count_down(void * arg);

//thread switching function implemented in assembly
void thread_switch(thread * old, thread * new);

//switch to new thread function
void thread_start(thread * old, thread * new);

//initial function caller wrapper
void thread_wrap();

//swap current thread and inactive thread
void yield();



int main(int argc, char * argv[]){

    //Allocate 'current' thread
    //allocate a struct in memory
    current_thread = (thread*)malloc(sizeof(thread));

    //set the fn ptr to point at our dumb function
    current_thread->initial_function = count_down;

    //give the initial arg ptr something to point at
    int * p = malloc(sizeof(int));
    *p = 666;
    current_thread->initial_argument = p;

    //allocate the thread stack on the heap
    current_thread->stack_pointer = malloc(STACK_SIZE) + STACK_SIZE;

    //Allocate 'inactive' thread space
    //allocate struct in memory
    inactive_thread = (thread*)malloc(sizeof(thread));

    //start new thread and initiate work
    thread_start(inactive_thread, current_thread);

    //do some work in main thread with shared resource
    count_down(p);

    return 0;
}

//decrement argument and print its value, yielding at bottom of each loop
void count_down(void * arg){
    while(*(int*)arg > 0){
        --*(int*)arg;
        printf("%d\n", *(int*)arg);
        yield();
    }
}

void thread_wrap(){
    current_thread->initial_function(current_thread->initial_argument);
    yield();
}

void yield(){
    thread * temp = current_thread;
    current_thread = inactive_thread;
    inactive_thread = temp;
    thread_switch(inactive_thread, current_thread);
}










