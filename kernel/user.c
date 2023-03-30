#include "user.h"

pid_t p_spawn(void (*func)(), char *argv[], int fd0, int fd1) {
    // forks a new thread that retains most of the attributes of the parent thread 
    pcb* pcb = k_process_create(active_process);
    if (pcb == NULL) {
        perror("failed to create pcb");
        return -1;
    }
    // TODO: attach them to PCB, they are used later in the shell

    // fd0 is the file descriptor for the input file, and 
    pcb->input_fd = fd0;
    // fd1 is the file descriptor for the output file.
    pcb->output_fd = fd1;

    // executes the function referenced by func with its argument array argv. 
    makeContext(&(pcb->ucontext), func, 1, &scheduler_context, argv);
    // default priority level is 0
    enqueue(ready_queue->mid, new_pcb_node(pcb));

    return pcb->pid;
}