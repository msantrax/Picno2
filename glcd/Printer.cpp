/* 
 * File:   Printer.cpp
 * Author: opus
 * 
 * Created on 14 de Dezembro de 2015, 15:26
 */

#include "../src/bsp.h"
#include <cstring>

#include "Printer.h"

//I2C i2c(PTE25, PTE24);

#define DR_ADDR 0x4F
#define DW_ADDR 0x4E
#define CR_ADDR 0x4D
#define CW_ADDR 0x4C

#define STROBEDELAY	1000
#define RESETDELAY	1000
#define OPDELAY	100

namespace blaine {

	
Printer::Printer() {
	
	ready = false;
	prnstate = STANDBY;
   
    pbufinit = &pbuf[0];
    pbufend = &pbuf[1024];
    pbufin = pbufinit;
    pbufout = pbufinit;
    bufbusy = false;
}

Printer::~Printer() {
}

Printer::Printer(const Printer& orig) {
}



void Printer::pushEvent(event_t evt){
	events.push(evt);
}



void Printer::serviceEvents(){
	
	while(!events.empty()){
		dummyevt = &events.front();
		if (dummyevt->evtype == PRINTSTRING_SIG){
			std::string * str = static_cast<std::string*>(dummyevt->payload);
			appendString(dummyevt->simple, (char*)str->c_str());
			delete(str);
		}
		if (dummyevt->evtype == FLUSHPRINTER_SIG){
			//test();
			bufbusy = true;
		}
		if (dummyevt->evtype == RESETPRINTER_SIG){
			prnstate = COLDSTART;
		}
		
		events.pop();
	}	
	

}

void Printer::test(){
	
//	appendString(0,"\r\n\n\n\n");
//	
//	appendString(Printer::EXPANDED, " ACP Instruments - 2016\n\r");
//	appendString(0, "          Polyperm 220 - V 2.4.0-RC\n\r");
//	appendString(Printer::RULLER,"");
//	appendString(Printer::BLANKLINE,"");
//	
//	bufbusy = true;
	
}



int Printer::appendString(uint8_t mod, char * mes){

	int counter=0;
	
	log (FINER, "Print Appending string %s\r\n", mes);
	
	if (bufbusy){
		return -1;
	}
	else{
		if (mod !=0){
			switch(mod){
				case EXPANDED:
					*pbufin++ = 0x1b;*pbufin++ = 0x57;*pbufin++ = 0x01;
					break;
				case RESET:
					*pbufin++ = 0x1b;*pbufin++ = 0x58;
					return 0;
					break;
				case COMPRESSED:
					*pbufin++ = 0x0F;//*pbufin++ = 0x57;*pbufin++ = 0x01;
					break;
				case EJECT:
					for (int i = 0; i < 8; i++) {
						*pbufin++ = '\n';		
					}
					*pbufin++ = '\r';
					return 0;
					break;
				case RULLER:
					for (int i = 0; i < 48; i++) {
						*pbufin++ = '-';		
					}
					*pbufin++ = '\r';
					*pbufin++ = '\n';
					return 0;
					break;
				case BLANKLINE:
					*pbufin++ = '\r';*pbufin++ = '\n';
					return 0;
					break;	
				case CMG_EX:
					//*pbufin++ = 0x1b;	*pbufin++ = 0x57;	*pbufin++ = 0x01;
					*pbufin++ = 'c';	*pbufin++ = 'm';
					*pbufin++ = 0x1b;	*pbufin++ = 0x53;	*pbufin++ = 0x02;
					*pbufin++ = '2';
					*pbufin++ = 0x1b;	*pbufin++ = 0x54;
					*pbufin++ = '.';*pbufin++ = 'g';
					*pbufin++ = 0x1b;	*pbufin++ = 0x53;	*pbufin++ = 0x02;
					*pbufin++ = '-';	*pbufin++ = '1';
					*pbufin++ = 0x1b;	*pbufin++ = 0x54;
					//*pbufin++ = 0x1b;	*pbufin++ = 0x57;	*pbufin++ = 0x02;
					return 0;
					break;	
					
				default:
					break;
			}
		}
		
		while (*mes != 0){
			*pbufin = *mes;
			mes++;
			counter++;
			if (pbufin < pbufend) {			
				pbufin++;
			}
			else{
				bufbusy = true;
				return counter;
			}
		}
		
		if (mod !=0){
			switch(mod){
				case EXPANDED:
					*pbufin++ = 0x1b;*pbufin++ = 0x57;*pbufin++ = 0x02;
					break;
				case COMPRESSED:
					*pbufin++ = 0x12;//*pbufin++ = 0x57;*pbufin++ = 0x01;
					break;
				default:
					break;
			}
		}
		
	}
	return 0;
}


void Printer::PrinterISR(){
	
	switch (prnstate){
		
		case STANDBY:
			
			break;
		
		case COLDSTART:
			
			pbufinit = &pbuf[0];
			pbufend = &pbuf[1024];
			pbufin = pbufinit;
			pbufout = pbufinit;
			bufbusy = false;
			
			writeData(0xff);
			writeCtrl(0xff);
			Reset();
			
			log(FINE, "Initializing printer services\n\r");
			log(FINE, "Busy=	%d\n\r", isBusy());
			
			prnstate = READY;
			break;
		
		case READY:
			if (bufbusy){
				prnstate = RUNNING;
				log(FINER, "Printer is running\r\n");
			}
			break;
	
		case RUNNING :
			if (pbufin > pbufout){
				//writeData(66);
				writeData(*pbufout);
				Strobe();
				pbufout++;
			}
			else if (pbufout != pbufinit){
				pbufin = pbufinit;
			    pbufout = pbufinit;
			    bufbusy = false;
				log(FINER, "Printer ended job ...\r\n");
				prnstate = READY;
			}		
			break;
			
		case UNHANDLED:
				
			break;

		default:
			log(WARNING,"Unhandled signal on Analise = %d\n\r", prnstate);
			prnstate = UNHANDLED;
			break;
		
	}
}


bool Printer::isBusy(){
	char foo[1];
	i2c.read(CR_ADDR, foo, 1);
	if ((foo[0] & 0x10) == 0){
		return false;
	}
	return true;
}

bool Printer::isACK(){
	char foo[1];
	i2c.read(CR_ADDR, foo, 1);
	if ((foo[0] & 0x02) == 0){
		return true;
	}
	return false;
}


void Printer::Strobe(){
	char foo[1];
	i2c.read(CR_ADDR, foo, 1);
	foo[0] = foo[0] & 0xFE;
	i2c.write(CW_ADDR, foo, 1);
	wait_us(STROBEDELAY);
	foo[0] = foo[0] | 0x01;
	i2c.write(CW_ADDR, foo, 1);
	
}

int Printer::readCtrl() {
    char foo[1];
    i2c.read(CR_ADDR, foo, 1);
    return foo[0];
}

void Printer::writeCtrl(int data) {
    char foo[1];
    foo[0] = data;
    i2c.write(CW_ADDR, foo, 1);
}

int Printer::readData() {
    char foo[1];
    i2c.read(DR_ADDR, foo, 1);
    return foo[0];
}

void Printer::writeData(int data) {
    char foo[1];
    foo[0] = data;
    i2c.write(DW_ADDR, foo, 1);
}

void Printer::Reset(){
	
	writeCtrl(0X8f);
	wait_us(RESETDELAY);
	writeCtrl(0XFF);	
}


bool Printer::isReady(){
    return ready;
}

void Printer::SetBufbusy(bool bufbusy) {
	this->bufbusy = bufbusy;
}

bool Printer::IsBufbusy() const {
	return bufbusy;
}

}