#include "keyboard.h"
#include "print.h"
#include "process.h"

static unsigned char shift_code[256] = {
    [0x2A] = SHIFT, [0x36] = SHIFT, [0xAA] = SHIFT, [0xB6] = SHIFT
};

static unsigned char lock_code[256] = {
    [0x3A] = CAPS_LOCK
};

static char key_map[256] = {
    0,0,'1','2','3','4','5','6','7','8','9','0',
    '-','=','\b',0,'q','w','e','r','t','y','u',
    'i','o','p','[',']','\n',0,'a','s','d','f',
    'g','h','j','k','l',';','\'','`',0,'\\','z',
    'x','c','v','b','n','m',',','.','/',0,'*',0,' '
};

static char shift_key_map[256] = {
    0,1,'!','@','#','$','%','^','&','*','(',')',
    '_','+','\b','\t','Q','W','E','R','T','Y','U',
    'I','O','P','{','}','\n',0,'A','S','D','F','G',
    'H','J','K','L',':','"','~',0,'|','Z','X','C',
    'V', 'B','N','M','<','>','?',0,'*',0,' '
};

static struct KeyboardBuffer key_buffer = {{0}, 0, 0, 500};
static unsigned int flag;

static void write_key_buffer(char ch){
    int front = key_buffer.front;
    int end = key_buffer.end;
    int size = key_buffer.size;

    //buffer full
    if((end + 1) % size == front){
        return;
    }
    key_buffer.buffer[end++] = ch;
    //if end it pting to last element start from beginning again
    key_buffer.end = end % size;
}

char read_key_buffer(void){
    int front = key_buffer.front;

    //check if buffer is empty
    if(front == key_buffer.end){
        sleep(-2);
    }

    //return key to caller and update front to next location
    key_buffer.front = (key_buffer.front + 1) % key_buffer.size;
    return key_buffer.buffer[front];
}

static char keyboard_read(void){
    unsigned char scancode;
    char ch;

    //read scancode from keyboard
    scancode = in_byte(0x60);

    //key is not valid
    if(scancode == 0xE0){
        flag |= E0_SIGN;
        return 0;
    }

    //function key,(not used in this system)
    if(flag & E0_SIGN){
        flag &= ~E0_SIGN;
        return 0;
    }

    //handle keyup, checks bit 7 which means keyup
    if(scancode & 0x80){
        flag &= ~(shift_code[scancode]);
        return 0;
    }


    //setup flags for shift and caps_lock keys, if caps isnt pressed xor wont change value, if key is pressed bitwise OR will change the value
    flag |= shift_code[scancode];
    flag ^= lock_code[scancode];

    //check if shift is pressed
    if(flag & SHIFT){
        ch = shift_key_map[scancode];
    }
    else{
        ch = key_map[scancode];
    }

    //check if caps is pressed, adjust character based on wheter or not flag is set
    if(flag & CAPS_LOCK){
        if('a' <= ch && ch <= 'z'){
            ch -= 32;
        }
        else if('A' <= ch && ch <= 'Z'){
            ch += 32;
        }
    }

    return ch;
}

void keyboard_handler(void){
    char ch = keyboard_read();

    if(ch > 0){
        write_key_buffer(ch);
        //wake up process waiting from keyboard, -2 is process waiting for keyboard i/o
        wake_up(-2);
    }
}