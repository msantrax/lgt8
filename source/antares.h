/*
 * antares.h
 *
 *  Created on: 22/10/2016
 *      Author: root
 */

#ifndef SOURCE_ANTARES_H_
#define SOURCE_ANTARES_H_

#ifdef __cplusplus
#include <cstdlib>
#include <cstdarg>
#include <cstring>
#endif


#include "board.h"
#include "clock_config.h"
#include "fsl_debug_console.h"
#include "fsl_gpio.h"
#include "fsl_pit.h"


#define FATAL 0
#define ERROR 1
#define WARNING 2
#define INFO 3
#define FINE 4
#define PLUMB 5
#define FINER 6
#define FINEST 7



enum OBJTYPES{
    MAIN	=0x00010000,
    SWEEP	=0x00020000,
    SERVO	=0x00040000,
    MONITOR	=0x00080000,
    USB		=0X00100000,
	MICRO   =0x00200000,
};

enum SIGTYPES{

	NONE=0,
	PARSECMD_SIG,

	ALARM_SIG,

	// Control signals
	CTRL_ETH_SIG,
	CTRL_END_SIGNALS,

	// Sweep Signals
	SWEEP_ETH_SIG,
	SWEEP_RESET_SIG,

	SWEEP_INTAP_SIG,
	SWEEP_OUTTAP_SIG,
	SWEEP_PARKED_SIG,
	SWEEP_JOBDONE_SIG,
	SWEEP_TICK_SIG,
	SWEEP_SWEEPJOB_SIG,
	SWEEP_END_SIG,


	SERVO_ETH_SIG,
	SERVO_RESET_SIG,
	SERVO_TICK_SIG,
	SERVO_INITJOB_SIG,
	SERVO_END_SIG,


	MONITOR_ETH_SIG,
	MONITOR_RESET_SIG,
	MONITOR_TICK_SIG,
	MONITOR_INITJOB_SIG,
	MONITOR_END_SIG,

	// Packet processong signals
	SENDPACKET_SIG = 128,
	RECVPACKET_SIG = 129,

};

enum COMANDS{
	CALLBACK_REQ = 0,
	CALLBACK_OK = 1,
	CALLBACK_NONE = 3,
	COMAND_ACK = 4,
	CMD_STRING = 5,


	CMD_STATUSUPDATE = 20,
	CMD_STATUSKEEP=21,

	SWEEP_PARK=30,
	SWEEP_GOTO=31,
	SWEEP_SCAN=32,
	SWEEP_ENABLE=33,
	SWEEP_DIR=34,
	SWEEP_PULSE=35,

	SERVO_ENABLE=40,
	SERVO_SETPWM=41,
	SERVO_SETSPEED=42,
	SERVO_SETPARAMS=43,
	SERVO_SETPID=44,

	MON_SETVAC=50,
	MON_SETDRUM=51,
	WG20_SETDIR=52,
	WG20_SETVEL=53,
	MON_RESET=54,

};


#define QUEUESNUM 3
#define QUEUESIZE 32
enum QUEUES{
	SUBS_QUEUE = 0,
	CONTROL_QUEUE = 1,
	STEP_QUEUE = 2,
};


#ifdef __cplusplus
struct event_t {
    int evtype;
    uint8_t simple;
    void * payload;
    event_t (int c_evtype, uint8_t c_simple, void* c_payload) {
        evtype = c_evtype; simple = c_simple; payload = c_payload;
    }
};
#endif


// Message structure to pass info to USB wire
typedef struct antares_msg_struct {
	uint8_t		csum;
	uint8_t		size;

	uint8_t		tstamp[3];
	uint8_t		cmd;
	uint8_t		arg;
	uint8_t		seq[2];
	uint8_t		orig;
	uint8_t		dest;
	uint8_t		magic;
	uint8_t     data[150];
} antares_msg_struct_t;



#define ACPMESSAGE_LENGTH 162 //sizeof(antares_msg_struct);

double statusvals[10];
uint8_t statusvals_ptr[10] = {0,11,22,33,44,55,66,77,88,99};
bool statusflags[13];
uint8_t statusflags_ptr[13] = {110,112,114,116,118,120,122,124,126,128,130,132,134};
char status_stream[150];
bool status_updated;

enum STVALS{

	SERVO_SPEED_SLOT = 0,
	SERVO_TORQUE_SLOT = 1,
	SWEEP_POSITION_SLOT = 2,
	SWEEP_SPEED_SLOT= 3,
	CVEX_SLOT = 4 ,
	DROP_SLOT = 5,
	WG2_SPEED_SLOT = 6,
	MICRONS_SLOT = 7,
	EXTRA1_SLOT = 8,
	EXTRA2_SLOT = 9
};


enum STFLAGS{

	SWEEP_INNER = 0,
	SWEEP_OUTER = 1,
	SWEEP_PARKFLAG = 2,
	SWEEP_ENABLED = 3,
	SWEEP_OUTDIR = 4,
	SERVO_ENABLED = 5,
	SERVO_FAULT = 6,
	wG20_ENABLED = 7,
	WG20_DIR = 8,
	WG20_PRESENT = 9,
	DRUM_ENABLED = 10,
	VAC_ENABLED = 11,
	DROP_SENSOR = 12,

};

//
//1.1234E+00,1.1234E+00,1.1234E+00,1.1234E+00,1.1234E+00,1.1234E+00,1.1234E+00,1.1234E+00,1.1234E+00,1.1234E+00,1,1,1,1,1,1,1,1,1,1,1

// Prototypes ====================================================================

#ifdef __cplusplus
extern "C" {
#endif

void logMessage (int level, const char* format, ...);
void enableLogger(bool enable);
void setLogLevel(int level);
void logString (int level, char* mes);
void logStringnl (int level, char* mes);
void TRACE(const char* format, ...);


long getTick();
bool isAlarmTime(uint8_t index);
void setAlarm (uint8_t index, unsigned long interval, unsigned long rld);
void resetAlarm(uint8_t index);
void clearAlarms();
void PIT0_IRQHandler(void);
void DEMO_ADC16_IRQ_HANDLER_FUNC(void);
void installADCHandler();

void ethergate(void * inmessage);

void informMessage(uint8_t * mptr);
//void callbackGate(uint8_t * mptr, antares_msg_struct_t * mout);
//void loadCallBack(antares_msg_struct_t  * mes);
void storeMonitorSlot (uint8_t slot, double value);
void storeMonitorFlag (uint8_t slot, bool flag);
void initStatus();

#ifdef __cplusplus
}
#endif



#endif /* SOURCE_ANTARES_H_ */
