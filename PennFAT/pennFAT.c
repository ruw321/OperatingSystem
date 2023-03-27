#include "pennFAT.h"



int main(int argc, char **argv) {
    printf("PennFAT\n");

    // int LSB = 0;
    // int MSB = 1;
    // FATConfig *config = createFATConfig("testFS", LSB, MSB);
    // createFATOnDisk(config);
    // free(config);

    fs_mount("testFS");
    // printf("FAT in memory: 0x%x\n", fs_FAT16InMemory[0]);
    // printf("Next empty block: %d\n", findEmptyFAT16Entry(fs_FATConfig, fs_FAT16InMemory));

    fs_touch("testFile");

    // int n = 4;
    // char dummyFiles[n][32]; 

    // for (int i = 0; i < n; i++) {
    //     sprintf(dummyFiles[i], "file_%d", i); 
    //     fs_touch(dummyFiles[i]);
    // }


    fs_unmount();


    return 0;
}