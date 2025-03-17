#ifndef _PRINT_H_
#define _PRINT_H_

//80 char each line * 2 bytes = 160 bytes per line
#define LINE_SIZE 160

struct ScreenBuffer {
    char* buffer;
    int column;
    int row;
};

//print used in kernel mode
int printk(const char* format, ...);
void write_screen(const char* buffer,int size, char color);

#endif