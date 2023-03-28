#include "pennFAT.h"

int main(int argc, char **argv) {
    printf("Testing PennFAT ...\n");
    
    // fs_mkfs("testFS", 0, 1);

    fs_mount("testFS");
    

    // int N = 5; 
    // char filenames[N][32];  

    // for (int i = 0; i < N; i++) {
    //     sprintf(filenames[i], "file_%d", i+1); 
    // }

    // for (int i = 0; i < N; i++) {
    //     fs_touch(filenames[i]);
    // }

    // fs_rm("file_1");

    // fs_touch("test");

    pf_writeFile("file_2", 4, "1234", PF_APPEND);

    char buffer[100];
    pf_readFile("file_2", 3, buffer);
    printf("Read %s\n", buffer);

    fs_unmount();

    return 0;
}