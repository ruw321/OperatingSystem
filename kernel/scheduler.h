#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <signal.h>    // sigaction, sigemptyset, sigfillset, signal
#include <stdbool.h>
#include <stdio.h>     
#include <stdlib.h>    
#include <sys/time.h>  // setitimer
#include <ucontext.h>  // getcontext, makecontext, setcontext, swapcontext    
#include <valgrind/valgrind.h> 

#include "utils.h"
#include "global.h"
#include "kernel.h"

extern pcb* active_process;

extern priority_queue* ready_queue;

extern ucontext_t main_context;

extern ucontext_t scheduler_context;

extern ucontext_t* p_active_context;

#define TICK 100000     // 1 tick = 100 milliseconds

void set_stack(stack_t *stack);        // initialize stack for ucontext

int set_alarm_handler();    // register signal handler for SIGALARM
void alarm_handler();       // The signal handler for SIGALARM
int set_timer();   // set up time interval for SIGALARM

pcb* next_process();    // get the next process to run from the ready queue based on priority 

void scheduler();   // The function of scheduler

void idle_func();   // The funciton for the idle process

int scheduler_init();   // initialize the scheduler

int idle_process_init();    // initialize the idle process

#endif