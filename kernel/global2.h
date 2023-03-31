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
    ZOMBIED,
    ORPHANED,
    EXITED,     // terminated normally
    SIGNALED,   // terminated by a signal
    TERMINATED,
};

#endif