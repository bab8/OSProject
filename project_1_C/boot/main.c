#include "graphics.h"

int start(){
  

   //string literals cannot be more than 61 chars
   char str1[] = "Welcome to Sapphire OS! \n\nText rendered by custom library.";
   char *p = str1;

   base = (unsigned int) &isr1;
   base12 = (unsigned int) &isr12;

   InitialiseMouse();
   InitialiseIDT();
   
   while(1){
    ClearScreen(90,90,90);
    DrawString(getArialCharacter, font_arial_width, font_arial_height, p, 100,100,0,0,0);

    DrawRect(x,y,10,10,0,0,0);

    Flush();
   }
}