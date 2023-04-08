#ifndef BEHAVIOR_H
#define BEHAVIOR_H

#define MAX_LINE_LENGTH 8192
#define PROMPT "pennOS> "

#include "job.h"
#include "../PennFAT/interface.h"

typedef enum {
    EXIT_SHELL,
    EMPTY_LINE,
    EXECUTE_COMMAND
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
ProgramType isKnownProgram(struct parsed_command *cmd);
bool executeProgram(struct parsed_command *cmd);

/* Known user programs */
void s_cat(struct parsed_command *cmd);
void s_sleep(struct parsed_command *cmd);
void s_busy(struct parsed_command *cmd);
void s_echo(struct parsed_command *cmd);
// void s_ls(struct parsed_command *cmd);
void s_touch(struct parsed_command *cmd);
void s_mv(struct parsed_command *cmd);
void s_cp(struct parsed_command *cmd);
void s_rm(struct parsed_command *cmd);
void s_chmod(struct parsed_command *cmd);
void s_ps(struct parsed_command *cmd);
void s_kill(struct parsed_command *cmd);
void s_zombify(struct parsed_command *cmd);
void s_orphanify(struct parsed_command *cmd);

#endif