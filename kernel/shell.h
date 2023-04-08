#ifndef SHELL_H
#define SHELL_H

#include "utils.h"
#include "job.h"
#include "user.h"
#include "behavior.h"
#include "../PennFAT/filesys.h"
#include "../PennFAT/interface.h"

void shell_process();

int shell_init(int argc, const char **argv);

#endif