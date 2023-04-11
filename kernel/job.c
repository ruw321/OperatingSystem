#include "job.h"

void printCommandLine(struct parsed_command *cmd) {
    for (int i = 0; i < cmd->num_commands; i++) {
        for (char **argument = cmd->commands[i]; *argument != NULL; argument++) {
            printf("%s ", *argument);
        }

        if (i == 0 && cmd->stdin_file != NULL) {
            printf("< %s ", cmd->stdin_file);
        }

        if (i != cmd->num_commands - 1) {
            printf("| ");
        }
    }

    if (cmd->stdout_file != NULL){
        printf(cmd->is_file_append ? ">> %s " : "> %s ", cmd->stdout_file);
    }
    
    puts("");
}

/* In this project, Terminated should never occur */
void writeJobStatePrompt(JobState state) {
    char runningPrompt[] = "Running: ";
    char stoppedPrompt[] = "Stopped: ";
    char finishedPrompt[] = "Finished: ";
    char terminatedPrompt[] = "Terminated: ";

    char *statePrompt;
    switch (state) {
        case JOB_RUNNING:
            statePrompt = runningPrompt;
            break;
        case JOB_STOPPED:
            statePrompt = stoppedPrompt;
            break;
        case JOB_FINISHED:
            statePrompt = finishedPrompt;
            break;
        case JOB_TERMINATED:
            statePrompt = terminatedPrompt;
            break;
        default:
            printf("state error\n");
            exit(EXIT_FAILURE);
    }

    if (write(STDERR_FILENO, statePrompt, strlen(statePrompt)) == -1) {
        perror("Failed to write the job state prompt");
        exit(EXIT_FAILURE);
    }
}

/* 
    Write the complete job state
    SYNC: Called when pollBackgroundProcesses
    ASYNC: Won't be called until read or fg finished
 */
void writeJobState(Job *job) {
    writeJobStatePrompt(job->state);
    printCommandLine(job->cmd);
}

void writeNewline() {
    if (write(STDERR_FILENO, "\n", 1) == -1) {
        perror("Failed to write the newline.");
        exit(EXIT_FAILURE);
    }
}

/* Externally reap zombie processes by calling poll everytime the shell reads */
void pollBackgroundProcesses() {
    /* polling consumes CPU cycles, we only poll once for each shell read */
    int wstatus;
    pid_t pid;

    // while ((pid = waitpid(-1, &wstatus, WNOHANG | WUNTRACED)) > 0) {
    while ((pid = p_waitpid(-1, &wstatus, true)) > 0) {
        Job *job = findJobByPid(&_jobList, pid);
        if (job == NULL) {
            perror("Job of pgid in pollBackgroundProcesses not found");
            return;
        }

        if (WIFEXITED(wstatus)) { // exit with error
            /* EXIT will never be caught by the poll. */
            job->pActiveNum --;
        } else if (WIFSTOPPED(wstatus)) { // stopped by a signal
            /* Foreground job process STOP will never be caught by the poll. */
            job->state = JOB_STOPPED;
        } else if (WIFSIGNALED(wstatus)) { // terminated by a signal
            /* Foreground job process KILL will never be caught by the poll. */
            job->state = JOB_TERMINATED;
        }

        if (job->pActiveNum == 0 && job->state == JOB_RUNNING) { // exit with error
            /* EXIT will never be caught by the poll. */
            job->state = JOB_FINISHED;
            writeJobState(job);
            removeJobList(&_jobList, job->pgpid);
        } else if (job->state == JOB_STOPPED) { // stopped by a signal
            /* Foreground job process STOP will never be caught by the poll. */
            writeJobState(job);
        } else if (job->state == JOB_TERMINATED) { // terminated by a signal
            /* Foreground job process KILL will never be caught by the poll. */
            writeJobState(job);
            removeJobList(&_jobList, job->pgpid);
        }
        
    } 
}

Job *createJob(struct parsed_command *cmd, pid_t *pids, JobState state) {
    Job *newJob = malloc(sizeof(Job));
    newJob->cmd = cmd;
    newJob->pids = pids;
    newJob->pgpid = pids[0];
    newJob->state = state;
    newJob->pActiveNum = cmd->num_commands;
    return newJob;
}

/* Initialize a JobList (LinkedList) */
void initJobList(JobList* jobList) {
    jobList->head = malloc(sizeof(JobListNode));
    jobList->head->job = NULL;
    jobList->head->jobId = 0;

    jobList->tail = malloc(sizeof(JobListNode));
    jobList->tail->job = NULL;
    jobList->tail->jobId = -1;

    jobList->head->prev = NULL;
    jobList->head->next = jobList->tail;
    jobList->tail->prev = jobList->head;
    jobList->tail->next = NULL;

    jobList->jobCount = 0;
}

/* Append to a JobList (LinkedList) */
void appendJobList(JobList *jobList, Job *job) {
    JobListNode *newNode = malloc(sizeof(JobListNode));

    newNode->job = job;
    newNode->jobId = jobList->tail->prev->jobId + 1;

    JobListNode *prev = jobList->tail->prev;
    prev->next = newNode;
    newNode->prev = prev;
    jobList->tail->prev = newNode;
    newNode->next = jobList->tail;
    jobList->jobCount += 1;
}

/* 
    Find a job in JobList (O[N]) providing pgpid
    Return the Job if found, else NULL
*/
Job *findJobList(JobList *jobList, pid_t pgpid) {
    JobListNode *curNode = jobList->head->next;
    while (curNode != jobList->tail) {
        if (curNode->job->pgpid == pgpid) {
            return curNode->job;
        }
        curNode = curNode->next;
    }
    return NULL; // not found
}

/* 
    Find a job in JobList (O[NM]) providing pid
    Return the Job if found, else NULL
*/
Job *findJobByPid(JobList *jobList, pid_t pid) {
    JobListNode *curNode = jobList->head->next;
    while (curNode != jobList->tail) {
        Job *job = curNode->job;
        for (int i = 0; i < job->cmd->num_commands; i++) {
            if (job->pids[i] == pid) {
                return job;
            }
        }
        curNode = curNode->next;
    }
    return NULL; // not found
}

/* 
    Update a job in JobList (O[N]) providing new pgpid and new state
    Return the Job if found and updated, else NULL
*/
Job *updateJobList(JobList *jobList, pid_t pgpid, JobState state) {
    JobListNode *curNode = jobList->head->next;
    while (curNode != jobList->tail) {
        if (curNode->job->pgpid == pgpid) {
            curNode->job->state = state;
            return curNode->job;
        }
        curNode = curNode->next;
    }
    return NULL; // not found
}

/* 
    Remove a job in JobList (O[N]) providing pgpid
    Return the Job if found and removed, else -1
    Will also free parsed_command and the Job
*/
int removeJobList(JobList *jobList, pid_t pgpid) {
    JobListNode *nextNode = jobList->head->next;
    JobListNode *curNode = nextNode;
    while (curNode != jobList->tail) {
        if (curNode->job->pgpid == pgpid) {

            JobListNode *prev = curNode->prev;
            JobListNode *next = curNode->next;
            prev->next = next;
            next->prev = prev;

            free(curNode->job->cmd);
            free(curNode->job->pids);
            free(curNode->job);
            free(curNode);

            jobList->jobCount -= 1;
            return 0;
        }
        curNode = curNode->next;
    }
    return -1; // not found
}



/*
    Cleanup the joblist
    Free all allocated resources.
    Will be called when shell exits.
*/
void clearJobList(JobList *jobList) {
    JobListNode *nextNode = jobList->head->next;
    JobListNode *curNode = nextNode;
    while (curNode != jobList->tail) {
        nextNode = curNode->next;

        pid_t pgpid = curNode->job->pgpid;
        killpg(pgpid, SIGKILL);
        int wstatus;
        do {
            // waitpid(pgpid, &wstatus, WUNTRACED | WCONTINUED);
            p_waitpid(pgpid, &wstatus, false);
        } while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));

        free(curNode->job->cmd);
        free(curNode->job->pids);
        free(curNode->job);
        free(curNode);

        curNode = nextNode;
    }

    free(jobList->head);
    free(jobList->tail);
}

/* 
    Remove a job in JobList (O[N]) providing pgpid
    Return the Job if found and removed, else -1
    Will only free listnode for async implementation.
*/
int removeJobListWithoutFree(JobList *jobList, pid_t pgpid) {
    JobListNode *nextNode = jobList->head->next;
    JobListNode *curNode = nextNode;
    while (curNode != jobList->tail) {
        if (curNode->job->pgpid == pgpid) {
            JobListNode *prev = curNode->prev;
            JobListNode *next = curNode->next;

            free(curNode);

            prev->next = next;
            next->prev = prev;
            jobList->jobCount -= 1;

            return 0;
        }
        curNode = curNode->next;
    }
    return -1; // not found
}

/* 
    Remove a job in JobList (O[N]) providing pgpid
    Return the Job if found and removed, else -1
    Will not free parsed_command and pids but the job.
*/
int removeJobListWithoutFreeCmdAndPids(JobList *jobList, pid_t pgpid) {
    JobListNode *nextNode = jobList->head->next;
    JobListNode *curNode = nextNode;
    while (curNode != jobList->tail) {
        if (curNode->job->pgpid == pgpid) {
            JobListNode *prev = curNode->prev;
            JobListNode *next = curNode->next;
            prev->next = next;
            next->prev = prev;

            curNode->job->cmd = NULL;
            free(curNode->job);
            free(curNode);

            jobList->jobCount -= 1;

            return 0;
        }
        curNode = curNode->next;
    }
    return -1; // not found
}

/* 
    Find a job in JobList (O[N]) providing jobId
    Return the Job if found, else NULL
*/
Job *findJobListByJobId(JobList *jobList, int jobId) {
    JobListNode *curNode = jobList->head->next;
    while (curNode != jobList->tail) {
        if (curNode->jobId == jobId) {
            return curNode->job;
        }
        curNode = curNode->next;
    }
    return NULL; // not found
}

/* 
    Update a job in JobList (O[N]) providing new jobId and new state
    Return the Job if found and updated, else NULL
*/
Job *updateJobListByJobId(JobList *jobList, int jobId, JobState state) {
    JobListNode *curNode = jobList->head->next;
    while (curNode != jobList->tail) {
        if (curNode->jobId == jobId) {
            curNode->job->state = state;
            return curNode->job;
        }
        curNode = curNode->next;
    }
    return NULL; // not found
}

/* 
    Remove a job in JobList (O[N]) providing jobId
    Return the Job if found and removed, else -1
    Will also free parsed_command and the Job
*/
int removeJobListByJobId(JobList *jobList, int jobId) {
    JobListNode *nextNode = jobList->head->next;
    JobListNode *curNode = nextNode;
    while (curNode != jobList->tail) {
        if (curNode->jobId == jobId) {

            JobListNode *prev = curNode->prev;
            JobListNode *next = curNode->next;
            prev->next = next;
            next->prev = prev;

            free(curNode->job->cmd);
            free(curNode->job->pids);
            free(curNode->job);
            free(curNode);

            jobList->jobCount -= 1;
            return 0;
        }
        curNode = curNode->next;
    }
    return -1; // not found
}

/* Refer to print_parsed_command() implemented in parser.c provided by the TA */
void printJobList(JobList *jobList) {
    char runningPrompt[] = "running";
    char stoppedPrompt[] = "stopped";
    char *statePrompt;

    JobListNode *curNode = jobList->head->next;
    while (curNode != jobList->tail) {
        int idx = curNode->jobId;
        struct parsed_command *cmd = curNode->job->cmd;
        JobState state = curNode->job->state;

        switch (state) {
            case RUNNING:
                statePrompt = runningPrompt;
                break;
            case STOPPED:
                statePrompt = stoppedPrompt;
                break;
            default:
                printf("ERROR: unexpected background job state");
                exit(EXIT_FAILURE);
        }
        /*
        [idx] commands (state) e.g. [1] sleep 1 (running) 
        */
        printf("[%d] ", idx);

        for (int i = 0; i < cmd->num_commands; ++i) {
            for (char **arguments = cmd->commands[i]; *arguments != NULL; ++arguments)
                printf("%s ", *arguments);

            if (i == 0 && cmd->stdin_file != NULL)
                printf("< %s ", cmd->stdin_file);

            if (i == cmd->num_commands - 1) {
                if (cmd->stdout_file != NULL)
                    printf(cmd->is_file_append ? ">> %s " : "> %s ", cmd->stdout_file);
            } else printf("| ");
        }

        printf(" (%s)", statePrompt);
        puts("");

        curNode = curNode->next;
    }
}

/* 
    Whether the command is built in
    Return the command type
 */
CommandType isBuiltinCommand(struct parsed_command *cmd) {
    if (cmd->num_commands == 0){
        return OTHERS;
    }
    if (strcmp(*cmd->commands[0], "bg") == 0) {
        return BG;
    } 
    else if (strcmp(*cmd->commands[0], "fg") == 0) {
        return FG;
    } 
    else if (strcmp(*cmd->commands[0], "jobs") == 0) {
        return JOBS;
    } 
    else if (strcmp(*cmd->commands[0], "nice") == 0) {
        return NICE;
    }
    else if (strcmp(*cmd->commands[0], "nice_pid") == 0) {
        return NICE_PID;
    }
    else if (strcmp(*cmd->commands[0], "man") == 0) {
        return MAN;
    }
    else if (strcmp(*cmd->commands[0], "logout") == 0) {
        return LOGOUT;
    }
    else {
        return OTHERS;
    }
}

/* Execute built-in command wrapped up */
bool executeBuiltinCommand(struct parsed_command *cmd) {
    CommandType cmdType = isBuiltinCommand(cmd);
    if (cmdType == OTHERS) {
        return false;
    } else {
        switch(cmdType) {
            case NICE:
                niceBuildinCommand(cmd);
                break;
            case NICE_PID:
                nicePidBuildinCommand(cmd);
                break;
            case MAN:
                manBuildinCommand();
                break;
            case BG:
                bgBuildinCommand(cmd);
                break;
            case FG:
                fgBuildinCommand(cmd);
                break;
            case JOBS:
                jobsBuiltinCommand();
                break;
            case LOGOUT:
                logoutBuiltinCommand();
                break;
            default:
                printf("ERROR: unexpected builtin command");
                return false;
        }
        free(cmd);
        return true;
    }
}

/* 
    Get the current job
    penn-shell has a notion of the current job. 
    If there are stopped jobs, the current job is the most recently stopped one. 
    Otherwise, it is the most recently created background job. 
*/
Job *findTheCurrentJob(JobList *jobList) {
    Job *theCurrentJob = NULL;
    JobListNode *curNode = jobList->tail->prev;
    int curNodeId = 0;
    while(curNode != jobList->head) {
        if (curNode->job->state == STOPPED) {
            theCurrentJob = curNode->job;
            break;
        }
        else if (curNode->job->state == RUNNING && curNode->jobId > curNodeId) {
            theCurrentJob = curNode->job;
            curNodeId = curNode->jobId;
        }
        curNode = curNode->prev;
    }
    return theCurrentJob;
}

/* 
    Execute bg built-in command
    Send SIGCONT to the current job process
*/
void bgBuildinCommand(struct parsed_command *cmd) {
    int jobId = (cmd->commands[0][1] != NULL)? atoi(cmd->commands[0][1]) : -1;
    Job *job;
    if (jobId != -1) {
        job = findJobListByJobId(&_jobList, jobId);
    } else { // use the current job by default
        job = findTheCurrentJob(&_jobList);
    }
    if (job == NULL || job->state == RUNNING) {
        printf("ERROR: failed to find the stopped job\n");
    } else {
        killpg(job->pgpid, SIGCONT);
        job->state = JOB_RUNNING;
        writeJobState(job);
    }
}

/* 
    Execute fg built-in command
    Send SIGCONT to the current job process
    Wait for its processing
*/
void fgBuildinCommand(struct parsed_command *cmd) {
    int jobId = (cmd->commands[0][1] != NULL)? atoi(cmd->commands[0][1]) : -1;
    Job *job;
    if (jobId != -1) {
        job = findJobListByJobId(&_jobList, jobId);
    } else { // use the current job by default
        job = findTheCurrentJob(&_jobList);
    }
    if (job == NULL) {
        printf("ERROR: failed to find the stopped job\n");
    } else {
        
        if (job->state == STOPPED) printf("Restarting: ");
        printCommandLine(job->cmd);

        pid_t pgpid = job->pgpid;
        tcsetpgrp(STDIN_FILENO, pgpid); // delegate the terminal control
        killpg(pgpid, SIGCONT);
        job->state = JOB_RUNNING;

        bool isStopped = false;
        bool isKilled = false;

        for (int i = 0; i < job->cmd->num_commands; i++) {
            int wstatus;
            do {
                // if (waitpid(job->pids[i], &wstatus, WUNTRACED | WCONTINUED) > 0) {
                if (p_waitpid(job->pids[i], &wstatus, false) > 0) {
                    if (WIFSIGNALED(wstatus)) isKilled = true;
                    if (WIFSTOPPED(wstatus)) isStopped = true;
                }
            } while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus) && !WIFSTOPPED(wstatus));
        }
        
        signal(SIGTTOU, SIG_IGN); // ignore the signal from UNIX when the main process come back from the background to get the terminal control
        tcsetpgrp(STDIN_FILENO, getpid()); // give back the terminal control to the main process

        if (isStopped) {
            struct parsed_command *cmd = job->cmd;
            pid_t *pids = job->pids;
            removeJobListWithoutFreeCmdAndPids(&_jobList, pgpid);
            Job *newBackgroundJob = createJob(cmd, pids, JOB_STOPPED);
            appendJobList(&_jobList, newBackgroundJob);
            writeNewline();
            writeJobState(newBackgroundJob);
        } else if (isKilled) {
            writeNewline();
            removeJobList(&_jobList, pgpid);
        } else {
            removeJobList(&_jobList, pgpid);
        }

    }
}


void jobsBuiltinCommand() {
    printJobList(&_jobList);
}


void niceBuildinCommand(struct parsed_command *cmd) {

}

void nicePidBuildinCommand(struct parsed_command *cmd) {

}

void manBuildinCommand() {

}

void logoutBuiltinCommand() {

}