#ifndef PENNFAT_H
#define PENNFAT_H

#include "filesys.h"
#include "signal.h"

#define PF_MAX_BUFFER_SIZE 32512
#define PF_MAX_FILE_NUM 4

extern FATConfig *fs_FATConfig;
extern uint16_t *fs_FAT16InMemory;

typedef enum {
    PF_OVERWRITE,
    PF_APPEND,
    PF_STDOUT
} PF_WRITEMODE;

bool pf_isMounted();

int pf_readFile(const char *fileName, int size, char *buffer);

int pf_writeFile(const char *fileName, int size, const char *buffer, PF_WRITEMODE mode);

int pf_mkfs(const char *fsName, int BLOCKS_IN_FAT, int BLOCK_SIZE_CONFIG);

int pf_mount(const char *fsName);

int pf_umount();

int pf_touch(const char *fileName);

int pf_rm(const char *fileName);

int pf_mv(const char *src, const char *dst);

int pf_ls();

int pf_chmod(const char *fileName, uint8_t perm);

int pf_catFiles(char **fileNames, int fileNum, int *size, char *buffer);


#define MAX_LINE_LENGTH 4096
#define MAX_ARGS_NUM 8

#define EXIT_SHREDDER -1
#define EMPTY_LINE 0
#define EXECUTE_COMMAND 1

void SIGINTHandler(int sig);

char *readInput(char *inputBuffer);

int parseInput(char *userInput, char **argsBuffer, int *argNum);

#endif