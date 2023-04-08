#include "pcb.h"

pcb *active_process;

pcb *newPCB() {
    pcb *p = malloc(sizeof(pcb));
    for (int i = 0; i < MAX_FILE_DESCRIPTOR; i++) {
        p->fds[i] = NULL;
    }
    return p;
}