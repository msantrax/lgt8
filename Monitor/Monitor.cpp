/*
 * Monitor.cpp
 *
 *  Created on: 09/01/2017
 *      Author: root
 */

#include <stdlib.h>

#include "fsl_gpio.h"
#include <stdio.h>

#include "Monitor.h"
#include "antares.h"

using namespace std;
using namespace antares;


namespace antares {


Monitor::Monitor() {
	// TODO Auto-generated constructor stub

}

Monitor::~Monitor() {
	// TODO Auto-generated destructor stub
}



Monitor* Monitor::Initialize(){
    	Monitor *retval(NULL);
    	retval = new Monitor();
		return (retval);
}

void Monitor::start(){

	setDrum(false);
	setVaccum(true);
	setWg20Cw(true);
	wg20pwm =0;
	setWG20Vel();

	logMessage(INFO, "Monitor Driver attached to core, we have LVDT and auxiliar services now\r\n");
    return;
}




// ===== Events =====================================================================================================
void Monitor::pushEvent(event_t evt){
	events.push(evt);
}

void Monitor::serviceEvents(){

	while(!events.empty()){
		dummyevt = &events.front();

		if (dummyevt->evtype == MONITOR_RESET_SIG){
			//_fsm.Tick();
		}

		else if (dummyevt->evtype == MONITOR_ETH_SIG){
			antares_msg_struct_t * ia = static_cast<antares_msg_struct_t*>(dummyevt->payload);
			uint8_t cmd = ia->cmd;
			uint8_t arg = ia->arg;
			char *mes = (char*)ia->data;
			//logMessage(INFO, "Received MONITOR USB Packet : %d %d\r\n", cmd, arg);


			if (cmd == MON_SETVAC){ // Vácuo
				if(arg==1){
					setVaccum(true);
					logMessage(INFO, "Vaccum activated\r\n");
				}
				else{
					setVaccum(false);
					logMessage(INFO, "Vaccum deactivated\r\n");
				}

			}
			else if (cmd == MON_SETDRUM){ // Drum
				if(arg==1){
					setDrum(true);
					logMessage(INFO, "Drum activated\r\n");
				}
				else{
					setDrum(false);
					logMessage(INFO, "Drum deactivated\r\n");
				}
			}
			else if (cmd == WG20_SETDIR){ // WG20 dir
				if(arg==1){
					setWg20Cw(true);
				}
				else{
					setWg20Cw(false);
				}
				logMessage(INFO, "WG20 dir set to : %d\r\n", arg);
		    }

			else if (cmd == WG20_SETVEL){ // WG20 vel
				if (arg == 0){
					setWg20Cw(false);
				}
				else{
					setWg20Cw(true);
				}
				sscanf(mes,"%d", &wg20pwm);
				setWG20Vel();
				logMessage(INFO, "WG20 Vel set to  %d / %d\r\n", wg20pwm, isWg20Cw());
			}

			else if (cmd == MON_RESET){ // monitor reset
				logMessage(INFO, "monitor reset received\r\n");
				setDrum(false);
				setVaccum(true);
				setWg20Cw(true);
				setPWM(1, 0);
			}

			else{
				logMessage(INFO, "Received unknown Monitor Packet : %d, %d\r\n", cmd, arg);
			}

			delete(ia);
		}

		events.pop();
	}
}

void Monitor::setWG20Vel(){

	//	if (wg20pwm == 0){
//		BOARD_WG2VEL_OFF();
//	}
//	else{
//		BOARD_WG2VEL_ON();
//	}

	setPWM(1,wg20pwm);
	storeMonitorSlot (WG2_SPEED_SLOT, wg20pwm);
}


void Monitor::serviceBoard(){





}


bool Monitor::isDrum() {
	return drum;
}

void Monitor::setDrum(bool drum) {
	this->drum = drum;
	DRUM_ENABLE_TOGGLE();

//	if(drum){
//		DRUM_ENABLE_ON();
//	}
//	else{
//		DRUM_ENABLE_OFF();
//	}
	storeMonitorFlag (DRUM_ENABLED, drum);

}

bool Monitor::isVaccum() {
	return vaccum;
}


void Monitor::setVaccum(bool vaccum) {
	this->vaccum = vaccum;
	if(vaccum){
		VAC_ENABLE_ON();
	}
	else{
		VAC_ENABLE_OFF();
	}
	storeMonitorFlag (VAC_ENABLED, vaccum);
}

bool Monitor::isWg20Cw() {
	return wg20_cw;
}


void Monitor::setWg20Cw(bool wg20Cw) {
	wg20_cw = wg20Cw;
	if(wg20Cw){
		BOARD_WG2DIR_ON();
	}
	else{
		BOARD_WG2DIR_OFF();
	}
	storeMonitorFlag (WG20_DIR, wg20_cw);
}





} /* namespace antares */
