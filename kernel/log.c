#include "log.h"
FILE *log_file;

void log_init() {
    log_file = fopen("log.txt", "w");
    if (log_file == NULL) {
        printf("Error opening file\n");
    }
}

void log_event(pcb* pcb, char* action) {
    //fprintf(log_file, "[%4d] %12s %2d %3d %6s\n", tick_tracker, action, pcb->pid, pcb->priority, pcb->pname);
    fprintf(log_file, "[%4d] %14s %2d %3d %6s\n", tick_tracker, action, pcb->pid, pcb->state, pcb->pname);
    
    fflush(log_file);
}

void log_cleanup() {
    fclose(log_file);
}