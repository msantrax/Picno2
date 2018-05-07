/* 
 * File:   glcd.h
 * Author: opus
 *
 * Created on 21 de Novembro de 2015, 11:38
 */

#ifndef GLCD_H
#define	GLCD_H

#include "../src/bsp.h"

#include "keyboard.h"
#include "shapes.h"
#include "fonts.h"
//#include "Salloc.h"

#include <stdint.h>
#include <vector>
#include <string>

namespace blaine {
                
class glcd {

   
    
public:    
    
    glcd();
    virtual ~glcd();
   
    void _loadPage (bool invert, bool left);
    void _WriteData (unsigned char data);
    void _WriteCmd(unsigned char data);
    void _initHardware(void);
    
    void putMessage(uint8_t x, uint8_t y, uint8_t width, bool inv, bool transp, bool clear, uint8_t align, string *msg);
    void _putMessage(uint8_t x, uint8_t y, bool inv, bool transp, char *Message);
    void invertArea (uint8_t x , uint8_t page,  uint8_t heigth, uint8_t lenght);
    void drawShapeRequest (uint16_t id, uint8_t x, uint8_t y, bool savebkg);
    void restoreBackground (uint16_t id);
    void activateHourGlass();
    void dismissHourGlass();
    void resetStatusbar();
    
    void _drawShape (uint16_t id, uint8_t x, uint8_t y, bool savebkg);
    
    void GLCDRefresh();
    void SBarRefresh();
    void setIcon (uint8_t slot, uint8_t status, uint8_t shape);
    void setStatus (uint8_t showtime, string * msg);
 
     
    void drawDlg (dialog_evt *dlgevt);
    uint8_t navigateDlg(uint8_t keycode);
 
    void drawCanvas(canvas_evt *cevt);
    uint8_t navigateCanvas(uint8_t keycode);
    void clearCanvas();
    
    void drawInput(input_evt * evt);
    uint8_t navigateInput(uint8_t keycode);
    void clearInput();
    void _updateInput (bool setcursor);
    void setNumeric(bool numeric);
    void _insertInputChar (uint8_t ichar);
    void _setInputCursor(int direction);
    void _resetInputIcons();
    double getFloatInput();
    
    
    void drawChoice(choice_evt *choice);
    uint8_t navigateChoice(uint8_t keycode);
    void clearChoice();
    void _drawChoiceItem (uint8_t vcursor, uint8_t pos, bool invert, bool clear);
    char * getLastChoice();
   
    void drawBGraph (bginit_evt *initbgevt);
    void updagteBGraph (bgupdate_evt *updatebgevt);
    
    
    void _WaitNotBusy(void);
    void setKbdcode(uint8_t kbdcode);
    uint8_t getScanCode();
    uint8_t getKbdCode();
    uint8_t getKbdcode() const;
    void setPublish_kbdcode(bool publish_kbdcode);
    bool isPublish_kbdcode() const;
     
    void pushEvent(event_t evt);
    void serviceEvents();
    
    enum{
        CHIPNONE = 0,
        LEFTCHIP,
        RIGHTCHIP
    }CHIPSIDE;    
        
    enum{
        GCLEAR = 0,
        DIRTYALL,
        DIRTYLEFT,
        DIRTYRIGHT,
        CLEARPAGE,
        INVERTALL,
        INVERTBOX          
    }FRAMESTATUS;
    
    enum {
        KBD_UP=0x11,
        KBD_LEFT=0X12,
        KBD_RIGHT=0X13,
        KBD_DOWN=0X14,
        KBD_CR=0X0D,
        KBD_BS=0X08,
    } KBD_SCANCODES;
    
    enum {ICACTIVE=0,ICICLEAR,ICINVERT,ICREFRESH,ICBLINK,ICBLINKED} ICONSTATUS; 
    
    enum{ HG_IDLE=300, HG_SHOW, 
            HG_UP,
            HG_STRIP30, HG_STRIP60, HG_STRIP90,
            HG_TURN,
            HG_INV
    } HG_STATES;
 
    
    
private:
 
//    enum LOGLEVEL{
//        FATAL=0,
//        ERROR=1,
//        WARNING=2,
//        INFO=3,
//        FINE=4,
//        FINER=5,
//        FINEST=6,			
//    };

    
    struct screen_slice{
        uint8_t id;
        uint8_t x;
        uint8_t page;
        uint8_t slots;
        uint8_t width;
        char * payload;
        screen_slice (uint8_t c_id, uint8_t c_x, uint8_t c_page, uint8_t c_slots, uint8_t c_width, char * c_payload)
            {id=c_id; x=c_x; page=c_page; slots=c_slots; width=c_width; payload=c_payload;}
    };
    vector<screen_slice>gslices;
    
    
    
    struct inptdesc_t{
        uint8_t type; // spec
        bool numeric; // spec
        bool insert; // spec
        bool uppercase; //spec
        
        bool hashistory;
        bool savehistory;
        string * hist;
        std::vector<string> * histitems;
        
        uint8_t error;
        bool functionbar; //ctrl
        uint8_t cursor; 
        uint8_t iconcursor;
        char data[20];
        uint8_t format;
        double limit_h;
        double limit_l;
        double result;
    };
    inptdesc_t *inptdesc;
    
    
    struct choicedesc_t{
        uint8_t itemnum;
        uint8_t cursor;
        uint8_t offset;
        uint8_t vptrs[3];
        std::vector<string> items;        
    }; 
    choicedesc_t *choicedesc;
   
    std::queue<event_t> events;
    event_t *dummyevt;
    input_evt *input_temp_evt;
    
    enum{
        SERVICE_NONE,
        SERVICE_CANVAS,
        SERVICE_DIALOG,
        SERVICE_IDIALOG,
        SERVICE_INPUT,
        SERVICE_CHOICE,
        
        LOAD_HISTORY,
    };
    uint8_t service_state;
    uint8_t service_status;
    
    
  
    uint8_t framebuffer[1024];
    uint8_t *framebufferptr;
    uint8_t *currentpositionptr;
    uint8_t framepages[8];
    uint8_t *framepagesptr;
    bool glcdmutex;

    volatile uint8_t framepage;// = 0;
    volatile bool updated;
    volatile bool writing_semaphore;
    volatile uint8_t background;

    volatile uint8_t XPos;      // 128 cols
    volatile uint8_t YPos;      // 64 rows
    volatile uint8_t blink;     // cursor timer
  
    
    // ICONS =========================================================  
    uint8_t iconplaces[8] ;
    
    struct c_iconsctrl {
        uint8_t status;
        bool on;
        uint16_t id;
        c_iconsctrl () { status=0; on=false; id=0;}
        c_iconsctrl (uint8_t s, bool c_on, uint8_t c_id ) { status=s; on=c_on; id=c_id;};  
    } iconsctrl[8];
    c_iconsctrl iconsave[8];
    
    
    struct bgraph_handle {
        uint8_t handle;
        uint8_t type;
        bool literal;
        
        char sbuf[48];
        char fmt[48];
        
        uint8_t bgxoffset;
        uint8_t ltxoffset;
        
        uint8_t yoffset;
        uint8_t barsize;
        
        double llimit;
        double hlimit;
        double litlimit;
        
        
    } bg_handles[4];
    
    
    // Status Bar services ================================================ 
    uint16_t timetolast;//=0;
    bool sb_request= false;

    // Keyboard Services ===================================================
    uint8_t kbdscan;// = 0;
    uint8_t kbdtimer;// = 0;
    uint8_t kbdoffset;// = 0;
    bool kbdpressed;
    uint8_t kbdthiscode;// = 0;
    uint8_t kbdlastcode;// =0;
    uint8_t * kbdtblptr;
    const uint8_t * kbdptrs[5];
    bool kbdnumeric;
    bool kbdinsert;
    bool kbduppercase;
    uint8_t kbdcode;
    bool publish_kbdcode;
    std::queue<int> kbdevents;

    
    
    // Canvas Services ====================================================
    static enum{	
        STATE_CANVAS_APP1,
        STATE_CANVAS_APP2,
        STATE_CANVAS_APP3,
        STATE_CANVAS_APP4,
        STATE_CANVAS_APP5,
        STATE_CANVAS_APP6,
        STATE_CANVAS_DISPATCH,                
    }CANVAS_STATES;
    
    uint8_t canvas_move_map[6][4];
    uint8_t canvas_widget_map [6][3];
    uint8_t canvas_signals [6];
    uint8_t canvas_selected;
    uint8_t canvas_rollback;
    
    uint8_t dlg_type;
    uint8_t dlg_status;
    uint8_t dlg_yesok_sig;
    uint8_t dlg_no_sig;
    uint8_t dlg_cancel_sig;
    
    
    uint16_t calculateStrips (char * msg);
    void _clearPage(uint8_t page);
    void _clearArea(uint8_t page, uint8_t x, uint8_t lenght);
    void _ClearScreen(bool invert);
    void _moveRight(void);
    void _setPosition(uint8_t x, uint8_t y);
    bool _putChar(unsigned char data, bool inv, bool transp);
    void clearLeftIcons ();
    void saveIconState();
    void restoreIconState();
    void updateIcons();
    
    void _drawShapeFragment(uint8_t * shapeptr, uint8_t * maskframe, uint8_t slots, uint16_t offset, bool usemask);
    const std::vector<std::string> explode(char * it, const char& c);
    uint8_t locateSlice(uint8_t id);
    std::queue<string> * formatHistory();
    
    
};


}

#endif	/* GLCD_H */



//   ={
//  //up                  lft                 rt                  dw
//    {0,                 0,                  STATE_CANVAS_APP2,  STATE_CANVAS_APP4  },  //app1
//    {0,                 STATE_CANVAS_APP1,  STATE_CANVAS_APP3,  STATE_CANVAS_APP5  },  //app2
//    {0,                 STATE_CANVAS_APP2,  0,                  STATE_CANVAS_APP6  },  //app3
//    {STATE_CANVAS_APP1, 0,                  STATE_CANVAS_APP5,  0                  },  //app4
//    {STATE_CANVAS_APP2, STATE_CANVAS_APP4,  STATE_CANVAS_APP6,  0                  },  //app5
//    {STATE_CANVAS_APP3, STATE_CANVAS_APP5,  0,                  0                  },  //app6
//    };

