#include "kernel.h"

pcb* k_process_create(pcb * parent) {
    pcb* new_pcb = (pcb *)malloc(sizeof(pcb));
    memset(new_pcb, 0, sizeof(pcb));
    new_pcb->ucontext = parent->ucontext; 
    new_pcb->pid = lastPID; 
    lastPID++; 
    new_pcb->ppid = parent->pid;

    // new_pcb->priority = 0; // default priority is 0
    new_pcb->state = READY; // initial state is ready, and should be pushed to ready queue later 
    return new_pcb;
}

int kernel_init() {
    // initialize ready queue
    ready_queue = new_priority_queue();

    if (ready_queue == NULL) {
        perror("error initializing the priority queue");
        return FAILURE;
    }

    lastPID = 1;
    return SUCCESS;
}