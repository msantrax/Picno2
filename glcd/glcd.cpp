/* 
 * File:   glcd.cpp
 * Author: opus
 * 
 * Created on 21 de Novembro de 2015, 11:38
 */

#include "../src/bsp.h"

#include "glcd.h"
#include "fonts.h"
#include "shapes.h"
#include "keyboard.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <stack>

DigitalOut cs1(PTB9);
DigitalOut cs2(PTA1);
DigitalOut glcde(PTC4);
DigitalOut glcdrw(PTA2);
DigitalOut glcddi(PTB23);
DigitalOut glcdrst(PTC5);

BusInOut glcdpins(PTD0, PTD1, PTD2, PTD3, PTC0, PTC1, PTC2, PTC3);
BusIn kbdpins (PTC7, PTC8, PTC9, PTC10);

#define GDELAY 50

namespace blaine {

glcd::glcd()  {
 
    framepage = 0;
    
	service_state = SERVICE_NONE;
	
    //kbdlocked = false;
    kbdptrs[0] = &kbdtbl0[0];
	kbdptrs[1] = &kbdtbl1[0];
	kbdptrs[2] = &kbdtbl2[0];
	kbdptrs[3] = &kbdtbl3[0];
	kbdptrs[4] = &kbdtbl4[0];
	
    kbdscan= 0;
    kbdtimer= 0;
	kbdoffset = 0;
	kbdpressed = false;
	kbdnumeric = false;
	kbdinsert = false;
	publish_kbdcode=true;
	
    
	iconplaces[0] = 3;
	iconplaces[1] = 19;
	iconplaces[2] = 34;
	iconplaces[3] = 49;
	iconplaces[4] = 64;
	iconplaces[5] = 79;
	iconplaces[6] = 94;
	iconplaces[7] = 109;
	
	iconsctrl[0] = c_iconsctrl(ICACTIVE    , false,    SHP_ICON_CLEAR);
	iconsctrl[1] = c_iconsctrl(ICACTIVE    , false,    SHP_ICON_CLEAR);
	iconsctrl[2] = c_iconsctrl(ICACTIVE    , false,    SHP_ICON_CLEAR);
	iconsctrl[3] = c_iconsctrl(ICACTIVE    , false,    SHP_ICON_CLEAR);
	iconsctrl[4] = c_iconsctrl(ICACTIVE    , false,    SHP_ICON_CLEAR);
	iconsctrl[5] = c_iconsctrl(ICACTIVE    , false,    SHP_ICON_CLEAR);
	iconsctrl[6] = c_iconsctrl(ICACTIVE    , false,    SHP_ICON_SDCARD_OFF);
	iconsctrl[7] = c_iconsctrl(ICACTIVE    , false,    SHP_ICON_PRINTER_OFF);
	
	canvas_widget_map[0][0]=3;	canvas_widget_map[0][1]=41;	canvas_widget_map[0][2]=0; 
	canvas_widget_map[1][0]=42;	canvas_widget_map[1][1]=43;	canvas_widget_map[1][2]=0; 
	canvas_widget_map[2][0]=85;	canvas_widget_map[2][1]=43;	canvas_widget_map[2][2]=0; 
	canvas_widget_map[3][0]=3;	canvas_widget_map[3][1]=41;	canvas_widget_map[3][2]=3; 
	canvas_widget_map[4][0]=43;	canvas_widget_map[4][1]=43;	canvas_widget_map[4][2]=3; 
	canvas_widget_map[5][0]=85;	canvas_widget_map[5][1]=43;	canvas_widget_map[5][2]=3; 
	
	
    inptdesc = new inptdesc_t() ;
    choicedesc = new choicedesc_t();

    _initHardware();
    
}

glcd::~glcd() {
}

void glcd::setNumeric(bool numeric){ kbdnumeric = numeric;}

// Dispatch Thread =======================================================================

void glcd::pushEvent(event_t evt){
	events.push(evt);
}


void glcd::serviceEvents(){
	
	while(!events.empty()){
		dummyevt = &events.front();
		
		if (dummyevt->evtype == DRAWSHAPE_SIG){
			drawshape_evt *dse = static_cast<drawshape_evt*>(dummyevt->payload);
			//log(FINE, "Retriving Drawshape : %d, %d, %d, %d\r\n", dse->shape, dse->x, dse->y, dse->savebkg);
			if (dse->x == 255){
				restoreBackground(dse->shape);
			}
			else{
				_drawShape(dse->shape, dse->x, dse->y, dse->savebkg);
			}
			
			delete dse;
		}
		else if (dummyevt->evtype == DRAWMESSAGE_SIG){
			drawmessage_evt *dme = static_cast<drawmessage_evt*>(dummyevt->payload);
			putMessage(dme->x, dme->y, dme->width, dme->invert, dme->transp, dme->clear, dme->align, dme->msg);
			delete dme;
		}
		else if (dummyevt->evtype == INITBARGRAPH_SIG){
			bginit_evt *bgi = static_cast<bginit_evt*>(dummyevt->payload);
			drawBGraph(bgi);
			delete bgi;
		}
		else if (dummyevt->evtype == UPDATEBARGRAPH_SIG){
			//log(INFO, "Updating Bar Graph\r\n");
			bgupdate_evt *bgu = static_cast<bgupdate_evt*>(dummyevt->payload);
			updagteBGraph(bgu);
			delete bgu;
		}
		
		
		else if (dummyevt->evtype == GLCDRESET_SIG){
			_initHardware();
		}
		else if (dummyevt->evtype == UPDATEICON_SIG){
			if (dummyevt->simple == 0){
				icon_evt *icon = static_cast<icon_evt*>(dummyevt->payload);
				if(icon->shape == 0) icon->shape = iconsctrl[icon->slot].id;
				//log(INFO, "Updating Icon = %d, %d, %d \r\n", icon->slot, icon->action, icon->shape);
				setIcon(icon->slot, icon->action, icon->shape);
				delete icon;
			}
			else if (dummyevt->simple == 1){
				saveIconState();
			}
			else if (dummyevt->simple == 2){
				restoreIconState();
				updateIcons();
			}
			else if (dummyevt->simple == 3){
				clearLeftIcons();
			}
		}
		else if (dummyevt->evtype == DRAWCANVAS_SIG){
			canvas_evt *cevt = static_cast<canvas_evt*>(dummyevt->payload);
			drawCanvas(cevt);
			clearLeftIcons();
			service_state = SERVICE_CANVAS;
			publish_kbdcode = false;
			delete cevt;
		}
		else if (dummyevt->evtype == DRAWDIALOG_SIG){
			dialog_evt *dlg = static_cast<dialog_evt*>(dummyevt->payload);
			drawDlg(dlg);
			service_state = SERVICE_DIALOG;
			publish_kbdcode = false;
			while(!kbdevents.empty()) kbdevents.pop();
			delete dlg;
		}
		else if (dummyevt->evtype == DRAWINPUT_SIG){
			input_evt *input = static_cast<input_evt*>(dummyevt->payload);
			if (!input->hist->empty()){
				input_temp_evt = input;
				inptdesc->hist = input->hist;
				log(FINE, "GLCD is loading History : \r\n");				
				event_t evt(LOADHISTORY_SIG, 0, (char*)input->hist->c_str());
				publish(evt);				
			}
			else{
				inptdesc->hashistory = false;
				inptdesc->savehistory = false;
				drawInput(input);
				service_state = SERVICE_INPUT;
				publish_kbdcode = false;
				delete input;
			}
		}
		else if (dummyevt->evtype == HISTORYANSWER_SIG){
			if (dummyevt->simple != 0){		
				inptdesc->histitems = (std::vector<string> *)dummyevt->payload;
				log(FINE, "Properties has answered with %d items to glcd\r\n", inptdesc->histitems->size());	
				inptdesc->hashistory = true;
				inptdesc->savehistory=input_temp_evt->savehistory;
			}
			else{
				inptdesc->hashistory = false;
				inptdesc->savehistory = false;
			}
			drawInput(input_temp_evt);
			service_state = SERVICE_INPUT;
			publish_kbdcode = false;
			//delete dummyevt->payload;
			delete input_temp_evt;
		}
		else if (dummyevt->evtype == DRAWCHOICE_SIG){
			choice_evt *choice = static_cast<choice_evt*>(dummyevt->payload);
			drawChoice(choice);
			service_state = SERVICE_CHOICE;
			publish_kbdcode = false;
			delete choice;
		}

		events.pop();
	}
	
	
	switch (service_state){
		
		case SERVICE_NONE:
			
			break;
			
		case SERVICE_CANVAS:
			if (!kbdevents.empty()){
				service_status = navigateCanvas(kbdevents.front());
				kbdevents.pop();
				if (service_status !=0){				
					event_t evt(CANVASANSWER_SIG, service_status, NULL);
					publish(evt);
					publish_kbdcode = true;
					service_state = SERVICE_NONE;
				}
			}
			break;
			
		case SERVICE_DIALOG :
			if (!kbdevents.empty()){
				service_status = navigateDlg(kbdevents.front());
				kbdevents.pop();
				if (service_status !=0){
					restoreBackground(SHP_DLG_INFO_TOP);
					if (dlg_type == 0){
						restoreBackground(SHP_DLG_BT_OK_SET);
					}
					else{
						restoreBackground(SHP_DLG_BT_YES);
					}
					event_t evt(DIALOGANSWER_SIG, service_status, NULL);
					publish(evt);
					publish_kbdcode = true;
					service_state = SERVICE_NONE;
				}
			}
			break;
		
		case SERVICE_INPUT:
			if (!kbdevents.empty()){
				if (inptdesc->error == 1){
					service_status = navigateDlg(kbdevents.front());
					kbdevents.pop();
					if (service_status == 1){
						restoreBackground(SHP_DLG_INFO_TOP);
						restoreBackground(SHP_DLG_BT_OK_SET);
						inptdesc->error=0;
					}	
					else if (service_status == 3){
						restoreBackground(SHP_DLG_INFO_TOP);	
						restoreBackground(SHP_DLG_BT_YES);
						inptdesc->error=0;
					}	
					else if (service_status == 2){
						restoreBackground(SHP_DLG_INFO_TOP);	
						restoreBackground(SHP_DLG_BT_YES);
						clearInput();
						if (inptdesc->hashistory) delete inptdesc->histitems;
						inputanswer_evt *ans = new inputanswer_evt(inptdesc->result, &inptdesc->data[0]);
						event_t evt(INPUTANSWER_SIG, service_status, ans);
						publish(evt);
						publish_kbdcode = true;
						service_state = SERVICE_NONE;
					}
					
				}
				else if (inptdesc->error == 2){
					service_status = navigateChoice(kbdevents.front());
					kbdevents.pop();
					if (service_status !=0){
						clearChoice();
						std::strncpy(&inptdesc->data[0], getLastChoice(), 18);
						inptdesc->data[18]=' ';
						inptdesc->data[19]='\0';
						inptdesc->cursor=255;
						_updateInput(true);
						inptdesc->functionbar = false;
						_resetInputIcons();
						inptdesc->error=0;
					}
				}
				else{
					service_status = navigateInput(kbdevents.front());
					kbdevents.pop();
					if (service_status == 1){
						clearInput();
						if(inptdesc->savehistory){
							std::queue<string> * histq = formatHistory();
							if (!histq->empty()){
								event_t evt(SAVEHISTORY_SIG, 0, histq);
								publish(evt);
							}
							else{
								log(FINE, "Item %s already on history\n\r", inptdesc->data);
								delete histq;
							}
							if (inptdesc->hashistory) delete inptdesc->histitems;
						}
						inputanswer_evt *ans = new inputanswer_evt(inptdesc->result, &inptdesc->data[0]);
						event_t evt(INPUTANSWER_SIG, service_status, ans);
						publish(evt);
						publish_kbdcode = true;
						service_state = SERVICE_NONE;					
					}
					else if (service_status == 2){
						std::string *s0 = new std::string(" Lamento mas\no valor digitado \n n\x83 o e um numero");
						dialog_evt *dlg = new dialog_evt(0,1,0,0, s0);
						drawDlg(dlg);
						inptdesc->error = 1;
						delete dlg;
					}
					else if (service_status == 3){
						std::string *s0 = new std::string("  Oops ! O valor\nesta fora de faixa\n  Continuar ?");
						dialog_evt *dlg = new dialog_evt(1,2,3,0, s0);
						drawDlg(dlg);
						inptdesc->error = 1;
						delete dlg;
					}
					else if (service_status == 4){
						choice_evt *choice = new choice_evt(new std::string("Historia armazenada"), inptdesc->histitems);
						drawChoice(choice);
						inptdesc->error = 2;
					}
				}
			}
			break;
			
			case SERVICE_CHOICE :
			if (!kbdevents.empty()){
				service_status = navigateChoice(kbdevents.front());
				kbdevents.pop();
				if (service_status == 1){
					event_t evt(CHOICEANSWER_SIG, choicedesc->cursor+choicedesc->offset, getLastChoice());
					publish(evt);					
					clearChoice();
					publish_kbdcode = true;
					service_state = SERVICE_NONE;
				}
				else if (service_status == 2 ) {
					event_t evt(CHOICEANSWER_SIG, 255 , NULL);
					publish(evt);					
					clearChoice();
					publish_kbdcode = true;
					service_state = SERVICE_NONE;
				}
			}
			break;
			
		default:
			log(WARNING, "Unhandled signal ON glcd = %d\n\r", service_state);
			service_state = SERVICE_NONE;
			break;		
	}
}


std::queue<string> * glcd::formatHistory(){
	
	std::queue<string> * stemp = new queue<string>;
	string * dta= new string(inptdesc->data);
	string scmp;
	uint8_t count;
	
	for (std::vector<string>::iterator it = inptdesc->histitems->begin() ; it != inptdesc->histitems->end(); ++it){
		scmp = *it;
		if (scmp.compare(*dta) == 0) return stemp;
	}
	
	count = 0;
	stemp->push(*inptdesc->hist);
	stemp->push(*dta);
	for (std::vector<string>::iterator it = inptdesc->histitems->begin() ; it != inptdesc->histitems->end(); ++it){
		stemp->push(*it);
		count++;
		if (count > 10) break;
	}
	return stemp;
}


// Graph refresh ISR =============================================================================
void glcd::GLCDRefresh(){

	uint8_t i;
    uint8_t updatecount = 0;
	
	updated = false;
	// If somebody is not writing at the framebuffer...
	if (!writing_semaphore){
		//dspl_dispatch_busy = true;
		// Roll pages at the buffer
		for (i=0 ; i<8 ; i++){
			// framepage rolls its last 3 bits, so it will scan 8 slots.
			switch (framepages[framepage]){
				case GCLEAR:
					// page is clear - nothing to do with the display
					// just verify if all is updated and adjust the semaphore;
					if (updatecount++ >= 7) updated = true;
					break;
				case DIRTYALL: // case FINE:
					//putch('.');
					_loadPage (false, true);
					_loadPage (false, false);
					i=8;
					framepages[framepage]=GCLEAR;
					break;
				default:
					i=8;
					break;
			}
			framepage++;
			framepage = framepage & 0x07;
		}
	}
	
}


uint8_t glcd::getScanCode(){
 
	kbdcode = 0;

	glcdmutex = true;

	//serial_drv->putc('*');

	// Now prepare lines to the next scan
	if (kbdscan & 0x20 || kbdscan == 0){
		// %th column - reset to first
		kbdscan=1;
	}
	else{
		// Activate proper column
		kbdscan=kbdscan<<1;
	}

	//serial_drv->putc(kbdscan && 0x8f);


	cs1 =0; cs2 = 0;
	glcdpins.output();
	glcdpins = kbdscan;
	wait_us(10);
	kbdthiscode = kbdpins;
	glcdpins = 0x00;

	// user pressed ?
	if (kbdthiscode !=0){
		//log(INFO, "KCode=%d\n\r", kbdthiscode );
		// add the scan lines to compose the vector
		kbdthiscode = ( (kbdthiscode << 4) | (kbdscan & 0x0F));        
		if(kbdlastcode == 0){
			// We are at init cycle
			kbdlastcode = kbdthiscode;
			//PDEBUG1("Kb = %d\n\r", kbdlastcode);
			if (!kbdnumeric || (kbdlastcode == 40)) kbdtimer = 4;				
			kbdpressed = true;
			kbdoffset = (kbdnumeric) ? 0:1;
		}
		else if (kbdthiscode != kbdlastcode){
			// Key has changed or is noise
			kbdlastcode =0;
			kbdtimer = 0;
			kbdpressed = false;
		}
		else if (!kbdpressed) {
			if (kbdoffset >= 4){
				kbdoffset = (kbdnumeric) ? 0:1;
			}
			else{
				//serial_drv->putc('!');
				kbdoffset++;
			}               
			kbdtimer=4;
			kbdpressed = true;
		}            
	}
	else{
		// Key was released ? = no press but code stored (disregard other scans)  
		if((kbdlastcode & 0x0F) == kbdscan){
			if (kbdtimer >0) {
				kbdtimer --;
				kbdpressed = false;
				//serial_drv->putc('0');
			}
			else{
				//serial_drv->putc('>');
				//kbdthiscode = ( kbdthiscode << 4 | kbdscan & 0x0F); 
				kbdtblptr = (uint8_t*)(kbdptrs[kbdoffset] + (kbdlastcode - 17)); 
				//kbdtblptr = (uint8_t*)(kbdtblinit + ((kbdthiscode | (kbdscan & 0x0f)) - 17)); 
				kbdcode = *kbdtblptr;
				if (!kbduppercase && (kbdcode > 64 && kbdcode < 91)) kbdcode += 32;
				kbdlastcode = 0;
				kbdoffset = (kbdnumeric) ? 0:1;
				kbdpressed = false;
			}
		}
	}            
		
	if (kbdcode !=0){
	    if (publish_kbdcode == true){
			
			event_t kbdevent(KEYCODE_SIG , kbdcode, NULL);
			publish(kbdevent);
		}
		else{
			kbdevents.push(kbdcode);
		}
	}
	
	glcdmutex = false;
    return kbdcode; 
}

// Icon Status Bar Services =========================================================================

void glcd::SBarRefresh(){
    
    uint8_t i, j;
 
    if (timetolast ==0){
        for (i=0 ; i<8 ; i++){
            if (iconsctrl[i].status == ICACTIVE){   }
            else if (iconsctrl[i].status == ICREFRESH){   
                //printf ("\n\r\n\rRequesting drawshape with REFRESH on icon %u", i);               
                _drawShape (iconsctrl[i].id, iconplaces[i], 48, false);
                iconsctrl[i].status = ICACTIVE;
            }
            else if (iconsctrl[i].status == ICBLINK){
                //printf ("\n\r\n\rRequesting drawshape with clear on icon %u", i);
                _drawShape (SHP_ICON_CLEAR, iconplaces[i], 48, false);
                iconsctrl[i].status = ICBLINKED;
            }
            else if (iconsctrl[i].status == ICBLINKED){
                //printf ("\n\r\n\rRequesting drawshare with icon on icon %u", i);
                _drawShape (iconsctrl[i].id, iconplaces[i], 48, false);
                iconsctrl[i].status = ICBLINK;
            }
            else if (iconsctrl[i].status == ICINVERT){
                //char * strptr = &iconsctrl[i].id;
				j = iconsctrl[i].id;
				iconsctrl[i].id = (j & 0x01) ? --j : ++j;
//				PDEBUG2 ("ICON %d WAS %d\n\r", i, j);
//				if ( (j & 0x01) == 1){
//					iconsctrl[i].id = --j;
//				}
//				else{
//					iconsctrl[i].id = ++j;
//				}
//				PDEBUG2 ("Now %d WAS SET TO %d\n\r", i, iconsctrl[i].id);
                _drawShape (iconsctrl[i].id, iconplaces[i], 48, false);
                iconsctrl[i].status = ICACTIVE;
            }
            else if (iconsctrl[i].status == ICICLEAR){
                //printf ("\n\r\n\rRequesting drawshape with clear on icon %u", i);
                _drawShape (SHP_ICON_CLEAR, iconplaces[i], 48, false);
                iconsctrl[i].status = ICACTIVE;
            }
        }    
    }
    else if (timetolast > 1){
        timetolast--;
        return;
    }
    else if (timetolast == 1){
        for (i=0 ; i<8 ; i++){
            if (iconsctrl[i].status == ICACTIVE){
                iconsctrl[i].status = ICREFRESH; 
            }
        }
        timetolast=0;
    }
	
}

void glcd::saveIconState(){
	
	for (int i = 0; i < 8; i++) {
		iconsave[i].id = iconsctrl[i].id;
		iconsave[i].on = iconsctrl[i].on;
		iconsave[i].status = iconsctrl[i].status;
	}
}

void glcd::restoreIconState(){
	
	for (int i = 0; i < 8; i++) {
		iconsctrl[i].id = iconsave[i].id;
		iconsctrl[i].on = iconsave[i].on;
		iconsctrl[i].status = iconsave[i].status;
	}
}

void glcd::updateIcons(){
	
	for (int i = 0; i < 8; i++) {
		iconsctrl[i].status = glcd::ICREFRESH;
	}	
}


void glcd::clearLeftIcons (){
	
	setIcon (0, glcd::ICREFRESH, SHP_ICON_CLEAR);
	setIcon (1, glcd::ICREFRESH, SHP_ICON_CLEAR);
	setIcon (2, glcd::ICREFRESH, SHP_ICON_CLEAR);
	setIcon (3, glcd::ICREFRESH, SHP_ICON_CLEAR);
	setIcon (4, glcd::ICREFRESH, SHP_ICON_CLEAR);
	setIcon (5, glcd::ICREFRESH, SHP_ICON_CLEAR);
}


void glcd::setIcon (uint8_t slot, uint8_t status, uint8_t shape){
    
    iconsctrl[slot].status = status;
    if (shape !=0) iconsctrl[slot].id = shape;
    timetolast = 0;	
}

void glcd::setStatus (uint8_t showtime, string * msg){
    
    uint8_t i,j,d,data; 
    uint8_t buffer[132];
    uint8_t * endbuffer = &buffer[122];
    uint8_t * droptr = &buffer[0];
    uint8_t * laststrip = &buffer[0];
    uint8_t * btto = framebufferptr + 899;
    uint8_t * upto = btto - 128;
  
    _drawShape (SHP_STATUSBARCLEAN, 0, 48, false);

    std::string::iterator p;
    for (p = msg->begin( ); p != msg->end( ) ; ++p){
        data = *p;
        if (droptr == endbuffer) break;
        for(i=0;i<7;i++){
            d=Font[data-32][i];
            if(d!=0x55){
                *droptr = ~d;
                droptr++;               
                if (droptr == endbuffer) break;
            }
        }
        *droptr = 0x00;
        droptr++;
    }
    
    delete msg;
 
    laststrip = droptr;
    droptr=&buffer[0];
    
    while(droptr < laststrip){
        j= (*droptr>>3) | 0x80;
        i= (*droptr<<5) | 0x04;
        *btto = j;
        *upto = i;
        droptr++;
        btto++;
        upto++;
    }
    
    framepages[6]=DIRTYALL;
    framepages[7]=DIRTYALL;
    
    timetolast = showtime; 
}


// GLCD General Services ==================================================================

void glcd::invertArea (uint8_t x , uint8_t page,  uint8_t heigth, uint8_t lenght){
    
    uint8_t *wptr;
    uint8_t data;
	uint8_t fill = 0x01;
	
	if (page > 64){
		page -= 64;
		fill = 0x00;
	}
	
	if (page > 128){
		page -= 128;
		fill = 0xff;
	}
	
    #ifdef GLCDDEBUG
        PDEBUG3 ("\r\ninvertArea called with : x=%d, page=%d, length=%d", x, page, lenght);  
    #endif
    
    do{
        wptr = framebufferptr + (page*128) + x;
        for (uint8_t i=0; i<lenght ; i++){
			if (fill == 0x01){
				data = *wptr;
				*wptr= ~data;
			}
			else{
				*wptr= fill;
			}
            wptr++;
        }
        framepages[page]=DIRTYALL;
        page++;
        heigth--;    
    } while (heigth !=0);
	
}


void glcd::putMessage(uint8_t x, uint8_t y, uint8_t width, bool inv, bool transp, bool clear, uint8_t align, string *msg){
	
	
	uint8_t newx;
	uint8_t wdt = 2;
	char * mes = (char*)msg->c_str();
	
	if (width !=0){
		wdt = calculateStrips(mes);
		if (clear){
			XPos=x;
			YPos=y;
			currentpositionptr = framebufferptr + ((YPos>>3)<<7) + XPos;
			for (int i = 0; i < width; i++) {				
				if (inv){
					*currentpositionptr = 0xff;
				}
				else{
					*currentpositionptr = 0x00;
				}
				_moveRight();
			}
		}
	}
	
	if (align == 1){
		newx = (x + ((width/2)) - ((wdt/2)+8));
		_putMessage(newx, y, inv, transp, mes);
	}
	else if (align == 2){
		newx = (x + width) - wdt;
		_putMessage(newx, y, inv, transp, mes);
	}
	else{
		_putMessage(x, y, inv, transp, mes);
	}
	
	delete(msg);
	
}


void glcd::_putMessage(uint8_t x, uint8_t y, bool inv, bool transp, char *Message){
 
    writing_semaphore = true;
    
    XPos=x;
    YPos=y;
    currentpositionptr = framebufferptr + ((YPos>>3)<<7) + XPos;
    while(*Message!=0){
        if (_putChar(*Message++, inv, transp)) Message++;
    } 
	
    writing_semaphore=false;
    
}	

bool glcd::_putChar(unsigned char data, bool inv, bool transp){

    uint8_t i,d;
	bool cursor = false;
	bool skip = false;
	
	//if (data == 0) return;
	if ((data > 127) && (data < 160)){
		skip = true;
	}
	else if (data > 159){
		cursor = true;
		data = data & 0x7f;
	}
	
	
    // Control codes
    if(data<32){
        switch(data){
            // Is carriage return ?
            case 13:
                _setPosition(0,YPos);
                break;
            // Is new line ?
            case 10:
                _setPosition(0,YPos+8);
                break;
        }
    }
    else{
		// Normal Char
		//log(INFO, "C:%d[%c]-", data, data);
        for(i=0;i<7;i++){
            d=Font[data-32][i];
            if(d!=0x55){
                if (cursor) d = d & 0x7f;
				if ((!inv) && (background == 0x00)) d = ~d;
                if (transp) d = d | *currentpositionptr;
				
                *currentpositionptr = d;
                _moveRight();
            }
        }
        d=background;
        if (inv) d = ~d;
        if (!transp) *currentpositionptr = d;
        _moveRight();
    }
    framepages[YPos>>3]=DIRTYALL;
	
	return skip;
}

uint8_t glcd::locateSlice(uint8_t id){
	
	for (uint8_t i=0; i < gslices.size(); ++i){
		if (gslices[i].id == id){
			return i;
		}
	}	
	return 255;
}

void glcd::restoreBackground (uint16_t id){
 
	uint8_t index;
    char * load;
    uint8_t w;
    int width, page;

	writing_semaphore = true;
 
    do{
		index = locateSlice(id);
		if (index != 255){
			page = gslices[index].page;
			load = gslices[index].payload;
			width = gslices[index].width;
			
			_setPosition(gslices[index].x, page<<3);
			
			for (w=0; w<=width; w++){
                *currentpositionptr = *load;
                load++;
                currentpositionptr++;
            }
            framepages[page]=DIRTYALL;
			free(gslices[index].payload);
			gslices.erase(gslices.begin()+index);
		}
    } while(index != 255);
	
    writing_semaphore = false;	
}


void glcd::_drawShape (uint16_t id, uint8_t x, uint8_t y, bool savebkg){

	log(FINER, "Drawing shape %d\r\n", id);
	
    uint8_t i,j,w; 
    uint8_t idx = 0;
    uint8_t pages, shp_slots, width;
    char * temptr1;  
  
    for (j=0; j<SHAPESNUM; j++){
        if (shapes_lkt[j].id == id){
            idx = j;
            break;
        }
    }
    
    if (j>=SHAPESNUM){        
        log(WARNING, "Inconsistent shape index -> %D", id);
        return;
    }
    
    writing_semaphore = true;
    
    pages = shapes_lkt[idx].pages;
    shp_slots = shapes_lkt[idx].shp_slots;
    width = shapes_lkt[idx].witdth;
 
    for (i=0; i<pages; i++){
        _setPosition(x,y);
        
		if (savebkg) {            
			char * slice = (char*)malloc(sizeof(char)*(width+1));
			screen_slice s_slice(id, x, y>>3, shp_slots, width, slice);
			gslices.push_back(s_slice);
			temptr1 = slice;
            for (w=0; w<=width; w++){
                *temptr1 = *currentpositionptr;
                temptr1++;
                currentpositionptr++;
            }
        }
	
        _setPosition(x,y);
        _drawShapeFragment((uint8_t *)shapes_lkt[idx].shape,
                           (uint8_t *)shapes_lkt[idx].mask,
                           shapes_lkt[idx].shp_slots,
                           i*(shp_slots*8),
                           shapes_lkt[idx].usemask); // & savebkg);
        framepages[y>>3]=DIRTYALL;
        y+=8;
    }    
    writing_semaphore = false;
	
}

void glcd::_drawShapeFragment(uint8_t * shapeptr, uint8_t * maskframe, 
                uint8_t dshp_slots, uint16_t offset, bool usemask){
    
    uint8_t i,j,w,k;
    
    uint8_t *drawptr;  
    uint8_t drawbuf[8];
    uint8_t *maskptr;  
    uint8_t maskbuf[8];
    
    uint8_t draw1, draw2;
    uint8_t mask1, mask2;
    
    uint8_t col, mask, bkg;
     
    for (w=0; w<dshp_slots; w++){    
        for (i=0; i<8; i++){
            drawptr = shapeptr+(i*dshp_slots) + offset;
            drawbuf[i] = *drawptr;
            if (usemask){
                maskptr = maskframe+(i*dshp_slots) + offset;
                maskbuf[i] = *maskptr;
            }
        }
        
        for (k=0; k<8; k++){
            col=0;
            mask=0;
            for (j=0; j<8; j++){
                draw1 = (drawbuf[j]>>k) & 0x01;
                draw2 = draw1 << j;
                col = col | draw2;
                
                if (usemask){
                    mask1 = (maskbuf[j]>>k) & 0x01;           
                    mask2 = mask1 << j;
                    mask = mask | mask2;
                }
            }
            if (usemask){
                bkg= *currentpositionptr & mask;
                mask =~mask;
                col = col & mask;
                col = col | bkg;
            }
            *currentpositionptr++=col; 
        }
        shapeptr++;
        maskframe++;
    }
}



void glcd::_clearArea(uint8_t page, uint8_t x, uint8_t lenght){
    
    uint8_t *wptr;
    uint8_t i;
    wptr = framebufferptr + (page*128) + x;
    for (i=0; i<lenght ; i++){
        *wptr=background;
        wptr++;
    }
    framepages[page]=DIRTYALL;
	GLCDRefresh();
}

void glcd::_ClearScreen(bool invert){

    uint16_t i;
    uint8_t j;
    uint8_t *fptr;
    uint8_t temp;
    
    updated = false;
    
    fptr = framebufferptr;
    
    writing_semaphore = true;
        for(i=0; i<1024; i++){
            if (invert){
                temp = *fptr;
                *fptr = ~temp;
            }
            else{
                *fptr = background;
            }           
            fptr++;
        }
	
        for(j=0;j<8;j++){
            framepages[j]=DIRTYALL;
        }

        _setPosition(0,0);
    writing_semaphore = false; 
	
}




// Widgets  Services =============================================================================================================

void glcd::drawBGraph (bginit_evt *initbgevt){

	bgraph_handle * bgh = &bg_handles[initbgevt->handle];
	bgh->handle = initbgevt->handle;
	bgh->literal = initbgevt->literal;
	strcpy(bgh->fmt, initbgevt->format->c_str());
	bgh->type = initbgevt->type;
	
	if (initbgevt->left){
		bgh->bgxoffset=0;
		bgh->ltxoffset=70;
	}
	else{
		bgh->bgxoffset=64;
		bgh->ltxoffset=0;
	}
	
	bgh->yoffset = initbgevt->line * 8;
	bgh->llimit = initbgevt->lowlim;
	bgh->hlimit = initbgevt->highlim;
	bgh->litlimit = initbgevt->litlimit;
	
	
	if (bgh->type == 0){
		// Corner bargrap
		_drawShape (SHP_BGRAPH_CORNER, bgh->bgxoffset, bgh->yoffset , false);
		bgh->bgxoffset += 2;
		bgh->barsize = 60;
	}
	else if (bgh->type == 1){
		// center bargraph
		_drawShape (SHP_BGRAPH_CENTER, bgh->bgxoffset, bgh->yoffset , false);
		bgh->bgxoffset += 32;
		bgh->barsize = 28;
	}
	
	if (bgh->literal){
		sprintf (&bgh->sbuf[0], bgh->fmt, 0.0);
		_putMessage (bgh->ltxoffset, bgh->yoffset, false, false, &bgh->sbuf[0]);
	}
	
}

void glcd::updagteBGraph (bgupdate_evt *updatebgevt){

	uint8_t i;
	bgraph_handle * bgh = &bg_handles[updatebgevt->handle];
	uint8_t bsize = bgh->barsize; 
	uint8_t barend;
	double bar_ratio;
	double vtemp;
	uint8_t *fbinit;
	
	if (updatebgevt->cmd == 0){
		
		
		YPos=bgh->yoffset;
		fbinit = framebufferptr + ((YPos>>3)<<7);
		
		
		// Is Centre Bargraph ?
		if (bgh->type == 1){
			if (updatebgevt->value > 0){
				vtemp = updatebgevt->value;
				if (vtemp > bgh->hlimit ) vtemp = bgh->hlimit;
				bar_ratio = vtemp / bgh->hlimit;
				barend = bgh->barsize * bar_ratio;
				// Clear negative side of bar
				XPos=bgh->bgxoffset-30;		
				currentpositionptr = fbinit + XPos;
				for (i = 0; i < 30; i++) {
					*currentpositionptr = 0x81;
					_moveRight();
				}
				// Now fill the positive side				
				XPos=bgh->bgxoffset + 2;
				currentpositionptr = fbinit + XPos;
				for (i = 0; i < bsize; i++) {
					if (i > barend){
						*currentpositionptr = 0x81;
					}
					else{
						*currentpositionptr = 0xBD;
					}			
					_moveRight();
				}	
			}
			else if (updatebgevt->value < 0){
				vtemp = updatebgevt->value * -1;
				if (vtemp > bgh->hlimit ) vtemp = bgh->hlimit;
				bar_ratio = 1-(vtemp / bgh->hlimit);
				barend = bgh->barsize * bar_ratio;
				
				// Clear positive side of bar
				XPos=bgh->bgxoffset+2 ;
				currentpositionptr = fbinit + XPos;
				for (i = 0; i < bsize; i++) {
					*currentpositionptr = 0x81;
					_moveRight();
				}
				
				// Now fill the negative				
				XPos=bgh->bgxoffset-30;
				currentpositionptr = fbinit + XPos;
				for (i = 0; i < bsize; i++) {
					if (i < barend){
						*currentpositionptr = 0x81;
					}
					else{
						*currentpositionptr = 0xBD;
					}			
					_moveRight();
				}					
			}
			else {
				_drawShape (SHP_BGRAPH_CENTER, bgh->bgxoffset-32, bgh->yoffset , false);
			}			
		}
		else if (bgh->type == 0){
			vtemp = updatebgevt->value;
			if (vtemp > bgh->hlimit ) vtemp = bgh->hlimit;
			if (vtemp < bgh->llimit ) vtemp = bgh->llimit;
			bar_ratio = (vtemp == 0) ?  0.001 : vtemp / (bgh->hlimit - bgh->llimit);
			barend = bgh->barsize * bar_ratio;
			
			// Fill  corner bar
			XPos=bgh->bgxoffset;
			currentpositionptr = fbinit + XPos;
			
			for (int i = 0; i < bsize; i++) {
				if (i > barend){
					*currentpositionptr = 0x81;
				}
				else{
					*currentpositionptr = 0xBD;
				}			
				_moveRight();
			}
		}
		
		
		if (bgh->literal){
			if (updatebgevt->value > bgh->litlimit) updatebgevt->value = bgh->litlimit;
			sprintf (&bgh->sbuf[0], bgh->fmt, updatebgevt->value);
			_putMessage (bgh->ltxoffset, bgh->yoffset, false, false, &bgh->sbuf[0]);
		}
		else{
			framepages[YPos>>3]=DIRTYALL;
		}
	}
		
}


void glcd::drawDlg (dialog_evt *dlgevt){
    
    uint8_t line = 15;
    //uint8_t data;
    //std::string::iterator p;
	
	dlg_type=dlgevt->type;
	dlg_status=0;
	dlg_yesok_sig=dlgevt->yesok;
	dlg_no_sig=dlgevt->no;
	dlg_cancel_sig=dlgevt->cancel;
	
    _drawShape (SHP_DLG_INFO_TOP, 0, 0, true);  
    
    if (dlg_type == 0){
        _drawShape (SHP_DLG_BT_OK_SET, 0, 32, true);
    }
    else{
        _drawShape (SHP_DLG_BT_YES, 0, 32, true);
    }
    
    //string * msg = new string("Teste1\nTeste2");
	
    
    _setPosition(20, line);
	
	std::vector<std::string> lines = explode((char*)dlgevt->msg->c_str(), '\n');
	for (std::vector<string>::iterator it = lines.begin() ; it != lines.end(); ++it){
		std::string str = *it;
		putMessage(20,line,100,true,false,true,1, new std::string(str.c_str()));
		line +=8;
	}
	
	//    for (p = dlgevt->msg->begin( ); p != dlgevt->msg->end( ) ; ++p){
//        data = *p;
//        if (data == '\n'){
//            line +=8;
//            _setPosition(20, line);
//        }
//        else{
//            _putChar(data, true,false);
//        }    
//    }
	
    delete dlgevt->msg;  
}


uint8_t glcd::navigateDlg(uint8_t keycode){
  
	// check if user wants to go back (BS key))
    if ((keycode == 0x08) & (dlg_cancel_sig !=0)){
        return dlg_cancel_sig;
    }
	
    // Now check if enter was pressed, return the signal then
    if (keycode == 0x0D){
        if (dlg_status ==0){
			return dlg_yesok_sig;
		}
		else{
			return dlg_no_sig;
		}
    }
    
	// Check move right
	if ((keycode == 19) & (dlg_type == 1)){
        if (dlg_status == 0){
			_drawShape (SHP_DLG_BT_NO, 0, 32, false);
			dlg_status=1;
		}
    }
	
	else if ((keycode == 18) & (dlg_type == 1)){
        if (dlg_status == 1){
			_drawShape (SHP_DLG_BT_YES, 0, 32, false);
			dlg_status=0;
		}
    }
	
	return 0;
}



void glcd::drawCanvas(canvas_evt *cevt){
    
    uint8_t i,j;
 
	//clearCanvas();
	_drawShape (cevt->bkgshape, 0, 0, false);
   
    invertArea (3, 0, 3, 41);  
    
    canvas_selected= 0;
    
    //Clear move map
    for (i=0; i<6; i++){
        for (j=0 ; j<4; j++){
            canvas_move_map[i][j]=0;
        }
    }
    
	for (j=0 ; j<6; j++){
		canvas_signals[j] = cevt->signals[j];
    }
	canvas_rollback = cevt->rollback;
	
    // Now populate movemap
    canvas_move_map[0][2]=canvas_signals[1];
    canvas_move_map[0][3]=canvas_signals[3];
    
    canvas_move_map[1][1]=canvas_signals[0];
    canvas_move_map[1][2]=canvas_signals[2];
    canvas_move_map[1][3]=canvas_signals[4];
    
    canvas_move_map[2][1]=canvas_signals[1];
    canvas_move_map[2][3]=canvas_signals[5];
    
    canvas_move_map[3][0]=canvas_signals[0];
    canvas_move_map[3][2]=canvas_signals[4];
    
    canvas_move_map[4][0]=canvas_signals[1];
    canvas_move_map[4][1]=canvas_signals[3];
    canvas_move_map[4][2]=canvas_signals[5];
    
    canvas_move_map[5][0]=canvas_signals[2];
    canvas_move_map[5][1]=canvas_signals[4];
	
	//log(FINE, "Canvas ready to navigate\r\n");
	
}

uint8_t glcd::navigateCanvas(uint8_t keycode){
    
    uint8_t move ;
    uint8_t i;
    
    // First check if enter was pressed, return the signal then
    if (keycode == 0x0D){
        return canvas_signals[canvas_selected];
    }
    
    // Now, check if user wants to go back (BS key))
    if (keycode == 0x08){
        return canvas_rollback;
    }
    
    
    // Filter to use only direction keys now on
    if(keycode < 0x11 || keycode > 0x14) return 0x00;
   
    // Lookup into map
    move = canvas_move_map[canvas_selected][keycode - 0x11];
    //PDEBUG4("\r\nCanvas move got kbdcode %d / move=%d / [%d][%d]", keycode, move, canvas_selected, keycode - 0x11);
    // Verify we got a valid move and if it doesn't overflow max app.
    if (move == 0){
        return 0x00;
    }
    
    // Valid move detected - update UI
    // First clear the old widget
    invertArea (  canvas_widget_map[canvas_selected][0], 
                  canvas_widget_map[canvas_selected][2], 
                  3, 
                  canvas_widget_map[canvas_selected][1]
                  ); 
    
    // Now set the next one
    for (i=0; i<6; i++){
        if(canvas_signals[i]==move){
            canvas_selected=i;
            break;
        }
    }
    
    invertArea (  canvas_widget_map[canvas_selected][0], 
                  canvas_widget_map[canvas_selected][2], 
                  3, 
                  canvas_widget_map[canvas_selected][1]
                  ); 
    return 0x00;
}


void glcd::_drawChoiceItem (uint8_t vcursor, uint8_t pos, bool invert, bool clear){
	
	char * cstr = new char [25];
	
	if (vcursor != 255){
		if (clear) invertArea (4, (pos+2) + 64, 1, 118);

		std::strcpy (cstr, choicedesc->items[vcursor].c_str());
		_putMessage(5, (8*pos)+16, false, false, cstr);
		choicedesc->vptrs[pos] = vcursor;
	}
	
	if (invert) invertArea (4, pos+2, 1, 118);
	choicedesc->cursor = pos;
	
}


char * glcd::getLastChoice(){	
	return (char *)choicedesc->items.at(choicedesc->cursor+choicedesc->offset).c_str();
}

void glcd::drawChoice(choice_evt *choice){

	kbdnumeric=true;
		
	_drawShape(SHP_CHOICE, 0U, 0U, true);	
	_putMessage(3,  0,true, false, (char *)choice->prompt->c_str());
	
	choicedesc->items.assign(choice->items->begin(), choice->items->end());
	choicedesc->itemnum=choicedesc->items.size();
	
	_drawChoiceItem (0, 0, true, false);
	_drawChoiceItem (1, 1, false, false);
	_drawChoiceItem (2, 2, false, false);
	choicedesc->cursor = 0;
	choicedesc->offset = 0;

	//delete choice->items;
	delete choice->prompt;
	
}


uint8_t glcd::navigateChoice(uint8_t keycode){

	switch (keycode){
		case 0x0d:
			return 1;
			break;
		
		case 0x08:
			return 2;
			break;
			
		case 0x11:
			// UP
			if (choicedesc->cursor == 0){
				// Add new item at the top
				if (choicedesc->vptrs[0] != 0){
					_drawChoiceItem (choicedesc->vptrs[1], 2, false, true);
					_drawChoiceItem (choicedesc->vptrs[0], 1, false, true);
					_drawChoiceItem (choicedesc->vptrs[0]-1, 0, false, true);
					_drawChoiceItem (255, 0, true, false);
					choicedesc->offset--;
				}
			}
			else{
				_drawChoiceItem (255, choicedesc->cursor, true, false);
				choicedesc->cursor --;
				_drawChoiceItem (255, choicedesc->cursor, true, false);
			}
			break;

		case 0x14:
			// Down
			if (choicedesc->cursor >= 2){
				// Add new item at bottom
				if (choicedesc->vptrs[2] < choicedesc->itemnum-1){
					_drawChoiceItem (choicedesc->vptrs[1], 0, false, true);
					_drawChoiceItem (choicedesc->vptrs[2], 1, false, true);
					_drawChoiceItem (choicedesc->vptrs[2]+1, 2, false, true);
					_drawChoiceItem (255, 2, true, false);
					choicedesc->offset++;
				}				
			}
			else{
				_drawChoiceItem (255, choicedesc->cursor, true, false);
				choicedesc->cursor ++;
				_drawChoiceItem (255, choicedesc->cursor, true, false);
			}
			break;

		case 0x12:
			// Shift to left

			break;
		case 0x13:
			// Shift to right

			break;

		default:

			break;
	}
	return 0;
}

void glcd::clearChoice(){
	//PDEBUG ("Clearing choice dlg\r\n");
	//choicedesc->items.clear();
	restoreBackground (SHP_CHOICE);
}



void glcd::drawInput(input_evt * evt){
  
	uint8_t line = 8;
    //uint8_t data;
    //std::string::iterator p;
	
	kbduppercase = false;
	inptdesc->functionbar=false;
	inptdesc->error = 0;
	inptdesc->iconcursor = 0;
	
	inptdesc->type = evt->type;
	inptdesc->numeric = evt->numeric;
	inptdesc->insert = evt->insert;
	inptdesc->uppercase = evt->uppercase;
	inptdesc->result = evt->val;
	inptdesc->limit_l = evt->llimit;
	inptdesc->limit_h = evt->hlimit;
	
	memset (inptdesc->data,0x0,20);
	
	_drawShape(inptdesc->type, 0U, 0U, true);	
	setNumeric(inptdesc->numeric);
	
	
	if (evt->data->size() == 0){
		sprintf(&inptdesc->data[0], (char*) evt->form->c_str() , inptdesc->result);	
	}
	else{
		strcpy(&inptdesc->data[0], evt->data->c_str());
	}
	
//	if (inptdesc->numeric){			
//		sprintf(&inptdesc->data[0], (char*) evt->form->c_str() , inptdesc->result);	
//	}
//	else{
//		strcpy(&inptdesc->data[0], evt->data->c_str());
//	}
	
	if (inptdesc->type == SHP_INPUTFULL){
		putMessage(6,line,116,true,false,true,0, new std::string(evt->line->c_str()));
	}
	else{
		std::vector<std::string> lines = explode((char*)evt->line->c_str(), '\n');
		for (std::vector<string>::iterator it = lines.begin() ; it != lines.end(); ++it){
			std::string str = *it;
			putMessage(4,line,70,true,false,true,1, new std::string(str.c_str()));
			line +=8;
		}	
	}
	
//	
//	_setPosition(3, line);
//    for (p = evt->line->begin( ); p != evt->line->end( ) ; ++p){
//        data = *p;
//        if (data == '\n'){
//            line +=8;
//            _setPosition(3, line);
//        }
//        else{
//            _putChar(data, true,false);
//        }    
//    }	

	inptdesc->cursor = strlen(inptdesc->data)-1;     // pformats[l_inpt->format].fsize;
    _updateInput (true);
	_resetInputIcons();
	
	delete evt->line;
	delete evt->data;
	delete evt->form;
	
}

const std::vector<std::string> glcd::explode(char * it, const char& c) {
    
    std::string buff = std::string("");
    std::vector<std::string> v;

    while (*it !=0){
    //for ( std::string::iterator it=s.begin(); it!=s.end(); ++it){
        if(*it != c){
             buff+=*it;
        }
        else{
            if(*it == c && buff != ""){
                v.push_back(buff); 
                buff = "";
            }     
        } 
        it++;            
    }
    if(buff != "") v.push_back(buff);
    return v;
}



void glcd::_resetInputIcons(){
	
	saveIconState();
	clearLeftIcons();
	
	if (inptdesc->hashistory == true){
		setIcon (0, glcd::ICREFRESH, SHP_ICON_HISTORY_OFF);
	}
	else{
		setIcon (0, glcd::ICREFRESH, SHP_ICON_CLEAR);
	}
	
	
	if (kbdinsert){
		setIcon (1, glcd::ICREFRESH, SHP_ICON_OVERSTRIKE_OFF);
	}
	else{
		setIcon (1, glcd::ICREFRESH, SHP_ICON_INSERT_OFF);
	}
	
	if (kbdnumeric){
		setIcon (2, glcd::ICREFRESH, SHP_ICON_NUMBER_OFF);
		setIcon (3, glcd::ICREFRESH, SHP_ICON_CLEAR);
	}
	else{
		setIcon (2, glcd::ICREFRESH, SHP_ICON_ALPHA_OFF);
		if (kbduppercase){
			setIcon (3, glcd::ICREFRESH, SHP_ICON_UPPERCASE_OFF);
		}
		else {
			setIcon (3, glcd::ICREFRESH, SHP_ICON_LOWERCASE_OFF);
		}
	}
	
}



void glcd::_updateInput (bool setcursor){
		
	//log(FINE, "Drawing input message with %d chars and %d strips\n\r", strlen(inptdesc->data), calculateStrips(&inptdesc->data[0]));

	
	if (setcursor){
		if (inptdesc->cursor == 255){
			inptdesc->cursor = strlen(inptdesc->data);
		}
		inptdesc->data[inptdesc->cursor] = inptdesc->data[inptdesc->cursor] | 0x80;
	}
	
	if (inptdesc->type == SHP_INPUT5){
		_putMessage(80, 16,false, false, (char*)"          ");
		_putMessage(82, 16,false, false, &inptdesc->data[0]);
	}
	else if (inptdesc->type == SHP_INPUTFULL){
		_putMessage(5, 24,false, false, (char*)"                             ");
		_putMessage(7, 24,false, false, &inptdesc->data[0]);
	}

}



void glcd::_insertInputChar (uint8_t ichar){
	
	if (!inptdesc->uppercase || inptdesc->numeric){
		inptdesc->data[inptdesc->cursor]=ichar;
	}
	else{
		inptdesc->data[inptdesc->cursor]=ichar;
	}
}

void glcd::_setInputCursor(int direction){
	
	inptdesc->data[inptdesc->cursor] = inptdesc->data[inptdesc->cursor] & 0x7f;
	
	if (direction == 1){
		inptdesc->cursor++;
		_updateInput (true);
	}
	else if (direction == -1){
		inptdesc->cursor--;
		_updateInput (true);
	}
	else{
		inptdesc->cursor += direction;
	}				
}

double glcd::getFloatInput(){ return inptdesc->result;}


uint8_t glcd::navigateInput(uint8_t keycode){
	
	uint8_t j;
	double number;
		
	log(FINER,"Navigate input was called with code %d\r\n", keycode);
	
	if (inptdesc->functionbar){
		switch (keycode){
			case 0x0d:				
				//PDEBUG2 ("Iconbar : cursor=%d  -- icon = %d\n\r", inptdesc->iconcursor, iconsctrl[inptdesc->iconcursor].id);
				// Trap the history first...
				if (inptdesc->iconcursor == 0 && inptdesc->hashistory == true){
					// History was called
					log(FINE,"HISTORY WAS CALLED \r\n");
					return 4;
				}
				
				if (iconsctrl[inptdesc->iconcursor].id == SHP_ICON_INSERT_ON){
					setIcon (inptdesc->iconcursor, glcd::ICREFRESH, SHP_ICON_OVERSTRIKE_ON);
					kbdinsert = false;
				}
				else if (iconsctrl[inptdesc->iconcursor].id == SHP_ICON_OVERSTRIKE_ON){
					setIcon (inptdesc->iconcursor, glcd::ICREFRESH, SHP_ICON_INSERT_ON);
					kbdinsert = true;
				}
				
				if (iconsctrl[inptdesc->iconcursor].id == SHP_ICON_NUMBER_ON){
					setIcon (inptdesc->iconcursor, glcd::ICREFRESH, SHP_ICON_ALPHA_ON);
					kbdnumeric = false;
				}
				else if (iconsctrl[inptdesc->iconcursor].id == SHP_ICON_ALPHA_ON){
					setIcon (inptdesc->iconcursor, glcd::ICREFRESH, SHP_ICON_NUMBER_ON);
					kbdnumeric = true;
				}
				
				if (iconsctrl[inptdesc->iconcursor].id == SHP_ICON_UPPERCASE_ON){
					setIcon (inptdesc->iconcursor, glcd::ICREFRESH, SHP_ICON_LOWERCASE_ON);
					kbduppercase = false;
				}
				else if (iconsctrl[inptdesc->iconcursor].id == SHP_ICON_LOWERCASE_ON){
					setIcon (inptdesc->iconcursor, glcd::ICREFRESH, SHP_ICON_UPPERCASE_ON);
					kbduppercase = true;
				}
				break;
				
			case 0x11:
				// UP
				_updateInput(true);
				inptdesc->functionbar = false;
				_resetInputIcons();
				break;

			case 0x14:
				// Down	
				break;
		
			case 0x12:
				// Left
				j = (inptdesc->hashistory==true) ? 0 : 1;
				if (inptdesc->iconcursor > j){
					setIcon (inptdesc->iconcursor, glcd::ICINVERT, 0);
					inptdesc->iconcursor--;
					setIcon (inptdesc->iconcursor, glcd::ICINVERT, 0);
				}
				break;

			case 0x13:
				// Right
				j = 2;
				if (!kbdnumeric) j++;
				if (inptdesc->iconcursor < j){
					setIcon (inptdesc->iconcursor, glcd::ICINVERT, 0);
					inptdesc->iconcursor++;
					setIcon (inptdesc->iconcursor, glcd::ICINVERT, 0);
				}
				break;			
		}
	}
	else{
		switch (keycode){	
			case 0x0d:
				inptdesc->data[inptdesc->cursor] = inptdesc->data[inptdesc->cursor] & 0x7f;
				if ( (inptdesc->type == SHP_INPUT5) | (inptdesc->type == SHP_INPUTFULL) ){
					if (inptdesc->numeric){
						number = atof(inptdesc->data);						
////						if (number == 0.0) {
////							log(WARNING,"Unable to convert number on input dlg\n\r");
////							return 2;
////						}
						if (!((inptdesc->limit_l == 0) & (inptdesc->limit_h == 0))){
							if ((number < inptdesc->limit_l) || (number > inptdesc->limit_h)) {
								inptdesc->result=number;
								log(WARNING,"Number out of range on input dlg\n\r");
								return 3;
							}
						}
						//log(FINE,"Input converted to %f\n\r", number);
						inptdesc->result = number;
						return 1;
					}
					else{						
						return 1;
					}
				}
				break;
			case 0x11:
				// UP
				break;

			case 0x14:
				// Down
				inptdesc->data[inptdesc->cursor] = inptdesc->data[inptdesc->cursor] & 0x7f;
				inptdesc->functionbar = true;
				if (inptdesc->hashistory ){
					inptdesc->iconcursor=0;
					setIcon (0, glcd::ICINVERT, 0);
			    }
				else{
					inptdesc->iconcursor=1;
					setIcon (1, glcd::ICINVERT, 0);
				}
				_updateInput(false);
				break;

			case 0x08:	
				// * (backspace now)

				inptdesc->data[inptdesc->cursor] = inptdesc->data[inptdesc->cursor] & 0x7f;

				// No inicio
				if (inptdesc->cursor == 0 && strlen(inptdesc->data)==1 && inptdesc->data[0] != ' '){
					inptdesc->data[0] = ' ';
					_updateInput (true);
				}
				// Are we at insert point ?
				else if (inptdesc->cursor == strlen(inptdesc->data)-1 && inptdesc->cursor != 0){
					inptdesc->cursor--;
					inptdesc->data[inptdesc->cursor]= ' ';
					inptdesc->data[inptdesc->cursor+1]= 0x00;
					_updateInput(true);
				}
				// So we are at some middle point
				else if (inptdesc->cursor != 0){
					for (uint i = inptdesc->cursor; i < strlen(inptdesc->data) ; i++) {
						inptdesc->data[i-1] = inptdesc->data[i];
					}				
					_setInputCursor (-1);
				}

				break;

			case 0x12:
				// Shift to left
				if (inptdesc->cursor != 0){
					_setInputCursor (-1);
				}
				break;


			case 0x13:
				// Shift to right
				if (inptdesc->cursor < strlen(inptdesc->data)-1){
					_setInputCursor (1);
				}
				break;

			default:
				// OK number... -- 
				// first clear the underscore
				inptdesc->data[inptdesc->cursor] = inptdesc->data[inptdesc->cursor] & 0x7f;

				// If cursor is at insert position, add the code to the final
				if ((inptdesc->cursor < 19) & (inptdesc->cursor == strlen(inptdesc->data)-1)){
					_insertInputChar(keycode);
					inptdesc->cursor++;
					inptdesc->data[inptdesc->cursor]= ' ';
					inptdesc->data[inptdesc->cursor+1]= 0x00;
				}
				else if (!inptdesc->insert){
					_insertInputChar(keycode);
				}
				_updateInput (true);
				break;
		}
	}
	return 0;
}


void glcd::clearInput(){
	
	log(FINER,"Clearing input dlg\r\n");
	
	kbdnumeric=true;
	restoreBackground (inptdesc->type);
	
	restoreIconState();
	//clearLeftIcons();
	updateIcons();
	
}


// From original C =======================================================================

void glcd::activateHourGlass(){ 
    
//    #ifdef GLCDDEBUG 
//        printf ("\n\rActivating Hour Glass");
//    #endif
//    hglassstate = HG_SHOW;
//    hglasstimer = 10;
}

void glcd::dismissHourGlass(){ 
//    hgdismiss = true;
}

void glcd::resetStatusbar(){
//    polltimer=0;
//    stripnum =0;
}

uint16_t glcd::calculateStrips (char * msg){
 
    uint8_t i,d,data;
    uint16_t stripsnum = 0 ;
 
    while ( *msg !=0){
        data = *msg++;
        for(i=0;i<7;i++){
            d=Font[data-32][i];
            if(d!=0x55){
                stripsnum++;
            }
        }
    }
    return stripsnum;
}


void glcd::_loadPage (bool invert, bool left){

    uint8_t j, data;
    uint8_t * bufptr;   
 
    if (left){
        cs1=1;
        cs2=0;
        bufptr = framebufferptr + (framepage * 128);
    }
    else{
        cs1=0;
        cs2=1;
        bufptr = framebufferptr + ((framepage * 128) + 64);
    }
  
    _WriteCmd(0x40);                           //y
    _WriteCmd(0xb8 + framepage);               //x (page)
    
    //PDEBUG2 ("\r\nFramepage = %d / bufptr = %d", framepage, bufptr);
    
    for(j=0;j<0x40;j++){
        data = *bufptr;
        if (invert) data = ~data;
        _WriteData(data);
        //_WriteData(0X55);
        bufptr++;
    } 
    cs1=0;
    cs2=0;
	
}

void glcd::_WaitNotBusy(void){
    
    uint8_t ldata = 0; 
    
    // Set Port to input 
    //TRIS_Data=0xff;
    
    glcddi=0; // is command
    glcdrw=1; // is reading
    
    glcdpins.input();
    do{
        glcde=1;         //strobe     
        wait_us(GDELAY);
        ldata = glcdpins;    //and sample status
        glcde=0;
        //wait_us(20);
    } while (ldata & 0x80); // wait to not busy (d7 == 0)
    glcdpins.output();
    
    // Port now is to output
    //TRIS_Data=0x00;
}

void glcd::setKbdcode(uint8_t kbdcode) {
	this->kbdcode = kbdcode;
}

uint8_t glcd::getKbdCode(){
	uint8_t temp;
	temp = kbdcode;
	kbdcode=0;
	return temp;
}

void glcd::setPublish_kbdcode(bool publish_kbdcode) {
	this->publish_kbdcode = publish_kbdcode;
}

bool glcd::isPublish_kbdcode() const {
	return publish_kbdcode;
}

void glcd::_WriteCmd(unsigned char data){
 
    _WaitNotBusy();
    glcdpins = data;
    glcddi=0;
    glcdrw=0;
    
    glcde=1;
    wait_us(GDELAY);
    glcde=0;

}

void glcd::_WriteData (unsigned char data){
 
    _WaitNotBusy();
    glcdpins.output();
    glcdpins = data;
    glcddi=1;
    glcdrw=0;
    
    glcde=1;
    wait_us(GDELAY);
    glcde=0;

}


void glcd::_initHardware(void){

    framebufferptr = &framebuffer[0];
    framepagesptr = &framepages[0];
 
    writing_semaphore=false;
    background = 0x00;
	glcdmutex = false;
    glcdrst=1;
   
    cs1=1;
    cs2=0;
    _WriteCmd(0x3f);	
    _WriteCmd(0xc0);	
    
    cs1=0;
    cs2=1;
    _WriteCmd(0x3f);	
    _WriteCmd(0xc0);	
    
    cs2=0;
	
    _ClearScreen(false);
}

void glcd::clearCanvas(){
    
    _clearArea(0, 0, 128);
    _clearArea(1, 0, 128);
    _clearArea(2, 0, 128);
    _clearArea(3, 0, 128);
    _clearArea(4, 0, 128);
    _clearArea(5, 0, 128);
}

void glcd::_clearPage(uint8_t page){
    
    _clearArea(page, 0, 128);
}

void glcd::_moveRight(void){

    if(++XPos>=128){
        XPos=0;
        YPos+=8;
    }
    currentpositionptr++; // = framebufferptr + ((YPos>>3)<<7) + XPos;
}

void glcd::_setPosition(uint8_t x, uint8_t y){
    
    XPos=x;
    YPos=y;
    currentpositionptr = framebufferptr + ((YPos>>3)<<7) + XPos;
    YPos=y;
}


}


















//void glcd::drawShapeRequest (uint16_t id, uint8_t x, uint8_t y, bool savebkg){
//    
//    drawshapepar * params1;      
//    
//    #ifdef GLCDDEBUG
//    printf ("\r\ndrawshapeREQUEST called with : id=%d, x=%d, y=%d", id, x, y);  
//    #endif
//    
//    Salloc::params_hdr * hdr = salloc->storeParams(sizeof(drawshapepar), id); //(rand()||0x80));
//    
//    params1 = (glcd::drawshapepar*)hdr->header.payload;
//    hdr->header.id = id;
//    params1->id = id;
//    params1->x=x;
//    params1->y=y;
//    params1->b=savebkg;
//    hdr->fp1 = (void (*)(char*))&glcd::_drawShapeWrapper;
//    _insertDispatchRequest(hdr);
//}
//
//void glcd::_drawShapeWrapper(Salloc::params_hdr * hdr){
//    
//    drawshapepar * pty = (glcd::drawshapepar*)hdr->header.payload;    
//    #ifdef GLCDDEBUG
//    printf ("\r\ndrawshapewrapper called with : id=%d, x=%d, y=%d", pty->id, pty->x, pty->y);  
//    #endif
//    _drawShape ( pty->id, pty->x, pty->y, pty->b);
//    salloc->SRAMfree(hdr->header.type, pty->id);
//}



























    
//    uint8_t i;
//    
//	for (i=0 ; i<8 ; i++){
//		// framepage rolls its last 3 bits, so it will scan 8 slots.
//		switch (framepages[i]){
//			case FINE:
//				break;
//			case DIRTYALL: // case FINE:
//				//putch('.');
//				framepage = i;
//				_loadPage (false, true);
//				_loadPage (false, false);
//				framepages[i]=FINE;
//				break;
//			default:
//				break;
//		}
//	}
	
	





//else if (dispatch_state == DPTH_THREADS){
//        if (dspl_dispatch_fifo_head != dspl_dispatch_fifo_tail){
//            dispatch_state = DPTH_REFRESH;
//            dspl_dispatch_busy = true;
//                Salloc::params_hdr * hdr;
//                void (*fp1) (Salloc::params_hdr* pty);   
//                hdr =  dspl_dispatch_fifo[dspl_dispatch_fifo_head];
//                fp1 = (void (*)(Salloc::params_hdr*))hdr->fp1;
//                //printf ("\n\rDispatching to %8.4x", fp1);
//                (*fp1)(hdr);
//                dspl_dispatch_fifo_head = (dspl_dispatch_fifo_head++) & 0x0F;         
//            dspl_dispatch_busy = false;
//        }
//        else{
//            dispatch_state = DPTH_REFRESH;
//        }
//    }






//    
//    else if (dispatch_state == DPTH_SBAR){
//        
//        // Abort if user pressed a key
//        if (kbdcode !=0){
//            polltimer=0;
//            stripnum =0;
//        }
//        
//        // Wait a while if the draw is already done
//        if (polltimer-- !=0) return;
//        
//        // 
//        if (stripnum == 0) {
//            salloc->SRAMfree(shdr->header.type, shdr->header.id);
//            for (i=0 ; i<8 ; i++){
//                if (iconsctrl[i].status == ICACTIVE){
//                    iconsctrl[i].status = ICREFRESH; 
//                }
//            }
//            dispatch_state = DPTH_REFRESH;
//        }
//        else if (stripnum == 1) {
//            polltimer = STSPERM;
//        }
//        else{ 



//            // load char
//            if (stripheader > 0) stripheader--;

//            if (droptr < stripslengthptr & stripheader ==0){
//                j= (*droptr>>3) | 0x80;
//                i= (*droptr<<5) | 0x04;
//                *droptr++;
//                polltimer = STSSLOW;
//            }
//            else{
//                j=0x80;
//                i=0x04;
//                polltimer = STSFAST;
//            }
//            
//            uint8_t * btto = framebufferptr + 1021;
//            uint8_t * upto = btto - 128;
//            *btto = j;
//            *upto = i;
//            



//            // Scroll to left
//            uint8_t * upfrom = framebufferptr + 772;
//            upto = framebufferptr + 771;
//            uint8_t * btfrom = framebufferptr + 900;
//            btto = framebufferptr + 899;
//            for (i=3 ; i<125 ; i++){               
//                *upto = *upfrom; //(*upfrom & 0x3f) | (*upto & 0xc0);
//                *btto = *btfrom;
//                btto++;btfrom++;
//                upto++;upfrom++;
//            }


//            framepage=6;
//            _loadPage (false, true);
//            _loadPage (false, false);
//            framepage=7;
//            _loadPage (false, true);
//            _loadPage (false, false);
//        }
//        if (stripnum >0) stripnum --;
//    }








//    
//    else if (dispatch_state == DPTH_HGLASS){
//        
//        //putch('.');
//        
//        if (hglasstimer > 2) {
//            hglasstimer--;
//            dispatch_state = DPTH_THREADS; 
//            return;
//        }
//       
//        switch (hglassstate){
//            case HG_IDLE:
//                break;
//            case HG_SHOW:
//                drawShapeRequest (SHP_AMP_UP, 48, 8, true);
//                hglasstimer = HG_TIME2;
//                hglassstate = HG_STRIP30;
//                break;
//            case HG_UP:
//                restoreBackground (SHP_AMP_HORZ);
//                if(!hgdismiss){
//                    drawShapeRequest (SHP_AMP_UP, 48, 8, true);
//                    hglasstimer = HG_TIME2;
//                    hglassstate = HG_STRIP30;
//                }
//                else{
//                    hglassstate = HG_IDLE;
//                    hglasstimer=0;
//                    hgdismiss = false;
//                }
//                break;
//            case HG_STRIP30:
//                if(!hgdismiss){
//                    drawShapeRequest (SHP_AMP_STRIP30, 48, 16, false);
//                    hglasstimer = HG_TIME2;
//                    hglassstate = HG_STRIP60;
//                }
//                else{
//                    restoreBackground (SHP_AMP_UP);
//                    hglassstate = HG_IDLE;
//                    hglasstimer=0;
//                    hgdismiss = false;
//                }
//                break;
//            case HG_STRIP60:
//                if(!hgdismiss){
//                    drawShapeRequest (SHP_AMP_STRIP60, 48, 16, false);
//                    hglasstimer = HG_TIME2;
//                    hglassstate = HG_STRIP90;
//                }
//                else{
//                    restoreBackground (SHP_AMP_UP);
//                    hglassstate = HG_IDLE;
//                    hglasstimer=0;
//                    hgdismiss = false;
//                }
//                break;    
//            case HG_STRIP90:
//                if(!hgdismiss){
//                    drawShapeRequest (SHP_AMP_STRIP90, 48, 16, false);
//                    hglasstimer = HG_TIME2;
//                    hglassstate = HG_TURN;
//                }
//                else{
//                    restoreBackground (SHP_AMP_UP);
//                    hglassstate = HG_IDLE;
//                    hglasstimer=0;
//                    hgdismiss = false;
//                }
//                break;     
//            case HG_TURN:
//                restoreBackground (SHP_AMP_UP);
//                if(!hgdismiss){
//                    drawShapeRequest (SHP_AMP_HORZ, 41, 8, true);
//                    hglasstimer = HG_TIME1;
//                    hglassstate = HG_UP;
//                }
//                else{
//                    hglassstate = HG_IDLE;
//                    hglasstimer=0;
//                    hgdismiss = false;
//                }
//                break;    
//        }
//        dispatch_state = DPTH_THREADS; 
//    }   
//





          
            
            
//            kbdfinal = true;
//            kbdlocked = true;
//            if (kbdthiscode != kbdlastcode){
//                // key has changed or is noise
//                kbdlastcode = kbdthiscode;
//                kbdbouncecounter = KBDBOUNCE;
//                putchar('.');
//            }
//            else{
//                if (kbdbouncecounter != 0) kbdbouncecounter--;
//                putchar('!');
//                if (kbdbouncecounter==0 && !kbdlocked ){
//                    // Stable by more than 40 ms, confirm it
//                    // First, Reset Timer
//                    kbdtimer= KBDRELOAD;
//                    //i =  (kbdthiscode | (kbdscan & 0x0f)) - 17 ;
//                    //printf ("[%d]", i);
//                    kbdtblptr = (uint8_t*)(kbdtblinit + ((kbdthiscode | (kbdscan & 0x0f)) - 17)); 
//                    putchar('#');
//                    kbdcode = *kbdtblptr;
//                    kbdfinal = true;
//                    kbdlocked = true;
//                }
//            }   



//    #ifdef BAREMETAL
//        //PORTB = kbdscan & 0x0F;
//    #else
//        central->setScanline(kbdscan & 0x0F);
//    #endif
//    // End of Keyboard services




//glcd::glcd() {
//    
//    framepage = 0;
//    dspl_dispatch_fifo_head = 0;
//    dspl_dispatch_fifo_tail = 0;
//    dspl_dispatch_busy = false;
//    
//    //kbdlocked = false;
//    kbdptrs[0] = &kbdtbl0[0];
//	kbdptrs[1] = &kbdtbl1[0];
//	kbdptrs[2] = &kbdtbl2[0];
//	kbdptrs[3] = &kbdtbl3[0];
//	kbdptrs[4] = &kbdtbl4[0];
//	
//    polltimer = ICONPOOL;
//    
//    kbdscan= 0;
//    kbdtimer= 0;
//	kbdoffset = 0;
//	kbdpressed = false;
//	kbdnumeric = false;
//	kbdinsert = false;
//    
//    kbdpins.mode(PullDown);
//  
//    _initHardware();
//	
//}



//cvdesc_t * glcd::getCanvasDescriptor() { return &cvdesc;}
//void glcd::initCanvas(uint8_t rollback, uint8_t sig0, uint8_t sig1, uint8_t sig2, uint8_t sig3, uint8_t sig4, uint8_t sig5,
//							uint8_t shape, 
//							uint8_t slots){
//	
//	log(FINE, "Entering Canvas descriptor factoryr\r\n");
//	
//	cvdesc->usebkg=true;
//	cvdesc->slots=slots;
//	canvas_selected = 0;
//	cvdesc->shapes[0]=shape;
//	canvas_rollback=rollback;
//	canvas_signals[0]=sig0;
//	canvas_signals[1]=sig1;
//	canvas_signals[2]=sig2;
//	canvas_signals[3]=sig3;
//	canvas_signals[4]=sig4;
//	canvas_signals[5]=sig5;
//	
//	log(FINE, "Canvas descriptor was built\r\n");
//	
//}


//void glcd::initInput (uint8_t type, bool num, bool ins, bool hist,
//						char * line, char * svalue, 
//						uint8_t form,
//						double val, double lval, double hval){
//	inptdesc->type=type;
//	inptdesc->numeric=num;
//	inptdesc->insert=ins;
//	inptdesc->hashistory=hist;
//	sprintf(inptdesc->line1, line);
//	if (!num) sprintf(inptdesc->data, svalue);
//	inptdesc->format=0;
//	inptdesc->result=val;
//	inptdesc->limit_h=hval;
//	inptdesc->limit_l=lval;
//	
//}