//Cody Shepherd
//CS 533
//#define SLICE
//#define TERM

#include <unistd.h>
#include <fcntl.h>
#include "scheduler.h"

//function that does some work
void count_down(void * arg);
void wrap_read_wrap(void * arg);
ssize_t read_wrap(int fd, void * buf, size_t count);

int main(int argc, char * argv[]){
  scheduler_begin();

  #ifdef TERM
  int fd = STDIN_FILENO;
  #else
  int fd = open("testfile.txt", O_RDONLY);
  #endif

  int n1 = 10, n2 = 9, n3 = 8, n4 = 7, n5 = 6, n6 = 35;
  int n7 = 5, n8 = 4, n9 = 3, n10 = 2, n11 = 1, n12 = 11;
  thread_fork(count_down, &n12);
  thread_fork(count_down, &n1);
  thread_fork(count_down, &n2);
  thread_fork(count_down, &n3);
  thread_fork(wrap_read_wrap, &fd);
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

  #ifndef TERM
  close(fd);
  #endif

    return 0;
}

void wrap_read_wrap(void*arg){
    printf("Starting wrap_read_wrap\n");
    int fd = *(int*)arg;
    int sz;

    #ifndef TERM
    lseek(fd, 10, SEEK_SET);
    
    struct stat stt;
    if(!stat("testfile.txt", &stt))
        sz = stt.st_size;
    else return;
    char * buf = malloc(sz);

    #else
    char buf[100];
    sz = 99;
    #endif

    int rd = read_wrap(fd, buf, sz);


    printf("Contents: \n");
    printf("%s", buf);
    printf("Size of file: \n");
    printf("%d\n", sz);
    printf("Return value: \n");
    printf("%d\n", rd);
    
    #ifndef TERM
    free(buf);
    buf = NULL;
    #endif
}

//decrement argument and print its value
void count_down(void * arg){
    printf("Counting down from %d\n", *(int*)arg);
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

