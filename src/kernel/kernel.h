#ifndef KERNEL_H
#define KERNEL_H

#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "utils.h"
#include "global.h"
#include "job.h"
#include "log.h"

extern ucontext_t scheduler_context;
extern pid_t lastPID;
extern priority_queue* ready_queue;
extern bool stopped_by_timer;
extern pcb_queue* exited_queue;
extern pcb_queue* stopped_queue;

extern JobList _jobList;
extern pid_t fgPid;

/**
 * create a new child thread and associated PCB. The new thread should retain much of the properties of the parent. The function should return a reference to the new PCB.
 */   
pcb *k_process_create(pcb * parent);

// kill the process referenced by process with the signal signal.
int k_process_kill(pcb *process, int signal);

// called when a terminated/finished thread’s resources needs to be cleaned up. Such clean-up may include freeing memory, setting the status of the child, etc.
int k_process_cleanup(pcb *process);

// initialize kernel 
int kernel_init();

// deconstruct kernel 
void kernel_deconstruct();

// block a process, add the process to stopped queue
int block_process(pid_t pid);

// unblock a process
int process_unblock(pid_t pid);

int clean_orphan(pcb * process);

#endif