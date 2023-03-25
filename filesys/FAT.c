#include "FAT.h"

FAT16 *createFAT16(char *name, uint16_t blockNum, uint16_t blockSize) {
    FAT16 *fat16 = (FAT16 *)malloc(sizeof(FAT16));
    strcpy(fat16->name, name);
    fat16->blockSize = blockSize;
    fat16->blockNum = blockNum;
    fat16->fatSize = blockNum * blockSize;
    fat16->fatEntryNum = fat16->fatSize / 2;
    fat16->dataRegionSize = blockSize * (fat16->fatEntryNum - 1);
    fat16->fat = (uint16_t *)malloc(fat16->fatSize);
    fat16->fat[0] = blockNum << 2 + blockSize >> 8;
    fat16->mounted = false;
    return fat16;
}

void deleteFAT16(FAT16 *fat16) {
    free(fat16->fat);
    free(fat16);
}