#include "shell.h"
#include "scheduler.h"

JobList _jobList; // store all background job


void shell_process() {
    signal(SIGTTOU, SIG_IGN);
    char *line = NULL;
    LineType lineType;
    while (true) {
        // TODO: flush log and errno
        
        // read command
        writePrompt();
        lineType = readAndParseUserInput(&line);

        // Reap zombie processes synchronously
        pollBackgroundProcesses();
        
        if (lineType == EXIT_SHELL) {
            return;
        }
        else if (lineType == EMPTY_LINE) {
            writeNewline();
        }
        else {
            struct parsed_command *cmd;
            int res = parseLine(line, &cmd);
            if (res == 0) {
                if (executeBuiltinCommand(cmd) == false) {
                    if (executeProgram(cmd) == false) {
                        printf("Error in executeProgram\n");
                    }
                }  
            }
        }
        free(line);

    } 
}

int shell_init(int argc, const char **argv) {
    // TODO: Error checking
    if (kernel_init() == FAILURE ) {
        return FAILURE;
    }
    // fs_mount(argv[1]);
    initJobList(&_jobList);
    if (scheduler_init() == FAILURE ) {
        return FAILURE;
    }

    // initialize main context
    getcontext(&main_context);
    sigemptyset(&(main_context.uc_sigmask));
    set_stack(&(main_context.uc_stack));
    main_context.uc_link = NULL;

    p_active_context = &main_context;
    active_process = NULL;

    // init shell ucontext
    pcb *terminal = malloc(sizeof(pcb));
    terminal->pid = lastPID ++;
    terminal->ppid = 0;
    terminal->state = READY;
    terminal->priority = 0;

    pcb_node *terminal_node = new_pcb_node(terminal);

    getcontext(&terminal->ucontext);
    set_stack(&(terminal->ucontext.uc_stack));
    
    sigemptyset(&(terminal->ucontext.uc_sigmask));
    terminal->ucontext.uc_link = &main_context;
    makecontext(&terminal->ucontext, shell_process, 0);


    // printf("initializing context for testing\n");
    // ucontext_t ctx1, ctx2;
    // char stack1[8192], stack2[8192];
    // // Initialize context 1
    // getcontext(&ctx1);
    // ctx1.uc_stack.ss_sp = stack1;
    // ctx1.uc_stack.ss_size = sizeof(stack1);
    // ctx1.uc_link = &scheduler_context;
    // makecontext(&ctx1, foo, 0);

    // // Initialize context 2
    // getcontext(&ctx2);
    // ctx2.uc_stack.ss_sp = stack2;
    // ctx2.uc_stack.ss_size = sizeof(stack2);
    // ctx2.uc_link = &scheduler_context;
    // makecontext(&ctx2, bar, 0);

    // printf("initializing PCBs for testing\n");
    // pcb* newPCB = (pcb *) malloc(sizeof(pcb));
    // newPCB->ucontext = ctx1;
    // newPCB->pid = 1;
    // newPCB->ppid = 4;
    // newPCB->state = READY;
    // newPCB->priority = 0;
    // pcb_node* newNode = new_pcb_node(newPCB);

    // pcb* newPCB2 = (pcb *) malloc(sizeof(pcb));
    // newPCB2->ucontext = ctx2;
    // newPCB2->pid = 2;
    // newPCB->ppid = 4;
    // newPCB2->state = READY;
    // newPCB2->priority = 1;
    // pcb_node* newNode2 = new_pcb_node(newPCB2);

    // // add this process to the process queue
    // enqueue(ready_queue->mid, newNode);
    // enqueue(ready_queue->low, newNode2);

    enqueue_by_priority(ready_queue, HIGH, terminal_node);

    return SUCCESS;
}

int main(int argc, char const *argv[])
{

    // init system
    // if (shell_init(argc, argv) == FAILURE) {
    //     perror("failed to init shell");
    //     return FAILURE;
    // }

    // swapcontext(&main_context, &scheduler_context);
    // shell_process();

    return 0;
}
