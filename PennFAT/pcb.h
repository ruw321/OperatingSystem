#include "fd-table.h"


typedef struct pcb {
    FdNode *fds[MAX_FILE_DESCRIPTOR];
} pcb;

pcb *newPCB();