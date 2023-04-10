#ifndef PROGRAMS_H
#define PROGRAMS_H
#include "behavior.h"

/* Known user programs */
void s_cat(char *argv[]);
void s_sleep(char *argv[]);
void s_busy(char *argv[]);
void s_echo(char *argv[]);
void s_ls(char *argv[]);
void s_touch(char *argv[]);
void s_mv(char *argv[]);
void s_cp(char *argv[]);
void s_rm(char *argv[]);
void s_chmod(char *argv[]);
void s_ps(char *argv[]);
void s_kill(char *argv[]);
void s_zombify(char *argv[]);
void s_orphanify(char *argv[]);

#endif