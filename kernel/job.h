#ifndef JOB_H
#define JOB_H

#include "user.h"
#include "utils.h"
#include "parser.h"
#include "unistd.h"
#include "../PennFAT/interface.h"

typedef enum {
    JOB_RUNNING,
    JOB_STOPPED,
    JOB_FINISHED,
    JOB_TERMINATED
} JobState;


typedef enum {
    NICE, // Syntax: nice priority command [args]
    NICE_PID, // Syntax: nice_pid priority pid
    MAN, // Syntax: man
    BG, // Syntax: bg [job_id]
    FG, // Syntax: fg [job_id]
    JOBS, // Syntax: jobs
    LOGOUT, // Syntax: logout
    OTHERS // non-builtin command
} CommandType;

typedef struct Job {
    struct parsed_command *cmd;
    pid_t pid;
    JobState state;
} Job;

typedef struct JobListNode {
    Job *job;
    struct JobListNode *prev;
    struct JobListNode *next;
    int jobId;
} JobListNode;

typedef struct JobList {
    JobListNode *head;
    JobListNode *tail;
    int jobCount;
} JobList;

extern JobList _jobList; // store all background job

/* Utility functions for writing job state */
void printCommandLine(struct parsed_command *cmd);
void writeJobStatePrompt(JobState state);
void writeJobState(Job *job);
void writeNewline();

/* Utility functions for job and job list */
Job *createJob(struct parsed_command *cmd, pid_t pid, JobState state);
void initJobList(JobList *jobList);
void appendJobList(JobList *jobList, Job *job);
Job *findJobList(JobList *jobList, pid_t pid);
Job *updateJobList(JobList *jobList, pid_t pid, JobState state);
int removeJobList(JobList *jobList, pid_t pid);

Job *findJobListByJobId(JobList *jobList, int jobId);
Job *updateJobListByJobId(JobList *jobList, int jobId, JobState state);
int removeJobListByJobId(JobList *jobList, int jobId);
int removeJobListWithoutFreeCmd(JobList *jobList, pid_t pid);
Job *popJobList(JobList *jobList, pid_t pid);

void printJobList(JobList *jobList);

/* Built-in commands */
CommandType parseBuiltinCommandType(struct parsed_command *cmd);
int executeBuiltinCommand(struct parsed_command *cmd);


void clearJobList(JobList *jobList);
void pollBackgroundProcesses();

Job *findTheCurrentJob(JobList *jobList);
void bgBuildinCommand(struct parsed_command *cmd);
void fgBuildinCommand(struct parsed_command *cmd);
void jobsBuiltinCommand();


void niceBuildinCommand(struct parsed_command *cmd);
void nicePidBuildinCommand(struct parsed_command *cmd);
void manBuildinCommand();
void logoutBuiltinCommand();

#endif