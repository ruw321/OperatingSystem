#include "behavior.h"

void writePrompt() {
    if (write(STDERR_FILENO, PROMPT, strlen(PROMPT)) == -1) {
        perror("Failed to write the prompt.");
        exit(EXIT_FAILURE);
    }
}

void readUserInput(char **line) {
    char inputBuffer[MAX_LINE_LENGTH];
    int numBytes = read(STDIN_FILENO, inputBuffer, MAX_LINE_LENGTH);
    if (numBytes == -1) {
        perror("Failed to read the user input.");
        exit(EXIT_FAILURE);
    }
   
    if (numBytes == 0) { // read nothing but EOF (Ctrl + D at the beginning of the input line)
        *line = NULL;
    } else {
        char *userInput = NULL;
        if (inputBuffer[numBytes - 1] != '\n') { // input ended with EOF (Ctrl + D)
            userInput = malloc(1);
            userInput[0] = '\0';
            *line = userInput;
        } else {
            if (numBytes == 1) { // read nothing but '/n'
                userInput = malloc(2);
                userInput[0] = '\n';
                userInput[1] = '\0';
                *line = userInput;
            }
            else {
                userInput = malloc(numBytes);
                for (int i = 0; i < numBytes; i++) {
                    if (inputBuffer[i] == '\n') {
                        userInput[i] = '\0';
                    } 
                    else {
                        userInput[i] = inputBuffer[i];
                    }
                }
                *line = userInput;
            }
        }
    }
}

LineType parseUserInput(char *line) {
    LineType lineType = EXECUTE_COMMAND;

    if (line == NULL) {
        lineType = EXIT_SHELL;
        return lineType;
    }

    if (line[0] == '\0') {
        lineType = EMPTY_LINE;
    } 

    return lineType;
}

LineType readAndParseUserInput(char **line) {
    readUserInput(line);
    return parseUserInput(*line);
}

int parseLine(char *line, struct parsed_command **cmd) {
    int res = parse_command(line, cmd);
    if (res < 0) {
        perror("parse_command");
    }
    else if (res > 0) {
        printf("syntax error: %d\n", res);
    }
    return res;
}

ProgramType isKnownProgram(char *argv) {
    if (strcmp(argv, "cat") == 0) {
        return CAT;
    } else if (strcmp(argv, "sleep") == 0) {
        return SLEEP;
    } else if (strcmp(argv, "busy") == 0) {
        return BUSY;
    } else if (strcmp(argv, "echo") == 0) {
        return ECHO;
    } else if (strcmp(argv, "ls") == 0) {
        return LS;
    } else if (strcmp(argv, "touch") == 0) {
        return TOUCH;
    } else if (strcmp(argv, "mv") == 0) {
        return MV;
    } else if (strcmp(argv, "cp") == 0) {
        return CP;
    } else if (strcmp(argv, "rm") == 0) {
        return RM;
    } else if (strcmp(argv, "chmod") == 0) {
        return CHMOD;
    } else if (strcmp(argv, "ps") == 0) {
        return PS;
    } else if (strcmp(argv, "kill") == 0) {
        return KILL;
    } else if (strcmp(argv, "zombify") == 0) {
        return ZOMBIFY;
    } else if (strcmp(argv, "orphanify") == 0) {
        return ORPHANIFY;
    } else {
        return UNKNOWN;
    }
}

bool executeLine(struct parsed_command *cmd) {

    int fd[2];
    fd[0] = STDIN_FILENO;
    fd[1] = STDOUT_FILENO;
    
    // TODO: handle the redirection

    pid_t pid = executeProgram(*cmd->commands, fd[0], fd[1]);
    if (pid == -1) {
        perror("Failed to execute the program.");
        exit(EXIT_FAILURE);
    }

    if (cmd->is_background == false) { // In non-interactive mode, & will be ignored

        // tcsetpgrp(STDIN_FILENO, pids[0]); // delegate the terminal control

        // bool isStopped = false;
        // bool isKilled = false;

        int wstatus;
        // do {
        //     // if (waitpid(pids[i], &wstatus, WUNTRACED | WCONTINUED) > 0) {
        //     if (p_waitpid(pid, &wstatus, false) > 0) {
        //         // if (WIFSIGNALED(wstatus)) isKilled = true;
        //         // if (WIFSTOPPED(wstatus)) isStopped = true;
        //     }
        // } while (wstatus != TERMINATED && wstatus != STOPPED);
        // } while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus) && !WIFSTOPPED(wstatus));
        p_waitpid(pid, &wstatus, false);
        // signal(SIGTTOU, SIG_IGN); // ignore the signal from UNIX when the main process come back from the background to get the terminal control
        // tcsetpgrp(STDIN_FILENO, getpid()); // give back the terminal control to the main process

        // if (isStopped) {
        //     Job *newBackgroundJob = createJob(cmd, pids, JOB_STOPPED);
        //     appendJobList(&_jobList, newBackgroundJob);
        //     writeNewline();
        //     writeJobState(newBackgroundJob);
        // } else if (isKilled) {
        //     writeNewline();
        //     free(pids);
        //     free(cmd);
        // } else {
        //     free(pids);
        //     free(cmd);
        // }

    } else { // cmd->is_background == true
        // Job *newBackgroundJob = createJob(cmd, pids, JOB_RUNNING);
        // appendJobList(&_jobList, newBackgroundJob);
        // writeJobState(newBackgroundJob);
    }
    return true; 
}

pid_t executeProgram(char **argv, int fd_in, int fd_out) {
    ProgramType programType = isKnownProgram(argv[0]);
    pid_t pid = -1;
    /* Switch case on user programs */
    switch (programType) {
        case CAT:
            pid = p_spawn(s_cat, argv, fd_in, fd_out);
            break;
        case SLEEP:
            pid = p_spawn(s_sleep, argv, fd_in, fd_out);
            break;
        case BUSY:
            pid = p_spawn(s_busy, argv, fd_in, fd_out);
            break;
        case ECHO:
            pid = p_spawn(s_echo, argv, fd_in, fd_out);
            break;
        case LS:
            pid = p_spawn(s_ls, argv, fd_in, fd_out);
            break;
        case TOUCH:
            pid = p_spawn(s_touch, argv, fd_in, fd_out);
            break;
        case MV:
            pid = p_spawn(s_mv, argv, fd_in, fd_out);
            break;
        case CP:
            pid = p_spawn(s_cp, argv, fd_in, fd_out);
            break;
        case RM:
            pid = p_spawn(s_rm, argv, fd_in, fd_out);
            break;
        case CHMOD:
            pid = p_spawn(s_chmod, argv, fd_in, fd_out);
            break;
        case PS:
            pid = p_spawn(s_ps, argv, fd_in, fd_out);
            break;
        case KILL:
            pid = p_spawn(s_kill, argv, fd_in, fd_out);
            break;
        case ZOMBIFY:
            pid = p_spawn(s_zombify, argv, fd_in, fd_out);
            break;
        case ORPHANIFY:
            pid = p_spawn(s_orphanify, argv, fd_in, fd_out);
            break;
        default:
            return pid;
    }
    return pid;
}