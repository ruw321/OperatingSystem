#ifndef FD_TABLE_H
#define FD_TABLE_H

#include "utils.h"
#include "FAT.h"

extern FATConfig *fs_FATConfig;
extern uint16_t *fs_FAT16InMemory;

#define MAX_FILE_DESCRIPTOR 8

/* file descriptor 0,1,2 were reserved */
#define F_STDIN_FD 0
#define F_STDOUT_FD 1
#define F_ERROR 2
#define F_MIN_FD 3

#define F_WRITE 0
#define F_READ 1
#define F_APPEND 2

/* We use linked list to implement the file descriptor table. */


/*
There are three open mode supported by PennFAT: F_WRITE, F_READ and F_APPEND.
According to ed #953, each file can only be read/write exclusivly which means only one instance can open() a file at a time.
Under F_APPEND mode, the fileOffset will be set to the end of the file initially and it can only be increased.
Under F_WRITE/F_APPEND mode, if the fileOffset is set to the position beyond the file size, the file system will occupy the space for the gap.
As a result, the size of the file will increase, however, the gap space may contain uninitialized contents.
Under F_READ mode, if the fileOffset is set to the position beyond the file size, f_read() will read nothing.
*/
typedef struct FdNode {
    int openMode;
    int directoryEntryOffset; // directory entry location
    /* If a file is an empty file, the fileOffset will be set to 0. */
    int fileOffset;

    struct FdNode* prev;
    struct FdNode* next;
} FdNode;

typedef struct FdTable {
    FdNode *head;
    FdNode *tail;
} FdTable;

FdNode *createFdNode(int openMode, int directoryEntryOffset, int fileOffset);
int initFdTable(FdTable *fdTable);
int clearFdTable(FdTable *fdTable);
int appendFdTable(FdTable *fdTable, FdNode *newNode);
int removeFdNode(FdNode *fdNode);
bool isFileBeingUsed(FdTable *fdTable, int directoryEntryOffset);
bool isFileBeingWritten(FdTable *fdTable, int directoryEntryOffset);

int findAvailableFd(FdNode **fds);

#endif