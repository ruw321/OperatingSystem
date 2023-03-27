#include "FAT.h"

FATConfig *createFATConfig(char *name, uint16_t LSB, uint16_t MSB) {
    FATConfig *newConfig = malloc(sizeof(FATConfig));

    strcpy(newConfig->name, name);
    newConfig->LSB = LSB;
    newConfig->MSB = MSB;
    newConfig->blockSize = 256 << LSB;
    newConfig->blockNum = MSB;
    newConfig->FATRegionSize = newConfig->blockSize * newConfig->blockNum;
    newConfig->FATEntryNum = newConfig->FATRegionSize / FAT_ENTRY_SIZE;
    newConfig->dataRegionSize = newConfig->blockSize * (newConfig->FATEntryNum - 1);

    int pageSize = (int) sysconf(_SC_PAGESIZE);
    newConfig->FATSizeInMemory = newConfig->FATRegionSize + pageSize - newConfig->FATRegionSize % pageSize; // align to page size
    return newConfig;
}

uint16_t *createFAT16InMemory(FATConfig *config) {
    uint16_t *newFAT16 = malloc(config->FATSizeInMemory); // uint16_t is of size 2-byte, therefore, it can be considered as the entry of PennFAT which is based on FAT16
    memset(newFAT16, 0x0000, config->FATSizeInMemory); // initialize FAT entries to 0x0000

    newFAT16[0] = config->LSB + (config->MSB << 8); // The first entry of the FAT will specify the config.
    newFAT16[1] = 0xFFFF; // The first block of data region will be used for root directory.
    /* The remaining entries are set to 0x0000. */

    return newFAT16;
}

/* Because of endianness, hexdump may display FAT differently from the one in memory.*/
int createFATOnDisk(FATConfig *config) {
    int fd = open(config->name, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);

    /* write the FAT region to file */
    uint16_t *FAT = createFAT16InMemory(config);
    write(fd, FAT, config->FATRegionSize);
    free(FAT);

    /* claim space for the data region */
    char writeBuffer[config->blockSize];
    memset(writeBuffer, '0', config->blockSize);
    for (int i = 1; i < config->FATEntryNum; i++) {
        write(fd, writeBuffer, config->blockSize);
    }

    close(fd);
    return 0;
}

int findEmptyFAT16Entry(FATConfig *config, uint16_t *FAT16) {
    for (int i = 1; i < config->FATEntryNum; i++) { // the first and second entry of FAT are config and root directory
        if (FAT16[i] == 0x0000) {
            return i;
        }
    }

    return -1; // no empty entry
}

DirectoryEntry *createDirectoryEntry(char *name, uint32_t size, uint16_t firstBlock, uint8_t type, uint8_t perm) {
    DirectoryEntry *newEntry = malloc(DIRECTORY_ENTRY_SIZE);
    memset(newEntry, 0, DIRECTORY_ENTRY_SIZE); // initialize the memory to 0 for the reserved space

    strcpy(newEntry->name, name);
    newEntry->size = size;
    newEntry->firstBlock = firstBlock;
    newEntry->type = type;
    newEntry->perm = perm;
    newEntry->mtime = time(NULL);
    /* the reserved space is set to 0 */

    return newEntry;
}

int createFileDirectoryOnDisk(FATConfig *config, uint16_t *FAT16, char *fileName, uint8_t fileType, uint8_t filePerm) {
    int fd = open(config->name, O_RDWR);
    int dataRegionOffset = config->FATRegionSize;
    int blockSize = config->blockSize;
    
    uint16_t curBlock = 1; // root directory is the first entry of the data region

    char marker; // name[0] will serve as a special marker to indicate the state of the directory entry
    while (true) { // Traverse all directory blocks.
        for (int i = 0; i < blockSize / DIRECTORY_ENTRY_SIZE; i++) { // Check all directory entries.
            int offset = dataRegionOffset + (curBlock - 1) * blockSize + i * DIRECTORY_ENTRY_SIZE;
            lseek(fd, offset, SEEK_SET);
            read(fd, &marker, sizeof(char));
            if (marker == DIRECTORY_END || marker == DELETED_DIRECTORY) {
                /* Write directory entry. */
                DirectoryEntry *newEntry = createDirectoryEntry(fileName, 0, 0xFFFF, fileType, filePerm);
                lseek(fd, offset, SEEK_SET);
                write(fd, newEntry, DIRECTORY_ENTRY_SIZE);

                free(newEntry);
                close(fd);
                return 0;
            }   
        }

        if (FAT16[curBlock] == 0xFFFF) { // There is no available entry in the current directory block and there is not another directory block.

            int newDirectoryBlock = findEmptyFAT16Entry(config, FAT16);

            if (newDirectoryBlock == -1) { // There is no empty block.
                printf("Error: Fail to create a new directory block.\n");
                close(fd);
                return -1;
            }

            #ifdef DEBUG_INFO
            printf("Claim a new directory block with idx = %d\n", newDirectoryBlock);
            #endif

            FAT16[curBlock] = newDirectoryBlock;

            /* Initialize the new directory block. */
            FAT16[newDirectoryBlock] = 0xFFFF;

            int offset = dataRegionOffset + (newDirectoryBlock - 1) * blockSize;
            lseek(fd, offset, SEEK_SET);

            char writeBuffer[blockSize];
            memset(writeBuffer, '0', blockSize);
            write(fd, writeBuffer, blockSize);
        }

        curBlock = FAT16[curBlock]; // Go to the next directory block.
    }

    printf("Error: Something unexpected happened in createFileDirectoryOnDisk().\n");
    close(fd);
    return -1;

}

int findFileDirectoryOnDisk(FATConfig *config, uint16_t *FAT16, char *fileName) {
    int fd = open(config->name, O_RDONLY);
    int dataRegionOffset = config->FATRegionSize;
    int blockSize = config->blockSize;
    
    uint16_t curBlock = 1; // root directory is the first entry of the data region
    char *readBuffer = malloc(MAX_FILE_NAME_LENGTH);

    while (true) { // Traverse all directory blocks.
        for (int i = 0; i < blockSize / DIRECTORY_ENTRY_SIZE; i++) { // Check all directory entries.
            int offset = dataRegionOffset + (curBlock - 1) * blockSize + i * DIRECTORY_ENTRY_SIZE;
            lseek(fd, offset, SEEK_SET);
            read(fd, readBuffer, sizeof(char) * MAX_FILE_NAME_LENGTH);
            if (strcmp(readBuffer, fileName) == 0) {

                #ifdef DEBUG_INFO
                printf("File directory of %s is found.\n", fileName);
                #endif

                free(readBuffer);
                close(fd);
                return offset;
            }
        }

        if (FAT16[curBlock] == 0xFFFF) { // There is no available entry in the current directory block and there is not another directory block.
            break;
        } else {
            curBlock = FAT16[curBlock];
        }

    }

    #ifdef DEBUG_INFO
    printf("File directory of %s does not exist.\n", fileName);
    #endif

    free(readBuffer);
    close(fd);
    return -1;
}