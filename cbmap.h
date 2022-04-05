#ifndef CBMAP_H
#define CBMAP_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#define MALLOC_STEP 1024

struct cbmap;
struct Room;

void CBMap_Load_Buttons(void);
void CBMap_Load_New(void);
void CBMap_Load(void);
void CBMap_Save(void);
void CBMap_Draw_Room(int x,int y,int posx,int posy,int size);
void CBMap_Draw_Grid(void);
void CBMap_Draw_Text(void);
void CBMap_Draw_Buttons(void);
void CBMap_Draw(void);
void CBMap_Update_Camera(void);
void CBMap_Update_Select(void);
void CBMap_Update_Button(void);
void CBMap_Update(void);
void CBMap_Exit(void);

typedef struct Button {
    int id;
    int x;
    int y;
    int w;
    char label[32];
    Color cd;
    Color cb;
    bool hover;
    void (*clicked)(void);
} Button;

typedef struct Room {
    int id;
    wchar_t *name;
    wchar_t *desc;
    int x;
    int y;
    int region;
    int type;
    int table[32];
    int exits[6];
    int exitid[6];
} Room;

enum direction{
    dir_East,dir_West,dir_North,dir_South,dir_Up,dir_Down
};
enum db_roomtype{
    db_roomtype_plain,
    db_roomtype_birth,
    db_roomtype_store,
    db_roomtype_shop=db_roomtype_store,
    db_roomtype_train,
    db_roomtype_gate
};

struct cbmap {
    bool updatebuffer;
    bool updatepoll;
    int width;
    int height;
    void (*load)(void);
    void (*save)(void);
    void (*draw)(void);
    void (*update)(void);
    void (*exit)(void);
    struct {
        int initid;
        int space;
        int count;
        Room *rooms;
    } room;
    struct {
        int x;
        int y;
        float scale;
        int recshown;
        int initx;
        int inity;
        int curx;
        int cury;
    } camera;
    struct {
        double framestart;
        double calcend;
        double drawend;
    } timer;
    struct {
        int target;
        int x;
        int y;
        int downx;
        int downy;
        int index;
        bool doubleclick;
    } select;
    struct {
        int count;
        bool buttonpress;
        bool buttonselect;
        Button *buttons;
    } button;
} cbmap={
    true,true,0,0,
    CBMap_Load,CBMap_Save,
    CBMap_Draw,CBMap_Update,CBMap_Exit,
    {0,1024,0,NULL},{0,0,1,0,0,0,0,0},{0,0,0},
    {0,0,0,0,0,0,false},{0,false,false,NULL}
};

void Room_Create(Room room);
void Room_Export(FILE *fp,Room *room);
void Room_Delete(int roomindex);
void Room_Delete_Current(void);
void Room_EditName(int roomindex);
void Room_EditName_Current(void);
void Room_EditDesc(int roomindex);
void Room_EditDesc_Current(void);
void Room_EditRegn(int roomindex);
void Room_EditRegn_Current(void);
void Room_EditType(int roomindex);
void Room_EditType_Current(void);
void Room_EditTabl(int roomindex);
void Room_EditTabl_Current(void);

void CBMap_Load_Buttons(void){
    const int bwidth=170;
    static Button buttons[]={
        {.id=1,.label="Delete room",
            .x=205,
            .y=5,
            .w=bwidth,
            .cd=MAROON,
            .cb=RED,
            .clicked=Room_Delete_Current
        },
        {.id=2,.label="Edit name",
            .x=205,
            .y=40,
            .w=bwidth,
            .cd=ORANGE,
            .cb=BEIGE,
            .clicked=Room_EditName_Current
        },
        {.id=3,.label="Edit description",
            .x=205,
            .y=75,
            .w=bwidth,
            .cd=ORANGE,
            .cb=BEIGE,
            .clicked=Room_EditDesc_Current
        },
        {.id=4,.label="Edit region",
            .x=205,
            .y=110,
            .w=bwidth,
            .cd=ORANGE,
            .cb=BEIGE,
            .clicked=Room_EditRegn_Current
        },
        {.id=5,.label="Edit type",
            .x=205,
            .y=145,
            .w=bwidth,
            .cd=ORANGE,
            .cb=BEIGE,
            .clicked=Room_EditType_Current
        },
        {.id=6,.label="Edit table",
            .x=205,
            .y=180,
            .w=bwidth,
            .cd=ORANGE,
            .cb=BEIGE,
            .clicked=Room_EditTabl_Current
        },
    };
    cbmap.button.buttons=buttons;
    cbmap.button.count=6;
}
void CBMap_Load_New(void){
    cbmap.room.space=MALLOC_STEP;
    cbmap.room.rooms=malloc(MALLOC_STEP*sizeof(Room));
    if(!cbmap.room.rooms)cbmap.exit();
    Room tmproom={0};
    for(int i=0;i<MALLOC_STEP;i++)cbmap.room.rooms[i]=tmproom;
    Room *room=&cbmap.room.rooms[0];
    room->id=1;
    room->type=db_roomtype_birth;
    room->name=malloc(256*sizeof(wchar_t));
    wmemset(room->name,0,256);
    wcscpy(room->name,L"Untitled room");
    room->desc=malloc(4096*sizeof(wchar_t));
    wmemset(room->desc,0,4096);
    wcscpy(room->desc,L"No description");
    cbmap.room.count=1;
}
void CBMap_Load(void){
    CBMap_Load_Buttons();
    CBMap_Load_New();
}
void CBMap_Save(void){
    FILE *fp=fopen("export.txt","w");
    fprintf(fp,"roomdb roomdbs[]={\n");
    for(int i=0;i<cbmap.room.count;i++){
        Room_Export(fp,&cbmap.room.rooms[i]);
    }
    fprintf(fp,"    {\n"
        "        .id=0\n"
        "//              -------------------------------------------------------------------\n"
        "    }\n"
        "};\n");
    fclose(fp);
}
void CBMap_Update_Camera(void){
    if(Mouse_IsDown(MOUSE_BUTTON_RIGHT)){
        Vector2 delta=Mouse_GetDelta();
        if(delta.x>0.1f||delta.x<-0.1f){
            cbmap.camera.x+=delta.x/cbmap.camera.scale;
            cbmap.updatebuffer=true;
        }
        if(delta.y>0.1f||delta.y<-0.1f){
            cbmap.camera.y+=delta.y/cbmap.camera.scale;
            cbmap.updatebuffer=true;
        }
    }
    {
        int speed=30/cbmap.camera.scale;
        int x=cbmap.camera.x,y=cbmap.camera.y;
        if(Key_IsDown(KEY_LEFT)){
            cbmap.camera.x+=speed;
        }if(Key_IsDown(KEY_RIGHT)){
            cbmap.camera.x-=speed;
        }if(Key_IsDown(KEY_DOWN)){
            cbmap.camera.y-=speed;
        }if(Key_IsDown(KEY_UP)){
            cbmap.camera.y+=speed;
        }
        if(cbmap.camera.x!=x||cbmap.camera.y!=y){
            cbmap.updatepoll=true;
            cbmap.updatebuffer=true;
        }
    }
    if(Key_IsPressed(KEY_R)){
        if(cbmap.camera.x!=0||cbmap.camera.y!=0||cbmap.camera.scale<0.99999f||cbmap.camera.scale>1.00001f)cbmap.updatebuffer=true;
        cbmap.camera.x=0;
        cbmap.camera.y=0;
        cbmap.camera.scale=1;
    }
    if(Mouse_GetWheelDelta()<-0.00001||Mouse_GetWheelDelta()>0.00001){
        float oldscale=cbmap.camera.scale;
        cbmap.camera.scale+=Mouse_GetWheelDelta()*0.1f;
        if(cbmap.camera.scale>2)cbmap.camera.scale=2;
        if(cbmap.camera.scale<0.4)cbmap.camera.scale=0.4;
        if(cbmap.camera.scale-oldscale>0.00001f||oldscale-cbmap.camera.scale>0.00001f){
            cbmap.updatebuffer=true;
            cbmap.updatepoll=true;
        }
    }
    cbmap.camera.recshown=0;
}
void CBMap_Update_Select(void){
    int tile=100.0f*cbmap.camera.scale;
    { // grid init pos
        cbmap.camera.curx=0;
        cbmap.camera.cury=0;
        cbmap.camera.initx=cbmap.camera.x*cbmap.camera.scale+(cbmap.width+200-tile)/2;
        cbmap.camera.inity=cbmap.camera.y*cbmap.camera.scale+(cbmap.height-tile)/2;
        for(;cbmap.camera.initx<cbmap.width;cbmap.camera.curx++)cbmap.camera.initx+=1.2*tile;
        for(;cbmap.camera.initx>200;cbmap.camera.curx--)cbmap.camera.initx-=1.2*tile;
        for(;cbmap.camera.inity<cbmap.height;cbmap.camera.cury++)cbmap.camera.inity+=1.2*tile;
        for(;cbmap.camera.inity>0;cbmap.camera.cury--)cbmap.camera.inity-=1.2*tile;
    }
    if((!cbmap.button.buttonpress)&&(!cbmap.button.buttonselect)){ // cursor
        int selx=cbmap.select.x,sely=cbmap.select.y,selt=cbmap.select.target;
        if((!Mouse_IsDown(MOUSE_BUTTON_LEFT))&&(!Mouse_IsReleased(MOUSE_BUTTON_LEFT)))cbmap.select.target=0;
        if(Cursor_IsOnScreen()){
            if(Collision_CheckPointRec(Mouse_GetPos(),(Rectangle){200,0,cbmap.width-200,cbmap.height})){
                if(!Mouse_IsDown(MOUSE_BUTTON_RIGHT)){
                    int curx=cbmap.camera.curx,cury=cbmap.camera.cury;
                    for(int tilex=cbmap.camera.initx;tilex<cbmap.width;tilex+=1.2*tile){
                        for(int tiley=cbmap.camera.inity;tiley<cbmap.height;tiley+=1.2*tile){
                            if(Collision_CheckPointRec(Mouse_GetPos(),(Rectangle){tilex,tiley,tile,tile})){
                                if((!Mouse_IsDown(MOUSE_BUTTON_LEFT))&&(!Mouse_IsReleased(MOUSE_BUTTON_LEFT)))
                                    cbmap.select.target=1;
                                cbmap.select.x=curx;
                                cbmap.select.y=cury;
                                break;
                            }
                            cury++;
                        }
                        cury=cbmap.camera.cury;
                        curx++;
                    }
                }
            }
        }
        if(selx!=cbmap.select.x||sely!=cbmap.select.y||selt!=cbmap.select.target){
            cbmap.updatebuffer=true;
        }
    }else if(cbmap.button.buttonselect){
        cbmap.select.target=0;
    }
    { // mouse button press
        if(Mouse_IsDown(MOUSE_BUTTON_LEFT)&&(!cbmap.button.buttonpress)&&(!cbmap.button.buttonselect)){
            if(!Collision_CheckPointRec(Mouse_GetPos(),(Rectangle){200,0,cbmap.width-200,cbmap.height}));
            else if(cbmap.select.x!=cbmap.select.downx||cbmap.select.y!=cbmap.select.downy){
                cbmap.select.target=4;
                cbmap.select.doubleclick=false;
            }else if(cbmap.select.index!=-1){
                cbmap.select.target=3;
            }
        }
        if(Mouse_IsPressed(MOUSE_BUTTON_LEFT)&&(!cbmap.button.buttonpress)&&(!cbmap.button.buttonselect)){
            if(!Collision_CheckPointRec(Mouse_GetPos(),(Rectangle){200,0,cbmap.width-200,cbmap.height})){
                cbmap.select.doubleclick=false;
            }
            else if(cbmap.select.downx==cbmap.select.x&&cbmap.select.downy==cbmap.select.y&&cbmap.select.index!=-1){
                cbmap.select.target=3;
                cbmap.updatebuffer=true;
            }else{
                cbmap.select.target=2;
                cbmap.select.index=-1;
                for(int i=0;i<cbmap.room.count;i++){
                    if(cbmap.room.rooms[i].x==cbmap.select.x){
                        if(cbmap.room.rooms[i].y==cbmap.select.y){
                            cbmap.select.index=i;
                            break;
                        }
                    }
                }
                cbmap.select.downx=cbmap.select.x;
                cbmap.select.downy=cbmap.select.y;
                cbmap.updatebuffer=true;
            }
        }
        if(Mouse_IsReleased(MOUSE_BUTTON_LEFT)&&(!cbmap.button.buttonpress)&&(!cbmap.button.buttonselect)){
            if(cbmap.select.target==4){
                bool creatable=false;
                bool connectible=false;
                int roomfromindex=-1;
                {
                    int x=cbmap.room.rooms[cbmap.select.index].x;
                    int y=cbmap.room.rooms[cbmap.select.index].y;
                    if((x==cbmap.select.x+1&&y==cbmap.select.y)||
                       (x==cbmap.select.x-1&&y==cbmap.select.y)||
                       (x==cbmap.select.x&&y==cbmap.select.y+1)||
                       (x==cbmap.select.x&&y==cbmap.select.y-1)){
                        creatable=true;
                    }
                }
                for(int i=0;i<cbmap.room.count;i++){
                    if(cbmap.room.rooms[i].x==cbmap.select.downx&&cbmap.room.rooms[i].y==cbmap.select.downy){
                        roomfromindex=i;
                    }
                    if(cbmap.room.rooms[i].x==cbmap.select.x&&cbmap.room.rooms[i].y==cbmap.select.y){
                        cbmap.select.index=i;
                        creatable=false;
                        if(cbmap.select.x==cbmap.select.downx+1&&cbmap.select.y==cbmap.select.downy){
                            if(cbmap.room.rooms[i].exits[dir_West]==0)connectible=true;
                        }else if(cbmap.select.x==cbmap.select.downx-1&&cbmap.select.y==cbmap.select.downy){
                            if(cbmap.room.rooms[i].exits[dir_East]==0)connectible=true;
                        }else if(cbmap.select.x==cbmap.select.downx&&cbmap.select.y==cbmap.select.downy+1){
                            if(cbmap.room.rooms[i].exits[dir_North]==0)connectible=true;
                        }else if(cbmap.select.x==cbmap.select.downx&&cbmap.select.y==cbmap.select.downy-1){
                            if(cbmap.room.rooms[i].exits[dir_South]==0)connectible=true;
                        }
                    }
                    if(roomfromindex!=-1&&creatable==false)break;
                }
                if(roomfromindex==-1)connectible=false;
                if(creatable){
                    Room room={0};
                    room.id=cbmap.room.count+1;
                    room.x=cbmap.select.x;
                    room.y=cbmap.select.y;
                    room.region=cbmap.room.rooms[cbmap.select.index].region;
                    if(cbmap.select.x==cbmap.select.downx+1&&cbmap.select.y==cbmap.select.downy){
                        room.exits[dir_West]=cbmap.room.rooms[roomfromindex].id;
                        cbmap.room.rooms[roomfromindex].exits[dir_East]=room.id;
                    }if(cbmap.select.x==cbmap.select.downx-1&&cbmap.select.y==cbmap.select.downy){
                        room.exits[dir_East]=cbmap.room.rooms[roomfromindex].id;
                        cbmap.room.rooms[roomfromindex].exits[dir_West]=room.id;
                    }if(cbmap.select.x==cbmap.select.downx&&cbmap.select.y==cbmap.select.downy+1){
                        room.exits[dir_North]=cbmap.room.rooms[roomfromindex].id;
                        cbmap.room.rooms[roomfromindex].exits[dir_South]=room.id;
                    }if(cbmap.select.x==cbmap.select.downx&&cbmap.select.y==cbmap.select.downy-1){
                        room.exits[dir_South]=cbmap.room.rooms[roomfromindex].id;
                        cbmap.room.rooms[roomfromindex].exits[dir_North]=room.id;
                    }
                    Room_Create(room);
                }
                else if(connectible){
                    Room *room=&cbmap.room.rooms[cbmap.select.index];
                    if(cbmap.select.x==cbmap.select.downx+1&&cbmap.select.y==cbmap.select.downy){
                        room->exits[dir_West]=cbmap.room.rooms[roomfromindex].id;
                        cbmap.room.rooms[roomfromindex].exits[dir_East]=room->id;
                    }else if(cbmap.select.x==cbmap.select.downx-1&&cbmap.select.y==cbmap.select.downy){
                        room->exits[dir_East]=cbmap.room.rooms[roomfromindex].id;
                        cbmap.room.rooms[roomfromindex].exits[dir_West]=room->id;
                    }else if(cbmap.select.x==cbmap.select.downx&&cbmap.select.y==cbmap.select.downy+1){
                        room->exits[dir_North]=cbmap.room.rooms[roomfromindex].id;
                        cbmap.room.rooms[roomfromindex].exits[dir_South]=room->id;
                    }else if(cbmap.select.x==cbmap.select.downx&&cbmap.select.y==cbmap.select.downy-1){
                        room->exits[dir_South]=cbmap.room.rooms[roomfromindex].id;
                        cbmap.room.rooms[roomfromindex].exits[dir_North]=room->id;
                    }
                }else{
                    Room *room=&cbmap.room.rooms[cbmap.select.index];
                    if(cbmap.select.x==cbmap.select.downx+1&&cbmap.select.y==cbmap.select.downy){
                        room->exits[dir_West]=0;
                        cbmap.room.rooms[roomfromindex].exits[dir_East]=0;
                    }else if(cbmap.select.x==cbmap.select.downx-1&&cbmap.select.y==cbmap.select.downy){
                        room->exits[dir_East]=0;
                        cbmap.room.rooms[roomfromindex].exits[dir_West]=0;
                    }else if(cbmap.select.x==cbmap.select.downx&&cbmap.select.y==cbmap.select.downy+1){
                        room->exits[dir_North]=0;
                        cbmap.room.rooms[roomfromindex].exits[dir_South]=0;
                    }else if(cbmap.select.x==cbmap.select.downx&&cbmap.select.y==cbmap.select.downy-1){
                        room->exits[dir_South]=0;
                        cbmap.room.rooms[roomfromindex].exits[dir_North]=0;
                    }
                }
            }
            if(cbmap.select.downx==cbmap.select.x&&cbmap.select.downy==cbmap.select.y&&cbmap.select.index!=-1){
                cbmap.select.doubleclick=!cbmap.select.doubleclick;
                cbmap.updatebuffer=true;
            }
            cbmap.updatebuffer=true;
        }
        if(Mouse_IsPressed(MOUSE_BUTTON_RIGHT)){
            cbmap.select.x=0;
            cbmap.select.y=0;
            cbmap.select.downx=0;
            cbmap.select.downy=0;
            cbmap.select.index=-1;
            cbmap.select.doubleclick=false;
            cbmap.updatebuffer=true;
        }
    }
}
void CBMap_Update_Button(void){
    cbmap.button.buttonpress=false;
    cbmap.button.buttonselect=false;
    if(!cbmap.select.doubleclick)return;
    if(cbmap.select.index==0){
        cbmap.button.buttons[0].cb=LIGHTGRAY;
        cbmap.button.buttons[0].cd=GRAY;
    }else{
        cbmap.button.buttons[0].cb=RED;
        cbmap.button.buttons[0].cd=MAROON;
    }
    for(int i=0;i<cbmap.button.count;i++){
        Button *btn=&cbmap.button.buttons[i];
        bool hover=btn->hover;
        btn->hover=false;
        if(Collision_CheckPointRec(Mouse_GetPos(),(Rectangle){btn->x,btn->y,btn->w,25})){
            cbmap.button.buttonselect=true;
            if(Mouse_IsReleased(MOUSE_BUTTON_LEFT)){
                if(i!=0){
                    btn->clicked();
                    cbmap.select.doubleclick=true;
                }else{
                    if(cbmap.select.index!=0){
                        btn->clicked();
                        cbmap.select.doubleclick=false;
                    }
                }
            }else if(Mouse_IsUp(MOUSE_BUTTON_LEFT)){
                btn->hover=true;
            }else if(Mouse_IsDown(MOUSE_BUTTON_LEFT)){
                cbmap.button.buttonpress=true;
            }
        }
        if(btn->hover!=hover){
            cbmap.updatebuffer=true;
        }
    }
}
void CBMap_Update(void){
    cbmap.updatepoll=false;
    cbmap.timer.framestart=Time_Get();
    cbmap.width=Window_GetWidth();
    cbmap.height=Window_GetHeight();
    if(Window_IsResized())cbmap.updatebuffer=true;
    CBMap_Update_Camera();
    CBMap_Update_Button();
    CBMap_Update_Select();
    cbmap.timer.calcend=Time_Get();
}
void CBMap_Draw_Room(int x,int y,int posx,int posy,int size){
    int tile=100.0f*cbmap.camera.scale;
    Room *room=NULL;
    for(int i=0;i<cbmap.room.count;i++){
        if(cbmap.room.rooms[i].x==x&&cbmap.room.rooms[i].y==y){
            room=&cbmap.room.rooms[i];
            break;
        }
    }
    { // draw selection
        if(x==cbmap.select.x&&y==cbmap.select.y&&(cbmap.select.target==1||cbmap.select.target==2||cbmap.select.target==3)){
            if(cbmap.select.target==1){
                Shape_DrawRec(posx-tile*0.05f,posy-tile*0.05f,tile*1.1f,tile*1.1f,SKYBLUE);
            }else if(cbmap.select.target==2){
                Shape_DrawRec(posx-tile*0.05f,posy-tile*0.05f,tile*1.1f,tile*1.1f,DARKBLUE);
            }else if(cbmap.select.target==3){
                Shape_DrawRec(posx-tile*0.05f,posy-tile*0.05f,tile*1.1f,tile*1.1f,RED);
            }
        }else{
            if(x==cbmap.select.downx&&y==cbmap.select.downy){
                if(cbmap.select.target!=4&&cbmap.select.index>=0)Shape_DrawRec(posx-tile*0.05f,posy-tile*0.05f,tile*1.1f,tile*1.1f,DARKBLUE);
                else if(cbmap.select.target==4){
                    Shape_DrawRec(posx-tile*0.05f,posy-tile*0.05f,tile*1.1f,tile*1.1f,RED);
                    if(x==cbmap.select.x+1&&y==cbmap.select.y){
                        Shape_DrawRec(posx-tile*0.2f,posy,tile*0.2f,tile,RED);
                    }else if(x==cbmap.select.x-1&&y==cbmap.select.y){
                        Shape_DrawRec(posx+tile,posy,tile*0.2f,tile,RED);
                    }else if(x==cbmap.select.x&&y==cbmap.select.y+1){
                        Shape_DrawRec(posx,posy-tile*0.2f,tile,tile*0.2f,RED);
                    }else if(x==cbmap.select.x&&y==cbmap.select.y-1){
                        Shape_DrawRec(posx,posy+tile,tile,tile*0.2f,RED);
                    }
                }
            }else if(x==cbmap.select.x&&y==cbmap.select.y&&cbmap.select.target==4){
                Shape_DrawRec(posx-tile*0.05f,posy-tile*0.05f,tile*1.1f,tile*1.1f,RED);
            }
        }
    }
    if(room){
        Color bg,fg;
        switch(room->type){
        case db_roomtype_birth:bg=GOLD;fg=BLACK;break;
        case db_roomtype_gate:bg=DARKGRAY;fg=WHITE;break;
        case db_roomtype_store:bg=BLUE;fg=WHITE;break;
        case db_roomtype_train:bg=ORANGE;fg=WHITE;break;
        default:bg=GRAY;fg=WHITE;break;
        }
        Shape_DrawRec(posx,posy,size,size,bg);
        Text_Draw(Text_Format("%d",room->id),posx+1,posy+1,10,fg);

        char *name=malloc(256*sizeof(wchar_t));
        char *desc=malloc(4096*sizeof(wchar_t));
        memset(name,0,256*sizeof(wchar_t));
        memset(desc,0,4096*sizeof(wchar_t));
        wcstombs(name,room->name,256*sizeof(wchar_t));
        wcstombs(desc,room->desc,4096*sizeof(wchar_t));
        Text_DrawRec(Font_GetDefault(),Text_Format("%s\n\n%s",name,desc),
            (Rectangle){posx+1,posy+16,tile-2,tile-17},10,2,true,fg);
        free(name);
        free(desc);

        // draw exitss
        if(room->exits[dir_West]){
            Shape_DrawRec(posx-tile*0.2f,posy,tile*0.2f,tile,Color_Fade(bg,0.5));
        }if(room->exits[dir_East]){
            Shape_DrawRec(posx+tile,posy,tile*0.2f,tile,Color_Fade(bg,0.5));
        }if(room->exits[dir_North]){
            Shape_DrawRec(posx,posy-tile*0.2f,tile,tile*0.2f,Color_Fade(bg,0.5));
        }if(room->exits[dir_South]){
            Shape_DrawRec(posx,posy+tile,tile,tile*0.2f,Color_Fade(bg,0.5));
        }
    }
    else{
        Shape_DrawRec(posx,posy,size,size,Color_Fade(DARKGRAY,0.4));
        Text_Draw(Text_Format("(%d,%d)",x,y),posx+1,posy+1,10,WHITE);
    }
    cbmap.camera.recshown++;
}
void CBMap_Draw_Grid(void){
    int tile=100.0f*cbmap.camera.scale;
    int curx=cbmap.camera.curx,cury=cbmap.camera.cury;
    for(int tilex=cbmap.camera.initx;tilex<cbmap.width;tilex+=1.2*tile){
        for(int tiley=cbmap.camera.inity;tiley<cbmap.height;tiley+=1.2*tile){
            CBMap_Draw_Room(curx,cury,tilex,tiley,tile);
            cury++;
        }
        cury=cbmap.camera.cury;
        curx++;
    }
}
void CBMap_Draw_Text(void){
    Text_DrawRec(Font_GetDefault(), // rooms
        Text_Format("Total rooms: %d\n"
            "Room data size: %.2fKB/%.2fKB",
            cbmap.room.count,sizeof(Room)*cbmap.room.count/1024.0f,
            sizeof(Room)*cbmap.room.space/1024.0f),
        (Rectangle){5,5,190,200},10,2,true,WHITE);
    Text_DrawRec(Font_GetDefault(), // camera
        Text_Format("Camera: (%d, %d)\n"
            "Scale: %.2f%%\n"
            "R to reset camera",
            cbmap.camera.x,cbmap.camera.y,
            cbmap.camera.scale*100.0f
        ),
        (Rectangle){5,55,190,200},10,2,true,SKYBLUE);
    float fps=1.0f/(Time_Get()-cbmap.timer.framestart);
    Text_DrawRec(Font_GetDefault(), // draw time
        Text_Format(
            "%s\ns"
            "Calculation time: %.1f us\n"
            "Draw time: %.4f ms\n"
            "Tiles drawn: %d\n"
            "Equivalent FPS: %.1f",
            cbmap.updatepoll==true?"Polling events":"Waiting events",
            (cbmap.timer.calcend-cbmap.timer.framestart)*1000000.0f,
            (Time_Get()-cbmap.timer.calcend)*1000.0f,
            cbmap.camera.recshown,
            fps<999.9f?fps:999.9f
            ),
        (Rectangle){5,105,190,200},10,2,true,SKYBLUE);
    { // selection
        int ind=cbmap.select.index;
        Text_DrawRec(Font_GetDefault(),
            Text_Format("%s\n"
                "Room id: %d\n",
                cbmap.select.target==0?"No selection":
                    cbmap.select.target==1?"Hover":
                    cbmap.select.target==2?"Select":
                    cbmap.select.target==3?"Double Click":
                    cbmap.select.target==4?"Drag":"--",
                ind>=0?cbmap.room.rooms[ind].id:0
            ),
            (Rectangle){5,195,190,200},10,2,true,WHITE);
        if(ind>=0){
            Room *rm=&cbmap.room.rooms[ind];
            char *name=malloc(256*sizeof(wchar_t));
            char *desc=malloc(4096*sizeof(wchar_t));
            memset(name,0,256*sizeof(wchar_t));
            memset(desc,0,4096*sizeof(wchar_t));
            wcstombs(name,rm->name,256*sizeof(wchar_t));
            wcstombs(desc,rm->desc,4096*sizeof(wchar_t));
            Text_DrawRec(Font_GetDefault(),
                Text_Format(
                    "%s\n"
                    "Description: %s\n",
                    name,desc
                ),
                (Rectangle){5,245,190,200},10,2,true,WHITE);
            free(name);
            free(desc);
        }
    }
}
void CBMap_Draw_Buttons(void){
    if(!cbmap.select.doubleclick)return;
    for(int i=0;i<cbmap.button.count;i++){
        Button *btn=&cbmap.button.buttons[i];
        if(btn->hover)Shape_DrawRec(btn->x,btn->y,btn->w,25,btn->cb);
        else Shape_DrawRec(btn->x,btn->y,btn->w,25,btn->cd);
        Text_Draw(btn->label,btn->x+3,btn->y+2,20,WHITE);
    }
}
void CBMap_Draw(void){
    Buffer_Begin();
    Buffer_Clear(BLACK);
    CBMap_Draw_Grid();
    Shape_DrawRec(0,0,200,cbmap.height,DARKGRAY);
    CBMap_Draw_Text();
    CBMap_Draw_Buttons();
    Buffer_Update();
    cbmap.updatebuffer=false;
}
void CBMap_Exit(void){
    for(int i=0;i<cbmap.room.count;i++){
        Room *room=&cbmap.room.rooms[i];
        if(room->name)free(room->name);
        if(room->desc)free(room->desc);
    }
    free(cbmap.room.rooms);
    exit(0);
}

wchar_t scan_str[1024];
void CBMap_GetInput(int chars){
    printf(">");
    wmemset(scan_str,0,4096);
    fflush(stdout);
    fgetws(scan_str,chars,stdin);
    if(scan_str[wcslen(scan_str)-1]==L'\n'){
        scan_str[wcslen(scan_str)-1]=0;
    }
    fflush(stdin);
}

void Room_Create(Room room){
    if(cbmap.room.count==cbmap.room.space){
        cbmap.room.space+=MALLOC_STEP;
        Room *rooms=malloc(sizeof(Room)*cbmap.room.space);
        for(int i=0;i<cbmap.room.count;i++){
            rooms[i]=cbmap.room.rooms[i];
        }
        free(cbmap.room.rooms);
        cbmap.room.rooms=rooms;
    }
    room.name=malloc(256*sizeof(wchar_t));
    wmemset(room.name,0,256);
    wcscpy(room.name,L"Untitled room");
    room.desc=malloc(4096*sizeof(wchar_t));
    wmemset(room.desc,0,4096);
    wcscpy(room.desc,L"No description");
    cbmap.room.rooms[cbmap.room.count]=room;
    cbmap.room.count++;
}
void Room_Export(FILE *fp,Room *room){
    fprintf(fp,"    {.id=%d,\n",room->id+cbmap.room.initid);
    fprintf(fp,"        .region=db_roomregion_%s,\n",
        room->region==0?"nlcity":"forest"
    );
    fwprintf(fp,L"        .name=L\"%ls\",\n",room->name);
    fwprintf(fp,L"        .desc=L\"%ls\",\n",room->desc);
    fprintf(fp,L"        .type=db_roomtype_%s,\n",
        room->type==db_roomtype_birth?"birth":
        room->type==db_roomtype_gate?"gate":
        room->type==db_roomtype_store?"store":
        room->type==db_roomtype_plain?"plain":"train"
    );
    fprintf(fp,"       .exits={");
    {
        bool exited=false;
        if(room->exits[dir_East]){
            fprintf(fp,"[dir_East]=%d",room->exits[dir_East]);
            exited=true;
        }
        if(room->exits[dir_West]){
            if(exited)fprintf(fp,",");
            fprintf(fp,"[dir_West]=%d",room->exits[dir_West]);
            exited=true;
        }
        if(room->exits[dir_North]){
            if(exited)fprintf(fp,",");
            fprintf(fp,"[dir_North]=%d",room->exits[dir_North]);
            exited=true;
        }
        if(room->exits[dir_South]){
            if(exited)fprintf(fp,",");
            fprintf(fp,"[dir_South]=%d",room->exits[dir_South]);
            exited=true;
        }
    }
    fprintf(fp,"}\n    },\n");
}
void Room_Delete(int roomindex){
    Room *room=&cbmap.room.rooms[roomindex];
    for(int i=0;i<cbmap.room.count;i++){
        Room *room2=&cbmap.room.rooms[i];
        if(i>roomindex)room2->id--;
        for(int j=0;j<6;j++){
            if(room2->exits[j]==room->id){
                room2->exits[j]=0;
            }else if(room2->exits[j]>roomindex+1){
                room2->exits[j]--;
            }
        }
    }
    for(int i=roomindex;i<cbmap.room.count-1;i++){
        cbmap.room.rooms[i]=cbmap.room.rooms[i+1];
    }
    Room emptyroom={0};
    cbmap.room.rooms[cbmap.room.count-1]=emptyroom;
    cbmap.room.count--;
}
void Room_EditName(int roomindex){
    Room *room=&cbmap.room.rooms[roomindex];
    CBMap_GetInput(256);
    wcscpy(room->name,scan_str);
}
void Room_EditDesc(int roomindex){
    Room *room=&cbmap.room.rooms[roomindex];
    CBMap_GetInput(4096);
    wcscpy(room->desc,scan_str);
}
void Room_EditRegn(int roomindex){
    Room *room=&cbmap.room.rooms[roomindex];
}
void Room_EditType(int roomindex){
    Room *room=&cbmap.room.rooms[roomindex];
}
void Room_EditTabl(int roomindex){
    Room *room=&cbmap.room.rooms[roomindex];
}

void Room_Delete_Current(void){
    Room_Delete(cbmap.select.index);
}
void Room_EditName_Current(void){
    Room_EditName(cbmap.select.index);
}
void Room_EditDesc_Current(void){
    Room_EditDesc(cbmap.select.index);
}
void Room_EditRegn_Current(void){
    Room_EditRegn(cbmap.select.index);
}
void Room_EditType_Current(void){
    Room_EditType(cbmap.select.index);
}
void Room_EditTabl_Current(void){
    Room_EditTabl(cbmap.select.index);
}
#endif // CBMAP_H
