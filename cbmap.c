#include "../src/ascede.h"
#include "cbmap.h"
int main(){
    TraceLog_SetLevel(LOG_NONE);
    Window_PresetFlags(FLAG_WINDOW_RESIZABLE);
    Window_Init(600,400,"Corburt Map Editor");
    Window_SetMinSize(600,400);
    cbmap.load();
    cbmap.room.initid=0;
    while(!Window_ShouldClose()){
        if(cbmap.updatepoll)Events_Poll();
        else Events_Wait();
        cbmap.update();
        if(cbmap.updatebuffer){
            cbmap.draw();
        }
        Time_Wait(60);
        Events_EndLoop();
    }
    cbmap.save();
    cbmap.exit();
}
