#include "graphics.h"

int start(){
   ClearScreen(90,90,90);

   //string literals cannot be more than 61 chars
   char str1[] = "Welcome to Sapphire OS! \n\nText rendered by custom library.";
   char *p = str1;

   DrawString(getArialCharacter, font_arial_width, font_arial_height, p, 100,100,0,0,0);
   
   while(1);
}