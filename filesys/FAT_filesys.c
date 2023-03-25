#include "FAT_filesys.h"

void makeFileSystem(char *fsName, int blocksInFat, int blockSizeConfig) {
    int blockSize = 256 << blockSizeConfig;
    FAT16 *fat16 = createFAT16(fsName, blocksInFat, blockSize);
}

void mountFileSystem(char *fsName) {
    
}

void unmountFileSystem() {
    
}

void touchFile(char *fileNames, int numFiles) {
    
}

void moveFile(char *oldName, char *newName) {
    
}

void removeFile(char *fileNames, int numFiles) {
    
}

void catFile(char *fileName, char* outputFile, bool overwrite, bool append, int numFiles) {
    
}

void copyFile(char *srcName, char *destName, bool fromHost, bool toHost) {
    
}

void listFiles() {
    
}

void changePermissions(char *fileName, char *permissions) {
    
}
