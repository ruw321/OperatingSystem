#include "behavior.h"

int argc(struct parsed_command *cmd) {
    int count = 0;
    while (cmd->commands[count] != NULL) {
        count++;
    }
    return count-1;
}

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

ProgramType isKnownProgram(struct parsed_command *cmd) {
    if (strcmp(*cmd->commands[0], "cat") == 0) {
        return CAT;
    } else if (strcmp(*cmd->commands[0], "sleep") == 0) {
        return SLEEP;
    } else if (strcmp(*cmd->commands[0], "busy") == 0) {
        return BUSY;
    } else if (strcmp(*cmd->commands[0], "echo") == 0) {
        return ECHO;
    } else if (strcmp(*cmd->commands[0], "ls") == 0) {
        return LS;
    } else if (strcmp(*cmd->commands[0], "touch") == 0) {
        return TOUCH;
    } else if (strcmp(*cmd->commands[0], "mv") == 0) {
        return MV;
    } else if (strcmp(*cmd->commands[0], "cp") == 0) {
        return CP;
    } else if (strcmp(*cmd->commands[0], "rm") == 0) {
        return RM;
    } else if (strcmp(*cmd->commands[0], "chmod") == 0) {
        return CHMOD;
    } else if (strcmp(*cmd->commands[0], "ps") == 0) {
        return PS;
    } else if (strcmp(*cmd->commands[0], "kill") == 0) {
        return KILL;
    } else if (strcmp(*cmd->commands[0], "zombify") == 0) {
        return ZOMBIFY;
    } else if (strcmp(*cmd->commands[0], "orphanify") == 0) {
        return ORPHANIFY;
    } else {
        return UNKNOWN;
    }
}

bool executeProgram(struct parsed_command *cmd) {
    ProgramType programType = isKnownProgram(cmd);
    if (programType == UNKNOWN) {
        return false;
    } else {
        /* Switch case on user programs */
        switch (programType) {
            case CAT:
                s_cat(cmd);
                break;
            case SLEEP:
                s_sleep(cmd);
                break;
            case BUSY:
                s_busy(cmd);
                break;
            case ECHO:
                s_echo(cmd);
                break;
            case LS:
                s_ls(cmd);
                break;
            case TOUCH:
                s_touch(cmd);
                break;
            case MV:
                s_mv(cmd);
                break;
            case CP:
                s_cp(cmd);
                break;
            case RM:
                s_rm(cmd);
                break;
            case CHMOD:
                s_chmod(cmd);
                break;
            case PS:
                s_ps(cmd);
                break;
            case KILL:
                s_kill(cmd);
                break;
            case ZOMBIFY:
                s_zombify(cmd);
                break;
            case ORPHANIFY:
                s_orphanify(cmd);
                break;
            default:
                return false;
        }
        return true;
    }
    return false;
}

void s_cat(struct parsed_command *cmd) {

}

void s_sleep(struct parsed_command *cmd) {
    int count = argc(cmd);
    if (count == 1) {
        printf("sleep: missing operand (sleep for how long?)\n");
    } else if (count > 2) {
        printf("sleep: too many arguments\n");
    } else {
        int sleepTime = atoi(*cmd->commands[1]) * 10;
        if (sleepTime == 0) {
            printf("sleep: invalid time interval '%s'\n", *cmd->commands[1]);
        } else {
            p_sleep(sleepTime);
        }
    }
}

void s_busy(struct parsed_command *cmd) {
    while (true);
}

void s_echo(struct parsed_command *cmd) {
    
}

void s_ls(struct parsed_command *cmd) {
    int count = argc(cmd);
    if (count == 1) {
        f_ls(".");
    } else if (count == 2) {
        f_ls(*cmd->commands[1]);
    } else {
        printf("ls: too many arguments\n");
    }
}

void s_touch(struct parsed_command *cmd) {

}

void s_mv(struct parsed_command *cmd) {

}

void s_cp(struct parsed_command *cmd) {

}

void s_rm(struct parsed_command *cmd) {

}

void s_chmod(struct parsed_command *cmd) {

}

void s_ps(struct parsed_command *cmd) {

}

void s_kill(struct parsed_command *cmd) {
    int count = argc(cmd);
    if (count == 1) {
        printf("kill: missing operand (kill what?)\n");
    } else if (count == 2) {
        if (p_kill(atoi(*cmd->commands[1]), S_SIGTERM) == -1) {
            printf("kill: invalid process id '%s'\n", *cmd->commands[1]);
        }
    } else {
        int signal = S_SIGTERM;
        if (strcmp(*cmd->commands[1], "term") == 0) {
            for (int i = 2; i < count; i++) {
                if (p_kill(atoi(*cmd->commands[i]), signal) == -1) {
                    printf("kill: invalid process id '%s'\n", *cmd->commands[i]);
                }
            }
        } else if (strcmp(*cmd->commands[1], "stop") == 0) {
            signal = S_SIGSTOP;
            for (int i = 2; i < count; i++) {
                if (p_kill(atoi(*cmd->commands[i]), signal) == -1) {
                    printf("kill: invalid process id '%s'\n", *cmd->commands[i]);
                }
            }
        } else if (strcmp(*cmd->commands[1], "cont") == 0) {
            signal = S_SIGCONT;
            for (int i = 2; i < count; i++) {
                if (p_kill(atoi(*cmd->commands[i]), signal) == -1) {
                    printf("kill: invalid process id '%s'\n", *cmd->commands[i]);
                }
            }
        } else {
            for (int i = 1; i < count; i++) {
                if (p_kill(atoi(*cmd->commands[i]), signal) == -1) {
                    printf("kill: invalid process id '%s'\n", *cmd->commands[i]);
                }
            }
        }
    }
}

void s_zombify(struct parsed_command *cmd) {
    
}

void s_orphanify(struct parsed_command *cmd) {

}