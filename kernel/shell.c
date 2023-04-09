#include "shell.h"
#include "scheduler.h"

JobList _jobList; // store all background job

void shell_process() {
    printf("shell process started\n");
    printf("shell process started\n");
    printf("shell process started\n");
    printf("shell process started\n");
    printf("shell process started\n");
    printf("shell process started\n");
    // signal(SIGTTOU, SIG_IGN);
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

    // initialize main context
    getcontext(&main_context);
    sigemptyset(&(main_context.uc_sigmask));
    set_stack(&(main_context.uc_stack));
    main_context.uc_link = NULL;

    p_active_context = &main_context;
    active_process = NULL;

    // init shell ucontext
    ucontext_t shell_context;
    getcontext(&shell_context);
    sigemptyset(&(shell_context.uc_sigmask));
    set_stack(&(shell_context.uc_stack));
    shell_context.uc_link = &main_context;

    if (makeContext(&shell_context, shell_process, 0, NULL, NULL) == FAILURE) {
        return FAILURE;
    }

    pcb *shell_pcb = malloc(sizeof(pcb));
    shell_pcb->pid = lastPID ++;
    shell_pcb->ppid = 0;
    shell_pcb->state = READY;
    shell_pcb->ucontext = shell_context;
    shell_pcb->priority = HIGH;

    pcb_node *shell_node = malloc(sizeof(pcb_node));

    enqueue_by_priority(ready_queue, HIGH, shell_node);

    if (scheduler_init() == FAILURE ) {
        return FAILURE;
    }

    printf("shell process initialized\n");

    return SUCCESS;
}

// int main(int argc, char const *argv[])
// {

//     // init system
//     if (shell_init(argc, argv) == FAILURE) {
//         perror("failed to init shell");
//         return FAILURE;
//     }

//     // swapcontext(&main_context, &scheduler_context);
//     shell_process();

//     return 0;
// }
