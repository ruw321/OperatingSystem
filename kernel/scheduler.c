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
    printf("triggered the alarm\n");
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
    printf("choosing the next process\n");
    if (is_priority_queue_empty(ready_queue)) {
        return idle_process;
    }

    pcb_queue* chosen_queue = NULL;
    while(true) {
        int proposal = pick_priority();
        printf("priority in proposal is: %i\n", proposal);
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

        if (chosen_queue->head->pcb->ticks_to_reach <= tick_tracker) {
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
}


void scheduler() {

    // clean up the previous process
    // make sure the current context is not the scheduler context and ready queue is not empty
    if (p_active_context != NULL && !is_priority_queue_empty(ready_queue) && memcmp(p_active_context, &scheduler_context, sizeof(ucontext_t)) != 0) {

        // first remove it from the ready queue
        dequeue_front_by_priority(ready_queue, active_process->priority);
        pcb_node* currNode = new_pcb_node(active_process);

        // setting the previous state
        active_process->prev_state = active_process->state;

        // check how the previous process ended
        if (stopped_by_timer) {
            printf("process is stopped by the timer\n");

            // since the process hasn't completed yet, we add it back to the ready queue
            enqueue_by_priority(ready_queue, active_process->priority, currNode);
            stopped_by_timer = false;
        } else {
            // check whether the process is completed or blocked or stopped
            if (active_process->ticks_to_reach <= tick_tracker && active_process->state == RUNNING) {
                // process completed, add it to the exit queue
                enqueue(exited_queue, currNode);
                printf("process is finished (not stopped by the timer)\n");
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
                            printf("failed to unblock the parent\n");
                        }
                    }
                    // remove the node from children queue and add it to the zombies queue
                    dequeue_by_pid(parent->pcb->children, active_process->pid);
                    enqueue(parent->pcb->zombies, currNode);
                } else {
                    printf("Parent node is not supposed to be null\n");
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
    printf("next selected process id: %i\n", active_process->pid);
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

void foo() {
    printf("In foo\n");
    sleep(2);
    printf("after sleep\n");
}

void bar() {
    printf("In bar\n");
}

int main(int argc, char const *argv[])
{
    // initialize process queue
    if (kernel_init() == -1) {
        perror("error with initializing kernel level\n");
    }
    printf("initializing context for testing\n");
    ucontext_t ctx1, ctx2, ctx3;
    // char stack1[8192], stack2[8192];
    // Initialize context 1
    getcontext(&ctx1);
    sigemptyset(&(ctx1.uc_sigmask));
    set_stack(&(ctx1.uc_stack));
    ctx1.uc_link = &scheduler_context;

    // ctx1.uc_stack.ss_sp = stack1;
    // ctx1.uc_stack.ss_size = sizeof(stack1);
    // ctx1.uc_link = &scheduler_context;
    makecontext(&ctx1, foo, 0);

    // Initialize context 2
    getcontext(&ctx2);
    sigemptyset(&(ctx2.uc_sigmask));
    set_stack(&(ctx2.uc_stack));
    ctx2.uc_link = &scheduler_context;
    makecontext(&ctx2, bar, 0);

    // // Initialize context 3
    getcontext(&ctx3);
    sigemptyset(&(ctx3.uc_sigmask));
    set_stack(&(ctx3.uc_stack));
    ctx3.uc_link = &scheduler_context;
    makecontext(&ctx3, bar, 0);

    printf("initializing PCBs for testing\n");
    pcb* newPCB = (pcb *) malloc(sizeof(pcb));
    newPCB->ucontext = ctx1;
    newPCB->pid = 1;
    newPCB->ppid = 4;
    newPCB->state = READY;
    newPCB->priority = 0;
    pcb_node* newNode = new_pcb_node(newPCB);

    pcb* newPCB2 = (pcb *) malloc(sizeof(pcb));
    newPCB2->ucontext = ctx2;
    newPCB2->pid = 2;
    newPCB2->ppid = 4;
    newPCB2->state = READY;
    newPCB2->priority = 0;
    pcb_node* newNode2 = new_pcb_node(newPCB2);

    // pcb* newPCB3 = (pcb *) malloc(sizeof(pcb));
    // newPCB3->ucontext = ctx2;
    // newPCB3->pid = 3;
    // newPCB3->ppid = 4;
    // newPCB3->state = READY;
    // newPCB3->priority = 1;
    // pcb_node* newNode3 = new_pcb_node(newPCB3);

    // add this process to the process queue
    enqueue_by_priority(ready_queue, MID, newNode);
    enqueue_by_priority(ready_queue, MID, newNode2);
    // enqueue_by_priority(ready_queue, HIGH, newNode3);

    scheduler_init();
    swapcontext(&main_context, &scheduler_context);


    return 0;
}