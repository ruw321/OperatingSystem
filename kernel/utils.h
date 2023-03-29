#ifndef UTILS_H
#define UTILS_H

#include <signal.h>    // sigaction, sigemptyset, sigfillset, signal
#include <stdbool.h>
#include <stdio.h>     
#include <stdlib.h>    
#include <sys/time.h>  // setitimer
#include <ucontext.h>  // getcontext, makecontext, setcontext, swapcontext    
#include <valgrind/valgrind.h> 
#include "global2.h"

typedef struct pcb {
    ucontext_t ucontext;
    pid_t pid;
    pid_t ppid;     // parent pid
    enum process_state state;       // state of the process
    // open file descriptors
    // TODO: other fields to be added
} pcb;

typedef struct pcb_node {
    pcb* pcb;
    struct pcb_node* next;
} pcb_node;

typedef struct pcb_queue {
    pcb_node* head;
    pcb_node* tail;
} pcb_queue;

typedef struct priority_queue {  
    pcb_queue* high;    // priority -1
    pcb_queue* mid;     // priority 0
    pcb_queue* low;     // priority 1
} priority_queue;

pcb* new_pcb(ucontext_t* ucontext, pid_t pid);
pcb_node* new_pcb_node(pcb* pcb);       // Create a new pcb node with the given data
pcb_queue* new_pcb_queue();     // Create an empty pcb queue
priority_queue* new_priority_queue();   // Create an empty priority queue

bool is_empty(pcb_queue* queue);     // Check if the queue is empty
bool is_priority_queue_empty(priority_queue* ready_queue);

void enqueue(pcb_queue* queue, pcb_node* node);     // Enqueue a new element to the queue
int dequeue_by_pid(pcb_queue* queue, pid_t pid);    // Dequeue the element with pid from the queue
int dequeue_front(pcb_queue* queue);      // Dequeue the first element from the queue
pcb_node* get_node_by_pid(pcb_queue* queue, pid_t pid);     // Find the element with pid from the queue

int pick_priority();        // Randomly pick a queue from ready queue based on the priority

void queue_init(pcb_queue** queue);

#endif