#include "kernel.h"

pcb* k_process_create(pcb * parent) {
    pcb* newPCB = new_pcb(&parent->ucontext, lastPID);
    lastPID++; 
    newPCB->ppid = parent->pid;

    newPCB->state = READY; // initial state is ready, and should be pushed to ready queue later
    newPCB->prev_state = READY;

    newPCB->priority = MID; // default priority is 0
    newPCB->input_fd = STDIN_FILENO;
    newPCB->output_fd = STDOUT_FILENO;

    newPCB->children = new_pcb_queue();
    newPCB->zombies = new_pcb_queue();
    
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


void kernel_deconstruct() {
    deconstruct_queue(exited_queue);
    deconstruct_queue(stopped_queue);
}

int k_process_kill(pcb *process, int signal) {
    if (process->pid == 1) {
        perror("Should not kill shell\n");
        return FAILURE;
    }

    if (signal == SIGSTOP) {
        process->prev_state = process->state;
        process->state = STOPPED;

        pcb_queue* cur_queue = get_pcb_queue_by_priority(ready_queue, process->priority);
        dequeue_by_pid(cur_queue, process->pid);
        
        // If this is a foreground process, unblock it's parent
        if (is_foreground(process->pid)) {
            unblock_process(process->ppid);
            setcontext(&scheduler_context);
        }

    } else if (signal == SIGCONT) {
        // TODO: special case for sleep
        // TODO: what should the prev_state be?
        if (process->state == STOPPED) {
            process->state = READY;
            pcb_node* new_node = new_pcb_node(process);
            enqueue_by_priority(ready_queue, process->priority, new_node);
        }
    } else if (signal == SIGTERM) {
        process->prev_state = process->state;
        process->state = TERMINATED;

        pcb_queue* cur_queue = get_pcb_queue_by_priority(ready_queue, process->priority);
        dequeue_by_pid(cur_queue, process->pid);

        //remove from its parent's children and add it to zombies
        pcb_node* parent_node = get_node_by_pid_from_priority_queue(ready_queue, process->ppid);
        if (parent_node == NULL) {
            perror("Parent node should not be NULL.\n");
            return FAILURE;
        }
        pcb* parent_pcb = parent_node->pcb;
        dequeue_by_pid(parent_pcb->children, process->pid);
        pcb_node* p_node = new_pcb_node(process);
        enqueue(parent_pcb->zombies, p_node);

        // If this is a foreground process, unblock it's parent
        if (is_foreground(process->pid)) {
            unblock_process(process->ppid);
            setcontext(&scheduler_context);
        }
    }
    return SUCCESS;
}

int k_process_cleanup(pcb* process) {
    if (process == NULL) {
        perror("The process to be cleanup cannot be NULL.\n");
    }

    // remove it from ready queue
    pcb_queue* cur_queue = get_pcb_queue_by_priority(ready_queue, process->priority);
    dequeue_by_pid(cur_queue, process->pid);

    // clean up zombies
    pcb_node* cur_node = process->zombies->head;
    while (cur_node != NULL) {
        pcb_node* tmp = cur_node;
        cur_node = tmp->next;

        // remove from zombies
        dequeue_front(process->zombies);

        if (k_process_cleanup(tmp->pcb) == FAILURE) {
            perror("Failed to cleanup zombies.\n");
            return FAILURE;
        }
    }

    // clean up children
    cur_node = process->children->head;
    while (cur_node != NULL) {
        pcb_node* tmp = cur_node;
        cur_node = tmp->next;

        // remove from children
        dequeue_front(process->children);

        if (k_process_cleanup(tmp->pcb) == FAILURE) {
            perror("Failed to cleanup children.\n");
            return FAILURE;
        }
    }

    deconstruct_queue(process->zombies);
    deconstruct_queue(process->children);
    return SUCCESS;
}


int block_process(pid_t pid) {
    // find the process in ready queue given its pid
    pcb_node* node = get_node_by_pid_from_priority_queue(ready_queue, pid);
    if (node == NULL) {
        perror("Process is not in the ready queue.\n");
        return FAILURE;
    }

    pcb* cur_pcb = node->pcb;
    cur_pcb->prev_state = READY;
    cur_pcb->state = BLOCKED;

    // remove from ready queue
    pcb_queue *cur_queue = get_pcb_queue_by_priority(ready_queue, cur_pcb->priority);
    dequeue_by_pid(cur_queue, pid);

    // added to stopped queue
    pcb_node* p_node = new_pcb_node(cur_pcb);
    enqueue(stopped_queue, p_node);

    return SUCCESS;
}

bool is_foreground(pid_t pid) {
    return false;
}

int unblock_process(pid_t pid) {
    // find the process in stopped queue given its pid
    pcb_node* node = get_node_by_pid(stopped_queue, pid);
    if (node == NULL) {
        perror("Process is not in the stopped queue.\n");
        return FAILURE;
    }

    pcb* cur_pcb = node->pcb;
    cur_pcb->prev_state = BLOCKED;
    cur_pcb->state = READY;

    // remove from stopped queue  
    dequeue_by_pid(stopped_queue, pid);

    // added to ready queue
    pcb_node* p_node = new_pcb_node(cur_pcb);
    pcb_queue *cur_queue = get_pcb_queue_by_priority(ready_queue, cur_pcb->priority);
    enqueue(cur_queue, p_node);

    return SUCCESS;
}

int process_unblock(pid_t pid) {
    // find the corresponding pcb
    pcb_node* unblock_node = get_node_by_pid(stopped_queue, pid);
    if (unblock_node == NULL) {
        printf("The process you are unblocking doesn't exist in the stopped queue\n");
        return -1;
    }

    unblock_node->pcb->state = READY;
    unblock_node->pcb->prev_state = READY;
    unblock_node->pcb->ticks_to_reach = 0;

    // remove from the stopped queue, and add it back to the ready queue
    if (unblock_node->pcb->state == BLOCKED && unblock_node->pcb->ticks_to_reach < 1) {
        if (dequeue_by_pid(stopped_queue, unblock_node->pcb->pid) == -1) {
            printf("Error with removing the node from the stopped queue\n");
            return -1;
        }
        enqueue_by_priority(ready_queue, unblock_node->pcb->priority, unblock_node);
    }
    return 0;
}