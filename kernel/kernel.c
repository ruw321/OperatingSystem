
pcb* k_process_create(pcb * parent) {
    pcb* pcb = (pcb *)malloc(sizeof(pcb));
    memset(pcb, 0, sizeof(pcb));
    pcb->ucontext = parent->ucontext; 
    pcb->pid = lastPID; 
    lastPID++; 
    pcb->ppid = parent->pid;

    pcb->priority = 0; // default priority is 0
    pcb->state = READY; // initial state is ready, and should be pushed to ready queue later 
    return pcb;
}

int kernel_init() {
    // initialize ready queue
    priorityQueue = malloc(sizeof(priority_queue));
    if (priorityQueue == NULL) {
        perror("error mallocing for priority queue");
        return FAILURE;
    }
    queue_init(&ready_queue->high);
    queue_init(&ready_queue->middle);
    queue_init(&ready_queue->low);

    lastPID = 1
    return SUCCESS;
}