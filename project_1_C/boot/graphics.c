#include "graphics.h"

//R - 4 bits
//G - 5 bits
//B - 4 bits
int rgb(int r, int g, int b){
    r = (int)(r>>3);
    g = (int)(g>>2);
    b = (int)(b>>3);

    return r << 11 | g << 5 | b;
}

void Draw(int x, int y, int r, int g, int b){
    VBEInfoBlock* VBE = (VBEInfoBlock*) VBEInfoAddress;
    unsigned short* buffer = (unsigned short*) ScreenBufferAddress;

    int index = y * VBE->x_resolution + x;
    *(buffer + index) = rgb(r,g,b);
}
void ClearScreen(int r, int g, int b){
    VBEInfoBlock* VBE = (VBEInfoBlock*) VBEInfoAddress;
    for(int y = 0; y < VBE->y_resolution;y++){
        for(int x = 0; x <  VBE->x_resolution;x++){
            Draw(x,y,r,g,b);
        }
    }
}
void DrawRect(int x, int y,int height, int width, int r, int g, int b){
    for(int j = y;  j< (y + height); j++){
        for(int i = x; i < (x + width); i++){
            Draw(i,j,r,g,b);
        }
    }
}

void DrawCharacter(int (*f)(int,int), int font_width, int font_height, char character, int x, int y, int r, int g, int b){
    for(int j = 0; j < font_height; j++){
        unsigned int row = (*f)((int)(character), j);
        int shift = font_arial_width - 1;
        int bit_val = 0;

        for(int i = 0; i < font_width; i++){
            bit_val = (row >> shift) & 0b00000000000000000000000000000001;
            if(bit_val == 1){
                Draw(x + i,y + j,r,g,b);
            }
            shift -= 1;
        }
    }
}

void DrawString(int (*f)(int,int), int font_width, int font_height, char* string, int x, int y, int r, int g, int b){
    //offsets
    int i = 0;
    int j = 0;

    for(int k = 0; *(string + k) != 0; k++){
        //draw current char
        if(*(string + k) != '\n'){
            DrawCharacter(f, font_width, font_height, *(string + k), x + i, y + j, r, g, b);
        }
        
        //move offsets
        i += font_width - (font_width / 4);

        if(*(string + k) == '\n'){
            i = 0;
            j += font_height;
        }
    }
}

void Flush() {
    VBEInfoBlock* VBE = (VBEInfoBlock*) VBEInfoAddress;
    unsigned short* buffer = (unsigned short*) ScreenBufferAddress;
    int index;

    for(int y = 0; y < VBE->y_resolution;y++){
        for(int x = 0; x < VBE->x_resolution;x++){
            index  = y * VBE->x_resolution + x;
            *((unsigned short*)VBE->screen_ptr + index) = *(buffer + index);
        }
    }


}