#include "global.h"

// global variables

pcb* active_process;

priority_queue* ready_queue;

ucontext_t main_context;

ucontext_t scheduler_context;

ucontext_t* p_active_context;

pid_t lastPID;