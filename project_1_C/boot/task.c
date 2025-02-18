//how many tasks currently running
int TaskLength = 0;

#define task_type_void 0
#define task_type_string_buffer 1
#define task_params_length 10

struct Task {
    // 0 to 5, 0 higher priority
    int priority;
    //id will determine what signature to use
    int taskId;
    //hold state of task
    char ca1[100];
    int i1;

    //function pointers
    int (*function)(int);
};

struct Task tasks[256];
//array for extra params of type int for tasks
int iparams[100] = {10};

void ProcessTasks(){
    int priority = 0;

    while(priority <= 5){
        for(int i = 0; i < TaskLength; i++){
            /*if(tasks[i].priority == priority){
                if(tasks[i].type == task_type_void){
                    tasks[i].function_void();
                }
                else if(tasks[i].type == task_type_string_buffer){
                    tasks[i].function_string_buffer(tasks[i].param1, &tasks[i].param2);
                }
            }*/
           tasks[i].function(tasks[i].taskId);
        }
        priority++;
    }
}

void CloseTask(int taskId){
    for(int i = taskId; i < TaskLength-1; i++){
        tasks[i] = tasks[i+1];
    }
    TaskLength--;
}

int ClearScreenTask(int taskId){
    ClearScreen(90,90,90);
    
    return 0;
}

int DrawMouseTask(int taskId){
    DrawMouse(mx,my,200,0,200);
    
    return 0;
}

int HandleKeyboardTask(int taskId){
    char* character_buffer = tasks[taskId].ca1; 
    int* character_buffer_length = &tasks[taskId].i1;
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

    
    DrawString(getArialCharacter, font_arial_width, font_arial_height, character_buffer, 100,100,16,16,16);
    
    return 0;
}

int TestGraphicalElementsTask(int taskId){
    int* r = &iparams[taskId * task_params_length + 4];
    int* g = &iparams[taskId * task_params_length + 5];
    int* b = &iparams[taskId * task_params_length + 6];

    if(DrawWindow(&iparams[taskId * task_params_length + 0], 
            &iparams[taskId * task_params_length + 1],
            &iparams[taskId * task_params_length + 2],
            &iparams[taskId * task_params_length + 3],
            *r,
            *g,
            *b,
            &iparams[taskId * task_params_length + 9]
        ) == 1){
            CloseTask(taskId);
        }

        char text[] = "Dark\0";
        char text1[] = "Light\0";

        if(DrawButton(iparams[taskId * task_params_length + 0] + 20, iparams[taskId * task_params_length + 1] + 20, 50, 20, 0, 120, 0,
            text, 100, 100, 100) == TRUE){
                *r = 16;
                *g = 16;
                *b = 16;
        }
        if(DrawButton(iparams[taskId * task_params_length + 0] + 100, iparams[taskId * task_params_length + 1] + 20, 50, 20, 0, 120, 0,
            text1, 100, 100, 100) == TRUE){
                *r = 200;
                *g = 200;
                *b = 200;
        }

    return 0;
}