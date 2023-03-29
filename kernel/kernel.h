#include "utils.h"
/**
 * create a new child thread and associated PCB. The new thread should retain much of the properties of the parent. The function should return a reference to the new PCB.
 */   
pcb* k_process_create(pcb * parent);

// kill the process referenced by process with the signal signal.
k_process_kill(pcb *process, int signal);

// called when a terminated/finished thread’s resources needs to be cleaned up. Such clean-up may include freeing memory, setting the status of the child, etc.
k_process_cleanup(pcb *process);