#include "shell.h"
#include "scheduler.h"

JobList _jobList; // store all background job

void shell_process() {
    printf("Shell process started\n");
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
                    if (executeLine(cmd) == false) {
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

    if (scheduler_init() == FAILURE ) {
        return FAILURE;
    }

    // fs_mount(argv[1]);
    initJobList(&_jobList);

    // initialize main context
    getcontext(&main_context);
    sigemptyset(&(main_context.uc_sigmask));
    set_stack(&(main_context.uc_stack));
    main_context.uc_link = NULL;

    active_process = new_pcb(&main_context, lastPID++);
    p_active_context = &main_context;

    // init shell ucontext
    ucontext_t shell_context;
    getcontext(&shell_context);
    sigemptyset(&(shell_context.uc_sigmask));
    set_stack(&(shell_context.uc_stack));
    shell_context.uc_link = &main_context;

    if (makeContext(&shell_context, shell_process, 0, NULL, NULL) == FAILURE) {
        return FAILURE;
    }

    pcb *shell_pcb = new_pcb(&shell_context, lastPID++);
    shell_pcb->priority = -1;   // the default is 0, but we want -1

    pcb_node *shell_node = new_pcb_node(shell_pcb);

    enqueue_by_priority(ready_queue, HIGH, shell_node);

    printf("shell process initialized\n");

    return SUCCESS;
}
