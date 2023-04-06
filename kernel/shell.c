#include "shell.h"
#include "scheduler.h"

// JobList _jobList; // store all background job

void shell_process() {
    // while (1) {
    //     printf("shell> ");
    //     break;
    // }
    printf("shell process\n");    
    printf("shell process\n");
    printf("shell process\n");    
    printf("shell process\n");    
    printf("shell process\n");    
}

int shell_init(int argc, char **argv) {
    // TODO: Error checking
    if (kernel_init() == FAILURE ) {
        return FAILURE;
    }
    // fs_mount(argv[1]);
    // initJobList(&_jobList);
    if (scheduler_init() == FAILURE ) {
        return FAILURE;
    }

    // initialize main context
    printf("init main context\n");
    getcontext(&main_context);
    sigemptyset(&(main_context.uc_sigmask));
    set_stack(&(main_context.uc_stack));
    main_context.uc_link = NULL;

    p_active_context = &main_context;
    active_process = NULL;

    // init shell ucontext
    printf("init shell pcb\n");
    pcb *terminal = malloc(sizeof(pcb));
    terminal->pid = lastPID ++;
    terminal->ppid = 0;
    terminal->state = READY;
    terminal->priority = 0;

    pcb_node *terminal_node = new_pcb_node(terminal);

    printf("init shell context\n");
    getcontext(&terminal->ucontext);
    set_stack(&(terminal->ucontext.uc_stack));
    
    sigemptyset(&(terminal->ucontext.uc_sigmask));
    terminal->ucontext.uc_link = &main_context;
    makecontext(&terminal->ucontext, shell_process, 0);

    enqueue_by_priority(ready_queue, HIGH, terminal_node);

    return SUCCESS;
}

int main(int argc, char const *argv[])
{

    // init system
    if (shell_init(argc, argv) == FAILURE) {
        perror("failed to init shell");
        return FAILURE;
    }

    swapcontext(&main_context, &scheduler_context);

    return 0;
}
