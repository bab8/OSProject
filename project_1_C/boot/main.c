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
   
   tasks[TaskLength].priority = 0;
   tasks[TaskLength].function = &ClearScreenTask;
   TaskLength++;

   tasks[TaskLength].priority = 0;
   tasks[TaskLength].function = &DrawMouseTask;
   TaskLength++;

   tasks[TaskLength].priority = 0;
   tasks[TaskLength].function = &HandleKeyboardTask;
   TaskLength++;


   while(1){
    //DrawRect(x,y,10,10,0,0,0); 
    ProcessTasks();

    Flush();
   }
}