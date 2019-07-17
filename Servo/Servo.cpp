/*
 * Servo.cpp
 *
 *  Created on: 18/11/2016
 *      Author: root
 */

#include <sys/time.h>
#include <stdlib.h>

#include "fsl_gpio.h"

#include "Servo.h"
#include "antares.h"

using namespace std;
using namespace antares;


namespace antares {

	Servo::Servo(const statemap::State& state) : _fsm(*this, state) {
        _fsm.setDebugFlag(false);
        present_pwm0 = 1;
        request_pwm0 = 1;

       clearTachBuffer();
       setServoPID(1);
       setServoTransfer();

       SERVO_STATE_OFF();
       SERVO_CLK_ON();
    }

	Servo::~Servo() {
		// TODO Auto-generated destructor stub
	}

	void Servo::start(){
		_fsm.enterStartState();
		logMessage(INFO, "Servo Driver attached to core, we have plate motor services now\r\n");
		return;
	}

	Servo* Servo::Initialize(){
		Servo *retval(NULL);
		retval = new Servo(antares::ServoAdm::ColdStart);
		return (retval);
	}


	// ===== Events =====================================================================================================
	void Servo::pushEvent(event_t evt){
		events.push(evt);
	}

	void Servo::serviceEvents(){

		while(!events.empty()){
			dummyevt = &events.front();

			if (dummyevt->evtype == SERVO_RESET_SIG){
				_fsm.Tick();
			}

			else if (dummyevt->evtype == SERVO_ETH_SIG){
				antares_msg_struct_t * ia = static_cast<antares_msg_struct_t*>(dummyevt->payload);
				uint8_t cmd = ia->cmd;
				uint8_t arg = ia->arg;
				char *mes = (char*)ia->data;
				//logMessage(INFO, "Received SERVO Packet : %d %d\r\n", cmd, arg);

				if (cmd == SERVO_ENABLE){ // PWM Enable (30)
					if (arg==1){
						enable_servo = true;
						clearTachBuffer();
						svctl.dState=0;
						svctl.iState=0;
						sscanf(mes,"%d", &request_pwm0);
						EnableServo(true);
					}
					else {
						enable_servo = false;
						request_pwm0 = 0;
						servo_setpoint = 0.0;
						EnableServo(false);
					}

					logMessage(INFO, "Servo enabled seto to : %d \r\n", enable_servo);
				}

				else if (cmd == SERVO_SETPWM){ // Set PWM 0 CMD (32)
					servo_setpoint=0;
					if (arg==1){
						sscanf(mes,"%d", &request_pwm0);
						logMessage(INFO, "Servo PWM set to : %d \r\n", request_pwm0);
					}
					else{
						request_pwm0 = 0;
						logMessage(INFO, "Servo PWM was reseted \r\n");
					}
				}

				else if (cmd == SERVO_SETSPEED){ // Set Speed (33)
					if (arg==1){
						int itemp;
						sscanf(mes,"%d", &itemp);
						servo_setpoint = itemp;
						logMessage(INFO, "Servo Setpoint set to : %d\r\n", itemp);
					}
					else{
						servo_setpoint=0;
						speed = 0;
						storeMonitorSlot (SERVO_SPEED_SLOT, 0);
						storeMonitorSlot (SERVO_TORQUE_SLOT, 0);
						logMessage(INFO, "Servo Setpoint was reseted \r\n");
					}
				}

				else{
					logMessage(INFO, "Received unknown SERVO Packet : %d, %d\r\n", cmd, arg);
				}
				delete(ia);
			}

			else if (dummyevt->evtype == SERVO_TICK_SIG){
				_fsm.Tick();
			}

			events.pop();
		}
	}


    void Servo::TickCallback(){
    	events.push(event_t(SERVO_TICK_SIG, 1, sweepmapptr));
    }


    void Servo::resetPWM(){

    	setPWM(0, 0);
    	present_pwm0 = 0;
    	request_pwm0 = 0;
    }

	void Servo::serviceBoard(){

		uint8_t i;
		uint32_t tmptick;
		double speed1=0;
		double speed2=0;

//		antares_msg_struct * ames;
//		char * amesptr;


		double l_gamma;
		double l_requestpwm;

		// Update PWM if requested
		if(request_pwm0 != present_pwm0){
			setPWM(0, request_pwm0);
			present_pwm0 = request_pwm0;
		}

		// Update Speed
		if (updatetach_flag > UPDATETACH_PERIOD){
			tachval=0;
			for (i=0; i<5; i++){
				tachval+=tachbuffer[i];
				tachbuffer[i]=0;
			}
			updatetach_flag=0;
			if (tachval !=0 || servo_setpoint != 0){
				tmptick = getTick();
				if (tachval == 0){
					speed = 0.0;
				}
				else{
					speed1 = 1.0/tachval;
					speed2 = speed1/32.0;
					speed = speed2 * 300000.0;
				}

				if(servo_setpoint == 0){
					//request_pwm0 = 0;
					l_gamma = 0.0;
				}
				else{
					l_gamma = updateServo(&svctl, speed-servo_setpoint, speed);
					l_requestpwm = calculatePWM(l_gamma);

					if (l_requestpwm > 80) l_requestpwm = 80;
					request_pwm0 = l_requestpwm;
				}
				storeMonitorSlot (SERVO_SPEED_SLOT, speed);
				storeMonitorSlot (SERVO_TORQUE_SLOT, l_gamma);

				//PRINTF((char*)"T:%d -> %d  - [%2.4f] [%2.4f / %2.4f] \r", tmptick, tachval, speed, l_gamma, l_requestpwm);
				updatestatus = true;
			}
		}
		else{
			updatetach_flag++;


//			if (updatestatus){
//				ames = new antares_msg_struct;
//				ames->cmd=CMD_CSTATUS;
//				ames->arg = 1;
//				ames->dest = 2;
//				ames->orig = 1;
//				ames->seq[0] = 0;
//				ames->seq[1] = 0;
//				amesptr = (char *)&ames->data[0];
//				ames->size = sprintf (amesptr, "%2.4f %2.4f %2.4f", l_gamma, l_requestpwm, speed);
//				//delete (ames);
//				loadCallBack(ames);
//				updatestatus = false;
//			}
		}

	}

	double Servo::updateServo(SPid * pid, double error, double process){

		double pTerm, dTerm, iTerm;

		// calculate the proportional term
		pTerm = pid->pGain * error;

		// calculate the integral state with appropriate limiting
		pid->iState += error;
		if (pid->iState > pid->iMax){
			pid->iState = pid->iMax;
		}
		else if (pid->iState < pid->iMin){
			pid->iState = pid->iMin;
		}
		iTerm = pid->iGain * pid->iState;  // calculate the integral term

		dTerm = pid->dGain * (process - pid->dState);
		pid->dState = process;

		return pTerm + iTerm - dTerm;

	}

	double Servo::calculatePWM(double gamma){

		double temp;

		temp = gamma * gamma;
		temp = temp * servotransfer.a2;
		temp = temp + (gamma * servotransfer.a1);
		temp = temp + servotransfer.a0;

		if (temp < 0) temp = 0;
		if (temp > 100) temp = 100;

		return temp;
	}

	void Servo::setServoPID(uint8_t profile){

		profile++;
		svctl.pGain = -0.2;
		svctl.iGain = -0.5;
		svctl.dGain = 0.1;

		svctl.iMax = 300;
		svctl.iMin = -300;

		servo_setpoint = 0;
	}


	void Servo::setServoTransfer(){
		servotransfer.a0 = 19.0818770428 ;
		servotransfer.a1 = 1.3861602127;
		servotransfer.a2 = -0.0059742764;
	}


	void Servo::serviceTach(uint32_t tick){

		this_tach=tick-last_tach;
		last_tach=tick;

		if (this_tach > 2000) this_tach=0;

		if (tachbufferptr++ > 4) tachbufferptr=0;
		tachbuffer[tachbufferptr] = this_tach;

		if (tach_theta > 160) tach_theta = 0;
		tach_theta++;

	}

	void Servo::clearTachBuffer(){

		last_tach = getTick();
		this_tach = 0;
		tachbufferptr = 0;
		updatetach_flag = 0;;
		tach_theta = 0;
		speed = 0.0;
		tachval = 0;

		for (int i=0; i<5; i++){
			tachbuffer[i]=0;
		}

	}

	void Servo::EnableServo(bool enable){

		int i;

		if (enable){

			//SERVO_STATE_OFF();
			//GPIO_WritePinOutput(BOARD_SERVO_STATE_GPIO, 1U , 1U);

			//SERVO_CLK_OFF();
			//for(i=0;i<1000;i++);
			//SERVO_CLK_ON();

			SERVO_STATE_OFF();
		}
		else{

			//SERVO_STATE_ON();
			//GPIO_WritePinOutput(BOARD_SERVO_STATE_GPIO, 1U , 0U);

			//SERVO_CLK_OFF();
			//for(i=0;i<1000;i++);
			SERVO_CLK_ON();

			SERVO_STATE_ON();
		}
		storeMonitorFlag (SERVO_ENABLED, enable);
	}


	void Servo::ShowStatus(char * mes){
		logStringnl(INFO, (char*)mes);
	}

	void Servo::ClearQueue(){
		//logStringnl(INFO, (char*)"Servo Clearing queue");
	}

	void Servo::SetupHardware(){
		//logStringnl(INFO, (char*)"Servo Setup hardware");
	}

	void Servo::JobisDone(){
		//logStringnl(INFO, (char*)"Servo Job is done");
	}

	void Servo::ToggleFlag(){
		//logStringnl(INFO, (char*)"Received USB Packet");
		//STEP_DIR_TOGGLE();
	}


} /* namespace antares */
