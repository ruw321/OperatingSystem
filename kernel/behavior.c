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
    pid_t *pids = malloc(sizeof(pid_t) * (cmd->num_commands+1));
    if (handleRedirection(cmd, pids) == false) {
        free(pids);
        return false;
    }
    
    if (cmd->is_background == false) { // In non-interactive mode, & will be ignored

        // tcsetpgrp(STDIN_FILENO, pids[0]); // delegate the terminal control

        bool isStopped = false;
        bool isKilled = false;

        for (int i = 0; i < cmd->num_commands; i++) {
            int wstatus;
            do {
                // if (waitpid(pids[i], &wstatus, WUNTRACED | WCONTINUED) > 0) {
                if (p_waitpid(pids[i], &wstatus, false) > 0) {
                    if (WIFSIGNALED(wstatus)) isKilled = true;
                    if (WIFSTOPPED(wstatus)) isStopped = true;
                }
            } while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus) && !WIFSTOPPED(wstatus));
        }

        // signal(SIGTTOU, SIG_IGN); // ignore the signal from UNIX when the main process come back from the background to get the terminal control
        // tcsetpgrp(STDIN_FILENO, getpid()); // give back the terminal control to the main process

        if (isStopped) {
            Job *newBackgroundJob = createJob(cmd, pids, JOB_STOPPED);
            appendJobList(&_jobList, newBackgroundJob);
            writeNewline();
            writeJobState(newBackgroundJob);
        } else if (isKilled) {
            writeNewline();
            free(pids);
            free(cmd);
        } else {
            free(pids);
            free(cmd);
        }

    } else { // cmd->is_background == true
        Job *newBackgroundJob = createJob(cmd, pids, JOB_RUNNING);
        appendJobList(&_jobList, newBackgroundJob);
        writeJobState(newBackgroundJob);
    }
    return true; 
}

bool handleRedirection(struct parsed_command *cmd, pid_t *pids) {
    if (cmd->num_commands > 0) { 
        /* create n-1 pipes for n commands in the pipeline */
        int pfds[cmd->num_commands - 1][2]; // pipe file descriptors
        for (int i = 0; i < cmd->num_commands - 1; i++) {
            pipe(pfds[i]);
        }

        int fd_in = STDIN_FILENO;
        int fd_out = STDOUT_FILENO;

        for (int idx = 0; idx < cmd->num_commands; idx++) {
            pid_t pid = fork();
            if (pid == 0) { // child process
                /* Set stdin/stdout redirection */
                if (idx == 0) {
                    if (cmd->stdin_file != NULL) {
                        // fd_in = open(cmd->stdin_file, O_RDONLY);
                        fd_in = f_open(cmd->stdin_file, F_READ);
                        dup2(fd_in, STDIN_FILENO);
                    }
                } else {
                    dup2(pfds[idx -1][0], STDIN_FILENO);
                }
                if (idx == cmd->num_commands - 1) {
                    if (cmd->stdout_file != NULL) {
                        // int createMode = O_RDWR | O_CREAT;
                        // if (cmd->is_file_append) {
                        //     createMode |= O_APPEND;
                        // } else {
                        //     createMode |= O_TRUNC;
                        // }
                        // fd_out = open(cmd->stdout_file, createMode, 0644);
                        fd_out = f_open(cmd->stdout_file, F_WRITE);
                        dup2(fd_out, STDOUT_FILENO);
                    }
                } else {
                    dup2(pfds[idx][1], STDOUT_FILENO);
                }

                for (int i = 0; i < cmd->num_commands - 1; i++) {
                    close(pfds[i][0]);
                    close(pfds[i][1]);
                }

                if (executeProgram(cmd->commands[idx], fd_in, fd_out) == false) {
                    return false;
                }
                /* if the command is executed successfully , the child process will end here.*/
                perror(cmd->commands[idx][0]);
                exit(EXIT_FAILURE);
            }
            pids[idx] = pid;
            // Set all piped processes pgid to the pid of the first process
            setpgid(pid, pids[0]);
        }
        
        
        // close all pipe ports
        for (int i = 0; i < cmd->num_commands - 1; i++) {
            close(pfds[i][0]);
            close(pfds[i][1]);
        }
        return true;
    }
    return false;
}

bool executeProgram(char **argv, int fd_in, int fd_out) {
    ProgramType programType = isKnownProgram(argv[0]);
    /* Switch case on user programs */
    switch (programType) {
        case CAT:
            p_spawn(s_cat, argv, fd_in, fd_out);
            break;
        case SLEEP:
            p_spawn(s_sleep, argv, fd_in, fd_out);
            break;
        case BUSY:
            p_spawn(s_busy, argv, fd_in, fd_out);
            break;
        case ECHO:
            p_spawn(s_echo, argv, fd_in, fd_out);
            break;
        case LS:
            p_spawn(s_ls, argv, fd_in, fd_out);
            break;
        case TOUCH:
            p_spawn(s_touch, argv, fd_in, fd_out);
            break;
        case MV:
            p_spawn(s_mv, argv, fd_in, fd_out);
            break;
        case CP:
            p_spawn(s_cp, argv, fd_in, fd_out);
            break;
        case RM:
            p_spawn(s_rm, argv, fd_in, fd_out);
            break;
        case CHMOD:
            p_spawn(s_chmod, argv, fd_in, fd_out);
            break;
        case PS:
            p_spawn(s_ps, argv, fd_in, fd_out);
            break;
        case KILL:
            p_spawn(s_kill, argv, fd_in, fd_out);
            break;
        case ZOMBIFY:
            p_spawn(s_zombify, argv, fd_in, fd_out);
            break;
        case ORPHANIFY:
            p_spawn(s_orphanify, argv, fd_in, fd_out);
            break;
        default:
            return false;
    }
    return true;
}