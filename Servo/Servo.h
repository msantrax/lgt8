/*
 * Servo.h
 *
 *  Created on: 18/11/2016
 *      Author: root
 */

#ifndef SERVO_H_
#define SERVO_H_

#include <vector>
#include <string>
#include <queue>

#include "servo_sm.h"
#include "antares.h"

#define UPDATETACH_PERIOD 2000
#define UPDATESTATUS_PERIOD 5000

namespace antares {

class Servo {

	ServoContext _fsm;


public:
	Servo();
	virtual ~Servo();

	static Servo* Initialize();
	void start();

	// Signal handles
	inline void Tick(){_fsm.Tick();};
	void TickCallback();

	// Services
	void ShowStatus(char * mes);
	void ToggleFlag();
	void ClearQueue();
	void SetupHardware();
	void JobisDone();

	// Events
	void serviceEvents();
	void pushEvent(event_t evt);

	void serviceBoard();
	void serviceTach(uint32_t tick);

	typedef struct {
	  double dState;      	// Last position input
	  double iState;      	// Integrator state
	  double iMax, iMin;
	  // Maximum and minimum allowable integrator state
	  double	iGain,    	// integral gain
	        	pGain,    	// proportional gain
	         	dGain;     	// derivative gain
	} SPid;

	SPid svctl;

	typedef struct {
		double a0;
		double a1;
		double a2;

	} Svtf;
	Svtf servotransfer;


private:

	Servo(const statemap::State& state);

	void clearTachBuffer();
	void setServoPID(uint8_t profile);
	void resetPWM();
	void setServoTransfer();
	double calculatePWM(double gamma);
	double updateServo(SPid * pid, double error, double process);
	void EnableServo (bool enable);

	std::queue<event_t> events;
	event_t *dummyevt;


	bool enable_servo;
	uint8_t present_pwm0;
	uint8_t request_pwm0;


	uint32_t last_tach;
	uint32_t this_tach;
	uint32_t tachbuffer[5];
	uint8_t  tachbufferptr;
	uint32_t updatetach_flag;
	uint32_t tachval;
	double speed;
	uint8_t  tach_theta;

	double servo_setpoint;
	bool updatestatus = false;

	//Dummy variables
	uint8_t *sweepmapptr;
	bool enable;


};

} /* namespace antares */

#endif /* SERVO_H_ */
