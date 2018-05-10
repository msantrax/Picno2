#ifndef bsp_h
#define bsp_h

#include "mbed.h"
#include "SDFileSystem.h"

#include <vector>
#include <string>
#include <queue>
#include <stack>
#include <deque>

#define BSP_TICKS_PER_SEC   64U

//namespace blaine {

#define FATAL 0
#define ERROR 1
#define WARNING 2
#define INFO 3
#define PLUMB 4
#define FINE 5
#define FINER 6
#define FINEST 7


enum OBJTYPES{
    MAIN=0x00010000,
    GLCD=0x00020000,
    ANALISE=0x00040000,
    PRINTER=0x00080000,
    PROPERTIES=0X00100000,
};

enum SIGTYPES{
        NONE=0,
        KEYCODE_SIG,
        ALARM_SIG,
        
        DRAWSHAPE_SIG,
        DRAWMESSAGE_SIG,
        UPDATEICON_SIG,
        
        GLCDRESET_SIG,
        DRAWCANVAS_SIG,
        CANVASANSWER_SIG,
        DRAWDIALOG_SIG,
        DIALOGANSWER_SIG,
        DRAWINPUT_SIG,
        INPUTANSWER_SIG,
        DRAWCHOICE_SIG,
        CHOICEANSWER_SIG,
        INITBARGRAPH_SIG,
        UPDATEBARGRAPH_SIG,
        
        
        LOADHISTORY_SIG,
        HISTORYANSWER_SIG,
        SAVEHISTORY_SIG,
        
        LOADPARAMS_SIG,
        LOADKEYS_SIG,
        ACTIVATEPROPERTIES_SIG,
        LISTPROPERTIES_SIG,
        APPENDTOFILE_SIG,
        SETUSER_SIG,
          
        INITANALISE_SIG,
        DOANALISE_SIG,
        ANALISEDONE_SIG,
        
        DOMANUAL_SIG,
        
        DOVACAL_SIG,
        DOVCCAL_SIG,
        
    
        DOCONFIG_SIG,
        CONFIGDONE_SIG,
            
        PRINTSTRING_SIG,
        FLUSHPRINTER_SIG,
        RESETPRINTER_SIG,    
        
};

   
struct pmaps{
    uint8_t keyrealm;
    uint8_t keytype;
    char * key;
    void * pvar;
    char pstring[32];
    pmaps(uint8_t c_keyrealm, uint8_t c_keytype, char * c_key, void * c_pvar, char * c_pstring ){
        keyrealm=c_keyrealm; keytype=c_keytype; key=c_key; pvar=c_pvar;
        std::strncpy(&pstring[0], &c_pstring[0], 30);
        pstring[30]=0x0;        
    }
};

struct event_t {
    int evtype;
    uint8_t simple;
    void * payload;
    event_t (int c_evtype, uint8_t c_simple, void* c_payload) {
        evtype = c_evtype; simple = c_simple; payload = c_payload;
    }
};

struct appendfile_evt {
    int actiontype;
    string * filepath;
    string * payload;
    appendfile_evt (int c_actiontype, string * c_filepath, string * c_payload) {
        actiontype = c_actiontype; filepath=c_filepath; payload=c_payload;
    }
};

struct drawshape_evt{
    uint8_t shape;
    uint8_t x;
    uint8_t y;    
    bool savebkg;
    drawshape_evt (uint8_t c_shape, uint8_t c_x, uint8_t c_y, bool c_savebkg) {
        shape = c_shape; x = c_x; y = c_y; savebkg = c_savebkg;
    }
};

// GLCD Structures
struct canvas_evt{
    uint8_t bkgshape;
    uint8_t slots;
    uint8_t signals[6];
    uint8_t rollback;
    canvas_evt(uint8_t c_bkg, uint8_t c_slots, uint8_t c_rollback, 
                    uint8_t sig0, uint8_t sig1, uint8_t sig2, uint8_t sig3, uint8_t sig4, uint8_t sig5)
    { bkgshape=c_bkg; slots=c_slots, rollback=c_rollback;
        signals[0]=sig0; signals[1]=sig1; signals[2]=sig2; signals[3]=sig3; signals[4]=sig4; signals[5]=sig5; }  
}; 

struct dialog_evt{
    uint8_t type;
    uint8_t yesok;
    uint8_t no;
    uint8_t cancel;
    string * msg;
    dialog_evt (uint8_t c_type, uint8_t c_yesok, uint8_t c_no,uint8_t c_cancel, string * c_msg) 
    { type=c_type; yesok=c_yesok, no=c_no, cancel=c_cancel, msg = c_msg; }
};

struct input_evt{
    uint8_t type; 
    bool numeric; 
    bool insert; 
    bool uppercase;
    bool savehistory;
    string * line;
    string * data;
    string * form;
    string * hist;
    double val;
    double llimit;
    double hlimit;
    
    input_evt(uint8_t c_type, bool c_numeric, bool c_insert, bool c_uppercase, bool c_savehistory, string * c_line, string * c_data,
                        string * c_form, string * c_hist, double c_val, double c_llimit, double c_hlimit)
    {type=c_type; numeric=c_numeric; insert=c_insert; uppercase=c_uppercase; savehistory=c_savehistory; line=c_line; data=c_data;
                        form=c_form; hist=c_hist, val=c_val; llimit=c_llimit; hlimit=c_hlimit;}  
};

struct inputanswer_evt {
    double val;
    char sval[32];
    inputanswer_evt (double c_val, char * c_sval){
        val=c_val;
        strcpy(&sval[0], c_sval);
    } 
};

struct choice_evt{
    string * prompt;
    std::vector<std::string> * items;
    choice_evt(string * c_prompt, std::vector<std::string> * c_items)
        {prompt=c_prompt; items=c_items;
    }
};


struct icon_evt{
    uint8_t slot;
    uint8_t action;
    uint8_t shape;
    icon_evt(uint8_t c_slot, uint8_t c_action, uint8_t c_shape){
        slot=c_slot; action=c_action; shape=c_shape;
    } 
};

struct drawmessage_evt{
    uint8_t x;
    uint8_t y;
    uint8_t width;
    bool invert;
    bool transp;
    bool clear;
    uint8_t align;
    string * msg;
    drawmessage_evt(uint8_t c_x, uint8_t c_y, uint8_t c_width, bool c_invert, bool c_transp,  bool c_clear, uint8_t c_align, string * c_msg){
        x=c_x; y=c_y; width=c_width; invert=c_invert; transp=c_transp; clear=c_clear; align=c_align; msg=c_msg;
    }
};

struct bginit_evt{
    uint8_t handle;
    uint8_t line;
    uint8_t type;
    bool left;
    bool literal;
    string * format;
    double lowlim;
    double highlim;
    double litlimit;
    bginit_evt(uint8_t c_handle, uint8_t c_line, uint8_t c_type, bool c_left, bool c_literal, string* c_format, double c_lowlim, double c_highlim, double c_litlimit){
        handle=c_handle; line=c_line; type=c_type; left=c_left; literal =c_literal; format=c_format; lowlim=c_lowlim; highlim=c_highlim; litlimit=c_litlimit; 
    }
};

struct bgupdate_evt{
    uint8_t handle;
    uint8_t cmd;
    double value;
    string * extra;
    bgupdate_evt (uint8_t c_handle, uint8_t c_cmd, double c_value, string* c_extra){
        handle=c_handle; cmd=c_cmd; value=c_value; extra=c_extra;
    }
};



void OSTick();
long getTick();
void clearAlarms();
bool isAlarmTime(uint8_t index);
void setAlarm(uint8_t index, unsigned long interval, unsigned long rld);
void resetAlarm(uint8_t index);
void setInitTick();
long getRelativeTick();
double getSeconds();


void log (int level, const char* format, ...);
void printLog();
void setLogLevel(int level);

void publish (event_t evt);
void verifymail();
void subscribe (int object, int signal);
void unsubscribe (int object, int signal);
void transferPlumbig (int from, int to);
bool locateEvent (int event_type);
uint8_t verifyKeyCode(uint8_t code);


void drawShapeRequest (uint8_t c_shape, uint8_t c_x, uint8_t c_y, bool c_savebkg);
void simpleEventRequest (int signal, uint8_t simple);
void drawCanvasRequest(uint8_t c_bkg, uint8_t c_slots, 
                    uint8_t c_rollback, 
                    uint8_t sig0, uint8_t sig1, uint8_t sig2, uint8_t sig3, uint8_t sig4, uint8_t sig5);
void drawDialogRequest (uint8_t type, uint8_t c_yesok, uint8_t c_no, uint8_t c_cancel, const char * cmsg );
void drawInputRequest(uint8_t c_type, bool c_numeric, bool c_insert, bool c_uppercase, char * c_line, char * c_data,
                        char * c_form, double c_val, double c_llimit, double c_hlimit);
void drawChoiceEvent (char * c_prompt, std::vector<std::string> * c_items);

std::vector<string> * dummyChoice();

void rtcSetTime (time_t settime);


#endif // bsp_h


//#ifdef BAREMETAL
//    
//    //#include "../mbed/TARGET_K64F/TARGET_Freescale/TARGET_KPSDK_MCUS/TARGET_MCU_K64F/TARGET_FRDM/PinNames.h"
//
//    #define PDEBUG(mes_) pdebug(0, mes_)
//    //#define PDEBUG1(mes_, p1_) pdebug(mes_, p1_)
//    //#define PDEBUG2(mes_, p1_, p2_) serial_drv.printf(mes_, p1_, p2_)
//    //#define PDEBUG3(mes_, p1_, p2_, p3_) serial_drv.printf(mes_, p1_, p2_, p3_)
//    //#define PDEBUG4(mes_, p1_, p2_, p3_, p4_) serial_drv.printf(mes_, p1_, p2_, p3_, p4_)
//    
//#else
//    #include <QtCore/QtDebug>
//    #include <QtGui>
//    #include "../mainwindow.h"
//    #define PDEBUG(mes_) qDebug(mes_)
//    #define PDEBUG1(mes_, p1_) qDebug(mes_, p1_)
//    #define PDEBUG2(mes_, p1_, p2_) qDebug(mes_, p1_, p2_)
//    #define SDPREFIX "/Bascon/Picnometro/sd/"
//    //inline char * buildPath(char path[]) { return strcat("/Bascon/Picnometro/sd/", path);} 
//    //#define PFOPEN(path_, mode_) fopen(SDPREFIX(path_)", (mode_));
//#endif



//struct printstring_evt{
//    uint8_t prefix;
//    string * payload;
//    printstring_evt( uint8_t c_prefix, string * c_payload){
//        prefix=c_prefix, payload=c_payload;
//    }
//};
