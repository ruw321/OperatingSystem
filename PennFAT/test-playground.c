#include "pennFAT.h"

int main(int argc, char **argv) {
    printf("Testing PennFAT ...\n");
    
    pf_mkfs("testFS", 1, 0);

    fs_mount("testFS");

    // int L = 100;
    // char str[L];
    // srand((unsigned int)time(NULL));
    // for (int i = 0; i < L - 1; i++) {
    //     str[i] = rand() % 26 + 'a';
    // }
    

    // int N = 2; 
    // char filenames[N][32];  

    // for (int i = 0; i < N; i++) {
    //     sprintf(filenames[i], "file%d", i+1); 
    // }

    // for (int i = 0; i < N; i++) {
    //     fs_touch(filenames[i]);
    // }


    fs_touch("test");
    

    pf_writeFile("test", 4, "123\n", PF_APPEND);
    pf_writeFile("test", 4, "abc\n", PF_APPEND);



    fs_unmount();
    return 0;
}