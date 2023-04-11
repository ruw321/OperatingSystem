#include "behavior.h"

void writePrompt() {
    f_write(F_ERROR, PROMPT, strlen(PROMPT));
}

void readUserInput(char **line) {
    char inputBuffer[S_MAX_LINE_LENGTH];
    int numBytes = f_read(F_STDIN_FD, S_MAX_LINE_LENGTH, inputBuffer);
   
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
    LineType lineType = S_EXECUTE_COMMAND;

    if (line == NULL) {
        lineType = S_EXIT_SHELL;
        return lineType;
    }

    if (line[0] == '\0') {
        lineType = S_EMPTY_LINE;
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
    } else if (strcmp(argv, "test_bg") == 0) {
        return TEST_BG;
    } else {
        return UNKNOWN;
    }
}

bool executeLine(struct parsed_command *cmd) {

    int fd_in = F_STDIN_FD;
    int fd_out = F_STDOUT_FD;
    
    // TODO: handle the redirection
    if (cmd->stdin_file != NULL) {
        int res = f_open(cmd->stdin_file, F_READ);
        if (res < 0) {
            return false;
        }
        fd_in = res;
    }

    if (cmd->stdout_file != NULL) {
        if (cmd->is_file_append) {
            fd_out = f_open(cmd->stdout_file, F_APPEND);
        } else {
            fd_out = f_open(cmd->stdout_file, F_WRITE);
        }
    }

    pid_t pid = executeProgram(*cmd->commands, fd_in, fd_out);
    
    if (!cmd->is_background) {
        int wstatus;
        p_waitpid(pid, &wstatus, false);
        if (cmd->stdin_file != NULL) {
            f_close(fd_in);
        }
        if (cmd->stdout_file != NULL) {
            f_close(fd_out);
        }
    } else {
        
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
        case TEST_BG:
            pid = p_spawn(s_test_bg, argv, fd_in, fd_out);
            break;
        default:
            return pid;
    }
    return pid;
}