#ifndef BEHAVIOR_H
#define BEHAVIOR_H

#define S_MAX_LINE_LENGTH 4096
#define PROMPT "pennOS> "

#include "job.h"
#include "../PennFAT/interface.h"
#include "programs.h"

typedef enum {
    S_EXIT_SHELL,
    S_EMPTY_LINE,
    S_EXECUTE_COMMAND
} LineType;

typedef enum {
    CAT,
    SLEEP,
    BUSY,
    ECHO,
    LS,
    TOUCH,
    MV,
    CP,
    RM,
    CHMOD,
    PS,
    KILL,
    ZOMBIFY,
    ORPHANIFY,
    UNKNOWN
} ProgramType;

extern JobList _jobList; // store all background job

/* Utility function for writing PROMPT */
void writePrompt();

/* Read and parse utilities */
void readUserInput(char **line); 
LineType parseUserInput(char *line);
LineType readAndParseUserInput(char **line);
int parseLine(char *line, struct parsed_command **cmd);


/* Execute a user program */
ProgramType isKnownProgram(char *argv);
bool executeLine(struct parsed_command *cmd);
pid_t executeProgram(char **argv, int fd_in, int fd_out);

#endif