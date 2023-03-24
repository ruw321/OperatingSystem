#include "scheduler.h"

int make_scheduler_context() {
    if (getcontext(&scheduler_context) == -1) {
        perror("Error in getcontext(scheduler_context)\n");
        return -1;
    }

    sigemptyset(&(scheduler_context.uc_sigmask));
    set_stack(&(scheduler_context.uc_stack));
    scheduler_context.uc_link = NULL;

    makecontext(&scheduler_context, scheduler, 0);
    return 0;
}


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
    // TODO: add logic fir sleep command
    swapcontext(p_active_context, &scheduler_context);
}


void set_timer(void)
{
    struct itimerval it;

    it.it_interval = (struct timeval){.tv_usec = TICK};
    it.it_value = it.it_interval;

    setitimer(ITIMER_REAL, &it, NULL);
}


pcb* next_process() {
    if (is_priority_queue_empty(ready_queue)) {
        // TODO: Change to idle process
        return NULL;
    }
    pcb_queue* chosen_queue;
    while(true) {
        int proposal = pick_priority();
        if (proposal == HIGH && !is_empty(ready_queue->high)) {
            chosen_queue = ready_queue->high;
            break;
        } else if (proposal == MID && !is_empty(ready_queue->mid)) {
            chosen_queue = ready_queue->mid;
            break;
        } else if (proposal == LOW && !is_empty(ready_queue->low)) { 
            chosen_queue = ready_queue->low;
            break;
        }
    }

    pcb_node* next_node = chosen_queue->head;
    dequeue_front(chosen_queue);
    return next_node->pcb;
}

void scheduler() {
    //TODO: do necessary clean up

    active_process = next_process();
    p_active_context = &active_process->ucontext;
    setcontext(p_active_context);
}
