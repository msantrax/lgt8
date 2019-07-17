/*!
 * @brief LGT8 Alpha -- Opus Equipamentos / ACP Instruments
 */


#include <cstdlib>
#include <cstdarg>
#include <cstring>
#include <cstdio>

#include <vector>
#include <string>
#include <queue>


#include "clock_config.h"
#include "board.h"
#include "antares.h"

#include "fsl_debug_console.h"
#include "fsl_gpio.h"
#include "fsl_pit.h"
#include "fsl_ftm.h"
#include "fsl_adc16.h"

#include "antares.h"
#include "sweep.h"
#include "Servo.h"
#include "Monitor.h"

#include "httpd.h"


// Namespace stuff
using namespace std;
using namespace antares;


// Postoffice advanced declarations
std::vector<int>subscriptions;
std::queue<event_t> postoffice;
std::queue<event_t> mainevents;
event_t event_temp(0,0,NULL);
std::queue<antares_msg_struct *>message_out;

Sweep *sweep;
Servo *servo;
Monitor *monitor;


// Miscelanea
#define TO_HEX(i) (i <= 9 ? '0' + i : 'A' - 10 + i)
typedef enum {
	INIT,
	IDLE,
	ALARM,
	SENDU,
	RECVU,
}CTRLSTATES;
CTRLSTATES ctrl_state;


// PIT section ==================================================================================
#define PIT_SLOWTICK_HANDLER PIT0_IRQHandler
#define PIT_SLOWIRQ_ID PIT0_IRQn
#define PIT_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_BusClk)
#define SLOWTICK_INTERVAL 200U


// Alarm Services
unsigned long bspcounter;
unsigned long alarms[4];
unsigned long reload[4];

// ADC Services
#define DEMO_ADC16_BASE ADC0
#define DEMO_ADC16_CHANNEL_GROUP 0U
#define DEMO_ADC16_USER_CHANNEL 12U /* A0 ?*/

#define DEMO_ADC16_IRQn ADC0_IRQn
#define DEMO_ADC16_IRQ_HANDLER_FUNC ADC0_IRQHandler

volatile bool g_Adc16ConversionDoneFlag = false;
volatile uint32_t g_Adc16ConversionValue;
volatile uint32_t g_Adc16InterruptCounter;
adc16_config_t adc16ConfigStruct;
adc16_channel_config_t adc16ChannelConfigStruct;

// Tratamento dos sinais do tacometro
extern "C" {
void BOARD_TACH_IRQ_HANDLER(void){

//	uint32_t iflag;
//	iflag=GPIO_GetPinsInterruptFlags(BOARD_TACH_GPIO);

	servo->serviceTach(bspcounter);

	/* Clear external interrupt flag. */
	GPIO_ClearPinsInterruptFlags(BOARD_TACH_GPIO, 1U << BOARD_TACH_GPIO_PIN);
}
} // extern C

// Conversor A/D
extern "C" {
void DEMO_ADC16_IRQ_HANDLER_FUNC(void){

    g_Adc16ConversionDoneFlag = true;
    /* Read conversion result to clear the conversion completed flag. */
    g_Adc16ConversionValue = ADC16_GetChannelConversionValue(DEMO_ADC16_BASE, DEMO_ADC16_CHANNEL_GROUP);
    g_Adc16InterruptCounter++;

}
} // extern C



extern "C" {
void PIT_SLOWTICK_HANDLER(void){

	/* Clear interrupt flag first*/
    PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, PIT_TFLG_TIF_MASK);

    sweep->serviceBoard();
    servo->serviceBoard();
    monitor->serviceBoard();

    bspcounter++;
	for (int i = 0; i < 4; i++) {
		if (alarms[i] < bspcounter){
			postoffice.push(event_t (ALARM_SIG, ALARM, NULL));
			if (reload[i] !=0){
				alarms[i] = bspcounter + reload[i];
			}
			else{
				alarms[i] = 0xffffffff;
			}
		}
	}
}
} // extern C


void installTickHandler(){

	/* Structure of initialize PIT */
	pit_config_t pitConfig;

	// Init Tick Timer
	PIT_GetDefaultConfig(&pitConfig);
	PIT_Init(PIT, &pitConfig);

	PIT_SetTimerPeriod(PIT, kPIT_Chnl_0, USEC_TO_COUNT(SLOWTICK_INTERVAL, PIT_SOURCE_CLOCK));
	//PIT_SetTimerPeriod(PIT, kPIT_Chnl_1, USEC_TO_COUNT(FASTTICK_INTERVAL, PIT_SOURCE_CLOCK));

	/* Enable timer interrupts */
	PIT_EnableInterrupts(PIT, kPIT_Chnl_0, kPIT_TimerInterruptEnable);
	//PIT_EnableInterrupts(PIT, kPIT_Chnl_1, kPIT_TimerInterruptEnable);

	/* Enable at the NVIC */
	EnableIRQ(PIT_SLOWIRQ_ID);
	//EnableIRQ(PIT_FASTIRQ_ID);

	PIT_StartTimer(PIT, kPIT_Chnl_0);
	//PIT_StartTimer(PIT, kPIT_Chnl_1);
}

void installADCHandler(){

	EnableIRQ(DEMO_ADC16_IRQn);

    /*
     * adc16ConfigStruct.referenceVoltageSource = kADC16_ReferenceVoltageSourceVref;
     * adc16ConfigStruct.clockSource = kADC16_ClockSourceAsynchronousClock;
     * adc16ConfigStruct.enableAsynchronousClock = true;
     * adc16ConfigStruct.clockDivider = kADC16_ClockDivider8;
     * adc16ConfigStruct.resolution = kADC16_ResolutionSE12Bit;
     * adc16ConfigStruct.longSampleMode = kADC16_LongSampleDisabled;
     * adc16ConfigStruct.enableHighSpeed = false;
     * adc16ConfigStruct.enableLowPower = false;
     * adc16ConfigStruct.enableContinuousConversion = false;
     */
    ADC16_GetDefaultConfig(&adc16ConfigStruct);
    adc16ConfigStruct.resolution = kADC16_ResolutionDF16Bit;
    adc16ConfigStruct.longSampleMode = kADC16_LongSampleCycle24 ;

    ADC16_Init(DEMO_ADC16_BASE, &adc16ConfigStruct);
    ADC16_EnableHardwareTrigger(DEMO_ADC16_BASE, false); /* Make sure the software trigger is used. */
    ADC16_SetHardwareAverage(DEMO_ADC16_BASE, kADC16_HardwareAverageCount32);

#if defined(FSL_FEATURE_ADC16_HAS_CALIBRATION) && FSL_FEATURE_ADC16_HAS_CALIBRATION
    if (kStatus_Success == ADC16_DoAutoCalibration(DEMO_ADC16_BASE)){
    	logMessage(INFO, "ADC is calibrated, we have analog conversion on channels 0 and 1 @ 16bits/1.8 ms\r\n");
    }
    else{
    	logMessage(INFO, "ADC failed to calibrate, analog conversion disabled\r\n");
    }
#endif /* FSL_FEATURE_ADC16_HAS_CALIBRATION */

    adc16ChannelConfigStruct.channelNumber = DEMO_ADC16_USER_CHANNEL;
    adc16ChannelConfigStruct.enableInterruptOnConversionCompleted = true; /* Enable the interrupt. */



#if defined(FSL_FEATURE_ADC16_HAS_DIFF_MODE) && FSL_FEATURE_ADC16_HAS_DIFF_MODE
    adc16ChannelConfigStruct.enableDifferentialConversion = false;
#endif /* FSL_FEATURE_ADC16_HAS_DIFF_MODE */


    g_Adc16InterruptCounter = 0U;

}



long getTick() {return bspcounter;}

bool isAlarmTime(uint8_t index) {return bspcounter > alarms[index];}

void setAlarm (uint8_t index, unsigned long interval, unsigned long rld) {
	alarms[index] =  bspcounter+interval;
	reload[index] = rld;
}

void resetAlarm(uint8_t index) {
	alarms[index] = 0xffffffff;
	reload[index] = 0L;
}

void clearAlarms(){
	for (int i = 0; i < 4; i++) {
		alarms[i]=0xffffffff;;
		reload[i]=0L;
	}
}


// Debug ==========================================================================================================

// Log services
int loglevel;
bool log_enabled;
char logbuffer[3500];
char *logbufferinit;
char *logbufferptr;

void logMessage(int level, const char* format, ...) {

	int printed;

	if (level <= loglevel && log_enabled){
		va_list arg;
		va_start(arg, format);
		printed = vsprintf(logbufferptr,format, arg);
		logbufferptr+= sizeof(char)*printed;
		va_end(arg);
	}
}

void logString (int level, char* mes) {

	while (*mes != 0x00){
		*logbufferptr = *mes;
		logbufferptr++;
		mes++;
	}
}

void logStringnl (int level, char* mes) {

	while (*mes != 0x00){
		*logbufferptr = *mes;
		logbufferptr++;
		mes++;
	}
	*logbufferptr++ = '\r';
	*logbufferptr++ = '\n';
}

void enableLogger(bool enable){ log_enabled = enable;}

void setLogLevel(int level) {loglevel = level;}

void printLog(){

	char *logbuf = logbufferinit;

	if (logbufferptr > logbufferinit){
		do{
			PUTCHAR(*logbuf);
		}while (logbuf++ < logbufferptr);
		logbufferptr=logbufferinit;
	}
}

void TRACE(const char* format, ...) {

	int printed;

	if (true){
		va_list arg;
		va_start(arg, format);
		printed = vsprintf(logbufferptr,format, arg);
		logbufferptr+= (sizeof(char)*printed) - (sizeof(char)*1);
		if (*logbufferptr == '\n'){
			logbufferptr++;
			*logbufferptr='\r';
		}
		logbufferptr++;
		*logbufferptr=0x00;
		va_end(arg);
	}
}

// PostOffice =================================================================================================================
void publish (event_t evt){
	postoffice.push(evt);
}

void verifymail(){

	int signal, subs, addr;

	while (!postoffice.empty()){
		event_t evt = postoffice.front();
		signal = evt.evtype;
		//logMessage(FINER, "Postoffice recebeu evt : %d com payload = %d\r\n", signal, evt.simple);
		for (unsigned i=0; i<subscriptions.size(); ++i){
			subs = subscriptions[i] & 0x0000ffff;
			addr = subscriptions[i] & 0xffff0000;
			if (subs == signal){
				if (addr == MAIN){
					mainevents.push(evt);
					//logMessage(INFO, "Postoffice transferiu evento %d para main\r\n", evt.evtype);
				}
				if (addr == SWEEP){
					sweep->pushEvent(evt);
					//logMessage(FINER, "Postoffice transferiu evento %d para sweep\r\n", evt.evtype);
				}
				if (addr == SERVO){
					servo->pushEvent(evt);
					//logMessage(FINER, "Postoffice transferiu evento %d para servo\r\n", evt.evtype);
				}
				if (addr == MONITOR){
					monitor->pushEvent(evt);
					//logMessage(FINER, "Postoffice transferiu evento %d para monitor\r\n", evt.evtype);
				}
			}
		}
		postoffice.pop();
	}
}

void subscribe (int object, int signal){
	subscriptions.push_back(object | signal);
	//logMessage(FINER, "Subscribing %d on %d - list now with %d items\r\n", signal, object, subscriptions.size());
}

void unsubscribe (int object, int signal){

	int index = object+signal;

	for (unsigned i=0; i<subscriptions.size(); ++i){
		if (subscriptions[i] == index){
			subscriptions.erase(subscriptions.begin()+i);
			//logMessage(FINER, "UNsubscribing %d on %d - list now with %d items\r\n", signal, object, subscriptions.size());
		}
	}
}

bool locateEvent (int event_type){

	while(!mainevents.empty()) {
		if (mainevents.front().evtype == event_type){
			//event_temp = mainevents.front();
			//logMessage(FINER, "Event type %d was localized\r\n", mainevents.front().evtype);
			return true;
		}
		//delete mainevents.front();
		mainevents.pop();
	}
	return false;
}


// Data gate ===================================================================================================================

void ethergate(void * message){

	antares_msg_struct_t *inmessage = (antares_msg_struct_t *)message;
	antares_msg_struct_t *dispach_message;
	int dest;
	char * mesptr;

	if (inmessage->cmd == CALLBACK_REQ){
		if (!message_out.empty()){
			//logMessage(INFO, "Callback arrived - queue not empty\r\n" );
			dispach_message = message_out.front();
			memcpy (message, dispach_message, sizeof(uint8_t)*64);
			message_out.pop();
			delete (dispach_message);
		}
		else{
			//logMessage(INFO, "Callback arrived - queue empty -> sending status\r\n" );
			if(status_updated){
				inmessage->cmd= CMD_STATUSUPDATE;
				mesptr = (char*)&inmessage->data[0];
				memcpy (mesptr, &status_stream[0], sizeof(char)*150);
				inmessage->size=127;
				mesptr++;
			}
			else{
				inmessage->cmd= CMD_STATUSKEEP;
				inmessage->size=0;
			}

		}
	}
	else{
		dest = inmessage->dest;
		//logMessage(INFO, "Message arrived\r\n" );
		dispach_message = new antares_msg_struct_t();
		memcpy(dispach_message, message, sizeof(antares_msg_struct_t));
		if (dest == 1){
			postoffice.push(event_t(CTRL_ETH_SIG, 1, dispach_message));
		}
		else if(dest == 2){
			postoffice.push(event_t(SWEEP_ETH_SIG, 1, dispach_message));
		}
		else if(dest == 3){
			postoffice.push(event_t(SERVO_ETH_SIG, 1, dispach_message));
		}
		else if(dest == 4){
			postoffice.push(event_t(MONITOR_ETH_SIG, 1, dispach_message));
		}

	}

}

void storeMonitorSlot (uint8_t slot, double value){

	char *slotptr;
	char buf[20];

	if (statusvals[slot] != value){
		statusvals[slot] = value;
		slotptr = (char*)status_stream + (sizeof(uint8_t)*(slot * 10))+(sizeof(uint8_t)*slot);

		sprintf (buf, "%6.4E,", value);
		memcpy (slotptr, &buf[0], sizeof(char)*11);

		//logMessage(INFO, "Storing monitor slot %d[%p] =  %6.4E\r\n", slot, slotptr, value);

		status_updated = true;
	}

}

void storeMonitorFlag (uint8_t slot, bool flag){

	//char *slotptr;
	uint8_t idx;

	if (statusflags[slot] != flag){
		statusflags[slot] = flag;
		idx = statusflags_ptr[slot];


		//slotptr = (char*)status_stream + (sizeof(uint8_t)* 110) + (sizeof(uint8_t)*(slot*2));
		if (flag == true){
			status_stream[idx] = 49;
			//*slotptr = 49;
		}
		else{
			status_stream[idx] = 48;
		}
		//slotptr++;

		if (slot == 11){
			status_stream[idx+1] = 0;
			//*slotptr = 0;
		}
		else{
			status_stream[idx+1] = 44;
			//*slotptr = 44;
		}

		status_updated = true;
	}

}

void initStatus(){

	int i, j;

	for(i=0; i<10; i++){
		statusvals[i]= -100000.0;
		storeMonitorSlot (i, (i*80.0)+(i/3));
	}

	for(j=0; j<12; j++){
		statusflags[j]= false;
		storeMonitorFlag (j, true);
	}

//	char *slotptr = (char*)status_stream;
//	slotptr++;
}




// Main state machine ===========================================================================================================
int main(void){

	/* Disable MPU. */
	MPU_Type *base = MPU;
	base->CESR &= ~MPU_CESR_VLD_MASK;

	bspcounter=0L;
	clearAlarms();
	log_enabled = false;
	loglevel=FINE;
	logbufferinit = &logbuffer[0];
	logbufferptr = &logbuffer[0];
	ctrl_state = INIT;
	log_enabled = true;
	initStatus();


	InitBSP();
	BOARD_BootClockRUN();
	BOARD_InitDebugConsole();
	setServoPWM();
	setupTach();

	// SUBSCRIPTIONS
	subscribe(MAIN, CTRL_ETH_SIG);
	subscribe(MAIN, ALARM_SIG);
	subscribe(SWEEP, SWEEP_ETH_SIG);
	subscribe(SERVO, SERVO_ETH_SIG);
	subscribe(MONITOR, MONITOR_ETH_SIG);


	logMessage(INFO, "\r\n\r\nAntares-LGT8 V-1.6.0 [NB Version 3] Kernel Booting - Hardware is OK \r\n");

	// Initialize services
	sweep = Sweep::Initialize();
	if (sweep == (Sweep *) NULL){
		printf("Failed to create sweep object." );
		exit(1);
	}
	sweep->start();

	servo = Servo::Initialize();
	if (servo == (Servo *) NULL){
		printf("Failed to create servo object." );
		exit(1);
	}
	servo->start();

	monitor = Monitor::Initialize();
	if (monitor == (Monitor *) NULL){
		printf("Failed to create monitor object." );
		exit(1);
	}
	monitor->start();

	logMessage(INFO, "Antares-LGT8 - Sensors and Services are UP, loading comms and management...\r\n");

	initEthernet();
	installTickHandler();

	installADCHandler();
	g_Adc16ConversionDoneFlag = true;
	g_Adc16InterruptCounter = 0U;

	logMessage(INFO, "Good, we are on businnes :-) -> State Machine is ready to go \r\n");


	// Main Control Loop
	while(true){

		//STEP_ENABLE_TOGGLE();

		serviceEthernet();
		verifymail();

		sweep->serviceEvents();
		servo->serviceEvents();
		monitor->serviceEvents();



		switch (ctrl_state){

			case INIT:
				//step->init();
				setAlarm (0, 100, 100);
				ctrl_state = IDLE;
				break;

			case IDLE:
				if(!mainevents.empty()) {
					event_temp = mainevents.front();
					if(event_temp.evtype == ALARM_SIG){
						ctrl_state = ALARM;
					}
					else if (event_temp.evtype == CTRL_ETH_SIG){
						ctrl_state = RECVU;
					}
					mainevents.pop();
				}
				printLog();
				break;

			case ALARM:

				if (g_Adc16ConversionDoneFlag == true){
					LED_BLUE_TOGGLE();
					g_Adc16ConversionDoneFlag = false;
					storeMonitorSlot (CVEX_SLOT, (double)g_Adc16ConversionValue);
					//logMessage(INFO, "ADC = %6d -> %6.4E\r", g_Adc16InterruptCounter, statusvals[4]);

					ADC16_SetChannelConfig(DEMO_ADC16_BASE, DEMO_ADC16_CHANNEL_GROUP, &adc16ChannelConfigStruct);
				}

				ctrl_state = IDLE;
				break;

			case RECVU:
				logStringnl(INFO, (char*)"Received CTRL ETH Packet");
//				pload = (antares_msg_struct *)event_temp.payload;
//				pcmd = pload->cmd;
				ctrl_state = IDLE;
				break;

			case SENDU:

				break;

			default:
				break;

		}
	}

}



//				ames = new antares_msg_struct;
//				ames->cmd=CMD_CSTATUS;
//				ames->arg = 1;
//				ames->dest = 2;
//				ames->orig = 1;
//				ames->seq[0] = 0;
//				ames->seq[1] = 0;
//				amesptr = (char *)&ames->data[0];
//				a = (rand() % 300 + 1) / 3.45;
//				b = (rand() % 300 + 1) / 3.60;
//				c = (rand() % 300 + 1) / 3.04;
//				ames->size = sprintf (amesptr, "%2.4f %2.4f %2.4f", a, b, c);
//				//loadCallBack(ames);
//				delete (ames);

//				storeMonitorSlot (0, (rand() % 300 + 1) / 3.45);
//				storeMonitorSlot (1, (rand() % 300 + 1) / 3.60);
//				storeMonitorSlot (2, (rand() % 300 + 1) / 3.04);

