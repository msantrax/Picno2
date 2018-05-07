
#include <vector>
#include <string>
#include <cstring>
#include <ctime>


#include "bsp.h"
#include "Properties.h"
#include "../glcd/glcd.h"
#include "Bundle.h"

#include "FreescaleIAP.h"


namespace blaine {


Properties::Properties() { 	
   
	// Cartão no slot
	//sd_drv = new SDFileSystem (PTE3, PTE1, PTE2, PTE4, "sd"); // MOSI, MISO, SCK, CS   
	
	// Cartão no 
	sd_drv = new SDFileSystem (PTD6, PTD7, PTD5, PTD4, "sd"); // MOSI, MISO, SCK, CS   
	
	
	flashaddr = flash_size() - SECTOR_SIZE;           //Write in last sector

	active=false;
	a_state = STANDBY;
	
	prop_pmap.push_back(pmaps(Properties::ACP  ,Properties::P_STRING     ,(char*)"prof_file"                 ,&profile_file[0]		,(char*)"acp.cfg")); 
	prop_pmap.push_back(pmaps(Properties::ACP  ,Properties::P_STRING     ,(char*)"language_file"             ,&language_file[0]		,(char*)"en-us.lan")); 
	prop_pmap.push_back(pmaps(Properties::ACP  ,Properties::P_STRING     ,(char*)"username"                  ,&username[0]		    ,(char*)"marcelo"));
	prop_pmap.push_back(pmaps(Properties::ACP  ,Properties::P_STRING     ,(char*)"password"                  ,&password[0]		    ,(char*)"f32Gt!3e7024ERg5$9907"));
	prop_pmap.push_back(pmaps(Properties::ACP  ,Properties::P_BOOLEAN    ,(char*)"logonstart"                ,&logonstart           ,(char*)"false"));
	prop_pmap.push_back(pmaps(Properties::ACP  ,Properties::P_STRING     ,(char*)"cur_time"                  ,&curtime[0]		    ,(char*)"03/19/16-15:07:27")); 
	
	addKeys(&prop_pmap);
	
	rawtime = convertTime(curtime);
    set_time(rawtime);
}

Properties::~Properties() {
}


std::vector<string> * Properties::dummyChoice(){
	
	std::vector<string> * ch_items = new std::vector<string>();
	ch_items->push_back("ACP-Sample 01");
	ch_items->push_back("Newstruct-0x452");
	ch_items->push_back("1234567890123456789");
	ch_items->push_back("ACP-Bestfit");
	ch_items->push_back("Antrax - 05:37:28");
	ch_items->push_back("Antrax - 05:37:28");
	ch_items->push_back("Antrax - 05:40:04");
	ch_items->push_back("Antrax - 05:47:15");
	
	return ch_items;	
}





void Properties::pushEvent(event_t evt){
	events.push(evt);
}


void Properties::serviceEvents(){
	
	uint8_t item_counter;
	char sbuf[64];
	
	while(!events.empty()){
		dummyevt = &events.front();
		
		if (dummyevt->evtype == KEYCODE_SIG){
			tempkcode = dummyevt->simple;
		}
		else if (dummyevt->evtype == ALARM_SIG){
			alarmset[dummyevt->simple]=true;
		}
		else if (dummyevt->evtype == INPUTANSWER_SIG){
			inputavailable = true;
			inputanswer_evt * ia = static_cast<inputanswer_evt*>(dummyevt->payload);
			inputval = ia->val;
			//if (inputval == 0.0){
				std::strncpy (&inputsval[0], ia->sval, 30);
				inputsval[31]=0x0;
			//}
			delete(ia);
		}
		else if (dummyevt->evtype == CHOICEANSWER_SIG){
			choiceavailable = true;
			choiceanswer=dummyevt->simple;
		}
		else if (dummyevt->evtype == DIALOGANSWER_SIG){
			dlgavailable = true;
			dlganswer=dummyevt->simple;
		}
		else if (dummyevt->evtype == LOADHISTORY_SIG){
			std::vector<string> * litems = loadHistory((char*)dummyevt->payload);
			if (litems->empty()){
				event_t evt(HISTORYANSWER_SIG, 1, dummyChoice());
				publish(evt);		
			}
			else{
				//event_t evt(HISTORYANSWER_SIG, 1, dummyChoice());
				event_t evt(HISTORYANSWER_SIG, 1, litems);
				publish(evt);
			}
		}
		else if (dummyevt->evtype == SAVEHISTORY_SIG){
			storeHistory((std::queue<string> *)dummyevt->payload);
		}
		else if (dummyevt->evtype == LOADPARAMS_SIG){
			addKeys((std::vector<pmaps> *)dummyevt->payload);
		}
		else if (dummyevt->evtype == ACTIVATEPROPERTIES_SIG){
			activate();
		}
		else if (dummyevt->evtype == LISTPROPERTIES_SIG){
			listKeys();
		}
		else if (dummyevt->evtype == LOADKEYS_SIG){
			loadkeys((char *)dummyevt->payload);
		}
		else if (dummyevt->evtype == DOCONFIG_SIG){
			a_state = SHOWOPTIONS;
			choicedepth = 0;
			header = 0;
		}
		else if (dummyevt->evtype == APPENDTOFILE_SIG){
			appendfile_evt * af = static_cast<appendfile_evt*>(dummyevt->payload);
			appendToFile (af->filepath, af->payload);
		}
	
		events.pop();
	}

	
	if (a_state == STANDBY){
		tempkcode = 0;
		inputavailable = false;
		dlgavailable = false;
		choiceavailable = false;
	}
	else if (a_state == ABORTCONFIG){
		publish(event_t(CONFIGDONE_SIG, 0, NULL));
		a_state = STANDBY;
		choicedepth = 0;
	}	
	else if (a_state == SHOWOPTIONS){	
		std::vector<string> * ch_items = new std::vector<string>();
		for (int i = 0; i < BUNDLEITEMS; i++) {
			if (bundle[i].clazz == choicedepth) ch_items->push_back(bundle[i].prompt);
		}
		publish(event_t(DRAWCHOICE_SIG, 0, new choice_evt(new std::string(bundle[header].msg), ch_items)));
		a_state=WAITOPTION;
	}
	else if (a_state == WAITOPTION){
		if(choiceavailable){
		    choiceavailable = false;
			if (choiceanswer == 255){
				a_state = ABORTCONFIG;
			}	
			else {
				item_counter=0;
				for (int i = 0; i < BUNDLEITEMS; i++) {
					//log(FINE, "Analysing item %d ->class %d / counter\r\n", i, );
					if (bundle[i].clazz == choicedepth){
						if (choiceanswer == item_counter){
							log(FINE, "Item %d on the list was selected : %s\r\n", i, bundle[i].prompt);							
							if (bundle[i].type == 2){
								// Dig one level down on menu
								header = i;
								choicedepth = bundle[i].pointer;
								a_state=SHOWOPTIONS;
							}
							else if (bundle[i].type == 1){
								// Edit parameter
								selecteditem = i;
								choicekey = findKey((char*)bundle[i].bkey);
								if (choicekey != NULL){
									a_state=EDITITEM;
								}
							}
							else if (bundle[i].type == 0){
								a_state=bundle[i].pointer;
							}
							break;
						}
						item_counter++;
					}	
				}
				if (item_counter == BUNDLEITEMS){
					log(FINE, "Could not find selected item : %d - counter = %d on choicedepth %d\r\n", choiceanswer, item_counter, choicedepth);
					a_state = ABORTCONFIG;
				}
			}	
		}
	}
	else if (a_state == EDITITEM){
		log(FINE, "Editing Item %s \r\n", choicekey->key);
		if (choicekey->keytype == P_BOOLEAN){
			log(FINE, "Is Boolean \r\n");
			publish(event_t(DRAWDIALOG_SIG, 0, new dialog_evt(1,
							FLAGYES,
							FLAGNO, 
							ABORTCONFIG, 
							new std::string(bundle[selecteditem].msg)
							)));
			a_state = WAITFLAG;
		}
		else if (choicekey->keytype == P_FLOAT){
			log(FINE, "Is Float \r\n");
			double * dblptr = (double *)choicekey->pvar;
			publish(event_t(DRAWINPUT_SIG, 0, new input_evt(SHP_INPUT5,
							true,false,false,false,
							new std::string(bundle[selecteditem].msg), 
							new std::string(""), 
							new std::string("%.3f "), 
							new std::string(""),
							*dblptr, 0.0, 0.0)));
			a_state = WAITINPUT;
		}
		else if (choicekey->keytype == P_STRING){
			log(FINE, "Is String \r\n");
			char * strptr = (char *)choicekey->pvar;
			log(FINE, "Is String %s\r\n", strptr);
			publish(event_t(DRAWINPUT_SIG, 0, new input_evt(SHP_INPUTFULL,
							false,false,false,false,
							new std::string(bundle[selecteditem].msg), 
							new std::string(strptr), 
							new std::string("%s "), 
							new std::string(""),
							0.0, 0.0, 0.0)));
			a_state = WAITSTRING;
		}
		else{
			a_state=SHOWOPTIONS;
		}
	}
	else if (a_state == WAITFLAG){
		if(dlgavailable){
			//log(PLUMB, "RSD dialog answered %d \r\n", dlganswer);
		    dlgavailable = false;
			a_state = dlganswer;
		}		
	}
	else if (a_state == FLAGYES){
		//log(FINE, "Setting %s to true\r\n", choicekey->key);
		bool * temp2ptr = (bool *)choicekey->pvar;
		*temp2ptr = true;
		a_state=SHOWOPTIONS;
	}
	else if (a_state == FLAGNO){
		//log(FINE, "Setting %s to false\r\n", choicekey->key);
		bool * temp2ptr = (bool *)choicekey->pvar;
		*temp2ptr = false;
		a_state=SHOWOPTIONS;
	}
	else if (a_state == WAITINPUT){
		if(inputavailable){
			inputavailable = false;
			double * temp2ptr = (double *)choicekey->pvar;
			*temp2ptr = inputval;
			a_state=SHOWOPTIONS;
		}		
	}
	else if (a_state == WAITSTRING){
		if(inputavailable){
			inputavailable = false;
			char * strptr = (char *)choicekey->pvar;
			std::strcpy(strptr, &inputsval[0]);
			a_state=SHOWOPTIONS;
		}		
	}
	else if (a_state == WAITSTRING){
		if(inputavailable){
			inputavailable = false;
			char * strptr = (char *)choicekey->pvar;
			std::strcpy(strptr, &inputsval[0]);
			a_state=SHOWOPTIONS;
		}		
	}
	else if (a_state == SAVEPROFILE){
		log(FINE, "Saving Profile\r\n");
		storeKeys(true, (char*)"poliperm.cfg");
		choicekey = findKey((char*)"prof_file");
		char * strptr = (char *)choicekey->pvar;
		storeKeys(false, strptr);
		a_state=SHOWOPTIONS;	
	}
	else if (a_state == LOADPROFILE){
		log(FINE, "Loading Profile\r\n");
		loadkeys((char*)"poliperm.cfg");
		choicekey = findKey((char*)"prof_file");
		char * strptr = (char *)choicekey->pvar;
		loadkeys((char*) strptr);
		a_state=SHOWOPTIONS;	
	}
	else if (a_state == SETDATETIME){	
		//time (&rawtime);
		timeinfo = localtime (&rawtime);
		//set_time(rawtime);
		log(FINE, "Setting Time %u\r\n", rawtime);
		strftime(sbuf, 64, "%d/%m/%y-%T", timeinfo);
		sbuf[63] = 0x0;
		publish(event_t(DRAWINPUT_SIG, 0, new input_evt(SHP_INPUTFULL,
							true,false,false,false,
							new std::string("Set Current Time :"), 
							new std::string(&sbuf[0]), 
							new std::string(""), 
							new std::string(""),
							0.0, 0.0, 0.0)));
		inputavailable = false;
		a_state=WAITDATETIME;	
	}
	else if (a_state == WAITDATETIME){
		if(inputavailable){
			inputavailable = false;
			log(FINE, "Input dlg Time returned %s\r\n", inputsval);
			rawtime = convertTime(inputsval);
			set_time(rawtime);
			rtcSetTime (rawtime);
			
			time (&rawtime);
			timeinfo = localtime (&rawtime);
			strftime(&curtime[0], 32, "%d/%m/%y-%T", timeinfo);
			curtime[31] = 0x0;
			a_state=SHOWOPTIONS;
		}		
	}	
}


time_t Properties::convertTime(char * fmttime){
	
	int d,m,y,h,min,s;
	struct tm * ti;
	time_t rt;
	
	ti =localtime (&rawtime);
	
	sscanf (fmttime, "%d%*c%d%*c%d%*c%d%*c%d%*c%d", &d,&m,&y,&h,&min,&s);
	ti->tm_sec=s;
	ti->tm_min=min;
	ti->tm_hour=h;
	ti->tm_mday=d;
	ti->tm_mon=m-1;
	ti->tm_year=y+100;
	
	rt=mktime(ti);
	
	return rt;
}


bool Properties::isActive(){ return active;}


void Properties::activate(){
	
	// Verify sd on board
	
	
//	if (sd_drv->card_type() == SDFileSystem::CARD_UNKNOWN){
//		printf("No SD card on board\n\r");
//		
//		//sd_drv = new SDFileSystem (PTE3, PTE1, PTE2, PTE4, "sds"); // MOSI, MISO, SCK, CS
//		//if (sd_drv->card_type() == SDFileSystem::CARD_UNKNOWN){
//			//printf ("No SD card on Shield\n\r");
//			active = false;
//	}
//	else{
//			active = testFS();
//	}
	
	active = testFS();
	
	//publish(event_t(UPDATEICON_SIG, 0, new icon_evt(6, glcd::ICREFRESH, SHP_ICON_FLASH)));
	
	if (active){
		log (INFO,"SDCard present -> properties are available.\n\r");
		loadkeys((char*)"poliperm.cfg");
		choicekey = findKey((char*)"prof_file");
		char * strptr = (char *)choicekey->pvar;
		loadkeys((char*) strptr);		
		//rawtime = convertTime(inputsval);
		set_time(1458458820);
		//log(FINE, "Init time : %s -> %u", inputsval, rawtime);	
		//publish(event_t(UPDATEICON_SIG, 0, new icon_evt(6, glcd::ICREFRESH, SHP_ICON_SDCARD_ON)));
	}
	else{
		log (WARNING,"No SDCard detected -> trying to load from flash.\n\r");
		if (!loadFlash()) storeFlash();
	}
	
	
	
}

bool Properties::loadFlash(){
	
	uint8_t cursor;
	int varlenght;
	int *fdata = (int*)flashaddr;
	
	cursor = _locateKey("startvar");
	char * varinitptr = (char *)pkeysv.at(cursor);
	cursor = _locateKey("endvar");
	char * varendptr = (char *)pkeysv.at(cursor);
	varlenght = (varendptr-varinitptr);
	
	if (fdata[0] == -1){
		log (WARNING,"No data on flash also, storing defaults from %p to %p (%d bytes)\n\r", varinitptr, varendptr, varlenght);
		return false;
	}
	else{
		log (WARNING,"Loading from flash (%p to %p (%d bytes))\n\r", varinitptr, varendptr, (varendptr-varinitptr));
		memcpy (varinitptr, &flashaddr, sizeof(char)*varlenght);
		//listKeys();
	}
	
	return true;
}


void Properties::storeFlash(){

	uint8_t cursor;
	int varlenght;
	
	cursor = _locateKey("startvar");
	char * varinitptr = (char *)pkeysv.at(cursor);
	cursor = _locateKey("endvar");
	char * varendptr = (char *)pkeysv.at(cursor);
	varlenght = (varendptr-varinitptr);
	
	erase_sector(flashaddr);
	program_flash(flashaddr,varinitptr, varlenght);	
	
}

// ========================================================
bool Properties::filterLine (char * in_string){
    
    if ((*in_string == '\n')|(*in_string == '#')) return false;
    
    while( *in_string !=0){
        if ((*in_string == '\n') | (*in_string == '\r')){
            *in_string = 0x00;    
        }
        in_string++;
    }
    return true;    
}

bool Properties::loadkeys (char * file){
	
	char line[150];
    //char * cstr = new char [150];
    uint8_t cursor;
    keycount = 0;
	uint8_t lines =0;
	
	FILE *fp = fopen(buildPath(file), "r");
    if(fp == NULL) {
        log(WARNING,"Failed to load properties keys\n\r");
		fclose(fp);
        return false;
    }
	else{
		while (fgets (line , 150 , fp) != NULL ){
			
			//PDEBUG1("Reading line:%s\n\r", &line[0]);
			
			if (filterLine(&line[0])){	
				std::vector<std::string>v = explode(&line[0], '=');
				if (v.size() !=2){
					log (WARNING,"Found bad key @ line %2d\n\r", lines);
				}
				else{
					cursor = _locateKey((char*)v.at(0).c_str());
					if (cursor != 255){
						//PDEBUG2("Storing key %s at cursor %0d\n\r", &line[0], cursor);
						updateKey(pkeysv.at(cursor),(char *)v.at(1).c_str());
						keycount++;		
					}
					else{
						log (WARNING,"Strange key found: %s \n\r", v.at(0).c_str());
					}
				}
			}
			lines++;
		}
		
		log (FINE,"Loaded %2d keys from %s\n\r", keycount, file);
		fclose(fp);
        return true;
	}
}


uint8_t Properties::_locateKey(char * key){
	
	uint8_t count =0 ;
	char * temp;
	pmaps * pmapsptr;
	
	for (std::vector<pmaps *>::iterator it = pkeysv.begin() ; it != pkeysv.end(); ++it){
		pmapsptr = *it;
		temp = pmapsptr->key;
		if (strcmp(temp, key) == 0){
			// Key was found
			//PDEBUG2("Locatekey found %s @ count %0d\n\r", key, count);
			return count;
		}
		count++;
	}
	
	return 255;
} 



std::vector<string> * Properties::loadHistory (char * file){
	
	char line[150];
	std::vector<string> *sitems = new std::vector<string>;
	uint8_t count;

	FILE *fp = fopen(buildPath(file), "r");
    if(fp == NULL) {
        log (WARNING,"Failed to open file %s to load history\n\r", file);
		fclose(fp);
        return sitems;
    }
	else{
		count = 0;
		while (fgets (line , 150 , fp) != NULL ){
			if (filterLine(&line[0])){	
				sitems->push_back(&line[0]);
				count++;
			}
		}		
		//log (FINE,"Loaded history with %d items from %s\n\r", count, file);
		fclose(fp);
        return sitems;
	}
}


void Properties::appendToFile (string * file, string * payload){
		
	FILE *fp = fopen(buildPath((char*)file->c_str()), "a");
    if(fp == NULL) {
        log (WARNING,"Failed to open file to append string\n\r");			
    }
	else{
		fputs((char*)payload->c_str(), fp);
		log(FINE, "Appending to %s -> %s", (char*)file->c_str(), (char*)payload->c_str() );
	}
	fclose(fp);
	delete file;
	delete payload;	
}


bool Properties::storeHistory(std::queue<string> * items){
	
	std::string stemp;
	uint8_t icount;
	string outfile = items->front();
	items->pop();
	
	FILE *fp = fopen(buildPath((char *)outfile.c_str()), "w");
    if(fp == NULL) {
        log (WARNING,"Failed to open file to store history\n\r");
		fclose(fp);
		delete items;
        return false;
    }
	
	icount = 0;
	while (!items->empty()){
		stemp = items->front();
		items->pop();
		stemp.append("\r\n");
		fputs(stemp.c_str(), fp);
		icount++;
	}
	
	log (FINE,"Stored history with %d items to %s\n\r", icount, outfile.c_str());
	fclose(fp);
	delete items;
    return true;
	
}


bool Properties::storeKeys (bool root, char * file){

	uint8_t count =0 ;
	char  * temp;
	char buf[128];
	pmaps * pmapsptr;
	
	FILE *fp = fopen(buildPath(file), "w");
    if(fp == NULL) {
        log (WARNING,"Failed to open file to store\n\r");
		fclose(fp);
        return false;
    }
	
	for (std::vector<pmaps *>::iterator it = pkeysv.begin() ; it != pkeysv.end(); ++it){
		pmapsptr = *it;
		temp = pmapsptr->key;
		if ((root == true) && (pmapsptr->keyrealm == ACP)){
			buildStoreEntry (temp, (char *)"0", &buf[0]);
			fputs(buf, fp);
			count++;
		}
		else if ((root == false) && (pmapsptr->keyrealm != ACP)){
			buildStoreEntry (temp, (char *)"0", &buf[0]);
			fputs(buf, fp);
			count++;		
		}		
	}
	log (FINE,"Stored %0d keys to %s\n\r", count, file);
	fclose(fp);
    return true;
}


void Properties::updateKey(pmaps * key, char * value){
	
	
	//log(FINE, "Adding key %p\n\r", key);
	
	if (key->keytype == P_FLOAT){
		double * temp2ptr = (double *)key->pvar;
		*temp2ptr = std::atof(value);
		//printf("[D@%p] %s = %9.3f \n\r", key, key->key, *temp2ptr);	
	}
	else if (key->keytype == P_BOOLEAN){
		bool * temp2ptr = (bool *)key->pvar;
		if (strcmp(value, "true") == 0){
			*temp2ptr = true;	
		}
		else if (strcmp(value, "false") == 0) {
			*temp2ptr = false;
		}
		else{
			log (WARNING,"Boolean key is not true nor false\n\r");			
		}
		//printf("[B@%p] %s = %d\n\r", key, key->key, *temp2ptr );	
	}
	else if (key->keytype == P_STRING){
		char * temp2ptr = (char *)key->pvar;
		std::strcpy(temp2ptr, value);
		//printf("[S@%p] %s = %s\n\r", key, key->key, temp2ptr );	
	}
}

bool Properties::addKeys (std::vector<pmaps> * keys){

	for (uint i = 0; i < keys->size(); i++) {
		pmaps * item;
		item = &keys->at(i);
		pkeysv.push_back(item);	
		updateKey (item, item->pstring);
	}

	return true;
}

bool Properties::addKey (pmaps * key){
	pkeysv.push_back(key);
	updateKey (key, key->pstring);
	return true;
}

pmaps * Properties::findKey (char * keytofind){
	
	for (std::vector<pmaps *>::iterator it = pkeysv.begin() ; it != pkeysv.end(); ++it){		
		pmaps *ktemp = *it;
		if (strcmp(keytofind, ktemp->key) == 0){
			return ktemp;
		}	
	}
	return NULL;
}


void Properties::listKeys(){
	
	uint8_t count = 0;
	
	log(INFO, "DOUBLE KEYS :\r\n");
	count = 0;
	for (std::vector<pmaps *>::iterator it = pkeysv.begin() ; it != pkeysv.end(); ++it){		
		pmaps *ktemp = *it;
		if (ktemp->keytype == Properties::P_FLOAT){
			double dtemp = *(double*)ktemp->pvar;
			log(INFO, "Key %02d -[%02d]:%-32s %p = %f\r\n", count, ktemp->keyrealm, ktemp->key, (double*)ktemp->pvar, dtemp );
		}
		count++;	
	}
	
	log(INFO, "BOOLEAN KEYS :\r\n");
	count = 0;
	for (std::vector<pmaps *>::iterator it = pkeysv.begin() ; it != pkeysv.end(); ++it){		
		pmaps *ktemp = *it;
		if (ktemp->keytype == Properties::P_BOOLEAN){
			bool btemp = *(bool*)ktemp->pvar;
			log(INFO, "Key %02d -[%02d]:%-32s %p = %s\r\n", count, ktemp->keyrealm, ktemp->key, (double*)ktemp->pvar, btemp == 1 ? "true":"False");
		}
		count++;	
	}
	
	log(INFO, "STRING KEYS :\r\n");
	count = 0;
	for (std::vector<pmaps *>::iterator it = pkeysv.begin() ; it != pkeysv.end(); ++it){		
		pmaps *ktemp = *it;
		if (ktemp->keytype == Properties::P_STRING){
			char * stemp = (char *)ktemp->pvar;
			log(INFO, "Key %02d -[%02d]:%-32s %p = %s\r\n", count, ktemp->keyrealm, ktemp->key, (double*)ktemp->pvar, stemp);
		}
		count++;	
	}	
}


char * Properties::buildPath(char * path){
    
    strcpy(path_buf, SDPREFIX);
    strcat(path_buf, path);    
    return &path_buf[0];
}

bool Properties::testFS(){
   
    //FILE *fp = fopen("/Bascon/Picnometro/sd/picno/main.cfg", "r");
    FILE *fp = fopen(buildPath((char *)"marker.dat"), "w");
    if(fp == NULL) {
        //log(WARNING, "SD Card not available\n\r");
		fclose(fp);
        return false;
    }
	else{
		//log(INFO, "SD Card present\n\r");
		fclose(fp);
        return true;
	}
}


const std::vector<std::string> Properties::explode(char * it, const char& c) {
    
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



double Properties::getDoubleParam(char * lkey, double defval){
	
	uint8_t cursor;
	double * temp;
	
	cursor = _locateKey(lkey);
	
	if(cursor != 255){
		// Key located, do the conversion
		temp = (double *) pkeysv.at(cursor)->pvar;
		//PDEBUG2("Getting key %s = %f\n\r", pkeysv.at(cursor)->key, *temp );		
		return  *temp;
	}
	else{
		log (WARNING,"Failed to locate key %s\n\r", lkey);
		return defval;
	}
}

bool Properties::getBooleanParam (char * lkey, bool defval){
	
	uint8_t cursor;
	bool * temp;
	
	
	cursor = _locateKey(lkey);

	if(cursor != 255){
		temp = (bool *) pkeysv.at(cursor)->pvar;
		//PDEBUG2("Getting key %s = %f\n\r", pkeysv.at(cursor)->key, *temp );		
		return  *temp;
	}
	else{
		log (WARNING,"Failed to locate key %s\n\r", lkey);
		return defval;
	}
}

char * Properties::getStringParam (char * lkey, char * defval){
	
	uint8_t cursor;
	char * temp;
	
	cursor = _locateKey(lkey);
	
	if(cursor != 255){
		temp = (char *) pkeysv.at(cursor)->pvar;
		//PDEBUG2("Getting key %s = %f\n\r", pkeysv.at(cursor)->key, *temp );		
		return temp;
	}
	else{
		log (WARNING,"Failed to locate key %s\n\r", lkey);
		return defval;
	}
}	


bool Properties::buildStoreEntry (char * lkey, char * defval, char * out){
	
	uint8_t cursor;
	
	cursor = _locateKey(lkey);
	
	if(cursor != 255){
		if (pkeysv.at(cursor)->keytype == P_FLOAT){
			double * temp2ptr = (double *)pkeysv.at(cursor)->pvar;
			sprintf (out , "%s=%f\n", lkey , *temp2ptr);
			//PDEBUG2("Double key %s updated to %6.3f \n\r", key->key, *temp2ptr);	
		}
		else if (pkeysv.at(cursor)->keytype == P_BOOLEAN){
			bool * temp2ptr = (bool *)pkeysv.at(cursor)->pvar;
			if (*temp2ptr == true){
				sprintf (out , "%s=true\n", lkey);	
			}
			else{
				sprintf (out , "%s=false\n", lkey);
				//PDEBUG1("Boolean key %s is not true nor false\n\r", lkey);			
			}
			//PDEBUG2("Double key %s updated to %d \n\r", key->key, *temp2ptr);	
		}
		else if (pkeysv.at(cursor)->keytype == P_STRING){
			char * temp2ptr = (char *)pkeysv.at(cursor)->pvar;
			sprintf (out , "%s=%s\n", lkey, temp2ptr);
			//PDEBUG3("String key %s updated from %s to %s \n\r", key->key, value,  temp2ptr);	
		}
	}
	else{
		log (WARNING,"Failed to locate key %s\n\r", lkey);
		return false;
	}
	return true;
}	



}














//void Properties::testFS(){PDEBUG2("Line %2d = %s\n\r", count, cstr);
//   
//    char line[150];
//    char * cstr = new char [150];
//    uint8_t count = 0;
//   
//    PDEBUG("Testing file system\n\r");  
//    
//    //FILE *fp = fopen("/Bascon/Picnometro/sd/picno/main.cfg", "r");
//    FILE *fp = fopen(buildPath("config.dat"), "r");
//    if(fp == NULL) {
//        PDEBUG("Could not open file\n\r");
//        return;
//    }
//    
//    
//    while (fgets (line , 150 , fp) != NULL ){
//        if (filterLine(&line[0])){
//            std::vector<std::string>v = explode(&line[0], ':');
//            for (std::vector<std::string>::iterator it = v.begin() ; it != v.end(); ++it){
//                std::strcpy (cstr, it->c_str());
//                PDEBUG2("Line %2d = %s\n\r", count, cstr);
//            }
//        } 
//        count++;             
//    }
//    
//    fclose(fp);
//    delete[] cstr;
//    
//    PDEBUG("\n\rTest finalized ... \n\r");  
//}



//#include "SDFileSystem.h"
//
//SDFileSystem sd(PTE3, PTE1, PTE2, PTE4, "sd"); // MOSI, MISO, SCK, CS
//Serial pc(USBTX, USBRX);
//FILE *fp;
//
//int file_copy(const char *src, const char *dst)
//{
//    int retval = 0;
//    int ch;
// 
//    FILE *fpsrc = fopen(src, "r");   // src file
//    FILE *fpdst = fopen(dst, "w");   // dest file
//    
//    while (1) {                  // Copy src to dest
//        ch = fgetc(fpsrc);       // until src EOF read.
//        if (ch == EOF) break;
//        fputc(ch, fpdst);
//    }
//    fclose(fpsrc);
//    fclose(fpdst);
//  
//    fpdst = fopen(dst, "r");     // Reopen dest to insure
//    if (fpdst == NULL) {          // that it was created.
//        retval = -1;           // Return error.
//    } else {
//        fclose(fpdst);
//        retval = 0;              // Return success.
//    }
//    return retval;
//}
//
//uint32_t do_list(const char *fsrc)
//{
//    DIR *d = opendir(fsrc);
//    struct dirent *p;
//    uint32_t counter = 0;
//
//    while ((p = readdir(d)) != NULL) {
//        counter++;
//        printf("%s\n", p->d_name);
//    }
//    closedir(d);
//    return counter;
//}
//
//// bool is_folder(const char *fdir)
//// {
////     DIR *dir = opendir(fdir);
////     if (dir) {
////         closedir(dir);
////     }
////     return (dir != NULL);
//
//// }
//
//// bool is_file(const char *ffile)
//// {
////     FILE *fp = fopen(ffile, "r");
////     if (fp) {
////         fclose(fp);
////     }
////     return (fp != NULL);
//// }
//
//void do_remove(const char *fsrc)
//{
//    DIR *d = opendir(fsrc);
//    struct dirent *p;
//    char path[30] = {0};
//    while((p = readdir(d)) != NULL) {
//        strcpy(path, fsrc);
//        strcat(path, "/");
//        strcat(path, p->d_name);
//        remove(path);
//    }
//    closedir(d);
//    remove(fsrc);
//}
//
//int main()
//{
//    pc.printf("Initializing \n");
//    wait(2);
//
//    do_remove("/sd/test1"); /* clean up from the previous Lab 5 if was executed */
//    if (do_list("/sd") == 0) {
//        printf("No files/directories on the sd card.");
//    }
//
//    printf("\nCreating two folders. \n");
//    mkdir("/sd/test1", 0777);
//    mkdir("/sd/test2", 0777);
//
//    fp = fopen("/sd/test1/1.txt", "w");
//    if (fp == NULL) {
//        pc.printf("Unable to write the file \n");
//    } else {
//        fprintf(fp, "1.txt in test 1");
//        fclose(fp);
//    }
//
//    fp = fopen("/sd/test2/2.txt", "w");
//    if (fp == NULL) {
//        pc.printf("Unable to write the file \n");
//    } else {
//        fprintf(fp, "2.txt in test 2");
//        fclose(fp);
//    }
//
//    printf("\nList all directories/files /sd.\n");
//    do_list("/sd");
//
//    printf("\nList all files within /sd/test1.\n");
//    do_list("/sd/test1");
//
//
//    printf("\nList all files within /sd/test2.\n");
//    do_list("/sd/test2");
//
//    int status = file_copy("/sd/test2/2.txt", "/sd/test1/2_copy.txt");
//    if (status == -1) {
//        printf("Error, file was not copied.\n");
//    }
//    printf("Removing test2 folder and 2.txt file inside.");
//    remove("/sd/test2/2.txt");
//    remove("/sd/test2");
//
//    printf("\nList all directories/files /sd.\n");
//    do_list("/sd");
//
//    printf("\nList all files within /sd/test1.\n");
//    do_list("/sd/test1");
//
//    printf("\nEnd of complete Lab 5. \n");
//}
