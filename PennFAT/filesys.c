#include "filesys.h"

FATConfig *fs_FATConfig;
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
uint16_t *fs_FAT16InMemory;

int fs_mount(char *fsName) {
    int fd = open(fsName, O_RDONLY);
    uint16_t FATSpec;
    read(fd, &FATSpec, 2);

    uint16_t LSB = FATSpec & 0x00FF;
    uint16_t MSB = FATSpec >> 8;
    fs_FATConfig = createFATConfig(fsName, LSB, MSB);

    /* Notice that FAT size in memory >= FAT region size. */
    fs_FAT16InMemory = mmap(NULL, fs_FATConfig->FATSizeInMemory, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);

    #ifdef DEBUG_INFO
    printf("%s is mounted.\n", fsName);
    #endif

    return 0;
}

int fs_unmount() {
    if (fs_FATConfig == NULL || fs_FAT16InMemory == NULL) {
        printf("Error: No mounted file system. Fail to unmount file system.\n");
        return -1;
    }

    munmap(fs_FAT16InMemory, fs_FATConfig->FATSizeInMemory);

    #ifdef DEBUG_INFO
    printf("%s is unmounted.\n", fs_FATConfig->name);
    #endif

    free(fs_FATConfig);
    return 0;
}

int fs_touch(char *fileName) {
    if (fs_FATConfig == NULL || fs_FAT16InMemory == NULL) {
        printf("Error: No mounted file system.\n");
        return -1;
    }

    int res = findFileDirectoryOnDisk(fs_FATConfig, fs_FAT16InMemory, fileName);
    if (res == -1) {
        /* Create the file directory. */
        res = createFileDirectoryOnDisk(fs_FATConfig, fs_FAT16InMemory, fileName, FILE_TYPE_REGULAR, FILE_PERM_READ_WRITE);
        if (res == -1) {
            printf("Error: Fail to touch a new file.\n");
            return -1;
        }

        #ifdef DEBUG_INFO
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

        #ifdef DEBUG_INFO
        printf("The time stamp of file %s is updated to %s", fileName, ctime(&mtime));
        #endif
    }

    return 0;
}