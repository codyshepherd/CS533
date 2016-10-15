//Cody Shepherd
//CS 533
#define SLICE

#include "scheduler.h"

//function that does some work
void count_down(void * arg);

int main(int argc, char * argv[]){
  scheduler_begin();

  int n1 = 10, n2 = 15, n3 = 20, n4 = 25, n5 = 30, n6 = 35;
  thread_fork(count_down, &n1);
  thread_fork(count_down, &n2);
  thread_fork(count_down, &n3);
  thread_fork(count_down, &n4);
  thread_fork(count_down, &n5);
  thread_fork(count_down, &n6);

  scheduler_end();

    return 0;
}

//decrement argument and print its value
void count_down(void * arg){
    while(*(int*)arg > 0){
        --*(int*)arg;
        printf("%d\n", *(int*)arg);
        #ifdef SLICE
            yield();
        #endif
    }
    #ifndef SLICE
    yield();
    #endif
}

