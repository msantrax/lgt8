

#include <sys/time.h>
#include <stdlib.h>

#include "fsl_gpio.h"

#include "sweep.h"
#include "antares.h"

using namespace std;
using namespace antares;


namespace antares {

    Sweep::Sweep(const statemap::State& state) : _fsm(*this, state) {
        _fsm.setDebugFlag(false);
    }

    void Sweep::start(){

    	//intap = GPIO_ReadPinInput(BOARD_STEP_INTAP_GPIO,BOARD_STEP_INTAP_GPIO_PIN);
    	//outtap = GPIO_ReadPinInput(BOARD_STEP_OUTTAP_GPIO,BOARD_STEP_OUTTAP_GPIO_PIN);
    	readSensors();

    	sweepmapinit = &sweepmap[0];
    	clearMap();
    	sweepmapptr = sweepmapinit + (2000 * sizeof(uint8_t));
    	sweeppos=2000;

    	setEnable(false);
    	setOutDir(false);
    	setPulse (true);

    	pulsegap=0;
    	sweepjob_enabled=false;

        _fsm.enterStartState();

        logMessage(INFO, "Sweep Driver attached to core, we have step motor services now\r\n");
        return;
    }

    Sweep* Sweep::Initialize(){
    	Sweep *retval(NULL);
    	retval = new Sweep(antares::Adm::ColdStart);
		return (retval);
    }



    // ===== Events =====================================================================================================
    void Sweep::pushEvent(event_t evt){
    	events.push(evt);
    }

    void Sweep::serviceEvents(){

    	while(!events.empty()){
    		dummyevt = &events.front();

    		if (dummyevt->evtype == SWEEP_RESET_SIG){
    			_fsm.Tick();
    		}

    		else if (dummyevt->evtype == SWEEP_ETH_SIG){
    			antares_msg_struct_t * ia = static_cast<antares_msg_struct_t*>(dummyevt->payload);
    			uint8_t cmd = ia->cmd;
    			uint8_t arg = ia->arg;
    			char *mes = (char*)ia->data;
    			//logMessage(INFO, "Received SWEEP Packet : %d %d\r\n", cmd, arg);

    			if (cmd == SWEEP_PARK){ // Park CMD (30)
    				logMessage(INFO, "Sweep Park requested\r\n");
    				_fsm.Park();
    			}

    			else if (cmd == SWEEP_GOTO){ // Goto CMD (31)
    				int stp_pos;
    				int stp_vel;
    				sscanf(mes,"%d %d", &stp_pos, &stp_vel);
    				logMessage(INFO, "Sweep goto : %d / %d\r\n", stp_pos, stp_vel);
					Goto(stp_pos, stp_vel);
				}

    			else if (cmd == SWEEP_SCAN){ // SweepJob CMD (32)
    				if (arg==1){
    					int jobin;
    					int jobout;
    					sscanf(mes,"%d %d %d", &jobin, &jobout, &sweepjob_speed);
    					sweepjob_out = jobout * 40;
    					sweepjob_in = jobin * 40;
    					sweepjob_outdir=true;
    					sweepjob_enabled=true;
    					logMessage(INFO, "Sweep scan : %d / %d / %d\r\n", sweepjob_in, sweepjob_out, sweepjob_speed );
    					events.push(event_t(SWEEP_SWEEPJOB_SIG, 1, sweepmapptr));
    				}
    				else{
    					logMessage(INFO, "Sweep scan was aborted\r\n");
    					sweepjob_enabled=false;
    				}
				}
    			else if (cmd == SWEEP_ENABLE){ // Enable CMD (40)
    				if(arg==1){
    					setEnable(true);
    				}
    				else{
    					setEnable(false);
    				}
    				//STEP_ENABLE_TOGGLE();

				}
    			else if (cmd == SWEEP_DIR){ // Dir CMD (41)
    				if(arg==1){
    					setOutDir(true);
					}
					else{
						setOutDir(false);
					}
    				//STEP_DIR_TOGGLE();
    				logMessage(INFO, "Sweep dir set to : %d\r\n", outdir);
				}
    			else if (cmd == SWEEP_PULSE){ // PulseCMD (42)
    				int bpulses;
					int bperiod;
					sscanf(mes,"%d %d", &bpulses, &bperiod);
					logMessage(INFO, "Sweep Pulse Burst requested : %d  / %d\r\n", bpulses, bperiod);
    				PulseBurst();

				}
    			else{
    				logMessage(INFO, "Received Unknown Sweep Packet : %d %d\r\n", cmd, arg);
    			}


    			delete(ia);
    		}

    		else if (dummyevt->evtype == SWEEP_JOBDONE_SIG){
    			_fsm.JobDone();
    		}

    		else if (dummyevt->evtype == SWEEP_TICK_SIG){
				_fsm.Tick();
			}

    		else if (dummyevt->evtype == SWEEP_SWEEPJOB_SIG){
    			SweepJob();
    		}

    		events.pop();
    	}
    }

    void Sweep::serviceBoard(){

    	uint8_t temp;

    	if (enable){
    		if (*sweepmapptr!= 255){
				if (pulsegap==0){
					pulsegap = *sweepmapptr;
					setOutDir(outdir);
				}
				if (*sweepmapptr==0){
					if(pulse){
						// drop pulse level
						*sweepmapptr=pulsegap;
						setPulse(false);
					}
					else{
						// raise pulse level
						setPulse(true);
						pulsegap=0;
						*sweepmapptr=255;
						// Goto next pulse
						if (outdir){
							sweepmapptr++;
							sweeppos--;
						}
						else{
							sweepmapptr--;
							sweeppos++;
						}
					}
					readSensors();
				}
				else{
					temp = *sweepmapptr;
					*sweepmapptr = --temp;
				}
    		}
    		else{
    			//setEnable(false);
    			setPulse (true);
    			pulsegap=0;
    			readSensors();
    			//events.push(event_t(SWEEP_JOBDONE_SIG, 1, sweepmapptr));
    			if (sweepjob_enabled) events.push(event_t(SWEEP_SWEEPJOB_SIG, 1, sweepmapptr));
    		}
    	}
    	else{
    		readSensors();
    	}


    }

    void Sweep::readSensors(){

    	uint8_t mon= 0x00;

    	temptap = GPIO_ReadPinInput(BOARD_STEP_INTAP_GPIO,BOARD_STEP_INTAP_GPIO_PIN);
    	intap=temptap;
    	mon = setFlag32(mon, 0x02, intap);

//    	if(temptap != intap){
//			intap=temptap;
//			intapptr = sweepmapptr;
//			mon = setFlag32(mon, 0x02, intap);
//			update = true;
//			//logMessage(INFO, "Intap : %d %p\r\n", intap, intapptr - sweepmapinit);
//		}

		temptap = GPIO_ReadPinInput(BOARD_STEP_OUTTAP_GPIO,BOARD_STEP_OUTTAP_GPIO_PIN);
		outtap=temptap;
		mon = setFlag32(mon, 0x04, outtap);

//		if(temptap != outtap){
//			outtap=temptap;
//			outtapptr = sweepmapptr;
//			mon = setFlag32(mon, 0x02, outtap);
//			update = true;
//			//logMessage(INFO, "Outtap : %d %p\r\n", outtap, outtapptr - sweepmapinit);
//		}

		tparked = ((intap==0) && (outtap==0));
		if(!parked && tparked){
			parkedptr = sweepmapptr;
			//logMessage(INFO, "Parked : %d %p\r\n", tparked, parkedptr - sweepmapinit);
		}
		parked=tparked;

		mon = setFlagB(mon, 0x01, enable);

		storeMonitorFlag (1, mon);


    }


    uint8_t Sweep::setFlag32(uint8_t mon, uint8_t mask, uint32_t value){

    	uint8_t out = mon;
    	//uint8_t nmask = ~mask;

    	if (value == 1){
    		out = mon | mask;
    	}
    	else{
    		out = mon & ~mask;
    	}

    	return out;
    }


    uint8_t Sweep::setFlagB(uint8_t mon, uint8_t mask, bool value){

       	uint8_t out = mon;

       	if (value){
       		out = mon | mask;
       	}
       	else{
       		out = mon & ~mask;
       	}

       	return out;
   }




    // ========= Core Routines ============================================================================================


    void Sweep::Park(){
    	logMessage(INFO, "Parked found @ %p\r\n", sweepmapptr-sweepmapinit);
    	clearMap();
    	sweepmapptr = sweepmapinit + (2000 * sizeof(uint8_t));
    	sweeppos=2000;
    }


    void Sweep::Slide (int step, int speed){

    	uint8_t *tptr;
    	int astep = abs(step);
    	tptr = sweepmapptr;

    	if (step>0){
    		setOutDir(false);
    	}
    	else{
    		setOutDir(true);
    	}


    	for (int i=0; i<astep; i++){
//			step = ((50-i)+1)/2;
//			if (step<2) step=2;
			*tptr = speed;

			if(outdir){
				tptr++;
			}
			else{
				tptr--;
			}
		}
    	setEnable(true);
    }

    void Sweep::Goto(int step, int speed){

    	int dif1 = step - sweeppos;
    	if (dif1 !=0){
    		Slide(dif1, speed);
    	}
    }

    void Sweep::SweepJob(){

    	if(sweepjob_enabled){
    		if (sweepjob_outdir){
    			Goto(sweepjob_out, sweepjob_speed);
    		}
    		else{
    			Goto(sweepjob_in, sweepjob_speed);
    		}
    		sweepjob_outdir = !sweepjob_outdir;
    	}
    }

    void Sweep::PulseBurst(){

    	int i,j;

    	for (i=0; i<10000; i++){
    		STEP_PULSE_TOGGLE();
    		for (j=0;j<5000; j++){}
    	}
    }


    void Sweep::clearMap(){
    	memset(sweepmapinit, 0xff, sizeof(uint8_t)*4000);
    }


    void Sweep::TickCallback(){
    	events.push(event_t(SWEEP_TICK_SIG, 1, sweepmapptr));
    }

    void Sweep::ShowStatus(char * mes){
		logStringnl(INFO, (char*)mes);
    }

    void Sweep::ToggleFlag(){
    	//logStringnl(INFO, (char*)"Received USB Packet");
    	//STEP_DIR_TOGGLE();
    }

    void Sweep::ClearQueue(){
    	//logStringnl(INFO, (char*)"Clearing queue");
    }

    void Sweep::SetupHardware(){
    	//logStringnl(INFO, (char*)"Setup hardware");
    }

    void Sweep::JobisDone(){
		//logStringnl(INFO, (char*)"Job is done");
    }

    // GetSet =============================================================================================================

    void Sweep::setEnable(bool set){

    	enable = set;
    	if(!enable){
    		STEP_ENABLE_ON();
    		logMessage(INFO, "Sweep enable off\r\n");
    	}
    	else{
    		STEP_ENABLE_OFF();
    		logMessage(INFO, "Sweep enable on\r\n", enable);
    	}
    	storeMonitorFlag (SWEEP_ENABLED, enable);
    }

    void Sweep::setOutDir(bool set){

		outdir = set;
		if(outdir){
			STEP_DIR_OFF();
		}
		else{
			STEP_DIR_ON();
		}
		storeMonitorFlag (SWEEP_OUTDIR, outdir);
	}

    void Sweep::setPulse(bool high){

		pulse = high;
		if(pulse){
			STEP_PULSE_ON();
		}
		else{
			STEP_PULSE_OFF();
		}
	}

    bool Sweep::isParked(){
       	readSensors();
       	return parked;
    }

    bool Sweep::isInside(){
		readSensors();
		return intap;
	}

    bool Sweep::isOutside(){
		readSensors();
		return outtap;
	}


}
