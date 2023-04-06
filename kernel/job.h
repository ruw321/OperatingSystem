#ifndef JOB_H
#define JOB_H

// #include "utils.h"
// #include "parser.h"

// typedef enum {
//     RUNNING,
//     STOPPED,
//     FINISHED,
//     TERMINATED
// } JobState;


// typedef enum {
//     NICE, // Syntax: nice priority command [args]
//     NICE_PID, // Syntax: nice_pid priority pid
//     MAN, // Syntax: man
//     BG, // Syntax: bg [job_id]
//     FG, // Syntax: fg [job_id]
//     JOBS, // Syntax: jobs
//     LOGOUT, // Syntax: logout
//     OTHERS // non-builtin command
// } CommandType;

// typedef struct Job {
//     struct parsed_command *cmd;
//     pid_t *pids; // job process's pgid is set to be its pid
//     pid_t pgpid;
//     JobState state;
//     // TODO: add more fields
// } Job;

// /* We use linked list as the working around data structure to store jobs */
// typedef struct JobListNode {
//     Job *job;
//     int jobId;
//     struct JobListNode *prev;
//     struct JobListNode *next;
// } JobListNode;

// typedef struct JobList {
//     JobListNode *head;
//     JobListNode *tail;
//     int jobCount;
// } JobList;

// extern JobList _jobList; // store all background job

// /* Utility functions for writing job state */
// void printCommandLine(struct parsed_command *cmd);
// void writeJobStatePrompt(JobState state);
// void writeJobState(Job *job);

// // Reap zombie processes synchronously
// void pollBackgroundProcesses();

// /* Utility functions for job and job list */
// Job *createJob(struct parsed_command *cmd, pid_t* pids, JobState state);
// void initJobList(JobList *jobList);
// void appendJobList(JobList *jobList, Job *job);
// Job *findJobList(JobList *jobList, pid_t pgpid);
// Job *findJobByPid(JobList *jobList, pid_t pid);
// Job *updateJobList(JobList *jobList, pid_t pgpid, JobState state);
// int removeJobList(JobList *jobList, pid_t pgpid);
// int removeJobListWithoutFree(JobList *jobList, pid_t pgpid);
// int removeJobListWithoutFreeCmdAndPids(JobList *jobList, pid_t pgpid);
// void clearJobList(JobList *jobList);
// Job *findJobListByJobId(JobList *jobList, int jobId);
// Job *updateJobListByJobId(JobList *jobList, int jobId, JobState state);
// int removeJobListByJobId(JobList *jobList, int jobId);
// void printJobList(JobList *jobList);

#endif