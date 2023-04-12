#include "programs.h"

int argc(char *argv[]) {
    int count = 0;
    while (argv[count] != NULL) {
        count++;
    }
    return count;
}

void s_cat(char *argv[]) {
    int count = argc(argv);
    char buffer[READ_BUFFER_SIZE];
    memset(buffer, 0, READ_BUFFER_SIZE);
    int bytesRead = 0;

    if (count == 1) {
        do {
            bytesRead = f_read(F_STDIN_FD, READ_BUFFER_SIZE, buffer);
            if (bytesRead == FAILURE) {
                printf("cat: error reading from stdin\n");
                return;
            }

            if (f_write(F_STDOUT_FD, buffer, bytesRead) == FAILURE) {
                printf("cat: error writing to stdout\n");
                return;
            }
        } while (bytesRead > 0);
    } else {
        for (int i = 1; i < count; i ++) {
            int fd = f_open(argv[i], F_READ);
            if (fd == FAILURE) {
                printf("cat: %s: No such file or directory\n", argv[i]);
                return;
            }
            do {
                bytesRead = f_read(fd, READ_BUFFER_SIZE, buffer);
                if (bytesRead == FAILURE) {
                    printf("cat: error reading from %s\n", argv[i]);
                    return;
                }
                if (f_write(F_STDOUT_FD, buffer, bytesRead) == FAILURE) {
                    printf("cat: error writing to stdout\n");
                    return;
                }
            } while (bytesRead > 0);

            if (f_close(fd) == FAILURE) {
                printf("cat: error closing file\n");
                return;
            }
        }
    }
    
}

void s_sleep(char *argv[]) {
    printf("sleeping...\n");
    int count = argc(argv);
    if (count == 1) {
        printf("sleep: missing operand (sleep for how long?)\n");
    } else if (count > 2) {
        printf("sleep: too many arguments\n");
    } else {
        int sleepTime = atoi(argv[1]) * 10;
        if (sleepTime == 0) {
            printf("sleep: invalid time interval '%s'\n", argv[1]);
        } else {
            p_sleep(sleepTime);
        }
    }
    
}

void s_busy(char *argv[]) {
    while (true);
    
}

void s_echo(char *argv[]) {
    int count = argc(argv);
    for (int i = 1; i < count; i++) {
        if (f_write(F_STDOUT_FD, argv[i], strlen(argv[i])) == FAILURE) {
            printf("echo: error writing to stdout\n");
            return;
        }
        if (i != count - 1) {
            if (f_write(F_STDOUT_FD, " ", 1) == FAILURE) {
                printf("echo: error writing to stdout\n");
                return;
            }
        } else {
            if (f_write(F_STDOUT_FD, "\n", 1) == FAILURE) {
                printf("echo: error writing to stdout\n");
                return;
            }
        }
    }
    
}

void s_ls(char *argv[]) {
    int count = argc(argv);
    if (count == 1) {
        f_ls(NULL);
    } else if (count == 2) {
        f_ls(argv[1]);
    } else {
        printf("ls: too many arguments\n");
    }
    
}

void s_touch(char *argv[]) {
    int count = argc(argv);
    if (count == 1) {
        printf("touch: missing operand (touch what?)\n");
    } else {
        for (int i = 1; i < count; i++) {
            int fd = f_open(argv[i], F_APPEND);
            if (fd == FAILURE) {
                printf("touch: %s: f_open failed\n", argv[i]);
                return;
            }
            if (f_close(fd) == FAILURE) {
                printf("touch: %s: f_close failed\n", argv[i]);
                return;
            }
        }
    }
    
}

void s_mv(char *argv[]) {
    int count = argc(argv);
    if (count == 1) {
        printf("mv: missing operand (mv what?)\n");
    } else if (count == 2) {
        printf("mv: missing operand (mv where?)\n");
    } else if (count > 3) {
        printf("mv: too many arguments\n");
    } else {
        if (fs_mv(argv[1], argv[2]) == FS_FAILURE) {
            printf("mv: %s: No such file or directory\n", argv[1]);
        }
    }
    
}

void s_cp(char *argv[]) {
    int count = argc(argv);
    if (count == 1) {
        printf("cp: missing operand (cp what?)\n");
    } else if (count == 2) {
        printf("cp: missing operand (cp where?)\n");
    } else if (count > 3) {
        printf("cp: too many arguments\n");
    } else {
        if (fs_cp(argv[1], argv[2]) == FS_FAILURE) {
            printf("cp: %s: No such file or directory\n", argv[1]);
        }
    }
    
}

void s_rm(char *argv[]) {
    int count = argc(argv);
    if (count == 1) {
        printf("rm: missing operand (rm what?)\n");
    } else {
        for (int i = 1; i < count; i++) {
            if (fs_rm(argv[i]) == FS_FAILURE) {
                printf("rm: %s: No such file or directory\n", argv[i]);
            }
        }
    }
    
}

void s_chmod(char *argv[]) {
    int count = argc(argv);
    if (count != 3) {
        printf("Error\n");
    } else {
        char *permOperation = argv[1];
        char *fileName = argv[2];
        if (f_find(fileName)) {
            if (strcmp(permOperation, "+x") == 0) {
                fs_chmod(fileName, 7);
            } else {
                /* TODO */
            }
        } else {
            printf("Error: No such file %s\n", argv[2]);
        }
    }
    
}

void s_ps(char *argv[]) {
    printf("PID PPID PRI STAT CMD\n");
//     pcb_node * node;
//     pcb* pcb = node->pcb;
//     enum process_state state = pcb->state;
//     if (state == RUNNING || state == READY) {
//         printf("%3d %4d %3d  R   %s\n", pcb->pid, pcb->ppid, pcb->priority, pcb->name);
//     } else if (state == BLOCKED) {
//         printf("%3d %4d %3d  B   %s\n", pcb->pid, pcb->ppid, pcb->priority, pcb->name);
//     } else if (state == STOPPED) {
//         printf("%3d %4d %3d  S   %s\n", pcb->pid, pcb->ppid, pcb->priority, pcb->name);
//     } else if (state == TERMINATED || state == TERMINATED) {
//         printf("%3d %4d %3d  Z   %s\n", pcb->pid, pcb->ppid, pcb->priority, pcb->name);
//     } else {
//         printf("%3d %4d %3d  ?   %s\n", pcb->pid, pcb->ppid, pcb->priority, pcb->name);
//     }
    
}

void s_kill(char *argv[]) {
    int count = argc(argv);
    if (count == 1) {
        printf("kill: missing operand (kill what?)\n");
    } else if (count == 2) {
        if (p_kill(atoi(argv[1]), S_SIGTERM) == -1) {
            printf("kill: invalid process id '%s'\n", argv[1]);
        }
    } else {
        int signal = S_SIGTERM;
        if (strcmp(argv[1], "term") == 0) {
            for (int i = 2; i < count; i++) {
                if (p_kill(atoi(argv[i]), signal) == -1) {
                    printf("kill: invalid process id '%s'\n", argv[i]);
                }
            }
        } else if (strcmp(argv[1], "stop") == 0) {
            signal = S_SIGSTOP;
            for (int i = 2; i < count; i++) {
                if (p_kill(atoi(argv[i]), signal) == -1) {
                    printf("kill: invalid process id '%s'\n", argv[i]);
                }
            }
        } else if (strcmp(argv[1], "cont") == 0) {
            signal = S_SIGCONT;
            for (int i = 2; i < count; i++) {
                if (p_kill(atoi(argv[i]), signal) == -1) {
                    printf("kill: invalid process id '%s'\n", argv[i]);
                }
            }
        } else {
            for (int i = 1; i < count; i++) {
                if (p_kill(atoi(argv[i]), signal) == -1) {
                    printf("kill: invalid process id '%s'\n", argv[i]);
                }
            }
        }
    }
    
}

void zombie_child() {
    
}

void orphan_child() {
    while (true);
}

void s_zombify(char *argv[]) {
    p_spawn(zombie_child, NULL, F_STDIN_FD, F_STDOUT_FD);
    while (true);
    
}

void s_orphanify(char *argv[]) {
    p_spawn(orphan_child, NULL, F_STDIN_FD, F_STDOUT_FD);
    
}

void s_hang(char *argv[]) {
    hang();
}

void s_nohang(char *argv[]) {
    nohang();
}   

void s_recur(char *argv[]) {
    recur();
}

void s_test(char *argv[]) {
    printf("start test\n");
    long long i = 0;
    while (i < 20000000000) {
        i++;
    }
    
    printf("end test\n");
    
}