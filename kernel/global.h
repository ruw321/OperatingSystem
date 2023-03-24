#ifndef GLOBAL_H
#define GLOBAL_H

#include "utils.h"

#define HIGH -1
#define MID 0
#define LOW 1

ucontext_t main_context;

ucontext_t scheduler_context;

ucontext_t* p_active_context;

pcb* active_process;

priority_queue* ready_queue;

#endif