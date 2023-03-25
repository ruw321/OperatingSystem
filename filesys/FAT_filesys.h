#ifndef FAT_FILESYS_H
#define FAT_FILESYS_H

#include "utils.h"
#include "FAT.h"

// Make a file system using the given parameters
void makeFileSystem(char *fsName, int blocksInFat, int blockSizeConfig);
// Mount a file system
void mountFileSystem(char *fsName);
// Unmount a file system
void unmountFileSystem();
// Create a file
void touchFile(char *fileNames, int numFiles);
// Move a file
void moveFile(char *srcName, char *destName);
// Remove a file
void removeFile(char *fileNames, int numFiles);
// Print the contents of a file, TRUE or FALSE for overwrite and append
void catFile(char *fileName, char* outputFile, bool overwrite, bool append, int numFiles);
// Copy a file, TRUE or FALSE for fromHost and toHost
void copyFile(char *srcName, char *destName, bool fromHost, bool toHost);
// List all files in the file system
void listFiles();
// Change the permissions of a file
void changePermissions(char *mode, char *fileName);

#endif