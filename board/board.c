/*
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* o Redistributions of source code must retain the above copyright notice, this list
*   of conditions and the following disclaimer.
*
* o Redistributions in binary form must reproduce the above copyright notice, this
*   list of conditions and the following disclaimer in the documentation and/or
*   other materials provided with the distribution.
*
* o Neither the name of Freescale Semiconductor, Inc. nor the names of its
*   contributors may be used to endorse or promote products derived from this
*   software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdint.h>
#include "fsl_device_registers.h"
#include "fsl_common.h"
#include "fsl_port.h"
#include "fsl_debug_console.h"
#include "fsl_gpio.h"
#include "fsl_ftm.h"
#include "MK64F12.h"

#include "board.h"

/* Get source clock for FTM driver */
#define FTM_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_BusClk)

/* Initialize debug console. */
void BOARD_InitDebugConsole(void){
    uint32_t uartClkSrcFreq = BOARD_DEBUG_UART_CLK_FREQ;
    DbgConsole_Init(BOARD_DEBUG_UART_BASEADDR, BOARD_DEBUG_UART_BAUDRATE, BOARD_DEBUG_UART_TYPE, uartClkSrcFreq);
}

void InitBSP(void){

	port_pin_config_t configENET = {0};

	CLOCK_EnableClock(kCLOCK_PortA);
	CLOCK_EnableClock(kCLOCK_PortB);
	CLOCK_EnableClock(kCLOCK_PortC);
	CLOCK_EnableClock(kCLOCK_PortD);
	CLOCK_EnableClock(kCLOCK_PortE);

	// Initialize UART0 pins ==========================================

	/* UART0 RX */
	PORT_SetPinMux(PORTB, 16u, kPORT_MuxAlt3);
	/* UART0 TX */
	PORT_SetPinMux(PORTB, 17u, kPORT_MuxAlt3);

	// ETHERNET INTERFACE PINS CONFIG  ===================================================
	/* ENET0_1588_TMR0 */
	PORT_SetPinMux(PORTC, 16u, kPORT_MuxAlt4);
	/* ENET0_1588_TMR1 */
	PORT_SetPinMux(PORTC, 17u, kPORT_MuxAlt4);
	/* ENET0_1588_TMR2 */
	PORT_SetPinMux(PORTC, 18u, kPORT_MuxAlt4);
	/* ENET0_1588_TMR3 */
	PORT_SetPinMux(PORTC, 19u, kPORT_MuxAlt4);
	/* RMII0_MDC MII0_MDC */
	PORT_SetPinMux(PORTB, 1u, kPORT_MuxAlt4);

	configENET.openDrainEnable = kPORT_OpenDrainEnable;
	configENET.mux = kPORT_MuxAlt4;
	configENET.pullSelect = kPORT_PullUp;
	/* Ungate the port clock */

	/* RMII0_MDIO/MII0_MDIO */
	PORT_SetPinConfig(PORTB, 0u, &configENET);

	/*  RMII0_RXD0/MII0_RXD0 */
	PORT_SetPinMux(PORTA, 13u, kPORT_MuxAlt4);
	/*  RMII0_RXD1/MII0_RXD1 */
	PORT_SetPinMux(PORTA, 12u, kPORT_MuxAlt4);
	/*  RMII0_CRS_ DV/MII0_RXDV */
	PORT_SetPinMux(PORTA, 14u, kPORT_MuxAlt4);
	/*  RMII0_RXER/MII0_RXER */
	PORT_SetPinMux(PORTA, 5u, kPORT_MuxAlt4);
	/*  RMII0_TXD0/MII0_TXD0 */
	PORT_SetPinMux(PORTA, 16u, kPORT_MuxAlt4);
	/*  RMII0_TXD1/MII0_TXD1 */
	PORT_SetPinMux(PORTA, 17u, kPORT_MuxAlt4);
	/*  RMII0_TXEN/MII0_TXEN */
	PORT_SetPinMux(PORTA, 15u, kPORT_MuxAlt4);
	/*  MII0_TXER */
	PORT_SetPinMux(PORTA, 28u, kPORT_MuxAlt4);

	// Set ADC Route
	PORT_SetPinMux(PORTA, 0U, kPORT_PinDisabledOrAnalog); /*ADC A0 */



	/* Enable SW port clock */
	//CLOCK_EnableClock(kCLOCK_PortA);
	/* Affects PORTA_PCR4 register */
	port_pin_config_t config = {0};
	config.pullSelect = kPORT_PullUp;
	config.mux = kPORT_MuxAsGpio;
	PORT_SetPinConfig(PORTA, 4U, &config);


	/* Configure LED's */
	PORT_SetPinMux(PORTB, 22U, kPORT_MuxAsGpio); // Red Led
	PORT_SetPinMux(PORTB, 21U, kPORT_MuxAsGpio); // Blue Led
	PORT_SetPinMux(PORTE, 26U, kPORT_MuxAsGpio); // Green Led


	// Step pins mux setup
	PORT_SetPinMux(BOARD_STEP_ENABLE_GPIO_PORT, BOARD_STEP_ENABLE_GPIO_PIN, kPORT_MuxAsGpio);
	PORT_SetPinMux(BOARD_STEP_PULSE_GPIO_PORT, BOARD_STEP_PULSE_GPIO_PIN, kPORT_MuxAsGpio);
	PORT_SetPinMux(BOARD_STEP_DIR_GPIO_PORT, BOARD_STEP_DIR_GPIO_PIN, kPORT_MuxAsGpio);

	// Servo pins mux
	PORT_SetPinMux(BOARD_SERVO_CLK_GPIO_PORT, BOARD_SERVO_CLK_GPIO_PIN, kPORT_MuxAsGpio);
	PORT_SetPinMux(BOARD_SERVO_STATE_GPIO_PORT, BOARD_SERVO_STATE_GPIO_PIN, kPORT_MuxAsGpio);

	PORT_SetPinMux(BOARD_DRUM_ENABLE_GPIO_PORT, BOARD_DRUM_ENABLE_GPIO_PIN, kPORT_MuxAsGpio);
	PORT_SetPinMux(BOARD_VAC_ENABLE_GPIO_PORT, BOARD_VAC_ENABLE_GPIO_PIN, kPORT_MuxAsGpio);
	PORT_SetPinMux(BOARD_WG2DIR_GPIO_PORT, BOARD_WG2DIR_GPIO_PIN, kPORT_MuxAsGpio);

	PORT_SetPinMux(BOARD_STEP_INTAP_GPIO_PORT, BOARD_STEP_INTAP_GPIO_PIN, kPORT_MuxAsGpio);
	PORT_SetPinMux(BOARD_STEP_OUTTAP_GPIO_PORT, BOARD_STEP_OUTTAP_GPIO_PIN, kPORT_MuxAsGpio);

	setGPIOPin(BOARD_LED_RED_GPIO, BOARD_LED_RED_GPIO_PIN, kGPIO_DigitalOutput , 1U);
	setGPIOPin(BOARD_LED_BLUE_GPIO, BOARD_LED_BLUE_GPIO_PIN, kGPIO_DigitalOutput , 1U);
	setGPIOPin(BOARD_LED_GREEN_GPIO, BOARD_LED_GREEN_GPIO_PIN, kGPIO_DigitalOutput, 1U);

	setGPIOPin(BOARD_STEP_ENABLE_GPIO, BOARD_STEP_ENABLE_GPIO_PIN, kGPIO_DigitalOutput, 1U);
	setGPIOPin(BOARD_STEP_PULSE_GPIO, BOARD_STEP_PULSE_GPIO_PIN, kGPIO_DigitalOutput, 1U);
	setGPIOPin(BOARD_STEP_DIR_GPIO, BOARD_STEP_DIR_GPIO_PIN, kGPIO_DigitalOutput, 1U);

	setGPIOPin(BOARD_DRUM_ENABLE_GPIO, BOARD_DRUM_ENABLE_GPIO_PIN, kGPIO_DigitalOutput, 1U);
	setGPIOPin(BOARD_VAC_ENABLE_GPIO, BOARD_VAC_ENABLE_GPIO_PIN, kGPIO_DigitalOutput, 1U);
	setGPIOPin(BOARD_WG2DIR_GPIO, BOARD_WG2DIR_GPIO_PIN, kGPIO_DigitalOutput, 1U);

	setGPIOPin(BOARD_SERVO_CLK_GPIO, BOARD_SERVO_CLK_GPIO_PIN, kGPIO_DigitalOutput, 1U);
	setGPIOPin(BOARD_SERVO_STATE_GPIO, BOARD_SERVO_STATE_GPIO_PIN, kGPIO_DigitalOutput, 1U);

	setDigitalInput (BOARD_STEP_INTAP_GPIO, BOARD_STEP_INTAP_GPIO_PORT, BOARD_STEP_INTAP_GPIO_PIN);
	setDigitalInput (BOARD_STEP_OUTTAP_GPIO, BOARD_STEP_OUTTAP_GPIO_PORT, BOARD_STEP_OUTTAP_GPIO_PIN);


	// PWM Pins
	PORT_SetPinMux(PORTB, 18U, kPORT_MuxAlt3);
	PORT_SetPinMux(PORTB, 19U, kPORT_MuxAlt3);


	// Tach Pin
	port_pin_config_t config2 = {
		kPORT_PullDisable ,
		kPORT_SlowSlewRate ,
		kPORT_PassiveFilterEnable,
		kPORT_OpenDrainDisable,
		kPORT_LowDriveStrength,
		kPORT_MuxAsGpio,
		0U,
	};
	PORT_SetPinConfig(PORTD, 01U, &config2);

}

// Setup function for FlexTimer0
void setupTach() {

	gpio_pin_config_t sw_config = {
		kGPIO_DigitalInput, 0,
	};

	PORT_SetPinInterruptConfig(BOARD_TACH_GPIO_PORT, BOARD_TACH_GPIO_PIN, kPORT_InterruptRisingEdge);
	EnableIRQ(BOARD_TACH_IRQ);

	//setDigitalInput (BOARD_TACH_GPIO, BOARD_TACH_GPIO_PORT, BOARD_TACH_GPIO_PIN);
	GPIO_PinInit(BOARD_TACH_GPIO, BOARD_TACH_GPIO_PIN, &sw_config);

	/* Init input switch GPIO. */
//	PORT_SetPinInterruptConfig(BOARD_SW_PORT, BOARD_SW_GPIO_PIN, kPORT_InterruptFallingEdge);
//	EnableIRQ(BOARD_SW_IRQ);
//	GPIO_PinInit(BOARD_SW_GPIO, BOARD_SW_GPIO_PIN, &sw_config);

}

void setServoPWM(){

	ftm_config_t ftmInfo;
	ftm_chnl_pwm_signal_param_t ftmParam[2];

	/* Configure first channel -> channel 0 @ B18 */
	ftmParam[0].chnlNumber = (ftm_chnl_t)kFTM_Chnl_0 ;
	ftmParam[0].level = kFTM_LowTrue;
	ftmParam[0].dutyCyclePercent = 0U;
	ftmParam[0].firstEdgeDelayPercent = 0U;

	/* Configure second channel -> channel 1 @ B19 */
	ftmParam[1].chnlNumber = (ftm_chnl_t)kFTM_Chnl_1 ;
	ftmParam[1].level = kFTM_LowTrue;
	ftmParam[1].dutyCyclePercent = 0U;
	ftmParam[1].firstEdgeDelayPercent = 0U;

	/* ftmInfo.prescale = kFTM_Prescale_Divide_1;
	 * ftmInfo.bdmMode = kFTM_BdmMode_0;
	 * ftmInfo.pwmSyncMode = kFTM_SoftwareTrigger;
	 * ftmInfo.reloadPoints = 0;
	 * ftmInfo.faultMode = kFTM_Fault_Disable;
	 * ftmInfo.faultFilterValue = 0;
	 * ftmInfo.deadTimePrescale = kFTM_Deadtime_Prescale_1;
	 * ftmInfo.deadTimeValue = 0;
	 * ftmInfo.extTriggers = 0;
	 * ftmInfo.chnlInitState = 0;
	 * ftmInfo.chnlPolarity = 0;
	 * ftmInfo.useGlobalTimeBase = false; */

	FTM_GetDefaultConfig(&ftmInfo);
	FTM_Init(FTM2, &ftmInfo);

	FTM_SetupPwm(FTM2, ftmParam, 2U, kFTM_EdgeAlignedPwm, 12000U, FTM_SOURCE_CLOCK);
	FTM_StartTimer(FTM2, kFTM_SystemClock);

	//setPWM(0, 60);
	//setPWM(1, 20);

}


void setPWM(uint8_t channel, uint8_t value){

	if (channel == 0){
		// Safety trap - prevents IGBT full throtle
		if (value > 80){
			value=80;
		}
		FTM_UpdatePwmDutycycle(FTM2, (ftm_chnl_t)kFTM_Chnl_0, kFTM_EdgeAlignedPwm, value);
	}
	else if (channel == 1){
		FTM_UpdatePwmDutycycle(FTM2, (ftm_chnl_t)kFTM_Chnl_1, kFTM_EdgeAlignedPwm, value);
	}
	FTM_SetSoftwareTrigger(FTM2, true);

}

void setGPIOPin (GPIO_Type *base, uint32_t pin, gpio_pin_direction_t dir, uint32_t status){
	gpio_pin_config_t pin_config = {dir,status};
	GPIO_PinInit(base, pin, &pin_config);
}


void setDigitalInput (GPIO_Type *gpiobase, PORT_Type *portbase, uint32_t pin){

	gpio_pin_config_t dir_config = {kGPIO_DigitalInput, 0U};
	port_pin_config_t pconfig = {
		kPORT_PullDisable ,
		kPORT_FastSlewRate,
		kPORT_PassiveFilterDisable,
		kPORT_OpenDrainDisable,
		kPORT_LowDriveStrength,
		kPORT_MuxAsGpio,
		0U,
	};
	GPIO_PinInit(gpiobase, pin, &dir_config);
	PORT_SetPinConfig(portbase, pin, &pconfig);

}



