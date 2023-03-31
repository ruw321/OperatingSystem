#include "kernel.h"

pcb* k_process_create(pcb * parent) {
    pcb* newPCB = new_pcb(&parent->ucontext, lastPID);
    lastPID++; 
    newPCB->ppid = parent->pid;
    newPCB->priority = 0; // default priority is 0
    newPCB->state = READY;
    newPCB->prev_state = READY;

    return newPCB;
}


int kernel_init() {
    // initialize ready queue
    ready_queue = new_priority_queue();

    // initialize all the queues
    exited_queue = new_pcb_queue();
    stopped_queue = new_pcb_queue();

    stopped_by_timer = false;
    
    if (ready_queue == NULL) {
        perror("error initializing the priority queue");
        return FAILURE;
    }

    lastPID = 1;
    return SUCCESS;
}

int k_process_kill(pcb *process, int signal) {
    if (process->pid == 1) {
        perror("Should not kill shell\n");
        return FAILURE;
    }

    if (signal == SIGSTOP) {
        process->state = STOPPED;

        pcb_queue* cur_queue = get_pcb_queue_by_priority(ready_queue, process->priority);
        dequeue_by_pid(cur_queue, process->pid);
        // TODO: if proc is fg, unblock its parent?
    } else if (signal == SIGCONT) {
        // TODO: special case for sleep
        if (process->state == STOPPED) {
            process->state = READY;
            pcb_node* new_node = new_pcb_node(process);
            enqueue_by_priority(ready_queue, process->priority, new_node);
        }
    } else if (signal == SIGTERM) {
        process->state = TERMINATED;

        pcb_queue* cur_queue = get_pcb_queue_by_priority(ready_queue, process->priority);
        dequeue_by_pid(cur_queue, process->pid);
        // TODO: if proc is fg, unblock its parent?
        // TODO: remove from its parent's children and add to zombies
    }
    return SUCCESS;
}