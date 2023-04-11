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

    f_write(F_ERROR, statePrompt, strlen(statePrompt));
}


void writeJobState(Job *job) {
    writeJobStatePrompt(job->state);
    printCommandLine(job->cmd);
}

void writeNewline() {
    f_write(F_ERROR, "\n", 1);
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

CommandType parseBuiltinCommandType(struct parsed_command *cmd) {
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
int executeBuiltinCommand(struct parsed_command *cmd) {
    CommandType cmdType = parseBuiltinCommandType(cmd);

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
            printf("ERROR: OTHER COMMAND\n");
            free(cmd);
            return -1;
    }
    free(cmd);
    return 0;
}


void jobsBuiltinCommand() {
    printJobList(&_jobList);
}

void bgBuildinCommand(struct parsed_command *cmd) {

}

void fgBuildinCommand(struct parsed_command *cmd) {

}

void niceBuildinCommand(struct parsed_command *cmd) {

}

void nicePidBuildinCommand(struct parsed_command *cmd) {

}

void manBuildinCommand() {

}

void logoutBuiltinCommand() {

}



Job *createJob(struct parsed_command *cmd, pid_t pid, JobState state) {
    Job *newJob = malloc(sizeof(Job));
    newJob->cmd = cmd;
    newJob->pid = pid;
    newJob->state = state;
    return newJob;
}

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

Job *findJobList(JobList *jobList, pid_t pid) {
    JobListNode *curNode = jobList->head->next;
    while (curNode != jobList->tail) {
        if (curNode->job->pid == pid) {
            return curNode->job;
        }
        curNode = curNode->next;
    }
    return NULL; // not found
}

Job *updateJobList(JobList *jobList, pid_t pid, JobState state) {
    JobListNode *curNode = jobList->head->next;
    while (curNode != jobList->tail) {
        if (curNode->job->pid == pid) {
            curNode->job->state = state;
            return curNode->job;
        }
        curNode = curNode->next;
    }
    return NULL; // not found
}

int removeJobList(JobList *jobList, pid_t pid) {
    JobListNode *nextNode = jobList->head->next;
    JobListNode *curNode = nextNode;
    while (curNode != jobList->tail) {
        if (curNode->job->pid == pid) {

            JobListNode *prev = curNode->prev;
            JobListNode *next = curNode->next;
            prev->next = next;
            next->prev = prev;

            free(curNode->job->cmd);
            free(curNode->job);
            free(curNode);

            jobList->jobCount -= 1;
            return 0;
        }
        curNode = curNode->next;
    }
    return -1; // not found
}


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
            free(curNode->job);
            free(curNode);

            jobList->jobCount -= 1;
            return 0;
        }
        curNode = curNode->next;
    }
    return -1; // not found
}

int removeJobListWithoutFreeCmd(JobList *jobList, pid_t pid) {
    JobListNode *nextNode = jobList->head->next;
    JobListNode *curNode = nextNode;
    while (curNode != jobList->tail) {
        if (curNode->job->pid == pid) {
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

Job *popJobList(JobList *jobList, pid_t pid) {
    JobListNode *nextNode = jobList->head->next;
    JobListNode *curNode = nextNode;
    while (curNode != jobList->tail) {
        if (curNode->job->pid == pid) {
            JobListNode *prev = curNode->prev;
            JobListNode *next = curNode->next;
            prev->next = next;
            next->prev = prev;

            Job *job = curNode->job;
            curNode->job = NULL;
            free(curNode);
            jobList->jobCount -= 1;

            return job;
        }
        curNode = curNode->next;
    }
    return NULL; // not found
}