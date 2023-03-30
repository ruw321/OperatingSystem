#include "kernel.h"
#include "utils.h"
#include "global.h"
// Works as a wrapper to the kernel functions so that users can use these functions

extern priority_queue* ready_queue;
extern pcb* active_process;
extern ucontext_t scheduler_context;

/**
* forks a new thread that retains most of the attributes of the parent thread (see k_process_create).
**/
pid_t p_spawn(void (*func)(), char *argv[], int fd0, int fd1);

// sets the calling thread as blocked (if nohang is false) until a child of the calling thread changes state
pid_t p_waitpid(pid_t pid, int *wstatus, bool nohang);

// sends the signal sig to the thread referenced by pid. It returns 0 on success, or -1 on error.
int p_kill(pid_t pid, int sig);

// exits the current thread unconditionally
void p_exit(void);

// sets the priority of the thread pid to priority
int p_nice(pid_t pid, int priority);

// sets the calling process to blocked until ticks of the system clock elapse, and then sets the thread to running.
void p_sleep(unsigned int ticks);