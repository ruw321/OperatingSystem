#include "programs.h"

int argc(char *argv[]) {
    int count = 0;
    while (argv[count] != NULL) {
        count++;
    }
    return count-1;
}

/* Shell Built-in Programs*/

void s_cat(char *argv[]) {

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
    
}

void s_ls(char *argv[]) {
    printf("Yes, s_ls is executed!\n");
    // int count = argc(argv);
    // if (count == 1) {
    //     f_ls(".");
    // } else if (count == 2) {
    //     f_ls(argv[1]);
    // } else {
    //     printf("ls: too many arguments\n");
    // }
}

void s_touch(char *argv[]) {

}

void s_mv(char *argv[]) {

}

void s_cp(char *argv[]) {

}

void s_rm(char *argv[]) {

}

void s_chmod(char *argv[]) {

}

void s_ps(char *argv[]) {

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

void s_zombify(char *argv[]) {
    
}

void s_orphanify(char *argv[]) {

}