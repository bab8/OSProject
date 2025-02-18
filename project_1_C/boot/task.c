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
    int priority;
    int i = 0;

    priority = 5;
    while(priority >= 0){
        i = mouse_possessed_task_id;
        if(left_clicked == TRUE && 
            mx > iparams[i * task_params_length + 0] &&
            mx < iparams[i * task_params_length + 0] + iparams[i * task_params_length + 2] &&
            my > iparams[i * task_params_length + 1] &&
            my < iparams[i * task_params_length + 1] + iparams[i * task_params_length + 3])
                break;
                
        for(i = 0; i < TaskLength; i++){
            if(left_clicked == TRUE && 
                mx > iparams[i * task_params_length + 0] &&
                mx < iparams[i * task_params_length + 0] + iparams[i * task_params_length + 2] &&
                my > iparams[i * task_params_length + 1] &&
                my < iparams[i * task_params_length + 1] + iparams[i * task_params_length + 3]){
                    tasks[mouse_possessed_task_id].priority = 0;
                    mouse_possessed_task_id = i;
                    tasks[i].priority = 2;
                    left_clicked = FALSE;
                }
        }
        priority--;
    }

    priority = 0;
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
           if(tasks[i].priority == priority){
              tasks[i].function(tasks[i].taskId);
           }
        }
        priority++;
    }
}

int NullTask(int taskId){
    return 0;
}

void CloseTask(int taskId){
    tasks[taskId].function = &NullTask;
    iparams[taskId * task_params_length + 0] = 0;
    iparams[taskId * task_params_length + 1] = 0;
    iparams[taskId * task_params_length + 2] = 0;
    iparams[taskId * task_params_length + 3] = 0;

}

int ClearScreenTask(int taskId){
    ClearScreen(90,90,120);
    
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

int ShellTask(int taskId){
    int* r = &iparams[taskId * task_params_length + 4];
    int* g = &iparams[taskId * task_params_length + 5];
    int* b = &iparams[taskId * task_params_length + 6];

    int closeClicked = DrawWindow(&iparams[taskId * task_params_length + 0], 
            &iparams[taskId * task_params_length + 1],
            &iparams[taskId * task_params_length + 2],
            &iparams[taskId * task_params_length + 3],
            *r,
            *g,
            *b,
            &iparams[taskId * task_params_length + 9],
            taskId
        );

    int x = iparams[taskId * task_params_length + 0];
    int y = iparams[taskId * task_params_length + 1];
    int width = iparams[taskId * task_params_length + 2];
    int height = iparams[taskId * task_params_length + 3];


    if(closeClicked == TRUE){
        CloseTask(taskId);
    }

    char text[] = "Dark\0";
    char text1[] = "Light\0";

    if(DrawButton(x + 20, y + 20, 50, 20, 0, 120, 0,
        text, 100, 100, 100, taskId) == TRUE){
            *r = 16;
            *g = 16;
            *b = 16;
    }
    if(DrawButton(x + 100, y + 20, 50, 20, 0, 120, 0,
        text1, 100, 100, 100, taskId) == TRUE){
            *r = 200;
            *g = 200;
            *b = 200;
    }

    return 0;
}

int BallTask(int taskId){
    int* r = &iparams[taskId * task_params_length + 4];
    int* g = &iparams[taskId * task_params_length + 5];
    int* b = &iparams[taskId * task_params_length + 6];

    int closeClicked = DrawWindow(&iparams[taskId * task_params_length + 0], 
            &iparams[taskId * task_params_length + 1],
            &iparams[taskId * task_params_length + 2],
            &iparams[taskId * task_params_length + 3],
            0,
            0,
            0,
            &iparams[taskId * task_params_length + 9],
            taskId
        );

    if(closeClicked == TRUE){
        CloseTask(taskId);
    }

    int x = iparams[taskId * task_params_length + 0];
    int y = iparams[taskId * task_params_length + 1];
    int width = iparams[taskId * task_params_length + 2];
    int height = iparams[taskId * task_params_length + 3];

    //delta x, delta y
    iparams[taskId * task_params_length + 5] += iparams[taskId * task_params_length + 7];
    iparams[taskId * task_params_length + 6] += iparams[taskId * task_params_length + 8];
    
    //x direction change
    if(iparams[taskId * task_params_length + 5] + 10 > iparams[taskId * task_params_length + 2] ||
       iparams[taskId * task_params_length + 5] - 10 < 0){
            iparams[taskId * task_params_length + 7] = -iparams[taskId * task_params_length + 7];
    }

    //y direction change
    if(iparams[taskId * task_params_length + 6] + 10 > iparams[taskId * task_params_length + 3] ||
        iparams[taskId * task_params_length + 6] - 10 < 20){
             iparams[taskId * task_params_length + 8] = -iparams[taskId * task_params_length + 8];
     }

    DrawCircle(x + iparams[taskId * task_params_length + 5], y + iparams[taskId * task_params_length + 6], 10, 64, 120, 64);

}

int TaskbarTask(int taskId){
    VBEInfoBlock* VBE = (VBEInfoBlock*) VBEInfoAddress;
    DrawRect(0,0, VBE->x_resolution, 40, 90, 90, 90);

    int i = iparams[taskId * task_params_length + 4];

    char text[] = "Shell\0";
    if(DrawButton(0,0,50,40,0,10,120,text, 255,255,255,taskId) == TRUE){
        tasks[TaskLength].priority = 0;
        tasks[TaskLength].taskId = TaskLength;
        tasks[TaskLength].function = &ShellTask;
        iparams[TaskLength * task_params_length + 0] = i * 40;
        iparams[TaskLength * task_params_length + 1] = i * 40;
        iparams[TaskLength * task_params_length + 2] = 300;
        iparams[TaskLength * task_params_length + 3] = 300;
        iparams[TaskLength * task_params_length + 4] = 0;
        iparams[TaskLength * task_params_length + 5] = 0;
        iparams[TaskLength * task_params_length + 6] = 0;
        TaskLength++;
        iparams[taskId * task_params_length + 4]++;
    }

    char text2[] = "Ball\0";
    if(DrawButton(50,0,50,40,10,120,0,text2, 255,255,255,taskId) == TRUE){
        tasks[TaskLength].priority = 0;
        tasks[TaskLength].taskId = TaskLength;
        tasks[TaskLength].function = &BallTask;
        iparams[TaskLength * task_params_length + 0] = i * 40;
        iparams[TaskLength * task_params_length + 1] = i * 40;
        iparams[TaskLength * task_params_length + 2] = 300;
        iparams[TaskLength * task_params_length + 3] = 300;
        iparams[TaskLength * task_params_length + 4] = 0;
        iparams[TaskLength * task_params_length + 5] = 20;
        iparams[TaskLength * task_params_length + 6] = 30;
        iparams[TaskLength * task_params_length + 7] = 5;
        iparams[TaskLength * task_params_length + 8] = 5;
        TaskLength++;
        iparams[taskId * task_params_length + 4]++;
    }
}