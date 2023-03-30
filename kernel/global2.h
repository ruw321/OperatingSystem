#ifndef GLOBAL2_H
#define GLOBAL2_H

/**
 * state of the process
 */ 
enum process_state {
    RUNNING,
    READY,
    BLOCKED,
    STOPPED,
    TERMINATED,
    ZOMBIED,
    ORPHANED,
    COMPLETED,
};

#endif