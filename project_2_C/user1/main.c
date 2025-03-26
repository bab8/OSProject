#include "lib.h"

int main(void){
    int pid;

    pid = fork();


    // pid = 0 means forked process as defined in the fork function of the kernel file
    if(pid == 0){
        printf("this is a new process\n");
    } else{
        printf("this is the current process\n");
        waitu(pid);
    }
    return 0;
}