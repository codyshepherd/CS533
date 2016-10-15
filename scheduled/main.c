//Cody Shepherd
//CS 533
#define SLICE

#include "scheduler.h"

//function that does some work
void count_down(void * arg);

int main(int argc, char * argv[]){
  scheduler_begin();

  int n1 = 10, n2 = 9, n3 = 8, n4 = 7, n5 = 6, n6 = 35;
  int n7 = 5, n8 = 4, n9 = 3, n10 = 2, n11 = 1, n12 = 11;
  thread_fork(count_down, &n12);
  thread_fork(count_down, &n1);
  thread_fork(count_down, &n2);
  thread_fork(count_down, &n3);
  thread_fork(count_down, &n4);
  thread_fork(count_down, &n5);
  thread_fork(count_down, &n7);
  thread_fork(count_down, &n8);
  thread_fork(count_down, &n9);
  thread_fork(count_down, &n10);
  thread_fork(count_down, &n11);
  thread_fork(count_down, &n6);
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

