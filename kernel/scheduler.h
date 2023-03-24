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

#define TICK 100000     // 1 tick = 100 milliseconds


int make_scheduler_context();     // init and make the context for scheduler

int set_alarm_handler();    // register signal handler for SIGALARM
void alarm_handler();       // The signal handler for SIGALARM
void set_timer();   // set up time interval for SIGALARM

pcb* next_process();    // get the next process to run from the ready queue based on priority 

void scheduler();   // The function of scheduler

#endif