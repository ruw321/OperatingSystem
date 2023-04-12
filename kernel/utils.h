#ifndef UTILS_H
#define UTILS_H

#include <signal.h>    // sigaction, sigemptyset, sigfillset, signal
#include <stdbool.h>
#include <stdio.h>     
#include <string.h>
#include <stdlib.h>    
#include <sys/time.h>  // setitimer
#include <ucontext.h>  // getcontext, makecontext, setcontext, swapcontext    
#include <time.h>
#include <valgrind/valgrind.h> 
#include "global2.h"
#include "../PennFAT/fd-table.h"

typedef struct pcb {
    ucontext_t ucontext;
    pid_t pid;
    pid_t ppid;     // parent pid
    enum process_state prev_state;
    enum process_state state;       // state of the process
    int priority;
    int input_fd;
    int output_fd;
    FdNode *fds[MAX_FILE_DESCRIPTOR];   // keep track of open FDs
    int ticks_to_reach;     // > 1 represents the wait times, -1 means parent is waiting
    struct pcb_queue* children;     // processes that have not completed yet
    struct pcb_queue* zombies;      // processes that are completed but the parent has not waited for it yet
    char* pname;    // name of the function 
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

pcb_queue* get_pcb_queue_by_priority(priority_queue* ready_queue, int priority);    // return the pointer to the pcb_queue of the required priority

void enqueue(pcb_queue* queue, pcb_node* node);     // Enqueue a new element to the queue
void enqueue_by_priority(priority_queue* ready_queue, int priority, pcb_node* node);
pcb_node* dequeue_by_pid(pcb_queue* queue, pid_t pid);    // Dequeue the element with pid from the queue
pcb_node *dequeue_front(pcb_queue* queue);      // Dequeue the first element from the queue
pcb_node *dequeue_front_by_priority(priority_queue* ready_queue, int priority);    // Dequeue the first element from the queue based on the priority

pcb_node* get_node_by_pid(pcb_queue* queue, pid_t pid);     // Find the element with pid from the queue
pcb_node* get_node_by_pid_from_priority_queue(priority_queue* ready_queue, pid_t pid);   // Find the element with pid from the priority queue

void deconstruct_queue(pcb_queue* queue);   // free the pcb queue
void deconstruct_priority_queue(priority_queue* ready_queue);   // free the pcb priority queue

pcb_node* get_node_from_ready_queue(priority_queue* ready_queue, pid_t pid);     // Find the element with pid from the ready queue

int pick_priority();        // Randomly pick a queue from ready queue based on the priority

void set_stack(stack_t *stack);        // initialize stack for ucontext

int makeContext(ucontext_t *ucp,  void (*func)(), int argc, ucontext_t *next_context, char *argv[]); // initializing context


typedef enum {
    JOB_RUNNING,
    JOB_STOPPED,
    JOB_FINISHED,
    JOB_TERMINATED
} JobState;


typedef enum {
    NICE, // Syntax: nice priority command [args]
    NICE_PID, // Syntax: nice_pid priority pid
    MAN, // Syntax: man
    BG, // Syntax: bg [job_id]
    FG, // Syntax: fg [job_id]
    JOBS, // Syntax: jobs
    LOGOUT, // Syntax: logout
    OTHERS // non-builtin command
} CommandType;

typedef struct Job {
    struct parsed_command *cmd;
    pid_t pid;
    JobState state;
} Job;

typedef struct JobListNode {
    Job *job;
    struct JobListNode *prev;
    struct JobListNode *next;
    int jobId;
} JobListNode;

typedef struct JobList {
    JobListNode *head;
    JobListNode *tail;
    int jobCount;
} JobList;



#endif