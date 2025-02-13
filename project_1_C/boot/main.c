#include "graphics.h"

int start(){
   VBEInfoBlock* VBE = (VBEInfoBlock*) VBEInfoAddress;

   x = VBE->x_resolution / 2;
   y = VBE->y_resolution / 2;

   //string literals cannot be more than 61 chars
   char str1[] = "Welcome to Sapphire OS! \n\nText rendered by custom library.";
   char *p = str1;

   char character_buffer[1000] = "\0";
   char* character_buffer_ptr = character_buffer;
   int character_buffer_length = 0;

   base = (unsigned int) &isr1;
   base12 = (unsigned int) &isr12;

   InitialiseMouse();
   InitialiseIDT();
   
   while(1){
    char character = ProcessScancode(Scancode);

    if(backspace_pressed == TRUE){
      character_buffer[character_buffer_length - 1] = '\0';
      character_buffer_length--;
      backspace_pressed = FALSE;
      Scancode = -1;
    }
    else if(character != '\0'){
      character_buffer[character_buffer_length] = character;
      character_buffer[character_buffer_length + 1] = '\0';
      character_buffer_length++;
      Scancode = -1;
    }

    ClearScreen(90,90,90);
    DrawString(getArialCharacter, font_arial_width, font_arial_height, character_buffer_ptr, 100,100,0,0,0);

    //DrawRect(x,y,10,10,0,0,0);
    DrawMouse(x,y,200,0,200);

    Flush();
   }
}