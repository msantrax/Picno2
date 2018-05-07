/* 
 * File:   polyperm220d.cpp
 * Author: opus
 *
 * Created on 29 de Fevereiro de 2016, 21:35

#include <cstdlib>
#include <cstdarg>
#include <cstring>
 */

#include "mbed.h"
#include "bsp.h"

#include "Properties.h"
#include "../glcd/glcd.h"
#include "Analise.h"
#include "Printer.h"
#include "Bundle.h"

#include "DS1307.h"
#include "DateTime.h"

Ticker ticker; 
Serial pc(USBTX , USBRX);
DigitalOut dbug(PTE26);
I2C i2c(PTE25, PTE24);
RtcDs1307 gRtc ( i2c );


blaine::Properties *properties = new blaine::Properties();
blaine::glcd *glcdptr = new blaine::glcd();
blaine::Analise *analise = new blaine::Analise();
//blaine::Printer *printer = new blaine::Printer();

using namespace std;
using namespace blaine;

enum {
	RESET = 0,
	RESETHOLD,
	
	INITBANNER,
	BANNER,
	INITCREDITS,
	CREDITS,
	INITMAINCANVAS,
	MAINCANVAS,
	
	ASKLOGIN,
	WAITLOGIN,
	ASKLOGOUT,
	WAITLOGOUT,
	
	INITCALIBCANVAS,
	WAITCALIBCANVAS,
	WARNTEMPCALIB,
	WAITTEMPCALIB,
	
	CANVAS1_ANALISE_CMD,
	DOINGANALYSIS,
	
	DOVACAL,
	WAITVACAL,
	
	DOVCCAL,
	WAITVCCAL,
	
	CANVAS1_CONFIG_CMD,
	DOINGCONFIG,
	
	SERVICE_CMD,
	DOINGSERVICE,
	
	CANVAS1_SERVICE_CMD,
	CANVAS1_HELP_CMD,
	HELPCANVAS,
	
	UNHANDLED,	
};


int user;
uint8_t state;
uint8_t maintempkcode;
uint8_t temp;
bool coldstart = true;

unsigned long bspcounter;
unsigned long inittick;
unsigned long alarms[4];
unsigned long reload[4];
//unsigned long alarm;

// Log services
int loglevel;
char logbuffer[3500];
char *logbufferinit;
char *logbufferptr;

// Mail box services
std::queue<event_t> postoffice;
std::vector<int>subscriptions;

std::queue<event_t> mainevents;

// Debug ======================================================================================================================
void log (int level, const char* format, ...) {

	int printed;
	
	if (level <= loglevel){
		std::va_list arg;
		va_start(arg, format);
		printed = vsprintf(logbufferptr,format, arg);
		logbufferptr+= sizeof(char)*printed;
		va_end(arg);
	}
}

void setLogLevel(int level) {loglevel = level;}

void printLog(){
	
	char *logbuf = logbufferinit;
	
	if (logbufferptr > logbufferinit){
		do{
			pc.putc(*logbuf);
		}while (logbuf++ < logbufferptr);
		logbufferptr=logbufferinit;
	}
	
}

// Tick and alarm ============================================================================================================
void OSTick(){
    
	uint8_t i;
	
	dbug = !dbug;
    
	bspcounter++;
	for (i = 0; i < 4; i++) {
		if (alarms[i] < bspcounter){
			postoffice.push(event_t (ALARM_SIG, i, NULL));
			if (reload[i] !=0){
				alarms[i] = bspcounter + reload[i];
			}
			else{
				alarms[i] = 0xffffffff;
			}
		}
	}

	glcdptr->GLCDRefresh();
	glcdptr->SBarRefresh();
	glcdptr->getScanCode();
	//printer->PrinterISR();

}

long getTick() { return bspcounter * 16;}

void setInitTick() {inittick = bspcounter;}

long getRelativeTick() { return (bspcounter - inittick) * 16;}

double getSeconds() {return ((bspcounter - inittick)*16) / 1000;}

bool isAlarmTime(uint8_t index) {return bspcounter > alarms[index];}

void setAlarm (uint8_t index, unsigned long interval, unsigned long rld) { 
	alarms[index] =  bspcounter+interval;
	reload[index] = rld;
}

void resetAlarm(uint8_t index) {
	alarms[index] = 0xffffffff;
	reload[index] = 0L;
}

void clearAlarms(){	
	for (int i = 0; i < 4; i++) { 
		alarms[i]=0xffffffff;;
		reload[i]=0L;
	}
}



// PostOffice =================================================================================================================

void publish (event_t evt){
	postoffice.push(evt);
}


void verifymail(){
	
	int signal, subs, addr;
	
	while (!postoffice.empty()){
		event_t evt = postoffice.front();
		signal = evt.evtype;
		//log(FINER, "Postoffice recebeu evt : %d com payload = %d\r\n", signal, evt.simple);
		for (unsigned i=0; i<subscriptions.size(); ++i){
			subs = subscriptions[i] & 0x0000ffff;
			addr = subscriptions[i] & 0xffff0000;
			if (subs == signal){
				if (addr == MAIN){
					mainevents.push(evt);
					log(FINER, "Postoffice transferiu evento %d para main\r\n", evt.evtype);
				}
				if (addr == GLCD){
					glcdptr->pushEvent(evt);
					log(FINER, "Postoffice transferiu evento %d para GLCD\r\n", evt.evtype);
				}
				if (addr == PROPERTIES){
					properties->pushEvent(evt);
					log(FINER, "Postoffice transferiu evento %d para PROPERTIES\r\n", evt.evtype);
				}
				if (addr == ANALISE){
					analise->pushEvent(evt);
					log(FINER, "Postoffice transferiu evento %d para ANALISE\r\n", evt.evtype);
				}
				if (addr == PRINTER){
					//printer->pushEvent(evt);
					log(FINER, "Postoffice transferiu evento %d para Printer\r\n", evt.evtype);
				}
			}
		}	
		postoffice.pop();
	}
}

void subscribe (int object, int signal){
	subscriptions.push_back(object | signal);
	log(FINER, "Subscribing %d on %d - list now with %d items\r\n", signal, object, subscriptions.size());
}

void unsubscribe (int object, int signal){

	int index = object+signal;
	
	for (unsigned i=0; i<subscriptions.size(); ++i){
		if (subscriptions[i] == index){
			subscriptions.erase(subscriptions.begin()+i);
			log(FINER, "UNsubscribing %d on %d - list now with %d items\r\n", signal, object, subscriptions.size());
		}
	}	
}

bool locateEvent (int event_type){

	while(!mainevents.empty()) {
		if (mainevents.front().evtype == event_type){
			//event_temp = mainevents.front();
			log(FINER, "Event type %d was localized\r\n", mainevents.front().evtype);
			return true;
		}
		//delete mainevents.front();
		mainevents.pop();
	}
	return false;
}

uint8_t verifyKeyCode(uint8_t code){
	
	uint8_t kk=0;
	
	if (!locateEvent(KEYCODE_SIG)) return 0;
	log(INFO, "Keycode event =%d was localized\r\n", mainevents.front().simple);
	
	if ((code == 0) | (mainevents.front().simple == code)){
		kk = mainevents.front().simple;
	}
	
	//delete mainevents.front();
	mainevents.pop();
	maintempkcode = kk;
	return kk;
}

uint8_t getKeyCode(){
	
	uint8_t kk=0;
	
	if (!locateEvent(KEYCODE_SIG)) return 0;
	log(INFO, "Keycode event =%d @getKeycode\r\n", mainevents.front().simple);
	
	kk = mainevents.front().simple;
	
	//delete mainevents.front();
	mainevents.pop();
	maintempkcode = kk;
	return kk;
}


void simpleEventRequest (int signal, uint8_t simple){
	event_t evt(signal, simple, NULL);
	postoffice.push(evt);
}

void drawShapeRequest (uint8_t c_shape, uint8_t c_x, uint8_t c_y, bool c_savebkg){
	
	drawshape_evt * dse = new drawshape_evt(c_shape, c_x, c_y, c_savebkg);
	event_t evt(DRAWSHAPE_SIG, 0, dse);
	//log(FINE, "Posting Drawshape : %d, %d, %d, %d\r\n", dse->shape, dse->x, dse->y, dse->savebkg);
	postoffice.push(evt);	
}

void drawCanvasRequest(uint8_t c_bkg, uint8_t c_slots, 
					uint8_t c_rollback, 
                    uint8_t sig0, uint8_t sig1, uint8_t sig2, uint8_t sig3, uint8_t sig4, uint8_t sig5){
	
	canvas_evt *canvas = new canvas_evt(c_bkg, c_slots, c_rollback, 
                    sig0, sig1, sig2, sig3, sig4, sig5);
	event_t evt(DRAWCANVAS_SIG, 0, canvas);
	postoffice.push(evt);		
}



void drawDialogRequest (uint8_t type, uint8_t c_yesok, uint8_t c_no, uint8_t c_cancel, const char * cmsg ) {
	
	std::string *s0 = new std::string(cmsg);
	
	dialog_evt * dlg = new dialog_evt(type, c_yesok, c_no, c_cancel, s0);
	event_t evt(DRAWDIALOG_SIG, 0, dlg);
	postoffice.push(evt);	
}

void drawInputRequest(uint8_t c_type, bool c_numeric, bool c_insert, bool c_uppercase, bool c_savehistory,
							const char * c_line, const char * c_data, const char * c_form, const char * c_hist, 
						    double c_val, double c_llimit, double c_hlimit){
	
	std::string *s0 = new std::string(c_line);
	std::string *s1 = new std::string(c_data);
	std::string *s2 = new std::string(c_form);
	std::string *s3 = new std::string(c_hist);
	
	input_evt *input = new input_evt(c_type, c_numeric, c_insert, c_uppercase, c_savehistory, 
										s0, s1, s2, s3, c_val, c_llimit, c_hlimit);
	event_t evt(DRAWINPUT_SIG, 0, input);
	postoffice.push(evt);		
}

void drawChoiceEvent (char * c_prompt, std::vector<std::string> * c_items){
	
	std::string *s0 = new std::string(c_prompt);
	
	choice_evt *choice = new choice_evt(s0, c_items);
	event_t evt(DRAWCHOICE_SIG, 0, choice);
	postoffice.push(evt);		
}

//
//std::vector<string> * dummyChoice(){
//	
//	std::vector<string> * ch_items = new std::vector<string>();
//	ch_items->push_back("Item 0");
//	ch_items->push_back("Item 1");
//	ch_items->push_back("Item 2");
//	ch_items->push_back("Item 3");
//	ch_items->push_back("Item 4");
//	
//	return ch_items;	
//}


void printDT(char *pre, DateTime &dt)
{
    pc.printf("%s - %u/%u/%02u %2u:%02u:%02u\r\n"
                ,pre
                ,dt.month(),dt.day(),dt.year()
                ,dt.hour(),dt.minute(),dt.second()
                );
}
 
bool rtcUpdate(RtcDs1307 &rtc, int32_t bias){ // this must be signed{

	bool bUpdated = false;
 
    // Use the compiled date/time as a basis for setting the clock.
    // We assign it to a signed integer so that negative biases work correctly
    int64_t compiledTime = DateTime(__DATE__,__TIME__).unixtime();
 
    // This assumes that the program is run VERY soon after the initial compile.
    time_t localt = DateTime(compiledTime + bias).unixtime(); // offset by bias
    
    // If the stored static time stamp does not equal the compiled time stamp,
    // then we need to update the RTC clock and the stored time stamp
    if(*((time_t *)&rtc[0]) != localt)
    {
        // Update the RTC time as local time, not GMT/UTC
        rtc.adjust(localt);
        // Store the new compiled time statically in the object ram image
        *((time_t *)&rtc[0]) = localt;
        // Push the object ram image to the RTC ram image
        bUpdated = rtc.commit();
    }
    
    return bUpdated;
}

void rtcSetTime (time_t settime){
	gRtc.adjust(settime);
}




int main() {

	user = -1;
	state = RESET;
	coldstart= true;
	
	
	char sbuf[64];
	time_t prawtime;
	struct tm * ptimeinfo;
	
	
	clearAlarms();
		
	loglevel = FINE;
	logbufferinit = &logbuffer[0];
	logbufferptr = &logbuffer[0];

	pc.printf("\r\n\n\n===============================================================================\r\n");
	pc.printf("=            Controlador Antares - servicos para o AutoDensity 100\r\n");
	pc.printf("=            Versao Compilada em  %s %s\r\n",__DATE__,__TIME__);
	pc.printf        ("===============================================================================\r\n\n");
	
	ticker.attach(&OSTick, 1.0/BSP_TICKS_PER_SEC);
	
	
	//time_t tick = 0;
 
    //if(rtcUpdate(gRtc, -(5*60*60) )) pc.printf("Updated RTC to compiled time\r\n");  
    //DateTime timeFlashed(*((time_t *)&gRtc[0]));
    //printDT("last flashed on",timeFlashed);
    
	
	if (gRtc.isRunning()){
		DateTime dt = gRtc.now();	
		pc.printf("RTC Services are available\r\n");
		properties->setRawTime(dt.unixtime());
		prawtime = properties->getRawTime();
		ptimeinfo = localtime (&prawtime);
		strftime(sbuf, 64, "%d/%m/%y-%T", ptimeinfo);
		
		printDT("RTC registered time : ",dt);
		pc.printf("System Time = [ %u ] %s \r\n", ptimeinfo, &sbuf[0]);
		
		pc.printf("\r\n");
	}
	else{
		pc.printf("Modulo RTC nao foi localizado :-( - ajuste a data manualmente por favor\r\n");
	}
 
   
	subscribe(MAIN, KEYCODE_SIG);
	subscribe(MAIN, ALARM_SIG);
	
	subscribe(GLCD, DRAWSHAPE_SIG);
	subscribe(GLCD, GLCDRESET_SIG);
	subscribe(GLCD, DRAWMESSAGE_SIG);
	subscribe(GLCD, UPDATEICON_SIG);
	subscribe(GLCD, INITBARGRAPH_SIG);
	subscribe(GLCD, UPDATEBARGRAPH_SIG);
	
	subscribe(GLCD, DRAWCANVAS_SIG);
	subscribe(MAIN, CANVASANSWER_SIG);
	subscribe(GLCD, DRAWDIALOG_SIG);
	subscribe(MAIN, DIALOGANSWER_SIG);
	subscribe(GLCD, DRAWINPUT_SIG);
	subscribe(MAIN, INPUTANSWER_SIG);
	subscribe(GLCD, DRAWCHOICE_SIG);
	subscribe(MAIN, CHOICEANSWER_SIG);	
	
	subscribe(PROPERTIES, LOADHISTORY_SIG);
	//subscribe(MAIN, HISTORYANSWER_SIG);
	subscribe(GLCD, HISTORYANSWER_SIG);
	subscribe(PROPERTIES, SAVEHISTORY_SIG);
	subscribe(PROPERTIES, APPENDTOFILE_SIG);
	
	subscribe(PROPERTIES, LOADPARAMS_SIG);
	subscribe(PROPERTIES, LOADKEYS_SIG);
	subscribe(PROPERTIES, ACTIVATEPROPERTIES_SIG);
	subscribe(PROPERTIES, LISTPROPERTIES_SIG);
	
	subscribe(ANALISE, INITANALISE_SIG),
	subscribe(ANALISE, DOANALISE_SIG),
	subscribe(MAIN, ANALISEDONE_SIG);
	
	subscribe(ANALISE, DOMANUAL_SIG),
	subscribe(ANALISE, DOVACAL_SIG),
	subscribe(ANALISE, DOVCCAL_SIG),		
			
	subscribe(PROPERTIES, DOCONFIG_SIG),
	subscribe(MAIN, CONFIGDONE_SIG);	
	
	subscribe(PRINTER,PRINTSTRING_SIG);
        subscribe(PRINTER,FLUSHPRINTER_SIG);
        subscribe(PRINTER,RESETPRINTER_SIG);
	
	
	// Super loop :
    while (true) {
		
		//pc.printf("Looping\r\n");
		printLog();
		verifymail();
		
		glcdptr->serviceEvents();
		properties->serviceEvents();
		analise->serviceEvents();
		//printer->serviceEvents();	
	
			
		if (state ==RESET){	
			simpleEventRequest(GLCDRESET_SIG,0);
			drawShapeRequest(SHP_WELLCOME2, 0U, 0U, false);
			drawShapeRequest(SHP_STATUSBAR, 0U, 48U, false);
			
			simpleEventRequest(INITANALISE_SIG,0);
			simpleEventRequest(RESETPRINTER_SIG,0);			
			state=RESETHOLD;
		}
		
		else if (state == RESETHOLD){
			simpleEventRequest(ACTIVATEPROPERTIES_SIG,0);
			analise->setUser(-1);
			state=INITBANNER;
		}
		
		else if (state == INITBANNER){
			//log(INFO,"Main em init banner\n\r");	
			simpleEventRequest(GLCDRESET_SIG,0);
			drawShapeRequest(SHP_WELLCOME1, 0U, 0U, false);
			publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(76,0,10,false,false,true,0, new std::string("          21"))));
			drawShapeRequest(SHP_STATUSBAR, 0U, 48U, false);				
			state=BANNER;
		}
		else if (state == BANNER){
			temp = getKeyCode(); 
			if ( temp != 0){
				//log(FINE, "Code %d was pressed in banner\r\n", temp );
				if (coldstart){
					coldstart = false;
					analise->setUser(-1);
				}
				else{
					if (temp == 13){
						if (analise->getUser() == -1){
							state=ASKLOGIN;
						}
						else{
							state=INITMAINCANVAS;
						}
					}
					else if (temp == 0x08){
						//log(FINE, "Logout requested\r\n");
						state=ASKLOGOUT;
					}
				}
			}
			temp=0;
		}

		else if (state == ASKLOGIN){
			//log(PLUMB, "Asking if sample ready \r\n");
			publish(event_t(DRAWINPUT_SIG, 0, new input_evt( SHP_INPUT5,
				true,false,false,false,
				new std::string("C\x8B digo de\nAcesso"), 
				new std::string(""), 
				new std::string("%d"), 
				new std::string(""),
				0.0, 0.0, 10000.0)));
			state = WAITLOGIN;		
		}
		else if (state == WAITLOGIN){
			if (locateEvent(INPUTANSWER_SIG)){
				event_t *dummyevt = &mainevents.front();
				inputanswer_evt * ia = static_cast<inputanswer_evt*>(dummyevt->payload);
				//double inputval = ia->val;
				if (ia->val != analise->getAccessCode()){
					analise->setUser(0);	
				}
				else{
					analise->setUser(1);
				}
				log(FINE, "User logged with %f - user=%d\r\n", ia->val, analise->getUser());
				state = BANNER;	
			}			
		}
		
		else if (state == ASKLOGOUT){
			//log(PLUMB, "Asking if sample ready \r\n");
			publish(event_t(DRAWDIALOG_SIG, 0, new dialog_evt(1,
							255,
							BANNER, 
							BANNER, 
							new std::string("Deseja\nsair\ndo sistema ?")
							)));
			state = WAITLOGOUT;		
		}
		else if (state == WAITLOGOUT){
			if (locateEvent(DIALOGANSWER_SIG)){
				temp = mainevents.front().simple;
				if (temp == 255){
					log(FINE, "User logged out of system\r\n");
					analise->setUser(-1);
				}
				state = BANNER;	
			}			
		}
		
		else if (state == INITMAINCANVAS){
			if ( analise->getUser()==1){
				drawCanvasRequest(SHP_CANVAS2, 5, 
									INITBANNER,		
									CANVAS1_ANALISE_CMD,
									INITCALIBCANVAS,
									CANVAS1_CONFIG_CMD,
									SERVICE_CMD,
									CANVAS1_HELP_CMD,
									0);
				state=MAINCANVAS;
			}
			else if (analise->getUser() == 0){
				drawCanvasRequest(SHP_CANVAS_PP, 2, 
									INITBANNER,		
									CANVAS1_ANALISE_CMD,
									0,//INITCALIBCANVAS,
									0,
									0,
									0,
									0);
				state=MAINCANVAS;
			}
			else{
				state = BANNER;	
			}
		}

		else if (state == MAINCANVAS){	
			if (locateEvent(CANVASANSWER_SIG) == true){
				state = mainevents.front().simple;
				mainevents.pop();
			}
		}

		else if (state == CANVAS1_HELP_CMD){
			log(INFO, "Help as called\r\n");
			state=INITBANNER;
		}

		
		// Aanalisi services ===================================================================================
		else if (state == CANVAS1_ANALISE_CMD){
			event_t evt(DOANALISE_SIG, 0, NULL);
			postoffice.push(evt);
			transferPlumbig (MAIN, ANALISE);
			state=DOINGANALYSIS;
		}

		else if (state == DOINGANALYSIS){
			if (locateEvent(ANALISEDONE_SIG) == true){
				transferPlumbig (ANALISE, MAIN);
				state = INITMAINCANVAS;
				mainevents.pop();
			}
		}	

		// Service services ======================================================================================
		else if (state == SERVICE_CMD){
			event_t evt(DOMANUAL_SIG, 0, NULL);
			postoffice.push(evt);
			transferPlumbig (MAIN, ANALISE);		
			state=DOINGSERVICE;
		}

		else if (state == DOINGSERVICE){
			if (locateEvent(ANALISEDONE_SIG) == true){
				transferPlumbig (ANALISE, MAIN);
				state = INITMAINCANVAS;
				mainevents.pop();
			}
		}	
		
		// Calibration Services  ===========================================================================
		else if (state == INITCALIBCANVAS){
			drawCanvasRequest(SHP_PICNOCALIBCANVAS, 3, 
								INITBANNER,		
								WARNTEMPCALIB,
								DOVACAL,
								DOVCCAL,
								0,
								0,
								0);
			state=WAITCALIBCANVAS;
		}
		else if (state == WAITCALIBCANVAS){	
			if (locateEvent(CANVASANSWER_SIG) == true){
				state = mainevents.front().simple;
				mainevents.pop();
			}
		}
		
		else if (state == DOVACAL){
			event_t evt(DOVACAL_SIG, 0, NULL);
			postoffice.push(evt);
			transferPlumbig (MAIN, ANALISE);
			state=WAITVACAL;
		}	
		
		else if (state == WAITVACAL){
			if (locateEvent(ANALISEDONE_SIG) == true){
				transferPlumbig (ANALISE, MAIN);
				state = INITMAINCANVAS;
				mainevents.pop();
			}
		}
			
		else if (state == DOVCCAL){
			event_t evt(DOVCCAL_SIG, 0, NULL);
			postoffice.push(evt);
			transferPlumbig (MAIN, ANALISE);
			state=WAITVCCAL;
		}	
		
		else if (state == WAITVCCAL){
			if (locateEvent(ANALISEDONE_SIG) == true){
				transferPlumbig (ANALISE, MAIN);
				state = INITMAINCANVAS;
				mainevents.pop();
			}
		}
		
		else if (state == WARNTEMPCALIB){	
			publish(event_t(DRAWDIALOG_SIG, 0, new dialog_evt(0,
								INITBANNER,
								INITBANNER, 
								INITBANNER, 
								new std::string("A ser implementado\nap\x8b s defini\x81 \x83 o\ndo sensor")
								)));
			state = WAITTEMPCALIB;
		}
		else if (state == WAITTEMPCALIB){	
			if (locateEvent(DIALOGANSWER_SIG) == true){
				state = mainevents.front().simple;
				mainevents.pop();
			}
		}
		
		
		
		/// Config Services ===============================================================================================
		else if (state == CANVAS1_CONFIG_CMD){
			event_t evt(DOCONFIG_SIG, 0, NULL);
			postoffice.push(evt);
			
			unsubscribe(MAIN, KEYCODE_SIG);
			unsubscribe(MAIN, DIALOGANSWER_SIG);
			unsubscribe(MAIN, INPUTANSWER_SIG);
			unsubscribe(MAIN, CHOICEANSWER_SIG);
			unsubscribe(MAIN, ALARM_SIG);
			
			subscribe(PROPERTIES, KEYCODE_SIG);
			subscribe(PROPERTIES, DIALOGANSWER_SIG);
			subscribe(PROPERTIES, INPUTANSWER_SIG);
			subscribe(PROPERTIES, CHOICEANSWER_SIG);
			subscribe(PROPERTIES, ALARM_SIG);
			
			state=DOINGCONFIG;
		}
		
		else if (state == DOINGCONFIG){
			if (locateEvent(CONFIGDONE_SIG) == true){
				
				unsubscribe(PROPERTIES, KEYCODE_SIG);
				unsubscribe(PROPERTIES, DIALOGANSWER_SIG);
				unsubscribe(PROPERTIES, INPUTANSWER_SIG);
				unsubscribe(PROPERTIES, CHOICEANSWER_SIG);
				unsubscribe(PROPERTIES, ALARM_SIG);
				
				subscribe(MAIN, KEYCODE_SIG);	
				subscribe(MAIN, DIALOGANSWER_SIG);
				subscribe(MAIN, INPUTANSWER_SIG);
				subscribe(MAIN, CHOICEANSWER_SIG);
				subscribe(MAIN, ALARM_SIG);
				
				state = INITMAINCANVAS;
				mainevents.pop();
			}
		}
		
		
		
		else if (state == CANVAS1_SERVICE_CMD){
			log(INFO, "Analisis as called\r\n");
			state=INITBANNER;
		}	

		else if (state == UNHANDLED){

		}
		else { 
			log(WARNING, "Unhandled signal on Main = %d\n\r", state);
			state = UNHANDLED;
		}
	}
    
	return 0;
}

void transferPlumbig (int from, int to){
	
	unsubscribe(from, KEYCODE_SIG);
	unsubscribe(from, DIALOGANSWER_SIG);
	unsubscribe(from, INPUTANSWER_SIG);
	unsubscribe(from, CHOICEANSWER_SIG);
	unsubscribe(from, ALARM_SIG);

	subscribe(to, KEYCODE_SIG);
	subscribe(to, DIALOGANSWER_SIG);
	subscribe(to, INPUTANSWER_SIG);
	subscribe(to, CHOICEANSWER_SIG);
	subscribe(to, ALARM_SIG);
	
}














				
//				drawDialogRequest(1,
//									CANVAS1_ANALISE_CMD,
//									CANVAS1_CALIBRATE_CMD,
//									0,
//									"Teste do Dialog\nLinha2");
				
//				drawInputRequest(SHP_INPUTFULL,
//									false,false,false,true,
//									" Please enter sample ID",
//									"ACP Instruments", "", "opus.hst",
//									0.0, 0.0, 0.0);

//				drawChoiceEvent ((char*)"Choice example :", dummyChoice());



//				if (locateEvent(DIALOGANSWER_SIG) == true){
//					state = mainevents.front().simple;
//					mainevents.pop();
//				}
//				
//				if (locateEvent(INPUTANSWER_SIG) == true){
//					inputanswer_evt *aevt = static_cast<inputanswer_evt*>(mainevents.front().payload);
//					double dbl = aevt->val;
//					log(INFO, "Input Dialog answered : %f - (%s)\r\n", dbl, (char*)aevt->sval);
//					delete aevt;
//					state=INITBANNER;
//					
//					mainevents.pop();
//				}
				
//				if (locateEvent(CHOICEANSWER_SIG) == true){
//					log(INFO, "Choice Returned : %d (%s)\r\n", mainevents.front().simple, (char *)mainevents.front().payload);
//					state=INITBANNER;
//					mainevents.pop();
//				}
