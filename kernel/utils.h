#ifndef UTILS_H
#define UTILS_H

#include <signal.h>    // sigaction, sigemptyset, sigfillset, signal
#include <stdbool.h>
#include <stdio.h>     
#include <stdlib.h>    
#include <sys/time.h>  // setitimer
#include <ucontext.h>  // getcontext, makecontext, setcontext, swapcontext    
#include <valgrind/valgrind.h> 

typedef struct pcb {
    ucontext_t ucontext;
    pid_t pid;
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
pcb_node* new_pcb_node(pcb* pcb);       // Function to create a new pcb node with the given data
pcb_queue* new_pcb_queue();     // Function to create an empty pcb queue
priority_queue* new_priority_queue();   // Function to create an empty priority queue

bool is_empty(pcb_queue* queue);     // Function to check if the queue is empty
bool is_priority_queue_empty(priority_queue* ready_queue);

void enqueue(pcb_queue* queue, pcb_node* node);     // Function to enqueue a new element to the queue
int dequeue_by_pid(pcb_queue* queue, pid_t pid);    // Function to dequeue the element with pid from the queue
int dequeue_front(pcb_queue* queue);      // Function to dequeue the first element from the queue
pcb_node* get_node_by_pid(pcb_queue* queue, pid_t pid);     // Function to find the element with pid from the queue


void set_stack(stack_t *stack);        // initialize stack for ucontext

int pick_priority();        // Randomly pick a queue from ready queue based on the priority

#endif