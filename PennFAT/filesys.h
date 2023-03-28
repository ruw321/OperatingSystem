#ifndef FILESYS_H
#define FILESYS_H

#include "utils.h"
#include "FAT.h"

int fs_mkfs(char *fsName, uint16_t blockSizeConfig, uint16_t FATRegionBlockNum);

/* 
Mount the file system. 
Return 0 if success.
*/
int fs_mount(char *fsName);

/* 
Unmount the file system.
Return 0 if success.
*/
int fs_unmount();

/* 
Creates the file if it does not exist, otherwise update its timestamp.
Return the offset of the file directory entry. 
*/
int fs_touch(char *fileName);

/* 
Remove the file from FAT. 
Return 0 if success
*/
int fs_rm(char *fileName);

/* 
Rename src to dst. If dst exists, remove it from FAT.
Return 0 if success
*/
int fs_mv(char *src, char *dst);

/* 
Encapsulation of readFAT() 
Return 0 if success
*/
int fs_read(int startBlock, int startBlockOffset, int size, char *buffer);

/* 
Encapsulation of writeFAT() 
Return 0 if success
*/
int fs_write(int startBlock, int startBlockOffset, int size, char *buffer);

#endif