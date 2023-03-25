#ifndef FAT_H
#define FAT_H

#include "utils.h"

#define BLOCK_END 0xFFFF
#define BLOCK_FREE 0x0000
#define ROOT_DIR_BLOCK 0x0001
#define FILE_START_BLOCK 0x0002

typedef struct FAT16 {
    char name[32];              // name of the file system
    uint16_t blockSize;         // size of a block
    uint16_t blockNum;          // number of blocks
    uint16_t fatSize;           // size of the FAT
    uint16_t fatEntryNum;       // number of entries in the FAT
    uint16_t dataRegionSize;    // size of the data region
    uint16_t *fat;              // FAT array
    bool mounted;               // if the file system is mounted
} FAT16;

FAT16 mountedFAT16;

// create a FAT16
FAT16 *createFAT16(char *name, uint16_t blockNum, uint16_t blockSize);
// delete a FAT16
void deleteFAT16(FAT16 *fat16);



#endif