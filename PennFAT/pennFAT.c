#include "pennFAT.h"

bool pf_isMounted() {
    if ((fs_FATConfig == NULL) || (fs_FAT16InMemory == NULL)) {
        return false;
    } else {
        return true;
    }
}

int pf_readFile(char *fileName, int size, char *buffer) {
    if (pf_isMounted() == false) {
        printf("Error: No mounted file system.\n");
        return -1;
    }

    int directoryEntryOffset = findFileDirectory(fs_FATConfig, fs_FAT16InMemory, fileName);
    if (directoryEntryOffset == FS_NOT_FOUND) {
        printf("Error: Fail to read %s. No such file.\n", fileName);
        return -1;
    }

    DirectoryEntry dir;
    readDirectoryEntry(fs_FATConfig, directoryEntryOffset, &dir);
    if (dir.firstBlock != NO_FIRST_BLOCK) {
        int startBlock = dir.firstBlock;
        int startBlockOffset = 0;
        fs_read(startBlock, startBlockOffset, size, buffer);
    }

    return 0;
}

int pf_writeFile(char *fileName, int size, char *buffer, PF_WRITEMODE mode) {
    if (pf_isMounted() == false) {
        printf("Error: No mounted file system.\n");
        return -1;
    }

    int directoryEntryOffset = findFileDirectory(fs_FATConfig, fs_FAT16InMemory, fileName);

    DirectoryEntry dir;
    int startBlock;
    int startBlockOffset;

    if (directoryEntryOffset == FS_NOT_FOUND) {
        directoryEntryOffset = fs_touch(fileName); // create a new file directory
    } else {
        if (mode == PF_OVERWRITE) {
            deleteFileDirectory(fs_FATConfig, fs_FAT16InMemory, fileName); // delete the original file directory
        }
        directoryEntryOffset = fs_touch(fileName);
    } 
    
    readDirectoryEntry(fs_FATConfig, directoryEntryOffset, &dir);
    if (dir.firstBlock == NO_FIRST_BLOCK) {
        startBlock = findEmptyFAT16Entry(fs_FATConfig, fs_FAT16InMemory);
        startBlockOffset = 0;
        fs_FAT16InMemory[startBlock] = NO_SUCC_FAT_ENTRY;
        dir.firstBlock = (uint16_t) startBlock;
    } else {
        int fileEndOffset = traceFileEnd(fs_FATConfig, fs_FAT16InMemory, fileName);
        startBlockOffset = fileEndOffset % fs_FATConfig->blockSize;
        startBlock = (fileEndOffset - fs_FATConfig->FATRegionSize) / fs_FATConfig->blockSize + 1;
    }
    
    int res = fs_write(startBlock, startBlockOffset, size, buffer);
    dir.mtime = time(NULL);
    dir.size += res;
    writeFileDirectory(fs_FATConfig, directoryEntryOffset, &dir);
    
    return 0;
}

int pf_mkfs(char *fsName, int BLOCKS_IN_FAT, int BLOCK_SIZE_CONFIG) {
    if ((BLOCKS_IN_FAT < 1) || (BLOCKS_IN_FAT > 32)) {
        printf("Error: Invalid BLOCKS_IN_FAT.");
        return FS_FAILURE;
    }

    if ((BLOCK_SIZE_CONFIG < 0) || (BLOCK_SIZE_CONFIG > 4)) {
        printf("Error: Invalid BLOCK_SIZE_CONFIG.");
        return FS_FAILURE;
    }

    return fs_mkfs(fsName, BLOCKS_IN_FAT, BLOCK_SIZE_CONFIG);
}

int pf_mount(char *fsName) {
    return fs_mount(fsName);
}

int pf_umount() {
    return fs_unmount();
}

int pf_touch(char *fileName) {
    return fs_touch(fileName);
}

int pf_rm(char *fileName) {
    return fs_rm(fileName);
}


// int main(int argc, char **argv) {

//     return 0;
// }