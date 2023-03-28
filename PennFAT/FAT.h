#ifndef FAT_H
#define FAT_H

#include "utils.h"

/* PennFAT is based on FAT16 */
#define MIN_BLOCK_SIZE 256
#define MAX_FAT_BLOCK_NUM 32
#define MAX_BLOCK_SCALE 4

#define FAT_ENTRY_SIZE 2
#define EMPTY_FAT_ENTRY 0x0000
#define NO_SUCC_FAT_ENTRY 0xFFFF

#define MAX_FILE_NAME_LENGTH 32

/* 
---------- Refer to PennOS document ----------

The LSB(rightmost under little endian) of the first entry of the FAT specifies the block size with the mapping as below:
{LSB : size in bytes} = {0:256; 1:512; 2:1024; 3:2048; 4:4096}
The MSB(leftmost under little endian) of the first entry of the FAT specifies the number of blocks that FAT region occupies.
The MSB should be ranged from 1-32 (numbers outside of this range will be considered an error).
FAT region size = block size * FAT region block number
FAT entry number = FAT region size / FAT entry size (2-byte in FAT16)
Data region size = block size * (FAT entry number - 1)
*/
typedef struct FATConfig {
    char name[MAX_FILE_NAME_LENGTH];

    uint16_t LSB;
    uint16_t MSB;

    int blockSize;
    int FATRegionBlockNum;
    int FATRegionSize;
    int FATEntryNum;
    int dataRegionSize;

    /*
    The FAT region will be mapped to the memory through mmap(). 
    The size of the mapping area must be a multiple of the memory page size.
    */
    int FATSizeInMemory;
} FATConfig;

/* PennFAT has a 64-byte fixed directory entry size (32 + 4 + 2 + 1 + 1 + 8 + 16 = 64 bytes)*/
#define DIRECTORY_ENTRY_SIZE 64

#define NO_FIRST_BLOCK 0x0000

#define DIRECTORY_END "0"
#define DELETED_DIRECTORY "1"
#define DELETED_DIRECTORY_IN_USE "2"

#define FILE_TYPE_UNKNOWN 0
#define FILE_TYPE_REGULAR 1
#define FILE_TYPE_DIRECTORY 2
#define FILE_TYPE_SYMBOLIC_LINK 4

#define FILE_PERM_NONE 0
#define FILE_PERM_WRITE 2
#define FILE_PERM_READ 4
#define FILE_PERM_READ_EXEC 5
#define FILE_PERM_READ_WRITE 6
#define FILE_PERM_READ_WRITE_EXEC 7

#define RESERVED_BYTES 16
/* 
---------- Refer to PennOS document ----------
The structure of the directory entry as stored in the filesystem is as follows:
- char name[32]: null-terminated file name 
  name[0] also serves as a special marker:
    – 0: end of directory
    – 1: deleted entry; the file is also deleted
    – 2: deleted entry; the file is still being used
- uint32_t size: number of bytes in file
- uint16_t firstBlock: the first block number of the file (undefined if size is zero)
  The block index of data region starts from 1. If the firstBlock is 0, it means that this is an empty file which has not occupied any data region block.
- uint8_t type: the type of the file, which will be one of the following:
  – 0: unknown
  – 1: a regular file
  – 2: a directory file
  – 4: a symbolic link
- uint8_t perm: file permissions, which will be one of the following:
  – 0: none
  – 2: write only
  – 4: read only
  – 5: read and executable (shell scripts) 
  – 6: read and write
  – 7: read, write, and executable
- time_t mtime: creation/modification time as returned by time(2) in Linux
*/
typedef struct DirectoryEntry {
    char name[MAX_FILE_NAME_LENGTH]; // 32-byte    
    uint32_t size; // 4-byte
    uint16_t firstBlock; // 2-byte
    uint8_t type; // 1-byte
    uint8_t perm; // 1-byte
    time_t mtime; // 8-byte
    char reserved[RESERVED_BYTES];   // 16-byte reserved space for extension
} DirectoryEntry;

FATConfig *createFATConfig(char *name, uint16_t LSB, uint16_t MSB);

uint16_t *createFAT16InMemory(FATConfig *config);

int createFATOnDisk(FATConfig *config);

/* ##### TODO: Thread Safety ##### */

/* Return the index of next empty FAT entry. */
int findEmptyFAT16Entry(FATConfig *config, uint16_t *FAT16);

DirectoryEntry *createDirectoryEntry(char *name, uint32_t size, uint16_t firstBlock, uint8_t type, uint8_t perm);

int createFileDirectoryOnDisk(FATConfig *config, uint16_t *FAT16, char *fileName, uint8_t fileType, uint8_t filePerm);

/* Return the offset of the file directory entry in data region. */
int findFileDirectory(FATConfig *config, uint16_t *FAT16, char *fileName);

/* Read the directory entry from FAT and set it to dir. */
int readDirectoryEntry(FATConfig *config, int offset, DirectoryEntry *dir);

/* Write the directory entry to the offset. */
int writeFileDirectory(FATConfig *config, int offset, DirectoryEntry *dir);

int deleteFileDirectory(FATConfig *config, uint16_t *FAT16, char *fileName);

int readFAT(FATConfig *config, uint16_t *FAT16, int startBlock, int startBlockOffset, int size, char *buffer);

int writeFAT(FATConfig *config, uint16_t *FAT16, int startBlock, int startBlockOffset, int size, char *buffer);

/* Return the offset of the file end. */
int traceFileEnd(FATConfig *config, uint16_t *FAT16, char *fileName);

#endif