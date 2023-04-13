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
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGALRM);
    sigprocmask(SIG_BLOCK, &mask, NULL);

    if (process->pid == 1) {
        perror("Should not kill shell\n");
        sigprocmask(SIG_UNBLOCK, &mask, NULL);
        return FAILURE;
    }

    if (signal == S_SIGSTOP) {
        process->prev_state = process->state;
        process->state = STOPPED;

        // remove process from ready queue if existed
        pcb_queue* cur_queue = get_pcb_queue_by_priority(ready_queue, process->priority);
        pcb_node *node = dequeue_by_pid(cur_queue, process->pid);
        if (node != NULL) {
            enqueue(stopped_queue, node);
        }

        // If this is a foreground process, unblock it's parent
        if (process->pid == fgPid) {
            process_unblock(process->ppid);
            // TODO: stopped by timer
            setcontext(&scheduler_context);
        }

    } else if (signal == S_SIGCONT) {
        if (process->state == STOPPED) {
            if (strcmp(process->pname, "sleep") == 0) {
                process->prev_state = BLOCKED;
                process->state = BLOCKED;
            } else {
                process->prev_state = READY;
                process->state = READY;
                pcb_node* new_node = dequeue_by_pid(stopped_queue, process->pid);
                if (new_node == NULL) {
                    perror("new_node should not be NULL.\n");

                } else {
                    enqueue_by_priority(ready_queue, process->priority, new_node);
                }
            }       
        }
    } else if (signal == S_SIGTERM) {

        process->prev_state = process->state;
        process->state = TERMINATED;

        pcb_node *tmpNode;
        if (strcmp(process->pname, "sleep") == 0) {
            tmpNode = dequeue_by_pid(stopped_queue, process->pid);
        } else {
            pcb_queue* cur_queue = get_pcb_queue_by_priority(ready_queue, process->priority);
            tmpNode = dequeue_by_pid(cur_queue, process->pid);
        }

        enqueue(exited_queue, tmpNode);


        //remove from its parent's children and add it to zombies
        pcb_node* parent_node = get_node_by_pid_all_queues(process->ppid);
        if (parent_node == NULL) {
            perror("Parent node should not be NULL.\n");
            sigprocmask(SIG_UNBLOCK, &mask, NULL);
            return FAILURE;
        }
        // printf("parent is %d\n", parent_node->pcb->pid);
        log_event(process, "ZOMBIE");
        pcb* parent_pcb = parent_node->pcb;
        pcb_node* p_node = dequeue_by_pid(parent_pcb->children, process->pid);
        
        enqueue(parent_pcb->zombies, p_node);

        clean_orphan(process);

        // If this is a foreground process, unblock it's parent
        if (process->pid == fgPid) {
            process_unblock(process->ppid);
            // TODO: stopped by timer
            setcontext(&scheduler_context);
        }
    }
    sigprocmask(SIG_UNBLOCK, &mask, NULL);
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
    free(process->pname);
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
    cur_pcb->prev_state = BLOCKED;
    cur_pcb->state = BLOCKED;

    // remove from ready queue
    pcb_queue *cur_queue = get_pcb_queue_by_priority(ready_queue, cur_pcb->priority);
    pcb_node* p_node = dequeue_by_pid(cur_queue, pid);

    enqueue(stopped_queue, p_node);

    log_event(cur_pcb, "BLOCKED");

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

    // remove from the stopped queue, and add it back to the ready queue
    if (cur_pcb->state == BLOCKED) {
        pcb_node* p_node = dequeue_by_pid(stopped_queue, cur_pcb->pid);

        cur_pcb->prev_state = cur_pcb->state;
        cur_pcb->state = READY;
        
        cur_pcb->ticks_left = 0; // to indicate that parent no longer waits for its child

        printf("adding the node back to the ready queue: %i\n", cur_pcb->pid);
        enqueue_by_priority(ready_queue, cur_pcb->priority, p_node);
    }
    return SUCCESS;
}

int process_unblock(pid_t pid) {
    // printf("Unblocking process %d\n", pid);
    // find the corresponding pcb
    pcb_node* unblock_node = get_node_by_pid(stopped_queue, pid);
    if (unblock_node == NULL) {
        printf("The process you are unblocking doesn't exist in the stopped queue\n");
        return -1;
    }

    // remove from the stopped queue, and add it back to the ready queue
    if (unblock_node->pcb->state == BLOCKED) {

        pcb_node* tempNode = dequeue_by_pid(stopped_queue, unblock_node->pcb->pid);

        // printf("removing the node from stopped queue: %i\n", unblock_node->pcb->pid);

        tempNode->pcb->prev_state = READY;
        tempNode->pcb->state = READY;
        tempNode->pcb->ticks_left = 0; // to indicate that parent no longer waits for its child

        // printf("adding the node back to the ready queue: %i\n", tempNode->pcb->pid);
        enqueue_by_priority(ready_queue, tempNode->pcb->priority, tempNode);

        log_event(unblock_node->pcb, "UNBLOCKED");
    }
    
    return 0;
}

int clean_orphan(pcb * process) {

    pcb_node * cur_node = process->children->head;

    while (cur_node != NULL) {
        pcb_node* tmp = cur_node;
        cur_node = tmp->next;

        log_event(tmp->pcb, "ORPHAN");
        // remove from children
        dequeue_front(process->children);
        
        pcb_node* p_node = dequeue_by_pid(stopped_queue, tmp->pcb->pid);
        if (p_node == NULL) {
            pcb_queue *cur_queue = get_pcb_queue_by_priority(ready_queue, tmp->pcb->priority);
            dequeue_by_pid(cur_queue, tmp->pcb->pid);
        }


        if (k_process_cleanup(tmp->pcb) == FAILURE) {
            perror("Failed to cleanup children.\n");
            return -1;
        }
    }

    cur_node = process->zombies->head;
    
    while (cur_node != NULL) {
        pcb_node* tmp = cur_node;
        cur_node = tmp->next;

        log_event(tmp->pcb, "ORPHAN");
        // remove from children
        dequeue_by_pid(stopped_queue, tmp->pcb->pid);
        dequeue_front(process->zombies);

        if (k_process_cleanup(tmp->pcb) == FAILURE) {
            perror("Failed to cleanup children.\n");
            return -1;
        }
    }

    return 0;
}