#ifndef FILESYS_H
#define FILESYS_H

#include "utils.h"
#include "FAT.h"

int fs_mkfs(const char *fsName, uint16_t blockSizeConfig, uint16_t FATRegionBlockNum);

/* 
Mount the file system. 
Return 0 if success.
*/
int fs_mount(const char *fsName);

/* 
Unmount the file system.
Return 0 if success.
*/
int fs_unmount();

/* 
Creates the file if it does not exist, otherwise update its timestamp.
Return the offset of the file directory entry. 
*/
int fs_touch(const char *fileName);

/* 
Remove the file from FAT. 
Return 0 if success
*/
int fs_rm(const char *fileName);

/* 
Rename src to dst. If dst exists, remove it from FAT.
Return 0 if success
*/
int fs_mv(const char *src, const char *dst);

/* 
Duplicate src to dst. If dst exists, replace it.
Return 0 if success
*/
int fs_cp(const char *src, const char *dst);

/* 
Encapsulation of readFAT() 
Return 0 if success
*/
int fs_readFAT(int startBlock, int startBlockOffset, int size, char *buffer);

/* 
Encapsulation of writeFAT() 
Return 0 if success
*/
int fs_writeFAT(int startBlock, int startBlockOffset, int size, char *buffer);

#endif