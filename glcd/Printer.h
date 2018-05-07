/* 
 * File:   Printer.h
 * Author: opus
 *
 * Created on 14 de Dezembro de 2015, 15:26
 */

#ifndef PRINTER_H
#define	PRINTER_H

#include "../src/bsp.h"

#define PBUFSIZE 1024;

namespace blaine {

class Printer {

public:

    enum{
        COLDSTART=0,
        STANDBY,
        READY,
        RUNNING,
       
        UNHANDLED, 
        
    } PRINTERSTATES;    
    
    enum{
       NONE=0,
       RESET,
       PREGAP,
       EJECT,
       RULLER,
       BLANKLINE,
       
       CMG_EX,
       CELCIUS,
       
       EXPANDED,
       COMPRESSED,
       ITALIC,
       UNDERLINE,
    }PMODS;
  
    Printer();
    Printer(const Printer& orig);
    virtual ~Printer();
    
    void PrinterISR();
    void pushEvent(event_t evt);
    void serviceEvents();
    
    void test();
    
private:

    int appendString(uint8_t mod, char * mes);
    bool isReady();
    void SetBufbusy(bool bufbusy);
    bool IsBufbusy() const;
    
    int readCtrl();
    void writeCtrl(int data);
    int readData();
    void writeData(int data);
    void Reset();
    void Strobe();
    bool isBusy();
    bool isACK();
  
    
    uint8_t a_state;
    std::queue<event_t> events;
    event_t *dummyevt;
    
    
    char pbuf[1024];
    char * pbufinit;
    char * pbufend;
    char * pbufin;
    char * pbufout;
    bool bufbusy;
  
    uint8_t prnstate;    
    bool ready;
};


}

#endif	/* PRINTER_H */

