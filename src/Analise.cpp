/* 
 * File:   Analise.cpp
 * Author: opus
 * 
 * Created on 23 de Fevereiro de 2016, 18:06
 */

#include "bsp.h"
#include "Properties.h"
#include "../glcd/glcd.h"
#include "../glcd/shapes.h"
#include "../glcd/Printer.h"

#include "Analise.h"

#include <string.h>

DigitalOut inputvalve_pin(PTB19); // db3
DigitalOut expandvalve_pin(PTB18); // db2
DigitalOut ventvalve_pin(PTC15); // db7
DigitalOut vaccumvalve_pin(PTC17); // db8
DigitalOut smallvalve_pin(PTC16); // db8

AnalogIn Temp(A0); // 
AnalogIn Press(A1); // 

namespace blaine {

    Analise::Analise() {

        a_state = STANDBY;

        set_time(1457888460);
        srand(time(NULL));

        initVars();
        
        spherevolumes.push_back(s_keymap("Esfera Certificada", "8.003"));
        spherevolumes.push_back(s_keymap("ACP-Beckman Large", "8.6004"));

        cellvolumes.push_back(s_keymap("[G] - Grande", "154.7174"));
        cellvolumes.push_back(s_keymap("[M] - Media", "131.7"));
        cellvolumes.push_back(s_keymap("[P] - Pequena", "10.8"));

    }

    void Analise::initVars(){
        
        apmap.erase(apmap.begin(), apmap.end());
        
        
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_BOOLEAN, (char*) "startvar", &dummyvar, (char*) "false"));

        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_FLOAT, (char*) "acode", &acode, (char*) "4556"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_FLOAT, (char*) "user", &user, (char*) "-1"));


        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_BOOLEAN, (char*) "usesequencer", &usesequencer, (char*) "false"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_STRING, (char*) "sidhistoryfile", &sidhistoryfile[0], (char*) "opus.hst"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_BOOLEAN, (char*) "storehistory", &storehistory, (char*) "true"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_BOOLEAN, (char*) "autostore", &autostore, (char*) "false"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_BOOLEAN, (char*) "asktostore", &asktostore, (char*) "false"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_STRING, (char*) "storefile", &storefile[0], (char*) "results1.csv"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_BOOLEAN, (char*) "autoprint", &autoprint, (char*) "false"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_BOOLEAN, (char*) "asktoprint", &asktoprint, (char*) "false"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_BOOLEAN, (char*) "print_runs", &printruns, (char*) "false"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_BOOLEAN, (char*) "print_extra_data", &printextra, (char*) "false"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_BOOLEAN, (char*) "print_qc", &printqc, (char*) "false"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_STRING, (char*) "header_type", &printheader, (char*) "default"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_STRING, (char*) "time_format", &timeformat, (char*) "%a %b %d %Y - %T"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_BOOLEAN, (char*) "eject_print_report", &ejectprint, (char*) "true"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_STRING, (char*) "sampleid", &sampleid[0], (char*) "ACP-Instrumens 01"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_BOOLEAN, (char*) "prefernumeric", &prefernumeric, (char*) "true"));


        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_BOOLEAN, (char*) "monitor_on_term", &monitor_on_term, (char*) "false"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_BOOLEAN, (char*) "monitor_on_file", &monitor_on_file, (char*) "false"));

        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_FLOAT, (char*) "tempa0", &tempa0, (char*) "8.0766"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_FLOAT, (char*) "tempa1", &tempa1, (char*) "159.6169"));

        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_FLOAT, (char*) "pressure_a0", &pa0, (char*) "-2.98081"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_FLOAT, (char*) "pressure_a1", &pa1, (char*) "29.7486241"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_FLOAT, (char*) "pressure offset", &poffset, (char*) "0.035"));

        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_FLOAT, (char*) "vent-pressure", &vent_tgtpressure, (char*) "0.8"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_FLOAT, (char*) "load-pressure", &load_tgtpressure, (char*) "17.0"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_FLOAT, (char*) "pulse-pressure", &pulse_tgtpressure, (char*) "0.0"));

        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_FLOAT, (char*) "load-trigfactor", &load_trigfactor, (char*) "0.555"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_FLOAT, (char*) "load-prate", &load_prate, (char*) "0.8"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_BOOLEAN, (char*) "use_rate_to_load", &useratetoload, (char*) "false"));

        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_BOOLEAN, (char*) "zero_transducer", &zerotransducer, (char*) "true"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_FLOAT, (char*) "zero_driftwstart", &zero_driftwstart, (char*) "5.0"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_FLOAT, (char*) "zero_driftwend", &zero_driftwend, (char*) "16.0"));

        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_FLOAT, (char*) "load_driftwstart", &load_driftwstart, (char*) "16.0"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_FLOAT, (char*) "load_driftwend", &load_driftwend, (char*) "50.0"));

        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_FLOAT, (char*) "abs_driftlimit", &absdriftlimit, (char*) "0.015"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_FLOAT, (char*) "rel_driftlimit", &reldriftlimit, (char*) "0.008"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_FLOAT, (char*) "equilibration_time", &equitime, (char*) "0.0"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_BOOLEAN, (char*) "blockon_driftfail", &blockondriftfail, (char*) "false"));

        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_FLOAT, (char*) "flush_type", &flushtype, (char*) "1.0"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_FLOAT, (char*) "flush_time", &flushtime, (char*) "10.0"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_FLOAT, (char*) "flush_cicles", &flushcicles, (char*) "5.0"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_FLOAT, (char*) "flush_cicle_time", &flushcicletime, (char*) "5.0"));

        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_FLOAT, (char*) "max_runs", &maxruns, (char*) "10.0"));

        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_FLOAT, (char*) "runs_averaged", &runsaveraged, (char*) "4.0"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_FLOAT, (char*) "analysis_rsd", &analysisrsd, (char*) "0.003"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_FLOAT, (char*) "info_gap", &infogap, (char*) "8.0")); //
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_BOOLEAN, (char*) "use_pratio", &usepratio, (char*) "true")); //

        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_FLOAT, (char*) "vadded", &vadded, (char*) "149.6678"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_FLOAT, (char*) "vadded_g", &vaddeds[0], (char*) "149.6678"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_FLOAT, (char*) "vadded_m", &vaddeds[1], (char*) "80.0"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_FLOAT, (char*) "vadded_p", &vaddeds[2], (char*) "12.0"));
        
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_FLOAT, (char*) "vcell", &vcell, (char*) "188.7066"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_FLOAT, (char*) "vcell_g", &vcells[0], (char*) "188.7066"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_FLOAT, (char*) "vcell_m", &vcells[1], (char*) "46.4619"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_FLOAT, (char*) "vcell_p", &vcells[2], (char*) "4.4420"));
        
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_FLOAT, (char*) "sphere_volume", &spherevol, (char*) "8.6004"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_FLOAT, (char*) "sphere_vol_g", &spherevols[0], (char*) "41.2"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_FLOAT, (char*) "sphere_vol_m", &spherevols[1], (char*) "26.8"));
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_FLOAT, (char*) "sphere_vol_p", &spherevols[2], (char*) "26.8"));
        
        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_FLOAT, (char*) "sample_weight", &mass, (char*) "67.943"));

        apmap.push_back(pmaps(Properties::BLAINEDEV, Properties::P_BOOLEAN, (char*) "endvar", &dummyvar, (char*) "false"));


        poffset = 0.0;
        usepoffset = false;
        userawcounts = false;
        monitorpressure = true;
        smallva = false;
        
        
    }
    
    
    
    Analise::Analise(const Analise& orig) {
    }

    Analise::~Analise() {
    }

    void Analise::serviceEvents() {

        //uint8_t ltemp;

        while (!events.empty()) {
            dummyevt = &events.front();

            if (dummyevt->evtype == KEYCODE_SIG) {
                tempkcode = dummyevt->simple;
            } else if (dummyevt->evtype == ALARM_SIG) {
                alarmset[dummyevt->simple] = true;
            } else if (dummyevt->evtype == INPUTANSWER_SIG) {
                inputavailable = true;
                inputanswer_evt * ia = static_cast<inputanswer_evt*> (dummyevt->payload);
                inputval = ia->val;
                //if (inputval == 0.0){
                std::strncpy(&inputsval[0], ia->sval, 30);
                inputsval[31] = 0x0;
                //}
                delete(ia);
            } else if (dummyevt->evtype == DIALOGANSWER_SIG) {
                dlgavailable = true;
                dlganswer = dummyevt->simple;
            } else if (dummyevt->evtype == CHOICEANSWER_SIG) {
                //log(INFO, "Choice Answer received ...\r\n");
                choiceavailable = true;
                choiceanswer = dummyevt->simple;
            }
            else if (dummyevt->evtype == INITANALISE_SIG) {
                a_state = INITPARAMS;
            } else if (dummyevt->evtype == DOANALISE_SIG) {
                a_state = DOANALISE;
            } else if (dummyevt->evtype == DOMANUAL_SIG) {
                a_state = INITMANUAL;
            } else if (dummyevt->evtype == DOVACAL_SIG) {
                a_state = DOVACAL;
            } else if (dummyevt->evtype == DOVCCAL_SIG) {
                a_state = DOVCCAL;
            }

            events.pop();
        }


        /// General Services ======================================================================================================

        if (a_state == STANDBY) {
            tempkcode = 0;
            inputavailable = false;
            dlgavailable = false;
        }
        else if (a_state == INITPARAMS) {
            event_t evt(LOADPARAMS_SIG, 0, (std::vector<pmaps> *) & apmap);
            publish(evt);
            a_state = STANDBY;
        }
        else if (a_state == ABORTRUN) {
            log(PLUMB, "Analysis canceled by user \r\n");
            resetAlarm(1);
            event_t evt(ANALISEDONE_SIG, 0, NULL);
            publish(evt);
            a_state = STANDBY;
        }
            // Manual services ========================================================================================================
        else if (a_state == INITMANUAL) {

            publish(event_t(GLCDRESET_SIG, 0, NULL));

            flushValves();
            buildIconBar(0);
            drawShapeRequest(SHP_MANOP_CANVAS, 0U, 0U, false);
            buildBarGraphs();
            publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(04, 8, 120, true, false, true, 1, new std::string("Aguard. - Press\x83 o Bruta"))));
            sensor_state = S_IDLE;
            a_state = RUNMANUAL;
        }
        else if (a_state == RUNMANUAL) {

            if (tempkcode == 0x08) {
                a_state = ABORTRUN;
            }                //Enter
            else if (tempkcode == 13) {
                if (man_iconptr == 4) {
                    // Work submenu (was zero sensor icon ...)
                    std::vector<string> * ch_items = new std::vector<string>();
                    ch_items->push_back(std::string("Full Cycle"));
                    ch_items->push_back(std::string("Flush Cell"));
                    ch_items->push_back(std::string("Zero Transducer"));
                    ch_items->push_back(std::string("Monitor Temperature"));
                    ch_items->push_back(std::string("Monitor Pressure"));
                    ch_items->push_back(std::string("Use Raw Counts"));
                    ch_items->push_back(std::string("Use Calibrated values"));
                    ch_items->push_back(std::string("Export to Terminal"));
                    publish(event_t(DRAWCHOICE_SIG, 0, new choice_evt(new std::string("Selecione Servi\x81 o"), ch_items)));
                    a_state = WAITSERVICEOPTION;
                } else if (man_iconptr == 5) {
                    // Log Data
                } else {
                    if (valves[man_iconptr] == 0) {
                        activateValve(man_iconptr, 1);
                        man_icons[man_iconptr] = man_icons[man_iconptr] | 0x01;
                        //log(PLUMB, "Changing valve %d (was 0 now is %d) \r\n", man_iconptr, valves[man_iconptr]);
                    } else if (valves[man_iconptr] == 1) {
                        activateValve(man_iconptr, 0);
                        man_icons[man_iconptr] = man_icons[man_iconptr] & 0xFE;
                        //log(PLUMB, "Changing valve %d (was 1 now is %d) \r\n", man_iconptr, valves[man_iconptr]);
                    }
                    publish(event_t(UPDATEICON_SIG, 0, new icon_evt(man_iconptr, glcd::ICREFRESH, man_icons[man_iconptr])));
                    //flushValves();
                }
            }                // Left
            else if (tempkcode == 18) {
                if (man_iconptr > 0) {
                    man_icons[man_iconptr] = man_icons[man_iconptr] & 0xFD;
                    publish(event_t(UPDATEICON_SIG, 0, new icon_evt(man_iconptr, glcd::ICREFRESH, man_icons[man_iconptr])));
                    man_iconptr--;
                    man_icons[man_iconptr] = man_icons[man_iconptr] | 0x02;
                    publish(event_t(UPDATEICON_SIG, 0, new icon_evt(man_iconptr, glcd::ICREFRESH, man_icons[man_iconptr])));
                }
            }                //Right
            else if (tempkcode == 19) {
                if (man_iconptr < 5) {
                    man_icons[man_iconptr] = man_icons[man_iconptr] & 0xFD;
                    publish(event_t(UPDATEICON_SIG, 0, new icon_evt(man_iconptr, glcd::ICREFRESH, man_icons[man_iconptr])));
                    man_iconptr++;
                    man_icons[man_iconptr] = man_icons[man_iconptr] | 0x02;
                    publish(event_t(UPDATEICON_SIG, 0, new icon_evt(man_iconptr, glcd::ICREFRESH, man_icons[man_iconptr])));
                }
            } else {
                SensorSM(S_IDLE_SIG);
            }
            tempkcode = 0;
        }
        else if (a_state == WAITSERVICEOPTION) {
            if (choiceavailable) {
                choiceavailable = false;
                //log(INFO, "Choice returned %d\r\n", choiceanswer);		
                if (choiceanswer == 255) {
                    a_state = RUNMANUAL;
                }
                else if (choiceanswer == 0) {
                    clearHistory();
                    lruns.clear();
                    setInitTick();
                    default_history = RUNMANUAL;
                    state_history.push(RUNMANUAL);
                    cycles = 0;
                    startingcycle = true;
                    rsdbypressure = true;
                    flip = false;
                    a_state = DOCYCLE;
                } else if (choiceanswer == 1) {
                    clearHistory();
                    default_history = RUNMANUAL;
                    state_history.push(RUNMANUAL);
                    state_history.push(FLUSHCELL);
                    a_state = state_history.top();
                    state_history.pop();
                }
                else if (choiceanswer == 2) {
                    clearHistory();
                    SensorSM(S_ZERO_SIG);
                    default_history = RUNMANUAL;
                    state_history.push(RUNMANUAL);
                    state_history.push(MEASUREZERO);
                    publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(04, 8, 120, true, false, true, 1, new std::string("Zerando Transdutor"))));

                    a_state = state_history.top();
                    state_history.pop();
                }
                else if (choiceanswer == 3) {
                    // Monitor temperature
                    monitorpressure = false;
                    publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(04, 8, 120, true, false, true, 1, new std::string("Lendo Temperatura"))));
                    a_state = RUNMANUAL;
                } else if (choiceanswer == 4) {
                    // Monitor pressure
                    monitorpressure = true;
                    publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(04, 8, 120, true, false, true, 1, new std::string("Lendo Press\x83 o"))));
                    a_state = RUNMANUAL;
                } else if (choiceanswer == 5) {
                    // Use Raw counts
                    userawcounts = true;
                    publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(04, 8, 120, true, false, true, 1, new std::string("Sinal Bruto"))));
                    a_state = RUNMANUAL;
                } else if (choiceanswer == 6) {
                    // Use calibrated values
                    userawcounts = false;
                    publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(04, 8, 120, true, false, true, 1, new std::string("Valores Calibrados"))));
                    a_state = RUNMANUAL;
                } else if (choiceanswer == 7) {
                    // Export to Terminal
                    if (monitor_on_term) {
                        monitor_on_term = false;
                        publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(04, 8, 120, true, false, true, 1, new std::string("Term. Monitor Desl."))));
                        a_state = RUNMANUAL;
                    } else {
                        monitor_on_term = true;
                        publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(04, 8, 120, true, false, true, 1, new std::string("Term. Monitor Lig."))));
                        a_state = RUNMANUAL;
                    }
                }

                //a_state = RUNMANUAL;
                tempkcode = 0;
            }
        }
        
        // Analysis Services ======================================================================================================

        else if (a_state == DOANALISE) {

            publish(event_t(GLCDRESET_SIG, 0, NULL));

            flushValves();
            buildIconBar(1);
            drawShapeRequest(SHP_ANALISE_CANVAS, 0U, 0U, false);
            buildBarGraphs();
            publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(04, 8, 120, true, false, true, 1, new std::string("Analise de Rotina"))));

            clearHistory();
            lruns.clear();
            setInitTick();
            cycles = maxruns;
            //analysisrsd=0.003;
            startingcycle = true;
            rsdbypressure = true;
            flip = false;

            state_history.push(ABORTRUN);
            state_history.push(VENTCELL);

            state_history.push(STOREANALISE);
            state_history.push(UPDATEANALISE);
            state_history.push(DOCYCLE);
            state_history.push(ASKSAMPLEREADY);

            if (user != 0) {
                state_history.push(ASKFLOW);
                state_history.push(ASKMULTIRUN);
            }

            state_history.push(ASKSID);
            state_history.push(ASKSAMPLEWEIGHT);

            sensor_state = S_IDLE;
            a_state = ASKCELLTYPE;

            doingcal = false;
            
            //a_state = UPDATEANALISE;

        }
        else if (a_state == ASKSAMPLEWEIGHT) {
            publish(event_t(DRAWINPUT_SIG, 0, new input_evt(SHP_INPUT5,
                    true, false, false, false,
                    new std::string("Peso da\nAmostra"),
                    new std::string(""),
                    new std::string("%7.4f "),
                    new std::string(""),
                    mass, 0.00001, 200.0)));
            a_state = WAITSAMPLEWEIGHT;
        } else if (a_state == WAITSAMPLEWEIGHT) {
            if (inputavailable) {
                if (inputval == -1){
                    // Escape requested go back to select Sisze
                    tempkcode = 0;
                    inputavailable = false;
                    a_state = ASKCELLTYPE;
                    state_history.push(ASKSAMPLEWEIGHT);
                    
                    //state_history.pop();
                }
                else{
                    mass = inputval;
                    tempkcode = 0;
                    inputavailable = false;
                    a_state = state_history.top();
                    state_history.pop();
                }
            }
        }
        else if (a_state == LOADSID) {
            if (usesequencer) {
                loadAutoSID();
                a_state = state_history.top();
                state_history.pop();
            } else {
                a_state = ASKSID;
            }
        }
        else if (a_state == ASKSID) {
            publish(event_t(DRAWINPUT_SIG, 0, new input_evt(SHP_INPUTFULL,
                    false, false, false, storehistory,
                    new std::string("Identifica\x81 \x83 o da amostra"),
                    new std::string(&sampleid[0]),
                    new std::string("%s"),
                    new std::string(&sidhistoryfile[0]), 0.0, 0.0, 0.0)));
                    a_state = WAITSID;
                    
        } else if (a_state == WAITSID) {
            if (inputavailable) {
                if (inputval == -1){
                    // Escape requested go back to select Sisze
                    tempkcode = 0;
                    inputavailable = false;
                    a_state = ASKSAMPLEWEIGHT;
                    state_history.push(ASKSID);
                }
                else{
                    tempkcode = 0;
                    inputavailable = false;
                    std::strncpy(&sampleid[0], &inputsval[0], 30);
                    a_state = state_history.top();
                    state_history.pop();
                }
            }
        }
        else if (a_state == ASKFLOW) {
            std::vector<string> * ch_items = new std::vector<string>();
            ch_items->push_back(std::string("N\x83 o use purga"));
            ch_items->push_back(std::string("Use Fluxo"));
            ch_items->push_back(std::string("Use Pulso"));
            ch_items->push_back(std::string("Use Vacuo"));

            publish(event_t(DRAWCHOICE_SIG, 0, new choice_evt(new std::string("Sel. Metodo de Purga"), ch_items)));
            a_state = WAITFLOW;
        }
        else if (a_state == WAITFLOW) {
            if (choiceavailable) {
                choiceavailable = false;
                if (choiceanswer == 255) {
                    a_state = ABORTRUN;
                    tempkcode = 0;
                } else {
                    flushtype = choiceanswer;
                    tempkcode = 0;
                    a_state = state_history.top();
                    state_history.pop();
                }
            }
        }

        else if (a_state == UPDATEANALISE) {

            uint8_t cnt = 0;
            log(INFO, "\n\rAnalysis report : \n\r");
            log(INFO, "Mass=%7.4f - VC=%7.4f - VA=%7.4f - Good after %d of %d cycles\n\r", mass, vcells[celltype], vaddeds[celltype], lruns.size(), cycles);
            for (std::vector<loadrun>::iterator it = lruns.begin(); it != lruns.end(); ++it) {
                
                it->volume = calculateVP(it->cycleavg);
                it->density = calculateDEN(it->volume);
                
                log(INFO, "%2d, %s, %6.4f, %6.4f, %6.4f, %6.4f, %5.3f, %d, %8.7f, %6.4f, %8.6f, %8.6f\r\n",
                        cnt, getFormatedTime("", it->timestamp),
                        it->loadpressure, 
                        it->expandpressure, 
                        it->deltap, 
                        it->pratio,
                        it->temperature, 
                        it->loadstatus,
                        it->cycleavg, 
                        it->cyclersd,
                        it->volume, 
                        it->density
                        );
                cnt++;
            }

            finalresptr = lruns.size() - 1;
            publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(04, 8, 120, true, false, true, 1, new std::string("RESULTADOS FINAIS"))));
            publish(event_t(DRAWSHAPE_SIG, 0, new drawshape_evt(SHP_WORKA_CLEAR, 0, 24, true)));
            publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(0, 24, 124, false, false, false, 3, getFinalString(0, lruns.size() - 1, true))));
            publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(0, 32, 124, false, false, false, 3, getFinalString(1, lruns.size() - 1, true))));
            publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(0, 40, 124, false, false, false, 1, getFinalString(2, lruns.size() - 1, true))));

            a_state = WAITENDANALISE;
        }
        else if (a_state == WAITENDANALISE) {

            if (tempkcode == 0x08) {
                state_history.push(UPDATEANALISE);
                a_state = ASKSAMPLEWEIGHT;
            }                //Enter
            else if (tempkcode == 13) {
                publish(event_t(DRAWSHAPE_SIG, 0, new drawshape_evt(SHP_WORKA_CLEAR, 255, 24, true)));
                a_state = state_history.top();
                state_history.pop();
            }                // Up
            else if (tempkcode == 0x11) {
                if (finalresptr == 0) {
                    finalresptr = lruns.size() - 1;
                } else {
                    finalresptr--;
                }
                publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(04, 8, 120, true, false, true, 1, getRunsString(finalresptr))));
                publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(0, 24, 124, false, false, true, 3, getFinalString(0, finalresptr, false))));
                publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(0, 32, 124, false, false, true, 3, getFinalString(1, finalresptr, false))));
                publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(0, 40, 124, false, false, true, 3, getFinalString(3, finalresptr, false))));
            }                //Donw
            else if (tempkcode == 0x14) {
                if (finalresptr == lruns.size() - 1) {
                    finalresptr = 0;
                } else {
                    finalresptr++;
                }
                publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(04, 8, 120, true, false, true, 1, getRunsString(finalresptr))));
                publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(0, 24, 124, false, false, true, 3, getFinalString(0, finalresptr, false))));
                publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(0, 32, 124, false, false, true, 3, getFinalString(1, finalresptr, false))));
                publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(0, 40, 124, false, false, true, 3, getFinalString(3, finalresptr, false))));
            } else {

            }
            tempkcode = 0;
        }


            // Calibration Services ======================================================================================================
        else if (a_state == DOVACAL) {

            publish(event_t(GLCDRESET_SIG, 0, NULL));

            flushValves();
            buildIconBar(1);
            drawShapeRequest(SHP_VOLCALIB_CANVAS, 0U, 0U, false);
            buildBarGraphs();
            publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(04, 8, 120, true, false, true, 1, new std::string("Calibra\x81 \x83 o VADDED"))));

            clearHistory();
            lruns.clear();
            setInitTick();
            cycles = maxruns;
            analysisrsd = 0.003;
            startingcycle = true;
            rsdbypressure = true;
            flip = false;

            state_history.push(ABORTRUN);
            state_history.push(VENTCELL);

            state_history.push(UPDATEVA);
            state_history.push(DOCYCLE);
            state_history.push(UPDATEVAEMPTY);
            state_history.push(VENTCELL);
            state_history.push(DOCYCLE);
            state_history.push(ASKSAMPLEREADY);
            //state_history.push(ASKSPHEREVOL);

            doingcal = true;
            
            sensor_state = S_IDLE;
            a_state = ASKCELLTYPE;

        }
        else if (a_state == DOVCCAL) {

            publish(event_t(GLCDRESET_SIG, 0, NULL));

            flushValves();
            buildIconBar(1);
            drawShapeRequest(SHP_VOLCALIB_CANVAS, 0U, 0U, false);
            buildBarGraphs();
            publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(04, 8, 120, true, false, true, 1, new std::string("Calibra\x81 \x83 o VCELL"))));

            clearHistory();
            lruns.clear();
            setInitTick();
            cycles = maxruns;
            startingcycle = true;
            rsdbypressure = true;
            flip = false;

            state_history.push(ABORTRUN);
            state_history.push(VENTCELL);

            state_history.push(UPDATEVC);
            state_history.push(DOCYCLE);
            state_history.push(ASKSAMPLEREADY);
            state_history.push(ASKMULTIRUN);
            //state_history.push(ASKSPHEREVOL);

            doingcal = true;
            
            sensor_state = S_IDLE;
            a_state = ASKCELLTYPE;
        }

        else if (a_state == ASKADVOL) {
            if (user == 1) {
                std::vector<string> * ch_items = new std::vector<string>();
                ch_items->push_back(std::string("ACP-Autodensity"));
                ch_items->push_back(std::string("ACP-Large"));
                ch_items->push_back(std::string("ACP-Small"));

                publish(event_t(DRAWCHOICE_SIG, 0, new choice_evt(new std::string("Volume de Expan\x81 \x83 o"), ch_items)));
                a_state = WAITADVOL;
            } else {
                smallva = false;
                a_state = state_history.top();
                state_history.pop();
            }
        }
        else if (a_state == WAITADVOL) {
            if (choiceavailable) {
                choiceavailable = false;
                if (choiceanswer == 255) {
                    a_state = ABORTRUN;
                }
                else if (choiceanswer == 0 || choiceanswer == 2) {
                    smallva = false;
                    a_state = state_history.top();
                    state_history.pop();
                } else if (choiceanswer == 1) {
                    smallva = true;
                    a_state = state_history.top();
                    state_history.pop();
                }
                tempkcode = 0;
            }
        }

        else if (a_state == ASKSPHEREVOL) {
            if (user == 1) {
                std::vector<string> * ch_items = new std::vector<string>();
                for (std::vector<s_keymap>::iterator it = spherevolumes.begin(); it != spherevolumes.end(); ++it) {
                    ch_items->push_back(std::string(it->key));
                }
                publish(event_t(DRAWCHOICE_SIG, 0, new choice_evt(new std::string("Volume da Esfera"), ch_items)));
                a_state = WAITSPHEREVOL;
            } else {
                a_state = state_history.top();
                state_history.pop();
            }
        }
        else if (a_state == WAITSPHEREVOL) {
            if (choiceavailable) {
                choiceavailable = false;
                if (choiceanswer == 255) {
                    a_state = ABORTRUN;
                }
                if (choiceanswer == 0) {
                    a_state = state_history.top();
                    state_history.pop();
                } else {
                    std::string sval = spherevolumes.at(choiceanswer).value;
                    if (spherevols[celltype] < 0) {
                        log(INFO, "Using Defined sphere Volume = %f", spherevols[celltype]);
                    } else {
                        spherevols[celltype] = atof(sval.c_str());
                    }
                    //log(INFO, "Sphere choice = %f \r\n", spherevol);
                    a_state = state_history.top();
                    state_history.pop();
                }
                tempkcode = 0;
            }
        }
        else if (a_state == ASKSAMPLEREADY) {
            //log(PLUMB, "Asking if sample ready \r\n");
            
            publish(event_t(DRAWDIALOG_SIG, 0, new dialog_evt(1,
                    255,
                    ABORTRUN,
                    ABORTRUN,
                    //getOKtoStartString()
                    new std::string("OK para iniciar\nos ciclos de\nanalises ?")
                    )));
            a_state = WAITSAMPLEREADY;
        }
        else if (a_state == UPDATEVAEMPTY) {
            emptyratio = lruns.front().cycleavg;
            lruns.clear();
            setInitTick();
            cycles = maxruns;
            startingcycle = true;
            rsdbypressure = true;
            flip = false;
            publish(event_t(DRAWDIALOG_SIG, 0, new dialog_evt(1,
                    255,
                    ABORTRUN,
                    ABORTRUN,
                    new std::string("OK para iniciar\nos ciclos de\nesfera ?")
                    )));
            a_state = WAITSAMPLEREADY;
        }
        else if (a_state == UPDATEVA) {
            sphereratio = lruns.front().cycleavg;
            calculateVA();
            publish(event_t(DRAWDIALOG_SIG, 0, new dialog_evt(1,
                    255,
                    ABORTRUN,
                    ABORTRUN,
                    getVAResultString()
                    )));
            a_state = WAITSAMPLEREADY;
        }
        else if (a_state == ASKVAREPORT) {
            publish(event_t(DRAWDIALOG_SIG, 0, new dialog_evt(1,
                    255,
                    ABORTRUN,
                    ABORTRUN,
                    new std::string("Deseja imprimir\num relat\x8b rio\ndessa calibra\x81 \x83 o ?")
                    )));
            a_state = WAITSAMPLEREADY;
        }
        else if (a_state == WAITSAMPLEREADY) {
            if (dlgavailable) {
                dlgavailable = false;
                if (dlganswer == 255) {
                    a_state = state_history.top();
                    state_history.pop();
                } else {
                    a_state = dlganswer;
                }
            }
        }
        else if (a_state == ASKCELLTYPE) {
            //if (user == 1){
            std::vector<string> * ch_items = new std::vector<string>();
            for (std::vector<s_keymap>::iterator it = cellvolumes.begin(); it != cellvolumes.end(); ++it) {
                ch_items->push_back(std::string(it->key));
            }
            publish(event_t(DRAWCHOICE_SIG, 0, new choice_evt(new std::string("Selecione tipo de Celula"), ch_items)));
            a_state = WAITCELLTYPE;
            //}
            //		else{
            //			cellptr=0;
            //			a_state = state_history.top();
            //			state_history.pop();
            //		}
        }
        else if (a_state == WAITCELLTYPE) {
            if (choiceavailable) {
                choiceavailable = false;
                //log(INFO, "Cell type returned %d\n\r", choiceanswer);
                if (choiceanswer == 255) {
                    tempkcode = 0;
                    a_state = ABORTRUN;
                } else {
                    celltype = choiceanswer;
                    
                    a_state = state_history.top();
                    state_history.pop();
                    tempkcode = 0;
                }
            }
        }
        else if (a_state == ASKMULTIRUN) {
            if (user != 0 ) {
                publish(event_t(DRAWDIALOG_SIG, 0, new dialog_evt(1,
                        INPUTMAXRUNS,
                        SINGLERUNSETUP,
                        ABORTRUN,
                        new std::string("Deseja acionar\nanalise\nmulti ciclo ?")
                        )));
                a_state = WAITSAMPLEREADY;
            } else {
                state_history.pop(); // jump flow
                a_state = state_history.top();
                state_history.pop();
            }

        }
        else if (a_state == SINGLERUNSETUP) {
            cycles = 0;
            a_state = state_history.top();
            state_history.pop();
        }

        else if (a_state == INPUTMAXRUNS) {
            publish(event_t(DRAWINPUT_SIG, 0, new input_evt(SHP_INPUT5,
                    true, false, false, false,
                    new std::string("Max. num\nde analises"),
                    new std::string(""),
                    new std::string("%3.1f"),
                    new std::string(""),
                    cycles, 3.0, 100.0)));
            a_state = WAITMAXRUNS;
        } else if (a_state == WAITMAXRUNS) {
            if (inputavailable) {
                if (inputval == -1){
                    // Escape requested go back to select Sisze
                    tempkcode = 0;
                    inputavailable = false;
                    a_state = ASKMULTIRUN;
                    state_history.push(INPUTMAXRUNS);
                }
                else{
                    cycles = inputval;
                    tempkcode = 0;
                    inputavailable = false;
                    a_state = INPUTMAXDEV;
                }
            }
        }
        else if (a_state == INPUTMAXDEV) {
            publish(event_t(DRAWINPUT_SIG, 0, new input_evt(SHP_INPUT5,
                    true, false, false, false,
                    new std::string("Desvio\nMaximo"),
                    new std::string(""),
                    new std::string("%6.4f "),
                    new std::string(""),
                    analysisrsd, 0.0, 60.0)));
            a_state = WAITMAXDEV;
        } else if (a_state == WAITMAXDEV) {
            if (inputavailable) {
                if (inputval == -1){
                    // Escape requested go back to select Sisze
                    tempkcode = 0;
                    inputavailable = false;
                    a_state = INPUTMAXRUNS;
                    state_history.push(INPUTMAXDEV);
                }
                else{
                    analysisrsd = inputval;
                    tempkcode = 0;
                    inputavailable = false;
                    a_state = state_history.top();
                    state_history.pop();
                }
            }
        }
        else if (a_state == UPDATEVC) {
            sphereratio = lruns.front().cycleavg;
            calculateVC();
            publish(event_t(DRAWDIALOG_SIG, 0, new dialog_evt(1,
                    ASKVAREPORT,
                    ABORTRUN,
                    ABORTRUN,
                    getVCResultString()
                    )));
            a_state = WAITSAMPLEREADY;
        }
        else if (a_state == ASKVAREPORT) {
            publish(event_t(DRAWDIALOG_SIG, 0, new dialog_evt(1,
                    255,
                    ABORTRUN,
                    ABORTRUN,
                    new std::string("Deseja imprimir\num relat\x8b rio\ndessa calibra\x81 \x83 o ?")
                    )));
            a_state = WAITSAMPLEREADY;
        }

            // Analysis Services ======================================================================================================

        else if (a_state == DOCYCLE) {

            if (!flip) {
                if (lruns.size() <= cycles) {
                    if (verifyCyclesRSD(rsdbypressure, runsaveraged)) {
                        // Deviation OK !
                        publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(04, 8, 120, true, false, true, 1, getCycleString(lruns.size(), cycles))));
                        publish(event_t(DRAWSHAPE_SIG, 0, new drawshape_evt(SHP_WORKA_CLEAR, 0, 24, true)));
                        publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(0, 24, 124, false, false, false, 1, new std::string("Desvio dentro dos limites"))));
                        publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(0, 32, 124, false, false, false, 1, getRSDString(cycledrift, cycleavg))));
                        setAlarm(2, infogap * 32, 0);
                        flip = true;
                        reportCycles();
                        temp_state = state_history.top();
                        state_history.pop();
                    }
                    else {
                        if (cycles == 0) {
                            publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(04, 8, 120, true, false, true, 1, new std::string("Analise unica"))));
                        } else {
                            publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(04, 8, 120, true, false, true, 1, getCycleString(lruns.size(), cycles))));
                        }
                        publish(event_t(DRAWSHAPE_SIG, 0, new drawshape_evt(SHP_WORKA_CLEAR, 0, 24, true)));
                        if (lruns.size() < 4) {
                            publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(0, 24, 124, false, false, false, 1, new std::string("Coletando dados"))));
                            setAlarm(2, infogap * 8, 0);
                        } else {
                            publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(0, 24, 124, false, false, false, 1, new std::string("Desvio fora dos limites"))));
                            publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(0, 40, 124, false, false, false, 1, getRSDString(cycledrift, cycleavg))));
                            setAlarm(2, infogap * 32, 0);
                        }
                        flip = true;
                        temp_state = VENTCELL;
                        state_history.push(DOCYCLE);
                        if (!doingcal) state_history.push(INTERRUNINFO);
                        state_history.push(REGISTEREXPANSION);
                        state_history.push(PREPAREEXPAND);
                        state_history.push(REGISTERRUN);
                        state_history.push(PREPARECHARGE);
                        if (startingcycle) state_history.push(PREPAREZERO);
                        if (startingcycle) state_history.push(VENTCELL);
                        if (startingcycle) state_history.push(FLUSHCELL);
                        startingcycle = false;
                    }
                } else {
                    // Max runs reached ---
                    if (cycles == 0) {
                        publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(04, 8, 120, true, false, true, 1, new std::string("Analise simples finalizou"))));
                        publish(event_t(DRAWSHAPE_SIG, 0, new drawshape_evt(SHP_WORKA_CLEAR, 0, 24, true)));
                        publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(0, 24, 124, false, false, false, 1, new std::string("Raz\x83 o das Press\x85 es"))));
                        publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(0, 40, 124, false, false, false, 1, getRSDString(lruns.front().expanddrift, lruns.front().deltap))));
                    } else {
                        verifyCyclesRSD(rsdbypressure, runsaveraged);
                        publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(04, 8, 120, true, false, true, 1, getCycleString(lruns.size(), cycles))));
                        publish(event_t(DRAWSHAPE_SIG, 0, new drawshape_evt(SHP_WORKA_CLEAR, 0, 24, true)));
                        publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(0, 24, 124, true, false, false, 1, new std::string("Acima do num max de analises"))));
                        publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(0, 32, 124, false, false, false, 1, new std::string("Utilizando ult. 4 resultados"))));
                        publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(0, 40, 124, false, false, false, 1, getRSDString(cycledrift, cycleavg))));
                    }
                    setAlarm(2, infogap * 32, 0);
                    flip = true;
                    reportCycles();
                    temp_state = state_history.top();
                    state_history.pop();
                }
            } else {
                if (tempkcode == 0x08) {
                    SensorSM(S_IDLE_SIG);
                    reportCycles();
                    a_state = default_history;
                    tempkcode = 0;
                }
                if (alarmset[2] == true) {
                    alarmset[2] = false;
                    publish(event_t(DRAWSHAPE_SIG, 0, new drawshape_evt(SHP_WORKA_CLEAR, 255, 24, true)));
                    flip = false;
                    a_state = temp_state;
                }
            }
        }

        else if (a_state == FAKECYCLE) {

            time(&rawtime);
            lruns.push_back(loadrun(rawtime, 17.9805, 0, 0.0002));

            lruns.back().expandpressure = 8.0765;
            lruns.back().temperature = 20.4;
            if (usepratio) {
                lruns.back().deltap = lruns.back().loadpressure / 8.0765;
                lruns.back().pratio = lruns.back().loadpressure - 8.0765;
            } else {
                lruns.back().pratio = lruns.back().loadpressure / 8.0765;
                lruns.back().deltap = lruns.back().loadpressure - 8.0765;
            }

        }

            // Operational Services =====================================================================================================

        // Vent Cell ----------------------------------------------------------------
        else if (a_state == VENTCELL) {
            
            if (celltype == 2){
                publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(04, 8, 120, true, false, true, 1, new std::string("Ventilando Celula [P]"))));
            }
            else if (celltype == 1){
                publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(04, 8, 120, true, false, true, 1, new std::string("Ventilando Celula [M]"))));
            }
            else if (celltype == 0){
                publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(04, 8, 120, true, false, true, 1, new std::string("Ventilando Celula [G]"))));
            }
            else {
                publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(04, 8, 120, true, false, true, 1, new std::string("Ventilando Celula [?]"))));
            }
            
            
            SensorSM(S_VENT_SIG);
            a_state = WAITTOVENT;
        }
        else if (a_state == WAITTOVENT) {
            if (tempkcode == 0x08) {
                SensorSM(S_IDLE_SIG);
                a_state = default_history;
                tempkcode = 0;
            } else if (SensorSM(S_NULL_SIG) == S_VENTING) {
                // Pressure dropped below limit
                //publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(04,8,120,true,false,true,1, new std::string("Cell Vented"))));
                a_state = state_history.top();
                state_history.pop();
            }
        }
            // Flush Cell ----------------------------------------------------------------
        else if (a_state == FLUSHCELL) {
            flushstarttag = getTick();
            flushendtag = flushstarttag + (flushtime * 1000);

            if (flushtype == 0) {      
                publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(04, 8, 120, true, false, true, 1, new std::string("Fluxo Cancelado"))));
                SensorSM(S_FLUSHNONE_SIG);            
            }
            else if (flushtype == 1) {
                // Flow type	
                publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(04, 8, 120, true, false, true, 1, new std::string("Fluxo por tempo"))));
                SensorSM(S_FLUSHTIME_SIG);
                
            } else if (flushtype == 2) {
                // Pulse type
                flushcicle = 1.0;
                flushendtag = flushstarttag + (flushcicletime * 1000);
                publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(04, 8, 120, true, false, true, 1, new std::string("Fluxo por pulso"))));
                SensorSM(S_FLUSHPULSEON_SIG);
            } else {
                // Vacuum type
                publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(04, 8, 120, true, false, true, 1, new std::string("Usando Vacuo"))));
                SensorSM(S_FLUSHVAC_SIG);
            }
            a_state = WAITTOFLUSH;
        }
        else if (a_state == WAITTOFLUSH) {
            if (tempkcode == 0x08) {
                SensorSM(S_IDLE_SIG);
                a_state = default_history;
                tempkcode = 0;
            } else if (SensorSM(S_NULL_SIG) == S_FLUSHTIME) {
                a_state = FLUSHEND;
            } else if (SensorSM(S_NULL_SIG) == S_FLUSHPULSEOFF) {
                flushcicle = flushcicle + 1.0;
                //log(INFO, "Flushpulseoff =  %f / %f\r\n", flushcicle, flushcicles);
                if (flushcicle >= flushcicles) {
                    a_state = FLUSHEND;
                } else {
                    flushstarttag = getTick();
                    flushendtag = flushstarttag + (flushcicletime * 1000);
                    publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(04, 8, 120, true, false, true, 1,
                            getFlushPulseString(flushcicle + 1, flushcicles))));

                    SensorSM(S_FLUSHPULSEON_SIG);
                }
            } else if (SensorSM(S_NULL_SIG) == S_FLUSHPULSEON) {
                flushstarttag = getTick();
                flushendtag = flushstarttag + (flushcicletime * 1000);
                SensorSM(S_FLUSHPULSEOFF_SIG);
            }
        }
        else if (a_state == FLUSHEND) {
            activateValve(0, 0);
            activateValve(1, 0);
            activateValve(2, 0);
            activateValve(3, 0);
            publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(04, 8, 120, true, false, true, 1, new std::string("Fluxo OK"))));
            a_state = state_history.top();
            state_history.pop();
        }

            // Transducer Calibration ------------------------------------------
        else if (a_state == PREPAREZERO) {
            if (zerotransducer || startingcycle) {
                publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(04, 8, 120, true, false, true, 1, new std::string("Calibrando Transdutor"))));
                SensorSM(S_ZERO_SIG);
                a_state = MEASUREZERO;
            } else {
                a_state = state_history.top();
                state_history.pop();
            }
        }
        else if (a_state == MEASUREZERO) {
            if (tempkcode == 0x08) {
                SensorSM(S_IDLE_SIG);
                a_state = default_history;
                tempkcode = 0;
            } else {
                uint8_t zstatus = SensorSM(S_NULL_SIG);
                if (zstatus == S_ZEROFAILED) {
                    publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(04, 8, 120, true, false, true, 1, new std::string("Erro na Estabiliza\x81 \x83 o"))));
                    SensorSM(S_RESET_SIG);
                    if (blockondriftfail) {
                        a_state = RUNMANUAL;
                    } else {
                        a_state = state_history.top();
                        state_history.pop();
                    }
                } else if (zstatus == S_ZEROENDED) {
                    publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(04, 8, 120, true, false, true, 1, new std::string("Transdutor Calibrado"))));
                    SensorSM(S_RESET_SIG);
                    a_state = state_history.top();
                    state_history.pop();
                }
            }
        }
            // Charge Cell -----------------------------------------------------------
        else if (a_state == PREPARECHARGE) {
            publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(04, 8, 120, true, false, true, 1, new std::string("Pressurizando Celula"))));
            SensorSM(S_CHARGE_SIG);
            a_state = WAITTOCHARGE;
        }
        else if (a_state == WAITTOCHARGE) {
            if (tempkcode == 0x08) {
                SensorSM(S_IDLE_SIG);
                a_state = default_history;
                tempkcode = 0;
            } else {
                uint8_t loadstatus = SensorSM(S_NULL_SIG);
                if (loadstatus == S_CHARGEFAILED) {
                    publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(04, 8, 120, true, false, true, 1, new std::string("Falha na carga"))));
                    SensorSM(S_RESET_SIG);
                    if (blockondriftfail) {
                        a_state = RUNMANUAL;
                    } else {
                        a_state = state_history.top();
                        state_history.pop();
                    }
                } else if (loadstatus == S_CHARGEENDED) {
                    publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(04, 8, 120, true, false, true, 1, new std::string("Celula carregada"))));
                    SensorSM(S_RESET_SIG);
                    flip = true;
                    a_state = state_history.top();
                    state_history.pop();
                }
            }
        }
        else if (a_state == REGISTERRUN) {

            if (flip) {
                time(&rawtime);
                lruns.push_back(loadrun(rawtime, lastpressure, driftstatus, lastdrift));
                if (infogap != 0.0) {
                    publish(event_t(DRAWSHAPE_SIG, 0, new drawshape_evt(SHP_WORKA_CLEAR, 0, 24, true)));
                    publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(0, 24, 124, false, false, false, 1, getPressureString(false))));
                    publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(0, 32, 124, false, false, false, 1, getDriftStatusString())));
                    publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(0, 40, 124, false, false, false, 1, getTempString(0))));
                    setAlarm(2, infogap * 32, 0);
                    //log(INFO, "Registered run (back): %6.4f -> %6.4f\r\n", lruns.back().loadpressure, lastpressure);
                    //log(INFO, "Registered run (front): %f6.4f \r\n", lruns.front().loadpressure);
                    flip = false;
                } else {
                    a_state = state_history.top();
                    state_history.pop();
                }
            } else {
                if (alarmset[2] == true) {
                    alarmset[2] = false;
                    publish(event_t(DRAWSHAPE_SIG, 0, new drawshape_evt(SHP_WORKA_CLEAR, 255, 24, true)));
                    a_state = state_history.top();
                    state_history.pop();
                }
            }
        }

        
        
            // Expand Cell -----------------------------------------------------------
        else if (a_state == PREPAREEXPAND) {
            publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(04, 8, 120, true, false, true, 1, new std::string("Expandindo Celula"))));
            SensorSM(S_EXPAND_SIG);
            a_state = WAITTOEXPAND;
        }
        else if (a_state == WAITTOEXPAND) {
            if (tempkcode == 0x08) {
                SensorSM(S_IDLE_SIG);
                a_state = default_history;
                tempkcode = 0;
            } else {
                uint8_t expandstatus = SensorSM(S_NULL_SIG);
                if (expandstatus == S_EXPANDFAILED) {
                    publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(04, 8, 120, true, false, true, 1, new std::string("Falha na expans\x83 o"))));
                    SensorSM(S_RESET_SIG);
                    if (blockondriftfail) {
                        a_state = RUNMANUAL;
                    } else {
                        a_state = state_history.top();
                        state_history.pop();
                    }
                } else if (expandstatus == S_EXPANDENDED) {
                    publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(04, 8, 120, true, false, true, 1, new std::string("Expans\x83 o completa"))));
                    SensorSM(S_RESET_SIG);
                    flip = true;
                    a_state = state_history.top();
                    state_history.pop();
                }
            }
        }

        else if (a_state == REGISTEREXPANSION) {
            if (flip) {
                if (infogap != 0.0) {
                    //loadrun lr = lruns.back();			
                    lruns.back().expandpressure = lastpressure;
                    lruns.back().temperature = lasttemperature;
                    if (usepratio) {
                        lruns.back().deltap = lruns.back().loadpressure / lastpressure;
                        lruns.back().pratio = lruns.back().loadpressure - lastpressure;
                        if (lruns.size() < 4) {
                            cycleavg = lruns.back().deltap;
                            cycledrift = lruns.back().expanddrift;
                            lruns.back().cycleavg = cycleavg;
                            lruns.back().cyclersd = cycledrift;
                        }
                    } else {
                        lruns.back().pratio = lruns.back().loadpressure / lastpressure;
                        lruns.back().deltap = lruns.back().loadpressure - lastpressure;
                        if (lruns.size() < 4) {
                            cycleavg = lruns.back().pratio;
                            cycledrift = lruns.back().expanddrift;
                            lruns.back().cycleavg = cycleavg;
                            lruns.back().cyclersd = cycledrift;
                        }
                    }
                    publish(event_t(DRAWSHAPE_SIG, 0, new drawshape_evt(SHP_WORKA_CLEAR, 0, 24, true)));
                    publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(0, 24, 124, false, false, false, 1, getPressureString(true))));
                    publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(0, 32, 124, false, false, false, 1, getDriftStatusString())));
                    publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(0, 40, 124, false, false, false, 1, getTempString(0))));
                    setAlarm(2, infogap * 32, 0);
                    flip = false;
                }
                else {
                    a_state = state_history.top();
                    state_history.pop();
                }
            } else {
                if (alarmset[2] == true) {
                    alarmset[2] = false;
                    publish(event_t(DRAWSHAPE_SIG, 0, new drawshape_evt(SHP_WORKA_CLEAR, 255, 24, true)));
                    if (!doingcal) flip=true;
                    a_state = state_history.top();
                    state_history.pop();
                }
            }

        }

        else if (a_state == INTERRUNINFO) {
    
            if (flip) {
                publish(event_t(DRAWSHAPE_SIG, 0, new drawshape_evt(SHP_WORKA_CLEAR, 0, 24, true)));
                publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(04, 8, 120, true, false, true, 1, new std::string("PARCIAL - ESTIMATIVA"))));
                
                //if (!doingcal){
                    publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(0, 24, 124, false, false, false, 1, new std::string("DENSIDADE"))));
                    publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(0, 24, 124, false, false, false, 1, getPartialDenString())));
                    publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(0, 40, 124, false, false, false, 1, new std::string("DEL = abortar analise"))));
//                }
//                else{
//                    publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(0, 24, 124, false, false, false, 1, new std::string("Razao de Pressoes"))));
//                    publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(0, 24, 124, false, false, false, 1, getPressureString(true))));
//                    publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(0, 40, 124, false, false, false, 1, new std::string("DEL = abortar analise"))));
//                }
                setAlarm(2, infogap * 32, 0);
                flip = false;
            } else {
                if (alarmset[2] == true) {
                    alarmset[2] = false;
                    publish(event_t(DRAWSHAPE_SIG, 0, new drawshape_evt(SHP_WORKA_CLEAR, 255, 24, true)));
                    //flip=true;
                    a_state = state_history.top();
                    state_history.pop();
                }
                else{
                    
                }
            }
        }
     

        else if (a_state == STOREANALISE) {
            if (autostore) {
                a_state = STOREREQUESTED;
            } else if (asktostore) {
                a_state = ASKTOSTORE;
            } else {
                a_state = state_history.top();
                state_history.pop();
            }
        } else if (a_state == ASKTOSTORE) {
            publish(event_t(DRAWDIALOG_SIG, 0, new dialog_evt(1,
                    STOREREQUESTED,
                    ABORTRUN,
                    ABORTRUN,
                    new std::string("Armazenar esse\n resultado ?")
                    )));
            a_state = WAITTOSTORE;
            dlgavailable = false;

        } else if (a_state == WAITTOSTORE) {
            if (dlgavailable) {
                dlgavailable = false;
                a_state = dlganswer;
            }
        }
        else if (a_state == STOREREQUESTED) {
            storeResult();
            a_state = state_history.top();
            state_history.pop();
        }

        else if (a_state == UNHANDLED) {
            log(WARNING, "Unhandled signal on Analise = %d\n\r", a_state);
            a_state = STANDBY;
        } else {
            log(WARNING, "Unknow signal on Analise = %d\n\r", a_state);
            a_state = UNHANDLED;
            a_state = STANDBY;
        }

    }


    //==================================================================================================================================

    void Analise::storeResult() {

        char sbuf[128];
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        std::string * result = new std::string();
        std::string * dest = new std::string(storefile);

        strftime(sbuf, 128, "%x,%X,", timeinfo);
        result->append(sbuf);

        result->append(sampleid);
        result->append(",");
        //              Mass    D     TP    VL    RSD   LP   EP   Run
        sprintf(sbuf, "%7.4f,%6.4f,%3.1f,%7.4f,%6.2f,%6.4f,%6.4f,%2d,",
                mass, // Massa
                getMedia(0), // DENSITY
                getMedia(1), // TEMP
                getMedia(2), // Volume
                lruns.at(0).cyclersd, // RSD
                getMedia(3), // Load pressure
                getMedia(4), // Expand pressure
                lruns.size() - 1 // runs
                );
        result->append(sbuf);



        //	for (int i = 0; i < 3; i++) {
        //		sprintf (sbuf, "%6.2f,%5.2f,%4.2f,",
        //						getValArea(i),
        //						getValTime(i),
        //						getValTemp(i)
        //			 );
        //		result->append(sbuf);
        //	}


        result->append("MRASO,ACP15061058\r\n");

        publish(event_t(APPENDTOFILE_SIG, 0, new appendfile_evt(0, dest, result)));
        //log(FINE,"Storing %s\n\r", result->c_str());
    }

    void Analise::clearAlarms() {
        for (int i = 0; i < 4; i++) {
            alarmset[i] = false;
        }
    }

    uint8_t Analise::_verifyKeyCode(uint8_t code) {

        uint8_t kk = 0;

        if (!_locateEvent(KEYCODE_SIG)) return 0;

        //log(FINE, "Analise : Keycode event =%d was localized\r\n", events.front().simple);

        if ((code == 0) | (events.front().simple == code)) {
            kk = events.front().simple;
        }

        events.pop();
        tempkcode = kk;
        return kk;
    }

    bool Analise::_locateEvent(int event_type) {

        while (!events.empty()) {
            if (events.front().evtype == event_type) {
                //event_temp = mainevents.front();
                //log(FINE, "Analise : Event type %d was localized\r\n", events.front().evtype);
                return true;
            }
            //delete mainevents.front();
            events.pop();
        }
        return false;
    }

    void Analise::clearHistory() {

        while (!state_history.empty()) {
            state_history.pop();
        }
    }

    void Analise::loadAutoSID() {

        time_t rawtime;
        struct tm * timeinfo;
        time(&rawtime);
        timeinfo = localtime(&rawtime);

        strftime(&sampleid[0], 32, "ACP-%d%m%H%M%S", timeinfo);
    }

    void Analise::loadParams() {

        //	PDEBUG("Loading device params\n\r");
        //	if (prop->isActive()){
        //		if (prop->loadkeys("config.dat")){		
        //		
        //		}
        //	}
    }

    void Analise::activateValve(uint8_t index, int val) {

        //log(PLUMB, "Changing valve %d to %d \r\n", index, val);

        if (index == 0) {
            inputvalve_pin = val;
            if (val == 0) {
                man_icons[0] = SHP_ICON_VINP_OFF;
            } else {
                man_icons[0] = SHP_ICON_VINP_ON;
            }
            publish(event_t(UPDATEICON_SIG, 0, new icon_evt(0, glcd::ICREFRESH, man_icons[0])));
            valves[0] = val;
        } else if (index == 1) {
            expandvalve_pin = val;
            if (val == 0) {
                man_icons[1] = SHP_ICON_VEXP_OFF;
            } else {
                man_icons[1] = SHP_ICON_VEXP_ON;
            }
            publish(event_t(UPDATEICON_SIG, 0, new icon_evt(1, glcd::ICREFRESH, man_icons[1])));
            valves[1] = val;
        } else if (index == 2) {
            ventvalve_pin = val;
            if (val == 0) {
                man_icons[2] = SHP_ICON_VVEN_OFF;
            } else {
                man_icons[2] = SHP_ICON_VVEN_ON;
            }
            publish(event_t(UPDATEICON_SIG, 0, new icon_evt(2, glcd::ICREFRESH, man_icons[2])));
            valves[2] = val;
        } else if (index == 3) {
            vaccumvalve_pin = val;
            valves[3] = val;
        } else if (index == 4) {
            smallvalve_pin = val;
            valves[4] = val;
        }
    }

    void Analise::flushValves() {

        activateValve(0, 0);
        activateValve(1, 0);
        activateValve(2, 0);
        activateValve(3, 0);
        activateValve(4, 0);

    }

    void Analise::print(uint8_t type) {

        time(&rawtime);
        timeinfo = localtime(&rawtime);

        char sbuf[128];
        std::string * stemp;

        //fillDummyResults();

        if (strncmp(&printheader[0], "default    ", 3) == 0) {
            publish(event_t(PRINTSTRING_SIG, 0, new std::string("\r\n\n\n\n")));
            publish(event_t(PRINTSTRING_SIG, Printer::EXPANDED, new std::string(" ACP Instruments - 2016\n\r")));
            publish(event_t(PRINTSTRING_SIG, 0, new std::string("          Autodensity 100 - V 2.6.0-RC\n\r")));
            publish(event_t(PRINTSTRING_SIG, Printer::RULLER, new std::string("")));
            publish(event_t(PRINTSTRING_SIG, Printer::BLANKLINE, new std::string("")));
        } else if (strncmp(&printheader[0], "none    ", 4) == 0) {
            publish(event_t(PRINTSTRING_SIG, 0, new std::string("\r\n\n\n\n")));
            publish(event_t(PRINTSTRING_SIG, Printer::RULLER, new std::string("")));
        } else if (strncmp(&printheader[0], "oneline   ", 7) == 0) {
            stemp = new std::string("");
            strftime(sbuf, 128, "%m/%d/%y-%T ", timeinfo);
            stemp->append(sbuf);
            ////			sprintf (sbuf, "%s %6.2f %5.3f%%\r\n", sampleid, average, rsd);
            stemp->append(sbuf);
            publish(event_t(PRINTSTRING_SIG, 0, stemp));
            publish(event_t(FLUSHPRINTER_SIG, 0, NULL));
            publish(event_t(PRINTSTRING_SIG, Printer::RESET, new std::string("")));
            return;
        }


        if (ejectprint) publish(event_t(PRINTSTRING_SIG, Printer::EJECT, new std::string("")));

        publish(event_t(PRINTSTRING_SIG, Printer::RESET, new std::string("")));

        publish(event_t(FLUSHPRINTER_SIG, 0, NULL));
    }

    bool Analise::verifyCyclesRSD(bool usepressure, uint8_t runs) {

        double eavg, edrift, temp, dif;
        uint8_t i, init, end;

        // Nothing to do if samples < 3
        if (lruns.size() < runs) {
            //cycleavg = (usepressure) ? lruns.back().deltap : lruns.back().density;
            ///cycledrift = lruns.back().expanddrift;
            return false;
        }
        end = lruns.size() - 1;
        init = end - (runs - 1);

        eavg = 0;
        for (i = init; i <= end; i++) {
            temp = (usepressure) ? lruns.at(i).deltap : lruns.at(i).density;
            eavg += temp;
        }
        eavg = eavg / runs;

        // Calculate SD
        dif = 0;
        for (i = init; i <= end; i++) {
            temp = (usepressure) ? lruns.at(i).deltap - eavg : lruns.at(i).density - eavg;
            dif += (temp * temp);
        }

        dif = dif / runs;
        dif = sqrt(dif);
        edrift = (dif / (eavg * 2)) * 100;

        cycleavg = eavg;
        cycledrift = edrift;

        lruns.back().cycleavg = cycleavg;
        lruns.back().cyclersd = cycledrift;

        if (edrift < analysisrsd) {
            log(INFO, "VerifyRSD approved: %f (%f) - from %d to %d\n\r", cycledrift, cycleavg, init, end);
            return true;
        }
        log(INFO, "VerifyRSD rejected cycle - %6.4f (%6.4f) - from %d to %d with avg =  %6.4f \n\r", cycledrift, dif, init, end, cycleavg);
        return false;
    }

    void Analise::calculateVA() {

        double ep, sp;

        ep = 1 / (emptyratio - 1);
        sp = 1 / (sphereratio - 1);
        vaddeds[celltype] = spherevols[celltype] / (ep - sp);
    }

    void Analise::calculateVC() {

        double sp;

        sp = sphereratio - 1;
        vcells[celltype] = spherevols[celltype] + (vaddeds[celltype] / sp);
    }

    double Analise::calculateVP(double ratio) {

        double sp;

        sp = 1 - ratio;
        return vcells[celltype] + (vaddeds[celltype] / sp);
    }

    double Analise::calculateDEN(double vol) {

        return mass / vol;
    }

    double Analise::calculateMean(uint8_t n, double v[]) {

        double sum;
        sum = 0;

        for (int i = 0; i < n; i++) {
            sum += v[i];
        }
        sum = sum / n;
        return sum;
    }

    double Analise::calculateSD(uint8_t n, double v[], double avg) {

        double temp, dif;
        dif = 0;

        //log(INFO, "SD :avg = %7.4f \r\n", avg);
        for (int i = 0; i < n; i++) {
            temp = v[i] - avg;
            //log(INFO, "SD :v[%d}=%7.4f\r\n", i, temp);
            dif += (temp * temp);
        }

        dif = dif / n;
        //log(INFO, "SD :final dif = %7.3f \r\n", dif);
        dif = sqrt(dif);
        return dif;

    }

    int Analise::LinReg(uint8_t n, double x[], double y[], double *b, double *m, double *r) {

        double sumx = 0.0; /* sum of x                      */
        double sumx2 = 0.0; /* sum of x**2                   */
        double sumxy = 0.0; /* sum of x * y                  */
        double sumy = 0.0; /* sum of y                      */
        double sumy2 = 0.0; /* sum of y**2					 */

        for (int i = 0; i < n; i++) {
            sumx += x[i];
            sumx2 += sqrt(x[i]);
            sumxy += x[i] * y[i];
            sumy += y[i];
            sumy2 += sqrt(y[i]);
        }

        double denom = (n * sumx2 - sqrt(sumx));
        if (denom == 0) {
            // singular matrix. can't solve the problem.
            *m = 0;
            *b = 0;
            if (r) *r = 0;
            return 1;
        }

        *m = (n * sumxy - sumx * sumy) / denom;
        *b = (sumy * sumx2 - sumx * sumxy) / denom;
        if (r != NULL) {
            *r = (sumxy - sumx * sumy / n) / /* compute correlation coeff     */
                    sqrt((sumx2 - sqrt(sumx) / n) *
                    (sumy2 - sqrt(sumy) / n));
        }

        return 0;
    }

    void Analise::updatePRuns(bool clear, double val) {

        int i;
        uint8_t sptr, dvptr;
        double sum, avg;
        double temp, dif, dv;


        if (clear) {
            for (i = 0; i < 16; i++) {
                pruns[i] = 0;
            }
            pruns_head = 0;
            pruns_tail = 0;
            cache = 0;
        } else {
            pruns[pruns_head] = val;
            pruns_head++;
            pruns_head = pruns_head & 0x0f;
            if (cache < 8) {
                cache++;
            } else {
                pruns_tail++;
                pruns_tail = pruns_tail & 0x0f;
            }
        }

        // Calculate Moving AVG
        sum = 0;
        sptr = pruns_tail;
        if (cache > 2) {
            for (i = 0; i < cache; i++) {
                sum += pruns[sptr];
                sptr++;
                sptr = sptr & 0x0f;
            }
            avg = sum / cache;
            pruns_mvavg = avg;
        } else {
            pruns_mvavg = val;
        }

        // Calculate SD
        dif = 0;
        if (cache > 2) {
            sptr = pruns_tail;
            for (i = 0; i < cache; i++) {
                temp = pruns[sptr] - avg;
                dif += (temp * temp);
                sptr++;
                sptr = sptr & 0x0f;
            }
            dif = dif / cache;
            dif = sqrt(dif);
            pruns_sd = (dif / avg) * 100;
            if (pruns_sd < 0.0) pruns_sd = pruns_sd * -1;
        } else {
            pruns_sd = 0.0;
        }

        // Caculate derivative
        dv = 0;
        if (cache > 2) {
            sptr = pruns_tail;
            for (i = 0; i < (cache - 1); i++) {
                dvptr = sptr + 1;
                dvptr = dvptr & 0x0f;
                temp = pruns[dvptr] - pruns[sptr];
                dv += temp;
                sptr++;
                sptr = sptr & 0x0f;
            }
            pruns_dv = dv;
        } else {
            pruns_dv = 0.0;
        }

    }

    uint8_t Analise::SensorSM(uint8_t signal) {

        double sum;
        double lesser;
        int chstatus;
        uint8_t sptr;

        // Process signals ...
        if (signal == S_NULL_SIG) {

        } else if (signal == S_IDLE_SIG) {
            sensor_state = S_IDLE;
            usepoffset = true;
        } else if (signal == S_VENT_SIG) {
            activateValve(0, 0);
            activateValve(1, 1);
            activateValve(2, 1);
            CalculateParams(SCALC_SETWINDOW, 10, false);
            sensor_state = S_VENTING;
            return S_IDLE;
        } else if (signal == S_ZERO_SIG) {
            usepoffset = false;
            CalculateParams(SCALC_SETWINDOW, zero_driftwstart, false);
            driftwindowend = zero_driftwend * -1;
            driftstatus = 0;
            //log(INFO, "Zero preset = %f / %f \r\n", driftwindowstart, driftwindowend );
            pdvs.clear();
            sensor_state = S_ZEROING;

            pbfptr = 0;
            pbfload = 0;
            updatePRuns(true, 0.0);
            clearAlarms();
            setAlarm(1, 32, 32);

            return S_IDLE;
        }
        else if (signal == S_CHARGE_SIG) {
            activateValve(0, 1);
            activateValve(1, 0);
            activateValve(2, 1);
            activateValve(3, 0);
            activateValve(4, 0);

            CalculateParams(SCALC_SETWINDOW, load_driftwstart, false);
            pdvs.clear();
            driftstatus = 0;

            pbfptr = 0;
            pbfload = 0;
            updatePRuns(true, 0.0);
            clearAlarms();
            setAlarm(1, 32, 32);

            sensor_state = S_CHARGING;
            return S_IDLE;
        }
        else if (signal == S_EXPAND_SIG) {

            activateValve(2, 0);
            activateValve(0, 0);
            activateValve(3, 0);
            if (smallva) {
                activateValve(4, 0);
            } else {
                activateValve(4, 1);
            }
            CalculateParams(SCALC_SETWINDOW, load_driftwstart, false);
            pdvs.clear();

            driftstatus = 0;
            pbfptr = 0;
            pbfload = 0;
            updatePRuns(true, 0.0);
            clearAlarms();
            setAlarm(1, 32, 32);

            activateValve(1, 1);

            sensor_state = S_EXPANDING;
            return S_IDLE;
        }
        else if (signal == S_FLUSHTIME_SIG) {
            activateValve(0, 1);
            activateValve(1, 1);
            activateValve(2, 1);
            sensor_state = S_FLUSHTIME;
            return S_IDLE;
        }
        else if (signal == S_FLUSHNONE_SIG) {
            activateValve(0, 0);
            activateValve(1, 1);
            activateValve(2, 1);
            sensor_state = S_FLUSHNONE;
            return S_IDLE;
        }
        else if (signal == S_FLUSHVAC_SIG) {
            activateValve(0, 0);
            activateValve(1, 1);
            activateValve(2, 1);
            activateValve(3, 1);
            sensor_state = S_FLUSHTIME;
            return S_IDLE;
        }
        else if (signal == S_FLUSHPULSEON_SIG) {
            activateValve(0, 1);
            activateValve(1, 0);
            activateValve(2, 0);
            sensor_state = S_FLUSHPULSEON;
            return S_IDLE;
        }
        else if (signal == S_FLUSHPULSEOFF_SIG) {
            activateValve(0, 0);
            activateValve(1, 1);
            activateValve(2, 1);
            sensor_state = S_FLUSHPULSEOFF;
            return S_IDLE;
        }
        else if (signal == S_RESET_SIG) {
            activateValve(0, 0);
            activateValve(1, 0);
            activateValve(2, 0);
            pbfptr = 0;
            pbfload = 0;
            updatePRuns(true, 0.0);
            clearAlarms();
            setAlarm(1, 32, 32);
            sensor_state = S_IDLE;

            return S_IDLE;
        }

        // Now run the state ...
        if (sensor_state == S_IDLE) {
            CalculateParams(SCALC_IDLE, 0.0, true);
            return S_IDLE;
        }
        else if (sensor_state == S_VENTING) {
            if (CalculateParams(SCALC_PRESSURELESS, vent_tgtpressure, true) == 1) {
                return S_VENTING;
            } else {
                return S_IDLE;
            }
        }
        
        else if (sensor_state == S_FLUSHNONE) {
           
           return S_FLUSHTIME;
        }
        
        else if (sensor_state == S_FLUSHTIME) {
            int flushstatus = CalculateParams(SCALC_FLUSHTIME, 0.0, true);

            if (getTick() > flushendtag) {
                return S_FLUSHTIME;
            } else {
                if (flushstatus == 1) {
                    double timenow = (getTick() - flushstarttag) / 1000;
                    publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(04, 8, 120, true, false, true, 1, getTimerString(timenow, flushtime))));
                }
                return S_IDLE;
            }
        }
        
        
        else if (sensor_state == S_FLUSHPULSEON) {
            CalculateParams(SCALC_FLUSHTIME, 0.0, true);
            if (getTick() > flushendtag) {
                return S_FLUSHPULSEON;
            } else {
                return S_IDLE;
            }
        }
        else if (sensor_state == S_FLUSHPULSEOFF) {
            CalculateParams(SCALC_FLUSHTIME, 0.0, true);
            if (getTick() > flushendtag) {
                return S_FLUSHPULSEOFF;
            } else {
                return S_IDLE;
            }
        }
        else if (sensor_state == S_ZEROING) {
            int zerostatus = CalculateParams(SCALC_DRIFTLIMIT, 0.0, true);
            if ((zerostatus == 0) || zerostatus == 2) return S_IDLE;
            if (zerostatus == -1) {
                //log(INFO, "Returning Drift Fail...\r\n");
                return S_ZEROFAILED;
            }
            sum = 0;
            lesser = 0;
            sptr = pruns_tail;
            for (int i = 0; i < zero_driftwstart; i++) {
                sum += pruns[sptr];
                sptr++;
                sptr = sptr & 0x0f;
                if (pruns[sptr] < lesser) lesser = pruns[sptr];
                //log(INFO, "Zero sample =  %f\r\n", pruns[sptr] );

            }
            poffset = (sum / zero_driftwstart)* -1;
            if (poffset < 0) poffset = lesser * -1;
            //log(INFO, "Adjusting poffset to %f\r\n", poffset);

            usepoffset = true;
            pbfptr = 0;
            pbfload = 0;
            updatePRuns(true, 0.0);
            clearAlarms();
            setAlarm(1, 32, 32);
            sensor_state = S_IDLE;
            return S_ZEROENDED;
        }
        else if (sensor_state == S_CHARGING) {

            chstatus = CalculateParams(SCALC_PRESSUREOVER, load_tgtpressure - (pruns_dv * load_trigfactor), true);

            if (chstatus == 1) {
                activateValve(0, 0);
                activateValve(1, 0);
                activateValve(2, 1);
                CalculateParams(SCALC_SETWINDOW, load_driftwstart, false);
                driftwindowend = load_driftwend * -1;
                sensor_state = S_CHARGED;
            }
            return S_IDLE;
        }
        else if (sensor_state == S_CHARGED) {

            chstatus = CalculateParams(SCALC_DRIFTLIMIT, 0.0, true);

            if (chstatus == 0) return S_IDLE;
            if (chstatus == -1) return S_CHARGEFAILED;

            usepoffset = true;
            //		pbfptr = 0;
            //		pbfload = 0;
            //		updatePRuns(true, 0.0);
            //		clearAlarms();
            //		setAlarm(1,32,32);
            sensor_state = S_IDLE;
            return S_CHARGEENDED;

        }
        else if (sensor_state == S_EXPANDING) {

            chstatus = CalculateParams(SCALC_DRIFTLIMIT, 0.0, true);

            if (chstatus == 0) return S_IDLE;
            if (chstatus == -1) return S_EXPANDFAILED;

            //		usepoffset = true;
            //		pbfptr = 0;
            //		pbfload = 0;
            //		updatePRuns(true, 0.0);
            //		clearAlarms();
            //		setAlarm(1,32,32);
            sensor_state = S_IDLE;
            return S_EXPANDENDED;
        }


        return S_FAILED;

    }

    int Analise::CalculateParams(uint8_t type, double limit, bool showstatus) {

        //int driftcounter;
        double ctemp;
        //double reltemp;
        double press;

        // Trap setpoints
        if (type == SCALC_SETWINDOW) {
            lwindow = limit;
            return 0;
        }

        if (type == SCALC_TRIPCHARGE) {
            lwindow = 0;
            //double trippress = 16.0;
            return 0;
        }

        if (pbfptr < 255) {
            if (monitorpressure) {
                pbf[pbfptr] = Press;
            } else {
                pbf[pbfptr] = Temp;
            }

            pbfload++;
            pbfptr = pbfptr + 1;
        }

        if (alarmset[1] == true) {
            alarmset[1] = false;

            double avg = calculateMean(pbfload, pbf);

            if (monitorpressure) {
                press = userawcounts ? avg : (avg * pa1) + pa0;
                press = usepoffset ? press + poffset : press;
            } else {
                press = userawcounts ? avg : (avg * tempa1) + tempa0;
            }
            //log(INFO, "Calculated pressure = %+7.5f - av=+7.5f\r\n", press, avg);
            updatePRuns(false, press);

            pbfload = 0;
            pbfptr = 0;

            lastpressure = pruns_mvavg;

            lasttemperature = Temp;
            lasttemperature = (lasttemperature * tempa1) + tempa0;

            if (showstatus) {
                publish(event_t(UPDATEBARGRAPH_SIG, 0, new bgupdate_evt(1, 0, pruns_mvavg, NULL)));
                publish(event_t(UPDATEBARGRAPH_SIG, 0, new bgupdate_evt(3, 0, pruns_sd, NULL)));
                publish(event_t(UPDATEBARGRAPH_SIG, 0, new bgupdate_evt(2, 0, pruns_dv, NULL)));
            }
            if (monitor_on_term) {
                log(INFO, "%7d,%+6.4f,%+6.4f,%+6.4f,%d,%d,%d,%d\r\n", getRelativeTick(), pruns_mvavg, pruns_sd, pruns_dv,
                        valves[0] + 1, valves[1] + 3, valves[2] + 5, valves[2] + 7);
            }

            if (type == SCALC_FLUSHTIME) {
                return 1;
            }

            if (type == SCALC_PRESSURELESS) {
                if (pruns_mvavg > limit) {
                    return -1;
                } else if (lwindow > 0) {
                    lwindow--;
                    return 0;
                }
                return 1;
            }
            else if (type == SCALC_PRESSUREOVER) {
                if (lwindow > 0) {
                    //log(INFO, "Pressure over (lwindow active) w=%d @ %d = %+6.4f of %+6.4f\r\n", lwindow, getTick(), pruns_mvavg, limit);
                    lwindow--;
                    return 0;
                } else if (pruns_mvavg < limit) {
                    //log(INFO, "Pressure over (limit below) w=%d @ %d = %+6.4f of %+6.4f\r\n", lwindow, getTick(), pruns_mvavg, limit);
                    return -1;
                }
                //log(INFO, "Pressure over (limit reached) w=%d @ %d = %+6.4f of %+6.4f\r\n", lwindow, getTick(), pruns_mvavg, limit);
                return 1;
            }
            else if (type == SCALC_DRIFTLIMIT) {

                if (fabs(pruns_dv) < 0.5 && (pruns_dv != 0.0)) {
                    pdvs.push_front(pruns_dv);
                    lwindow--;
                    log(FINER, "Drift lwindow %d @ %d = %f\r\n", lwindow, getTick(), pruns_dv);
                } else {
                    log(FINER, "--lwindow of %f @ %d was rejected \r\n", pruns_dv, getTick());
                }

                if (lwindow >= 0) {
                    return 0;
                } else if (lwindow < driftwindowend) {
                    ctemp = 0;
                    for (std::deque<double>::iterator it = pdvs.begin(); it != pdvs.end(); ++it) {
                        ctemp += *it;
                    }
                    ctemp = fabs(ctemp / pdvs.size());
                    driftresult = ctemp;
                    if (ctemp > reldriftlimit) {
                        driftstatus = 1;
                        lastdrift = ctemp;
                        return -1;
                    } else {
                        lastdrift = ctemp;
                        driftstatus = 2;
                        return 2;
                    }
                } else {
                    pdvs.pop_back();
                    ctemp = 0;
                    //driftcounter = 0;
                    for (std::deque<double>::iterator it = pdvs.begin(); it != pdvs.end(); ++it) {
                        ctemp += *it;
                        //driftcounter++;
                        log(FINER, "\tDrift= %7.5f - %7.5f\r\n", *it, ctemp);
                    }
                    ctemp = fabs(ctemp);
                    if (ctemp < absdriftlimit) {
                        log(FINER, "Drift of %7.5f was accepted\r\n", ctemp);
                        lastdrift = ctemp;
                        return 1;
                    } else {
                        log(FINER, "Drift of %7.5f was rejected\r\n", ctemp);
                    }
                }
            }
        }
        return 0;
    }

    // Auxiliary plumbing Services

    void Analise::buildIconBar(uint8_t mode) {

        drawShapeRequest(SHP_STATUSBAR, 0U, 48U, false);
        man_icons[0] = SHP_ICON_VINP_OFF;
        man_icons[1] = SHP_ICON_VEXP_OFF;
        man_icons[2] = SHP_ICON_VVEN_OFF;
        man_icons[3] = SHP_ICON_VVAC_OFF;


        if (mode == 0) {
            man_icons[4] = SHP_ICON_TRANSZRO_SOFF;
        } else {
            man_icons[4] = SHP_ICON_CLEAR;
        }

        man_icons[5] = SHP_ICON_CLEAR;

        if (autostore) {
            man_icons[6] = SHP_ICON_SDCARD_ON;
        } else {
            man_icons[6] = SHP_ICON_SDCARD_OFF;
        }

        if (autoprint) {
            man_icons[7] = SHP_ICON_PRINTER_ON;
        } else {
            man_icons[7] = SHP_ICON_PRINTER_OFF;
        }


        //publish(event_t(UPDATEICON_SIG, 3, new icon_evt(0, 0, 0)));

        for (int i = 0; i < 8; i++) {
            publish(event_t(UPDATEICON_SIG, 0, new icon_evt(i, glcd::ICREFRESH, man_icons[i])));
        }

        man_iconptr = 4;
    }

    void Analise::buildBarGraphs() {



        if (monitorpressure) {
            publish(event_t(INITBARGRAPH_SIG, 0, new bginit_evt(1,
                    3,
                    0, // corner bargraph 
                    false, // right side of creen  
                    true, // draw literal
                    new std::string("%7.4f PSI"),
                    0.0,
                    20.0,
                    25.0
                    )));
        } else {
            publish(event_t(INITBARGRAPH_SIG, 0, new bginit_evt(1,
                    3,
                    0, // corner bargraph 
                    false, // right side of creen  
                    true, // draw literal
                    new std::string("%7.4f C"),
                    0.0,
                    20.0,
                    25.0
                    )));
        }
        publish(event_t(INITBARGRAPH_SIG, 0, new bginit_evt(3,
                4,
                0, // corner bargraph 
                false, // right side of creen  
                true, // draw literal
                new std::string("%7.4f %%SD"),
                0.0,
                1.0,
                9.9
                )));

        publish(event_t(INITBARGRAPH_SIG, 0, new bginit_evt(2,
                5,
                1, // center 
                false, // right side of creen  
                true, // draw literal
                new std::string("%+2.4f Dv"),
                0.0,
                0.1,
                9.9
                )));

        clearAlarms();
        setAlarm(1, 32, 32);
        pbfptr = 0;
        pbfload = 0;
        updatePRuns(true, 0.0);
    }

    std::string * Analise::getFinalString(uint8_t mode, uint8_t slot, bool media) {
        char buf[64];
        double temp1, temp2;

        if (media) {
            if (mode == 0) {
                temp1 = getMedia(0); //lruns.at(slot).density;
                temp2 = getMedia(1); //lruns.at(slot).temperature;
                sprintf(buf, "DEN: %6.4f g/cc @ %3.1fC", temp1, temp2);
            } else if (mode == 1) {
                temp1 = getMedia(2); //lruns.at(slot).volume;
                temp2 = lruns.at(slot).cyclersd;
                sprintf(buf, "VOL: %6.3f cc (%6.3f%%)", temp1, temp2);
            } else if (mode == 2) {
                sprintf(buf, "Estavel apos %d exp.", lruns.size());
            } else {
                temp1 = getMedia(3); //lruns.at(slot).loadpressure;
                temp2 = getMedia(4); //lruns.at(slot).expandpressure;
                sprintf(buf, "RAZAO: %6.4f / %6.4f", temp1, temp2);
            }
        } else {
            if (mode == 0) {
                temp1 = lruns.at(slot).density;
                temp2 = lruns.at(slot).temperature;
                sprintf(buf, "DEN: %6.4f g/cc @ %3.1fC", temp1, temp2);
            } else if (mode == 1) {
                temp1 = lruns.at(slot).volume;
                temp2 = lruns.at(slot).cyclersd;
                sprintf(buf, "VOL: %7.4f cc (%6.4f%%)", temp1, temp2);
            } else if (mode == 2) {
                sprintf(buf, "Estavel apos %d exp.", lruns.size());
            } else if (mode == 3) {
                temp1 = lruns.at(slot).loadpressure;
                temp2 = lruns.at(slot).expandpressure;
                sprintf(buf, "RAZAO: %6.4f / %6.4f", temp1, temp2);
            }
        }
        return new string(buf);
    }

    double Analise::getMedia(uint8_t mode) {

        double val = 0;
        double temp = 0;
        int count = 0;
        int lsize = lruns.size() - 1;
        int i;

        if (mode == 0) { // Density
            for (i = 0; i < lsize; i++) {
                temp += lruns.at(i).density;
                count++;
            }
            val = temp / count;
        } else if (mode == 1) { // temp
            for (i = 0; i < lsize; i++) {
                temp += lruns.at(i).temperature;
                count++;
            }
            val = temp / count;
        } else if (mode == 2) { // Volume 
            for (i = 0; i < lsize; i++) {
                temp += lruns.at(i).volume;
                count++;
            }
            val = temp / count;
        } else if (mode == 3) { // Load Pressure
            for (i = 0; i < lsize; i++) {
                temp += lruns.at(i).loadpressure;
                count++;
            }
            val = temp / count;
        } else if (mode == 4) { // Expand Pressure
            for (i = 0; i < lsize; i++) {
                temp += lruns.at(i).expandpressure;
                count++;
            }
            val = temp / count;
        } else {
            val = 0;
        }
        return val;

    }

    std::string * Analise::getRunsString(uint8_t run) {
        char buf[64];
        sprintf(buf, "Exp. %2d @ %s", run + 1, getFormatedTime("", lruns.at(run).timestamp));
        return new string(buf);
    }

    std::string * Analise::getVCResultString() {
        char buf[64];
        sprintf(buf, "VC=%6.4fcc\nPRatio=%6.4f\nAtual. Valor ?", vcells[celltype], sphereratio);
        return new string(buf);
    }

    std::string * Analise::getVAResultString() {
        char buf[64];
        sprintf(buf, "VA=%6.4fcc\nE=%6.4f S=%6.4f\nAtual. Valor ?", vaddeds[celltype], emptyratio, sphereratio);
        return new string(buf);
    }

    std::string * Analise::getRSDString(double rsd, double mean) {
        char buf[48];
        sprintf(buf, "%%RSD:%6.4f Media:%6.4f", rsd, mean);
        return new string(buf);
    }

    char * Analise::getFormatedTime(char * format, long rtime) {
        strftime(tsbuf, 128, "%T", localtime(&rtime));
        return &tsbuf[0];
    }

    std::string * Analise::getTimerString(double val, double of) {
        char buf[20];
        sprintf(buf, "Fluxo: %3.0f de %3.0f Seg", val, of);
        return new string(buf);
    }

    std::string * Analise::getCycleString(double cycle, double max) {
        char buf[20];
        sprintf(buf, "Ciclo %3.0f de %3.0f", cycle, max);
        return new string(buf);
    }

    std::string * Analise::getFlushPulseString(double pulse, double of) {
        char buf[20];
        sprintf(buf, "Pulso: %3.0f de %3.0f", pulse, of);
        return new string(buf);
    }

    std::string * Analise::getPressureString(bool added) {
        char buf[32];
        if (added) {
            sprintf(buf, "Expancao:%7.4f PSI", lastpressure);
        } else {
            sprintf(buf, "Carga : %7.4f PSI", lastpressure);
        }

        return new string(buf);
    }

    std::string * Analise::getPartialDenString() {
        char buf[32];
        double lvp = calculateVP(lruns.back().pratio);
        double lden = calculateDEN(lvp);
        
        sprintf(buf, "Den : %7.4f g/cc", lden);
        
        return new string(buf);
    }
    
    std::string * Analise::getOKtoStartString() {
        char buf[32];
       
//        if (celltype == 2){
//            sprintf(buf, "OK para iniciar analises\nusando a celula\nPequena ?" );
//        }
//        else if (celltype == 1){
//            sprintf(buf, "OK para iniciar analises\nusando a celula\nMedia ?" );
//        }
//        else  if (celltype == 0){
//            sprintf(buf, "OK para iniciar analises\nusando a celula\nGrande ?" );
//        }
//        else{
            sprintf(buf, "OK para iniciar\nos ciclos de\nanalises ?%d", 1 );
//        }
        
        return new string(buf);
    }
    
    
    std::string * Analise::getDriftStatusString() {
        char buf[32];
        if (driftstatus == 0) {
            return new string("Estavel - sem desvio.");
        } else if (driftstatus == 2) {
            sprintf(buf, "Desvio @ %7.4f", driftresult);
        } else {
            sprintf(buf, "Instavel: > %7.4f", reldriftlimit);
        }
        return new string(buf);
    }

    void Analise::reportCycles() {
        uint8_t cnt = 0;
        for (std::vector<loadrun>::iterator it = lruns.begin(); it != lruns.end(); ++it) {
            log(INFO, "%2d, %s, %6.4f, %6.4f, %6.4f, %6.4f, %5.2f, %d, %6.4f, %6.4f\r\n", cnt, getFormatedTime("", it->timestamp),
                    it->loadpressure, it->expandpressure, it->deltap, it->pratio,
                    it->temperature, it->loadstatus,
                    it->cycleavg, it->cyclersd);
            cnt++;
        }

    }

    std::string * Analise::getTempString(uint8_t mode) {
        char buf[32];
        sprintf(buf, "Temperatura:%6.2f C", lasttemperature);
        return new string(buf);
    }

    void Analise::pushEvent(event_t evt) {
        events.push(evt);
    }

    double Analise::getAccessCode() {
        return acode;
    }

    double Analise::getUser() {
        return user;
    }

    void Analise::setUser(double usertype) {
        user = usertype;
    }

}



            //		int r;
            //		double dr;
            //		
            //		for (int i = 0; i < 10; i++) {
            //			time (&rawtime);
            //			r = rand();
            //			r = (r % 100);
            //			dr = r/100.0;
            //			lruns.push_back(loadrun (rawtime, 17.9805, 0, 0.0002));
            //			//lruns.back().density = 7.9 + dr;
            //			lruns.back().temperature = 20.0 + dr;
            //			lruns.back().cycleavg = 2.02 + (dr/100);
            //			lruns.back().cyclersd = 0.0 + (dr/10);
            //		}




            //	else if (a_state == PRINTANALISE){
            //		if (!calibrate) publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(04,8,120,true,false,true,1, getFinalString())));
            //		if (autoprint){
            //			print(1);
            //			a_state = ANALISEGAP;
            //		}
            //		else if ((asktoprint) && (!autorun)){
            //			//log(PLUMB, "Ask to print \r\n");
            //			publish(event_t(DRAWDIALOG_SIG, 0, new dialog_evt(1,
            //								PRINTREQUESTED,
            //								ANALISEGAP, 
            //								ABORTRUN, 
            //								new std::string("Print this last\nResult ?")
            //								)));
            //			a_state = WAITPRINT;
            //			dlgavailable = false; 
            //		}
            //		else{
            //			a_state = ANALISEGAP;
            //		}
            //	}
            //	else if (a_state == WAITPRINT){
            //		if(dlgavailable){
            //			//log(PLUMB, "Print dialog answered %d \r\n", dlganswer);
            //		    dlgavailable = false; 
            //			a_state = dlganswer;
            //		}
            //	}
            //	else if (a_state == PRINTREQUESTED){
            //		print(1);
            //		a_state = ANALISEGAP;
            //	}





            //	
            //	
            //	else if (a_state == ASKSID){		
            //		log(PLUMB, "Asking for sample id \r\n");
            //		publish(event_t(DRAWINPUT_SIG, 0, new input_evt( SHP_INPUTFULL,
            //							false,false,false,storehistory,
            //							new std::string("Please enter sample ID"), 
            //							new std::string(&sampleid[0]), 
            //							new std::string("%s"), 
            //							new std::string(&sidhistoryfile[0]),
            //							0.0, 0.0, 0.0)));
            //		a_state = WAITSID;
            //	}
            //	else if (a_state == WAITSID){
            //		if(inputavailable){
            //			tempkcode=0;
            //			inputavailable = false;
            //			std::strncpy (&sampleid[0], &inputsval[0], 30);
            //		    sampleid[31]=0x0;
            //			a_state = ASKSAMPLEREADY;
            //		}
            //	}
            //	
            //	else if (a_state == ASKSAMPLEREADY){		
            //		//log(PLUMB, "Asking if sample ready \r\n");
            ////		publish(event_t(DRAWDIALOG_SIG, 0, new dialog_evt(1,
            ////							INITRUN,
            ////							ASKPOROSITY, 
            ////							ABORTRUN, 
            ////							"Test"
            ////							)));
            //		a_state = WAITSAMPLEREADY;
            //	}
            //	else if (a_state == WAITSAMPLEREADY){
            //		if(dlgavailable){
            //		    dlgavailable = false; 
            //			a_state = dlganswer;
            //		}
            //	}
            //	
            //	else if (a_state == INITRUN){
            ////		//log(PLUMB, "Run is starting... \r\n");
            ////		publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(04,8,120,true,false,true,1, new std::string("Charging Column"))));
            ////		publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(24,24,0,false,false,false,0, new std::string("        "))));
            ////		setTemperature(runnumber);
            ////		publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(20,40,0,false,false,false,0, getTempString(runnumber, (char*)"%4.2f"))));
            ////		
            ////		uint8_t lin = 24+(runnumber*8);		
            ////		publish(event_t(DRAWMESSAGE_SIG, 0, new drawmessage_evt(70,lin,0,true,false,false,0, new std::string(" ----.-"))));
            ////
            ////		checkInstrument();
            ////		chargeSystem(true);
            ////		a_state = CHARGING;
            //	}


//        spherevolumes.push_back(s_keymap("Sphere-Small", "7.0699"));
//        spherevolumes.push_back(s_keymap("1000 Large Compose", "70.699"));
//        spherevolumes.push_back(s_keymap("Micro Large Compose", "2.1450"));
//        spherevolumes.push_back(s_keymap("ACP-Beckman Small", "2.0673"));
//        spherevolumes.push_back(s_keymap("Sphere-Large", "56.5592"));
//        spherevolumes.push_back(s_keymap("Sphere-Medium", "28.9583"));
//        spherevolumes.push_back(s_keymap("Sphere-Micro", "1.0725"));
//        spherevolumes.push_back(s_keymap("Sphere-Nano", "0.0898"));

        //	cellvolumes.push_back(s_keymap("ACP-AutoDensity 100", "154.7174"));
        //	cellvolumes.push_back(s_keymap("Cell-Large", "131.7"));
        //	cellvolumes.push_back(s_keymap("Cell-Medium", "48.1"));
        //	cellvolumes.push_back(s_keymap("Cell-Small", "10.8"));
        //	cellvolumes.push_back(s_keymap("Cell-Micro", "4.5"));
        //	cellvolumes.push_back(s_keymap("Cell-Meso", "1.8"));
        //	cellvolumes.push_back(s_keymap("Cell-Nano", "0.25"));