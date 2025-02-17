//how many tasks currently running
int TaskLength = 0;

#define task_type_void 0
#define task_type_string_buffer 1

struct Task {
    // 0 to 5, 0 higher priority
    int priority;
    //determines type of task
    int type;
    //hold state of task
    char param1[10000];
    int param2;

    //function pointers
    int (*function_void)(void);
    int (*function_string_buffer)(char*, int*);
};

struct Task tasks[256];

void ProcessTasks(){
    int priority = 0;

    while(priority <= 5){
        for(int i = 0; i < TaskLength; i++){
            if(tasks[i].priority == priority){
                if(tasks[i].type == task_type_void){
                    tasks[i].function_void();
                }
                else if(tasks[i].type == task_type_string_buffer){
                    tasks[i].function_string_buffer(tasks[i].param1, &tasks[i].param2);
                }
            }
        }
        priority++;
    }
}

int ClearScreenTask(){
    ClearScreen(90,90,90);
    
    return 0;
}

int DrawMouseTask(){
    DrawMouse(x,y,200,0,200);
    
    return 0;
}

int HandleKeyboardTask(char* character_buffer, int* character_buffer_length){
    char character = ProcessScancode(Scancode);

    if(backspace_pressed == TRUE){
      character_buffer[*character_buffer_length - 1] = '\0';
      (*character_buffer_length)--;
      backspace_pressed = FALSE;
      Scancode = -1;
    }
    else if(character != '\0'){
      character_buffer[*character_buffer_length] = character;
      character_buffer[*character_buffer_length + 1] = '\0';
      (*character_buffer_length)++;
      Scancode = -1;
    }

    
    DrawString(getArialCharacter, font_arial_width, font_arial_height, character_buffer, 100,100,0,0,0);
    
    return 0;
}