#include "scheduler.h"

static pcb* idle_process;

int set_alarm_handler() {
    struct sigaction act;

    act.sa_handler = alarm_handler;
    act.sa_flags = SA_RESTART;
    sigfillset(&act.sa_mask);

    if (sigaction(SIGALRM, &act, NULL) == -1) {
        perror("Error in set_alarm_handler.\n");
        return -1;
    }
    return 0;
}


void alarm_handler(int signum)
{
    // TODO: add logic for sleep command
    // printf("triggered the alarm\n");
    stopped_by_timer = true;
    tick_tracker++;
    swapcontext(p_active_context, &scheduler_context);
}


int set_timer(void)
{
    struct itimerval it;

    it.it_interval = (struct timeval){.tv_usec = TICK};
    it.it_value = it.it_interval;

    // schedule a SIGALRM signal to be delivered
    if (setitimer(ITIMER_REAL, &it, NULL) == -1) {
        return FAILURE;
    }
    return SUCCESS;
}


pcb* next_process() {
    // printf("choosing the next process\n");
    if (is_priority_queue_empty(ready_queue)) {
        return idle_process;
    }

    pcb_queue* chosen_queue = NULL;
    while(true) {
        int proposal = pick_priority();
        // printf("priority in proposal is: %i\n", proposal);
        if (proposal == HIGH && !is_empty(ready_queue->high)) {
            chosen_queue = ready_queue->high;
        } else if (proposal == MID && !is_empty(ready_queue->mid)) {
            chosen_queue = ready_queue->mid;
        } else if (proposal == LOW && !is_empty(ready_queue->low)) { 
            chosen_queue = ready_queue->low;
        }

        if (chosen_queue == NULL) {
            continue;
        }

        // printf("ticks: %i\n", chosen_queue->head->pcb->ticks_to_reach);
        if (chosen_queue->head->pcb->ticks_to_reach != 0) {
            if ( chosen_queue->head->pcb->ticks_to_reach <= tick_tracker) {
                if (chosen_queue->head->pcb->ticks_to_reach > 0) {
                    process_unblock(chosen_queue->head->pcb->pid);
                }
                return chosen_queue->head->pcb;
            } else {
                // move the head to the tail
                // TODO: there might be a problem of adding the node back in 
                // since the node is already freed

                // pcb_node* temp = chosen_queue->head;
                // dequeue_front(chosen_queue);
                // enqueue(chosen_queue, temp);

                pcb* temp = chosen_queue->head->pcb;
                dequeue_front(chosen_queue);
                pcb_node* temp_node = new_pcb_node(temp);
                enqueue(chosen_queue, temp_node);
            }
        }
        return chosen_queue->head->pcb;
    }
}


void scheduler() {
    // printf("scheduler is running\n");
    // clean up the previous process
    // make sure the current context is not the scheduler context and ready queue is not empty
    if (p_active_context != NULL && !is_priority_queue_empty(ready_queue) && memcmp(p_active_context, &scheduler_context, sizeof(ucontext_t)) != 0) {
        
        // printf("active priority: %d\n", active_process->priority);
        // first remove it from the ready queue
        dequeue_front_by_priority(ready_queue, active_process->priority);
        pcb_node* currNode = new_pcb_node(active_process);

        // setting the previous state
        active_process->prev_state = active_process->state;

        // check how the previous process ended
        if (stopped_by_timer) {
            // printf("process is stopped by the timer\n");

            // since the process hasn't completed yet, we add it back to the ready queue
            enqueue_by_priority(ready_queue, active_process->priority, currNode);
            stopped_by_timer = false;
        } else {
            // check whether the process is completed or blocked or stopped
            if (active_process->ticks_to_reach <= tick_tracker && active_process->state == RUNNING) {
                // process completed, add it to the exit queue
                enqueue(exited_queue, currNode);
                // printf("process is finished (not stopped by the timer)\n");
                //TODO: if the process exited either normally or by signal
                if (true) {
                    // exited normally
                    active_process->state = TERMINATED;
                } else {
                    // stopped by signal
                    active_process->state = TERMINATED;
                }
                pcb_node* parent = get_node_by_pid_all_queues(active_process->ppid);
                if (parent != NULL) {
                    // if the parent is blocked waiting for it, unblock the parent
                    if (parent->pcb->ticks_to_reach == -1) {
                        if (process_unblock(active_process->ppid) == -1) {
                            // printf("failed to unblock the parent\n");
                        }
                    }
                    // remove the node from children queue and add it to the zombies queue
                    dequeue_by_pid(parent->pcb->children, active_process->pid);
                    enqueue(parent->pcb->zombies, currNode);
                } else {
                    // printf("Parent node is not supposed to be null\n");
                }
                // TODO: orphan clean ups
                // k_process_cleaup_orphan(active_process);
            } else {
                // if it is BLOCKED or STOPPED
                if (active_process->ticks_to_reach > 0) {
                    // BLOCKED by p_sleep 
                    enqueue_by_priority(ready_queue, active_process->priority, currNode);
                } else {
                    // BLOCKED by p_waitpid, add it to the stopped queue
                    enqueue(stopped_queue, currNode);
                }
                // TODO: deal with processes stopped by signals
            }
        }
    }

    p_active_context = &scheduler_context;
    active_process = next_process();
    active_process->prev_state = active_process->state;
    active_process->state = RUNNING;
    // printf("next selected process id: %i\n", active_process->pid);
    p_active_context = &active_process->ucontext;
    setcontext(p_active_context);
}


int scheduler_init() {

    printf("Initializing scheduler\n");

    // initialize context for the scheduler 
    if (makeContext(&scheduler_context, scheduler, 0, NULL, NULL) == FAILURE) {
        return FAILURE;
    }

    printf("initialized context for the scheduler \n");

    // catch the SIGALRM signal
    if (set_alarm_handler() == FAILURE) {
        return FAILURE;
    }

    printf("set the SIGALRM signal handler\n");

    // schedule a SIGALRM signal to your OS every 100 milliseconds
    if (set_timer() == FAILURE) {
        return FAILURE;
    }

    printf("set the timer\n");

    // initialize the idle process
    if (idle_process_init() == FAILURE) {
        return FAILURE;
    }

    printf("initialized idle process\n");

    return SUCCESS;
}


// suspend the idle process until a signal is delivered to it
void idle_func() {
    sigset_t mask;
    // initializes the signal mask to be an empty set
    if (sigemptyset(&mask) == -1) {
        perror("Failed to initialize signal mask in idle_func");
        return;
    }
    sigsuspend(&mask);
}


int idle_process_init() {
    //malloc space and initialize attributes
    idle_process = malloc(sizeof(pcb));
    if (makeContext(&idle_process->ucontext, idle_func, 0, NULL, NULL) == -1) {
        return FAILURE;
    }
    idle_process->pid = 0;
    idle_process->ppid = 0;
    idle_process->state = READY;

    return SUCCESS;
}

pcb_node* get_node_by_pid_all_queues(pid_t pid) {
    pcb_node* ready_node = get_node_from_ready_queue(ready_queue, pid);
    if (ready_node == NULL) {
        pcb_node* stop_node = get_node_by_pid(stopped_queue, pid);
        if (stop_node == NULL) {
            pcb_node* exit_node = get_node_by_pid(exited_queue, pid);
            return exit_node;
        } else {
            return stop_node;
        }
    } else {
        return ready_node;
    }
}