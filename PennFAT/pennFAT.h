#ifndef PENNFAT_H
#define PENNFAT_H

#include "filesys.h"

extern FATConfig *fs_FATConfig;
extern uint16_t *fs_FAT16InMemory;

typedef enum {
    PF_OVERWRITE,
    PF_APPEND
} PF_WRITEMODE;

bool pf_isMounted();

int pf_readFile(char *fileName, int size, char *buffer);

int pf_writeFile(char *fileName, int size, char *buffer, PF_WRITEMODE mode);

int pf_mkfs(char *fsName, int BLOCKS_IN_FAT, int BLOCK_SIZE_CONFIG);

int pf_mount(char *fsName);

int pf_umount();

int pf_touch(char *fileName);

int pf_rm(char *fileName);

int pf_mv();

int pf_cat();

int pf_cp();

int pf_ls();

int pf_chmod();


#endif