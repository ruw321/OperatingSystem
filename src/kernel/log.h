#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include "global.h"
#include "utils.h"

extern int tick_tracker;

/**
 * @brief initialize the log
 * 
 */
void log_init();

/**
 * @brief do the log event
 * 
 * @param pcb 
 * @param action 
 */
void log_event(pcb* pcb, char* action);

/**
 * @brief the log event for pnice system call
 * 
 * @param pcb 
 * @param new 
 */
void log_pnice(pcb* pcb, int new);

/**
 * @brief clean up the log
 * 
 */
void log_cleanup();

#endif