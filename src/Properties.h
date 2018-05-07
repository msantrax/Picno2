#ifndef PROPERTIES_H
#define PROPERTIES_H

#include "bsp.h"

#include <vector>
#include <string>

namespace blaine {

#define SDPREFIX "/sd/"        
        
class Properties {

public:
    
    Properties();
    virtual ~Properties();
    
    enum{ACP,BLAINEDEV} PROPKEYREALM;
    enum{P_FLOAT,P_BOOLEAN,P_STRING} PROPKEYTYPE;
   
    void serviceEvents();
    void pushEvent(event_t evt);
    
    bool isActive();
    void activate();
    
    bool loadkeys (char * file);
    bool storeKeys (bool root, char * file);
    bool addKey (pmaps * key);
    void updateKey(pmaps * key, char * value);
    void listKeys();
    
    double getDoubleParam(char * lkey, double defval);
    bool getBooleanParam (char * lkey, bool defval);
    char * getStringParam (char * lkey, char * defval);
    
    bool buildStoreEntry (char * lkey, char * defval, char * out);
    bool storeHistory(std::queue<string> * items);
    std::vector<string> * loadHistory (char * file);
    
    void setRawTime(time_t rawt) {rawtime = (time_t)rawt; set_time(rawtime);};
    time_t getRawTime() {return rawtime;};
  
private:
    
    std::queue<event_t> events;
    event_t *dummyevt;
    
     enum {
        RESET = 0,
        STANDBY,
        SHOWOPTIONS,
        WAITOPTION,
        ABORTCONFIG,
        
        EDITITEM,
        WAITINPUT,
        WAITSTRING,
        WAITFLAG,
        FLAGYES,
        FLAGNO,
        
        LOADPROFILE=100,
        SAVEPROFILE=101,
        SETDATETIME=102,
        WAITDATETIME=103,
  
    };
     
    char profile_file[32];
    char language_file[32];
    char username[32];
    char password[32];
    char curtime[32];
    bool logonstart;
    
    time_t rawtime;
	struct tm * timeinfo;
    
    vector<pmaps>prop_pmap;
     
    uint8_t a_state;
    uint8_t a_status;
    
    uint8_t state_history;
    uint8_t tempkcode;
    
    
    bool alarmset[4];
    
    bool inputavailable;
    double inputval;
    char inputsval[32];
    
    bool dlgavailable;
    uint8_t dlganswer;
    
    bool choiceavailable;
    uint8_t choiceanswer;
    uint8_t choicedepth;
    uint8_t selecteditem;
    uint8_t header;
    pmaps * choicekey;
    
    
    char * buildPath(char * path);
    uint8_t _locateKey(char * key);
    bool testFS();
    bool filterLine (char * in_string);
    const std::vector<std::string>explode(char * it, const char& c) ;  
    std::vector<string> * dummyChoice();
    bool addKeys (std::vector<pmaps> * keys);
    pmaps * findKey (char * keytofind); 
    time_t convertTime(char * fmttime);
    void appendToFile (string * file, string * payload);
    
    bool loadFlash();
    void storeFlash();
    
    SDFileSystem  * sd_drv;
    
    bool active;         
    char path_buf[256];     

    std::vector<pmaps *> pkeysv;
    int flashaddr;
    int *flashinitptr;
    //char *varinitptr;
    //char *varendptr;
    
    
    uint8_t keycount;
};


}

#endif  /* PROPERTIES_H */




   
//    struct bundlemap_t {
//        char * key;
//        char * lstring;          
//    }bundlemap[200] = {
//            {"teste1", "test2"},
//            {"teste1", "test2"},         
//    };
    

 
//    typedef struct pkeys_t {
//        uint8_t keyrealm;
//        uint8_t keytype;
//        char key[48];
//        char value[32];
//        void * pvar;   
//    };
    
    