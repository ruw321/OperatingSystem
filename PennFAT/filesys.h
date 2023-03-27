#ifndef FILESYS_H
#define FILESYS_H

#include "utils.h"
#include "FAT.h"

int fs_mount(char *fsName);

int fs_unmount();

int fs_touch(char *fileName);

#endif