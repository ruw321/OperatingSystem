#include "filesys.h"

FATConfig *fs_FATConfig = NULL;
/* 
The FAT region will be mapped to the memory through mmap(). 
For the modifications of the file mapped by mmap(), they may not be written back to the file immediately because of the virtual memory management mechanism. 
Here are some situations that will trigger the write back:
1. msync()
2. mummap()
3. Out of memory
4. Periodic write back by OS

So the consistency of PennFAT may not be ensured.
*/
uint16_t *fs_FAT16InMemory = NULL;

int fs_mkfs(char *fsName, uint16_t blockSizeConfig, uint16_t FATRegionBlockNum) {
    FATConfig *config = createFATConfig("testFS", blockSizeConfig, FATRegionBlockNum);
    int res = createFATOnDisk(config);
    if (res == FS_FAILURE) {
        printf("Error: Fail to create FAT on disk.\n");
    }

    free(config);
    return res;
}

int fs_mount(char *fsName) {
    int fd = open(fsName, O_RDWR);
    if (fd == -1) {
        printf("Error: Fail to open the file system %s.\n", fsName);
        return FS_FAILURE;
    }

    uint16_t FATSpec;
    read(fd, &FATSpec, 2);

    uint16_t LSB = FATSpec & 0x00FF;
    uint16_t MSB = FATSpec >> 8;
    fs_FATConfig = createFATConfig(fsName, LSB, MSB);

    /* Notice that FAT size in memory >= FAT region size. */
    fs_FAT16InMemory = mmap(NULL, fs_FATConfig->FATSizeInMemory, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); // We use MAP_SHARED flag because we may want to examine the FAT through other tool when filesystem is running
    if (fs_FAT16InMemory == MAP_FAILED) {
        printf("Error: Fail to mmap FAT from filesystem %s.\n", fsName);
        return FS_FAILURE;
    }

    #ifdef FS_DEBUG_INFO
    printf("%s is mounted.\n", fsName);
    printf("FAT Spec:\n- Block size: %d\n- FAT region: %d-byte\n- Data region: %d-byte\n", fs_FATConfig->blockSize, fs_FATConfig->FATRegionSize, fs_FATConfig->dataRegionSize);
    #endif

    return FS_SUCCESS;
}

int fs_unmount() {
    if ((fs_FATConfig == NULL) || (fs_FAT16InMemory == NULL)) {
        printf("Error: No mounted file system. Fail to unmount the file system.\n");
        return FS_FAILURE;
    }

    #ifdef FS_DEBUG_INFO
    printf("%s is unmounted.\n", fs_FATConfig->name);
    #endif

    free(fs_FATConfig);
    munmap(fs_FAT16InMemory, fs_FATConfig->FATSizeInMemory);
    fs_FATConfig = NULL;
    fs_FAT16InMemory = NULL;

    return FS_SUCCESS;
}

int fs_touch(char *fileName) {
    if ((fs_FATConfig == NULL) || (fs_FAT16InMemory == NULL)) {
        printf("Error: No mounted file system.\n");
        return FS_FAILURE;
    }

    if (strlen(fileName) > MAX_FILE_NAME_LENGTH) {
        printf("Error: Invalid filename %s.\n", fileName);
        return FS_FAILURE;
    }

    int res = findFileDirectory(fs_FATConfig, fs_FAT16InMemory, fileName);
    if (res == FS_NOT_FOUND) {
        /* Create the file directory. */
        res = createFileDirectoryOnDisk(fs_FATConfig, fs_FAT16InMemory, fileName, FILE_TYPE_REGULAR, FILE_PERM_READ_WRITE);
        if (res == FS_FAILURE) {
            printf("Error: Fail to touch a new file.\n");
            return res;
        }

        #ifdef FS_DEBUG_INFO
        printf("Empty file %s is created.\n", fileName);
        #endif

    } else {
        /* Update the time stamp. */
        int fd = open(fs_FATConfig->name, O_WRONLY);
        int offset = res + 32 + 4 + 2 + 1 + 1;
        lseek(fd, offset, SEEK_SET);
        time_t mtime = time(NULL);
        write(fd, &mtime, sizeof(time_t));
        close(fd);

        #ifdef FS_DEBUG_INFO
        printf("The time stamp of file %s is updated to %s", fileName, ctime(&mtime));
        #endif
    }

    return res;
}

int fs_rm(char *fileName) {
    if ((fs_FATConfig == NULL) || (fs_FAT16InMemory == NULL)) {
        printf("Error: No mounted file system.\n");
        return FS_FAILURE;
    }

    if (strlen(fileName) > MAX_FILE_NAME_LENGTH) {
        printf("Error: Invalid filename %s.\n", fileName);
        return FS_FAILURE;
    }

    int res = findFileDirectory(fs_FATConfig, fs_FAT16InMemory, fileName);
    if (res == FS_NOT_FOUND) {
        printf("Error: Fail to remove %s. No such file.\n", fileName);
        return FS_FAILURE;
    }

    res = deleteFileDirectory(fs_FATConfig, fs_FAT16InMemory, fileName);
    if (res == FS_FAILURE) {
        printf("Error: Fail to delete file directory %s from FAT\n", fileName);
    }

    return res;
}

int fs_mv(char *src, char *dst) {
    if ((fs_FATConfig == NULL) || (fs_FAT16InMemory == NULL)) {
        printf("Error: No mounted file system.\n");
        return FS_FAILURE;
    }

    if (strlen(src) > MAX_FILE_NAME_LENGTH) {
        printf("Error: Invalid filename %s.\n", src);
        return FS_FAILURE;
    }

    if (strlen(dst) > MAX_FILE_NAME_LENGTH) {
        printf("Error: Invalid filename %s.\n", dst);
        return FS_FAILURE;
    }

    int offset = findFileDirectory(fs_FATConfig, fs_FAT16InMemory, src);
    if (offset == FS_NOT_FOUND) {
        printf("Error: Fail to move %s. No such file.\n", src);
        return FS_FAILURE;
    }

    /* Remove dst from FAT. */
    deleteFileDirectory(fs_FATConfig, fs_FAT16InMemory, dst);

    char buffer[MAX_FILE_NAME_LENGTH];
    memset(buffer, '\0', MAX_FILE_NAME_LENGTH);
    strcpy(buffer, dst);

    int fd = open(fs_FATConfig->name, O_WRONLY);
    lseek(fd, offset, SEEK_SET);
    write(fd, buffer, sizeof(char) * MAX_FILE_NAME_LENGTH);
    close(fd);

    #ifdef FS_DEBUG_INFO
    printf("File %s is moved to %s", src, dst);
    #endif

    return FS_SUCCESS;
}

int fs_read(int startBlock, int startBlockOffset, int size, char *buffer) {
    if ((fs_FATConfig == NULL) || (fs_FAT16InMemory == NULL)) {
        printf("Error: No mounted file system.\n");
        return FS_FAILURE;
    }

    if ((startBlock < 2) || (startBlockOffset > fs_FATConfig->FATEntryNum - 1)) {
        printf("Error: Invalid block idx %d.\n", startBlock);
        return FS_FAILURE;
    }

    if ((startBlockOffset < 0) || (startBlockOffset > fs_FATConfig->blockSize)) {
        printf("Error: Invalid block offset %d.\n", startBlockOffset);
        return FS_FAILURE;
    }

    if ((size < 0) || (size > fs_FATConfig->dataRegionSize - fs_FATConfig->blockSize)) {
        printf("Error: Invalid read size %d.\n", size);
        return FS_FAILURE;
    }

    return readFAT(fs_FATConfig, fs_FAT16InMemory, startBlock, startBlockOffset, size, buffer);
}

int fs_write(int startBlock, int startBlockOffset, int size, char *buffer) {
    if ((fs_FATConfig == NULL) || (fs_FAT16InMemory == NULL)) {
        printf("Error: No mounted file system.\n");
        return FS_FAILURE;
    }

    if ((startBlock < 2) || (startBlockOffset > fs_FATConfig->FATEntryNum - 1)) {
        printf("Error: Invalid block idx %d.\n", startBlock);
        return FS_FAILURE;
    }

    if ((startBlockOffset < 0) || (startBlockOffset > fs_FATConfig->blockSize)) {
        printf("Error: Invalid block offset %d.\n", startBlockOffset);
        return FS_FAILURE;
    }

    if ((size < 0) || (size > fs_FATConfig->dataRegionSize - fs_FATConfig->blockSize)) {
        printf("Error: Invalid write size %d.\n", size);
        return FS_FAILURE;
    }

    return writeFAT(fs_FATConfig, fs_FAT16InMemory, startBlock, startBlockOffset, size, buffer);
}