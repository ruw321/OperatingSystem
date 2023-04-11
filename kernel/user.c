#include "user.h"

pid_t p_spawn(void (*func)(), char *argv[], int fd0, int fd1) {
    // forks a new thread that retains most of the attributes of the parent thread 
    pcb* pcb = k_process_create(active_process);
    if (pcb == NULL) {
        perror("failed to create pcb");
        return -1;
    }

    // copy parent's fd table
    for (int fd_idx = 0; fd_idx < MAX_FILE_DESCRIPTOR; fd_idx ++) {
        FdNode *parent_fnode = (active_process->fds)[fd_idx];
        (pcb->fds)[fd_idx] = parent_fnode;
    }
    
    // redirection of file descriptors
    if (fd0 != STDIN_FILENO) {
        FdNode *dst_node = pcb->fds[fd0];
        if (dst_node == NULL) {
            printf("[Error] Input file not opened\n");
            return FAILURE;
        }
        
        pcb->fds[STDIN_FILENO] = dst_node;
    }
    
    if (fd1 != STDOUT_FILENO) {
        FdNode *dst_node = pcb->fds[fd1];
        if (dst_node == NULL) {
            printf("[Error] Output file not opened\n");
            return FAILURE;
        }
        
        pcb->fds[STDOUT_FILENO] = dst_node;
    }

    // we might not need this:
    int num_args = 0;
    while (argv[num_args] != NULL) {
        num_args++;
    }

    // executes the function referenced by func with its argument array argv. 
    // TODO: change it to the number of arguments instead of 1
    makeContext(&(pcb->ucontext), func, num_args, &scheduler_context, argv);

    // assign process name
    pcb->pname = malloc(sizeof(char) * (strlen(argv[0]) + 1));
    strcpy(pcb->pname, argv[0]);    // the first arg is the name of the func

    pcb_node* newNode = new_pcb_node(pcb);
    // default priority level is 0
    enqueue(ready_queue->mid, newNode);
    // add to the children list for the parent
    enqueue(active_process->children, newNode);
    log_event(pcb, "CREATE");
    return pcb->pid;
}

void cleanup(pcb_queue* queue, pcb_node* child) {
    if (!is_empty(queue) && child != NULL) {
        if (child->pcb->state == TERMINATED) {
            // clean up the child process
            dequeue_by_pid(queue, child->pcb->pid);
            //k_process_cleanup(child->pcb);
        }
    } else {
        printf("parent queue and child are both not supposed to be null\n");
    }
}

pid_t wait_for_one(pid_t pid, int *wstatus) {
    pcb* parent = active_process; // the calling thread
    // check the zombie first 
    pcb_node* zombie = get_node_by_pid(parent->zombies, pid); 

    // then check if it is in children 
    pcb_node* child = get_node_by_pid(parent->children, pid); 

    if (zombie != NULL) {
        // set the status 
        if (wstatus != NULL) {
            *wstatus = zombie->pcb->state;
        }
        // clean up the zombie process
        cleanup(parent->zombies, zombie);
        return pid;
    }
    if (child == NULL) {
        printf("Error: cannot find a child with this pid from the calling thread\n");
        return -1;
    }
    // check if the state has changed
    if (child->pcb->prev_state != child->pcb->state) {
        child->pcb->prev_state = child->pcb->state;
        if (wstatus != NULL) {
            *wstatus = child->pcb->state;
        }
        return pid;
    }
    // if WNOHANG was specified and one or more child(ren) specified by pid exist, but have not yet changed state, then 0 is returned.
    return 0;
}

pid_t wait_for_anyone(pid_t pid, int *wstatus) {
    // if zombie queue is not empty, then we return the first zombie
    if (!is_empty(active_process->zombies)) {
        pcb_node* zombie_node = active_process->zombies->head;
        pid_t zombiePID = zombie_node->pcb->pid;
        zombie_node->pcb->prev_state = zombie_node->pcb->state;
        // set the status
        if (wstatus != NULL) {
            *wstatus = zombie_node->pcb->state;
        }
        cleanup(active_process->zombies, zombie_node);
        return zombiePID;
    }
    // then we traverse through the children 
    pcb_queue* children = active_process->children;
    if (!is_empty(children)) {
        for (pcb_node* child = children->head; child != children->tail; child = child->next) {
            if (child->pcb->prev_state != child->pcb->state) {
                child->pcb->prev_state = child->pcb->state;
                // set the status
                if (wstatus != NULL) {
                    *wstatus = child->pcb->state;
                }
                return child->pcb->pid;
            }
        }
    }
    return 0;
}

pid_t p_waitpid(pid_t pid, int *wstatus, bool nohang) {
    // if there is no children to wait for, return -1
    if (is_empty(active_process->children) && is_empty(active_process->zombies)) {
        return -1;
    }

    if (pid > 0) {  // a particule process

        if (nohang) {   
            // non blocking
            return wait_for_one(pid, wstatus);
        } else {
            // block the calling thread
            // TODO: still dk how this will block the process
            active_process->prev_state = BLOCKED;
            active_process->state = BLOCKED;
            // this is how children would know parent is waiting
            active_process->ticks_to_reach = -1;    

            // switch context to scheduler 
            stopped_by_timer = false;
            swapcontext(&active_process->ucontext, &scheduler_context);

            // at this point, the parent process should be unblocked
            pid_t result = wait_for_one(pid, wstatus);

            if (result == 0) {
                printf("cannot 0, should return pid instead because nohang is false\n");
                return -1;
            }
            return result;
        }
    } else {
        // pid == -1
        if (pid == -1) {

            if (nohang) {
                return wait_for_anyone(pid, wstatus);
            } else {
                // block parent, remove it from the ready queue and switch context
                active_process->prev_state = BLOCKED;
                active_process->state = BLOCKED;
                stopped_by_timer = true;
                swapcontext(&active_process->ucontext, &scheduler_context);

                pid_t result = wait_for_anyone(pid, wstatus);

                if (result == 0) {
                    printf("cannot 0, should return pid instead because nohang is false\n");
                    return -1;
                }
                return result;
            }
        } else {
            printf("the pid is not greater than 0 or -1, error!\n");
            return -1;
        }
    }
    return -1;
}

int p_kill(pid_t pid, int sig) {
    // TODO: search by pid in all queues
    pcb_node* target_node = get_node_by_pid_all_alive_queues(pid);
    if (target_node == NULL) {
        printf("target node with the pid %i does not exist\n", pid);
        return -1;
    }
    return k_process_kill(target_node->pcb, sig);
}

pcb_node* get_node_by_pid_all_alive_queues(pid_t pid) {
    pcb_node* ready_node = get_node_from_ready_queue(ready_queue, pid);
    if (ready_node == NULL) {
        pcb_node* stop_node = get_node_by_pid(stopped_queue, pid);
        return stop_node;
    } else {
        return ready_node;
    }
}

void p_exit(void) {
    // k_process_cleanup(active_process);
    // check if it the first process (shell)
    if (active_process->pid == 1) {
        // TODO: clean up the shell's memory
    }
}

int p_nice(pid_t pid, int priority) {
    if (priority < -1 || priority > 1) {
        printf("Priority has to be 1, 0, or -1\n");
        return -1;
    }
    // check if it is in the ready queue
    pcb_node* target_node = get_node_from_ready_queue(ready_queue, pid);
    if (target_node == NULL) {
        // if it is not, just update the priority 
        target_node = get_node_by_pid(stopped_queue, pid);
        if (target_node == NULL) {
            printf("node with the pid %i does not exist\n", pid);
            return -1;
        }
        target_node->pcb->priority = priority;
    } else {
        pcb* target_pcb = target_node->pcb;
        // if it is, change the queue if necessary
        if (target_pcb->priority != priority) {
            // change the queue
            pcb_queue* orginal_queue = get_pcb_queue_by_priority(ready_queue, target_pcb->priority);
            enqueue_by_priority(ready_queue, priority, dequeue_by_pid(orginal_queue, pid));
        } 
    }

    return pid;
}

void p_sleep(unsigned int ticks) {
    if (ticks < 1) {
        printf("ticks has to be greater than 1\n");
        return;
    }
    // sets the calling process to blocked until ticks of the system clock elapse
    // and then sets the thread to running 
    active_process->prev_state = BLOCKED;
    active_process->state = BLOCKED;
    active_process->ticks_to_reach = tick_tracker + ticks;
    swapcontext(&active_process->ucontext, &scheduler_context);
}