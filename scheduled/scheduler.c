/*
 * Cody Shepherd
 * CS533 Course Project
 * scheduler.c
 */

#include "scheduler.h"

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










