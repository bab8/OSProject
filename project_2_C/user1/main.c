#include "lib.h"
#include "stdint.h"

static int read_cmd(char *buffer){
    char ch[2] = { 0 };
    int buffer_size = 0;

    while(1){
        ch[0] = keyboard_readu();

        //command issued or buffer exceeded
        if(ch[0] == '\n' || buffer_size >= 80){
            printf("%s",ch);
            break;
        }
        //character deleted
        else if(ch[0] == '\b'){
            //don't delete if no char on screen
            if(buffer_size > 0){
                buffer[buffer_size--] = 0;
                printf("%s",ch);
            }
        }else{
            //print char
            buffer[buffer_size++] = ch[0];
            printf("%s", ch);
        }
    }
    return buffer_size;
}


int main(void){
    char buffer[100] = { 0 };
    int buffer_size = 0;

    while(1){
        printf("Shell# ");
        //clear buffer before reading next cmd
        memset(buffer,0, 100);
        buffer_size = read_cmd(buffer);

        if(buffer_size == 0){
            continue;
        }

        //look for cmd file and store in buffer
        int fd = open_file(buffer);

        if(fd == -1){
            printf("Command not found\n");
        } else{
            close_file(fd);
            int pid = fork();

            if(pid == 0){
                exec(buffer);
            } else{
                waitu(pid);
            }
        }
    }
    return 0;
}