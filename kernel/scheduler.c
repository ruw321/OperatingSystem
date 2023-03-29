#include "scheduler.h"

static pcb* idle_process;

void set_stack(stack_t *stack)
{
    void *sp = malloc(SIGSTKSZ);
    VALGRIND_STACK_REGISTER(sp, sp + SIGSTKSZ);

    *stack = (stack_t) { .ss_sp = sp, .ss_size = SIGSTKSZ };
}

int makeContext(ucontext_t *ucp,  void (*func)()) {
    // intialize the context
    if (getcontext(ucp) == -1) {
        perror("Error in getcontext(context)\n");
        return FAILURE;
    }

    sigemptyset(&ucp->uc_sigmask);
    set_stack(&ucp->uc_stack);
    ucp->uc_link = NULL;

    // set up the stack and instruction pointer for the context
    makecontext(ucp, func, 0);
    return SUCCESS;
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
    // TODO: add logic for sleep command
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
    if (is_priority_queue_empty(ready_queue)) {
        return idle_process;
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
    p_active_context = &scheduler_context;

    //TODO: block sigalrm

    active_process = next_process();
    printf("next selected process id: %i", active_process->pid);
    p_active_context = &active_process->ucontext;
    setcontext(p_active_context);
}


int scheduler_init() {

    printf("Initializing scheduler\n");

    // initialize context for the scheduler 
    if (makeContext(&scheduler_context, scheduler) == FAILURE) {
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
    if (makeContext(&idle_process->ucontext, idle_func) == -1) {
        return FAILURE;
    }
    idle_process->pid = 0;
    idle_process->ppid = 0;
    idle_process->state = READY;

    return SUCCESS;
}


int main(int argc, char const *argv[])
{
    // initialize process queue
    if (kernel_init() == -1) {
        perror("error with initializing kernel level\n");
    }

    // TODO: instead of active_process, create multiple random process
    // and add it to the queue
    pcb* newPCB = k_process_create(active_process);
    pcb_node newNode = {newPCB, NULL};
    // add this process to the process queue
    enqueue(ready_queue->mid, &newNode);

    scheduler_init();

    swapcontext(&main_context, &scheduler_context);

    return 0;
}