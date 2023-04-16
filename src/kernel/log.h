#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include "global.h"
#include "utils.h"

extern int tick_tracker;

void log_init();
void log_event(pcb* pcb, char* action);
void log_pnice(pcb* pcb, int new);
void log_cleanup();

#endif