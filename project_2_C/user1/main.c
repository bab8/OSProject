#include "lib.h"

int main(void){
    int pid;

    pid = fork();


    // pid = 0 means forked process as defined in the fork function of the kernel file
    if(pid == 0){
        exec("TEST.BIN");
    } else{
        waitu(pid);
        printf("test process exits\n");
    }
    return 0;
}