#ifndef SHELL_H
#define SHELL_H

#include "utils.h"
#include "job.h"
#include "user.h"
#include "behavior.h"
#include "log.h"
#include "../PennFAT/filesys.h"
#include "../PennFAT/interface.h"

void shell_process();

int shell_init(int argc, const char **argv);

bool isBuildinCommand(struct parsed_command *cmd);

bool isKnownProgram(struct parsed_command *cmd);


#endif