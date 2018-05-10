/* 
 * File:   Analise.h
 * Author: opus
 *
 * Created on 23 de Fevereiro de 2016, 18:06
 */

#ifndef ANALISE_H
#define	ANALISE_H

#include "bsp.h"


#define VARNUM 32 

namespace blaine {

class Analise {
    
    enum {
        RESET = 0,
        STANDBY,
        INITPARAMS,
        TESTPOINT,
        ABORTRUN,
        
        INITMANUAL,
        RUNMANUAL,
        
        WAITSERVICEOPTION,
  
       
        ASKSAMPLEREADY,
        WAITSAMPLEREADY,    
 
        CHARGING,
        SYSTEMCHARGED,
        MEASURING,
        CALCULATERESULTS,
 
        DOANALISE,
        ASKSAMPLEWEIGHT,
        WAITSAMPLEWEIGHT,
        LOADSID, 
        ASKSID,
        WAITSID,
        ASKFLOW,
        WAITFLOW,
        UPDATEANALISE,
        WAITENDANALISE,
        
        
        PRINTANALISE,
        PRINTCALIB,
        WAITPRINT,
        PRINTREQUESTED,
        ASKTOPRINT,
        WAITASKTOPRINT,
        
        STOREANALISE,
        STOREREQUESTED,
        ASKTOSTORE,
        WAITTOSTORE,
        
        VENTCELL,
        WAITTOVENT,
        
        FLUSHCELL,
        WAITTOFLUSH,
        FLUSHEND,
        
        PREPAREZERO,
        MEASUREZERO,
        SHOWZEROERROR,
        WAITZEROERROR,
        
        PREPARECHARGE,
        WAITTOCHARGE,
        
        REGISTERRUN,
        INTERRUNINFO,
        
        PREPAREEXPAND,
        WAITTOEXPAND,     
        REGISTEREXPANSION,
       
        DOCYCLE,
        FAKECYCLE,
        
        DOVACAL,
        UPDATEVAEMPTY,
        WAITVASPHEREREADY,
        UPDATEVA,
        ASKVAREPORT,
        
        DOVCCAL,
        ASKCELLTYPE,
        WAITCELLTYPE,
        ASKMULTIRUN,
        SINGLERUNSETUP,
        INPUTMAXRUNS,
        WAITMAXRUNS,
        INPUTMAXDEV,
        WAITMAXDEV,
        UPDATEVC,
        ASKVCREPORT,
   
        ASKCELL,
        WAITCELL,
       
        ASKADVOL,
        WAITADVOL,
       
        ASKSPHEREVOL,
        WAITSPHEREVOL,
   
        UNHANDLED,	
	
    } ASTATES;        

    enum{
        S_IDLE,
        
        S_CHARGING,
        S_CHARGED,
        S_CHARGEFAILED,
        S_CHARGEENDED,
        
        S_EXPANDING,
        S_EXPANDENDED,
        S_EXPANDFAILED,
        
        S_VENTING,
        S_VENTED,
        
        S_ZEROING,
        S_ZEROSTABLE,
        S_ZEROFAILED,
        S_ZEROENDED,
        
        S_FLUSHTIME,
        S_FLUSHNONE,
        S_FLUSHPULSEON,
        S_FLUSHPULSEOFF,
   
        S_FAILED,
        
    } SENSORSTATES;
    
    enum{
        S_IDLE_SIG,
        
        S_FLUSHTIME_SIG,
        S_FLUSHNONE_SIG,
        S_FLUSHPULSEON_SIG,
        S_FLUSHPULSEOFF_SIG,
        S_FLUSHVAC_SIG,
        
        S_CHARGE_SIG,
        S_EXPAND_SIG,
        S_VENT_SIG,
        S_ZERO_SIG,
        
        S_NULL_SIG,
        S_RESET_SIG,
        
    };
    
    
    enum{
        SCALC_IDLE,
        SCALC_SETWINDOW,
        SCALC_PRESSURELESS,
        SCALC_PRESSUREOVER,
        SCALC_RSDLIMIT,
        SCALC_DRIFTLIMIT,
        SCALC_TRIPCHARGE,
        SCALC_FLUSHTIME,
        
        
    }SCALCTYPE;
    
public:
        Analise();
        Analise(const Analise& orig);
        virtual ~Analise();
        
        void serviceEvents();
        void pushEvent(event_t evt);
        
        double getAccessCode();
        double getUser();
        void setUser(double usertype);
        
        void initVars();
        
        
private:
    
    
    bool dummyvar;
    double acode;
    double user;
    
    uint8_t a_state;
    uint8_t temp_state;
    
    std::stack<uint8_t> state_history;
    uint8_t default_history;
    
    bool boolhistory;
    double doublehistory;
    bool flip;
    
    uint8_t show_mode;
    bool monitor_on_term;
    bool monitor_on_file;
    uint8_t finalresptr;
    
    uint8_t tempkcode;
   
    bool alarmset[4];
    
    bool inputavailable;
    double inputval;
    char inputsval[32];
    
    bool dlgavailable;
    uint8_t dlganswer;
    
    bool choiceavailable;
    uint8_t choiceanswer;
 
    std::queue<event_t> events;
    event_t *dummyevt;
 
    vector<pmaps>apmap;
        
    time_t rawtime;
    struct tm * timeinfo;
    char tsbuf[64];
   
    bool usesequencer;
    char sidhistoryfile[32];
    bool storehistory;
    
    bool autostore;
    bool asktostore;
    char storefile[32];
    
    bool autoprint;
    bool asktoprint;
    bool printruns;
    bool printextra;
    bool printqc;
    bool ejectprint; 
    char printheader[32];
    char timeformat[32];
    
    double tempa0;
    double tempa1;
    
    char sampleid[32];
    bool prefernumeric;
  
    double cellsize;
    
    double sampleweight;
    bool singlerun;
    
    double deviation;
    double purgemode;
    double purgetime;
    double purgecicles;
    double purgepumptime;
    
    
    double pbf[256];
    uint8_t pbfptr;
    uint8_t pbfload;
    double pvalue;
  
    double pruns[16];
    uint8_t pruns_head;
    uint8_t pruns_tail;
    uint8_t cache;
    double pruns_mvavg;
    double pruns_sd;
    double pruns_dv;
    double temperature;
    
    uint8_t sensor_state;
    
    double pa0;
    double pa1;
    double poffset;
    bool usepoffset;
    bool userawcounts;
    bool monitorpressure;
    bool zerotransducer;
  
    double vent_tgtpressure;
    double load_tgtpressure;
    double pulse_tgtpressure;
    
    double flushtime;
    double flushtype;
    double flushcicles;
    double flushcicletime;
    double flushcicle;
    long flushstarttag;
    long flushendtag;
    
    double load_prate;
    bool useratetoload;
    
    bool forcezerocal;
    double zero_driftwstart;
    double zero_driftwend;
    
    double load_trigfactor;
    double load_driftwstart;
    double load_driftwend;
  
    double driftwindowstart;
    double driftwindowend;
    double absdriftlimit;
    double reldriftlimit;
    bool blockondriftfail;
    double equitime;
    
    double cycledrift;
    double cycleavg;
    
    std::deque<double>pdvs;
    int lwindow;
    uint8_t driftstatus;
    double driftresult;
    double lastpressure;
    double lasttemperature;
    double lastdrift;
 
    double chargetrip;
    
    struct loadrun{
        long timestamp;
        
        double loadpressure;
        int loadstatus;
        double loaddrift;
        
        double expandpressure;
        int expandstatus;
        double expanddrift;
        
        double deltap;
        double pratio;
        
        double cyclersd;
        double cycleavg;
        
        double volume;
        double density;
        double temperature;
        
        loadrun(){}
        
        loadrun (long c_timestamp, double c_loadpressure, int c_loadstatus, double c_loaddrift) {
            timestamp = c_timestamp; loadpressure = c_loadpressure; loadstatus = c_loadstatus; loaddrift = c_loaddrift; 
            
            temperature = 21.0; 
            expandpressure = 6.0;
            expandstatus=0;
            expanddrift=0.0;
            pratio=1.0;
            volume= 0.0;
            density = 0.0;
            cyclersd=0.0;
            cycleavg=0.0;
        }
        loadrun(const loadrun& orig) {
            timestamp=orig.timestamp;
            loadpressure=orig.loadpressure;
            loadstatus=orig.loadstatus;
            loaddrift=orig.loaddrift;
            expandpressure=orig.expandpressure;
            expandstatus=orig.expandstatus;
            expanddrift=orig.expanddrift;
            deltap=orig.deltap;
            pratio=orig.pratio;
            temperature=orig.temperature;
            volume = orig.volume;
            density = orig.density;
            cyclersd=orig.cyclersd;
            cycleavg=orig.cycleavg;
        }
        
    };
    vector<loadrun>lruns;
    
    char s_spherevol[16];
    
    struct s_keymap{
        std::string key;
        std::string value;
        
        s_keymap (std::string c_key, std::string c_value){
            key=c_key; value=c_value;
        }
        s_keymap(const s_keymap& orig){
            key=orig.key;
            value=orig.value;
        }
        s_keymap (const char* ch_key, const char* ch_value){
            key = std::string(ch_key);
            value = std::string (ch_value);
        }
       
        
    };
 
    vector<s_keymap> cellvolumes;
    vector<s_keymap> spherevolumes;
    uint8_t cellptr;
    uint8_t sphereptr;
    
    
    uint8_t cycles;
    double maxruns;
    double runsaveraged;
    double analysisrsd;
    double lastrsdavg;
    double lastrsd;
    double infogap;
    bool startingcycle;
    bool rsdbypressure;
    bool usepratio;
    
    bool smallva;
//    double smallvolume;
//    double largevolume;
//    double cellvolume;
    
    double spherevol;
    
    double emptyratio;
    double sphereratio;
    
    double vadded;
    double vcell;
    
    double vaddeds[3];
    double vcells[3];
    double spherevols[3];
    uint8_t celltype = 0;
    bool doingcal = false;
    
    double mass;
    double volume;
    double density;
    
    // Icon Navigation
    uint8_t man_iconptr;
    uint8_t man_icons[8];
    uint8_t man_icontemp;
    
    int valves[5];
  
    void loadAutoSID();
    void loadParams();
    void clearAlarms();
    void clearHistory();

    std::string * getTimerString(double val, double of);
    std::string * getFlushPulseString(double pulse, double of);
    std::string * getPressureString(bool added);
    std::string * getDriftStatusString();
    std::string * getTempString(uint8_t mode);
    std::string * getRSDString(double rsd, double mean);
    std::string * getCycleString(double cycle, double max);
    std::string * getVAResultString();
    std::string * getVCResultString();
    std::string * getFinalString(uint8_t mode, uint8_t slot, bool media);
    std::string * getRunsString(uint8_t run);
    std::string * getPartialDenString();
    std::string * getOKtoStartString();
   
    double getMedia(uint8_t mode);
    
    char * getFormatedTime ( char * format, long rtime);
    
    void buildBarGraphs();
    void buildIconBar(uint8_t mode);
    
    void calculateVA();
    void calculateVC();
    double calculateVP(double ratio);
    double calculateDEN(double vol);
    
    void storeResult();    
    void print(uint8_t type);
    
    uint8_t _verifyKeyCode(uint8_t code);
    bool _locateEvent (int event_type);
    
    void flushValves();
    void activateValve (uint8_t index, int val);
    
    int LinReg(uint8_t n, double x[], double y[], double *b, double *m, double *r );
    double calculateMean(uint8_t n, double v[]);
    double calculateSD(uint8_t n, double v[], double avg);
    void updatePRuns(bool clear, double val);
    
    uint8_t SensorSM(uint8_t signal);
    int CalculateParams( uint8_t type, double limit, bool showstatus);
    bool verifyCyclesRSD(bool usepressure, uint8_t runs);
    void reportCycles();
    
    
};
 
}

#endif	/* ANALISE_H */
