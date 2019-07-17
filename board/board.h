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

#ifndef _BOARD_H_
#define _BOARD_H_

#include "clock_config.h"
#include "fsl_gpio.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* The board name */
#define BOARD_NAME "FRDM-K64F"

/* The UART to use for debug messages. */
#define BOARD_DEBUG_UART_TYPE DEBUG_CONSOLE_DEVICE_TYPE_UART
#define BOARD_DEBUG_UART_BASEADDR (uint32_t) UART0
#define BOARD_DEBUG_UART_CLKSRC SYS_CLK
#define BOARD_DEBUG_UART_CLK_FREQ CLOCK_GetCoreSysClkFreq()
#define BOARD_UART_IRQ UART0_RX_TX_IRQn
#define BOARD_UART_IRQ_HANDLER UART0_RX_TX_IRQHandler

#ifndef BOARD_DEBUG_UART_BAUDRATE
#define BOARD_DEBUG_UART_BAUDRATE 115200
#endif /* BOARD_DEBUG_UART_BAUDRATE */

/* Define the port interrupt number for the board switches */
#define BOARD_SW2_GPIO GPIOC
#define BOARD_SW2_PORT PORTC
#define BOARD_SW2_GPIO_PIN 6U
#define BOARD_SW2_IRQ PORTC_IRQn
#define BOARD_SW2_IRQ_HANDLER PORTC_IRQHandler
#define BOARD_SW2_NAME "SW2"

#define BOARD_SW3_GPIO GPIOA
#define BOARD_SW3_PORT PORTA
#define BOARD_SW3_GPIO_PIN 4U
#define BOARD_SW3_IRQ PORTA_IRQn
#define BOARD_SW3_IRQ_HANDLER PORTA_IRQHandler
#define BOARD_SW3_NAME "SW3"

/* Board led color mapping */
#define LOGIC_LED_ON 0U
#define LOGIC_LED_OFF 1U
#define BOARD_LED_RED_GPIO GPIOB
#define BOARD_LED_RED_GPIO_PORT PORTB
#define BOARD_LED_RED_GPIO_PIN 22U
#define BOARD_LED_GREEN_GPIO GPIOE
#define BOARD_LED_GREEN_GPIO_PORT PORTE
#define BOARD_LED_GREEN_GPIO_PIN 26U
#define BOARD_LED_BLUE_GPIO GPIOB
#define BOARD_LED_BLUE_GPIO_PORT PORTB
#define BOARD_LED_BLUE_GPIO_PIN 21U

#define LED_RED_INIT(output)                                 \
    GPIO_PinInit(BOARD_LED_RED_GPIO, BOARD_LED_RED_GPIO_PIN, \
                 &(gpio_pin_config_t){kGPIO_DigitalOutput, (output)}) /*!< Enable target LED_RED */
#define LED_RED_ON() \
    GPIO_ClearPinsOutput(BOARD_LED_RED_GPIO, 1U << BOARD_LED_RED_GPIO_PIN) /*!< Turn on target LED_RED */
#define LED_RED_OFF() \
    GPIO_SetPinsOutput(BOARD_LED_RED_GPIO, 1U << BOARD_LED_RED_GPIO_PIN) /*!< Turn off target LED_RED */
#define LED_RED_TOGGLE() \
    GPIO_TogglePinsOutput(BOARD_LED_RED_GPIO, 1U << BOARD_LED_RED_GPIO_PIN) /*!< Toggle on target LED_RED */

#define LED_GREEN_INIT(output)                                   \
    GPIO_PinInit(BOARD_LED_GREEN_GPIO, BOARD_LED_GREEN_GPIO_PIN, \
                 &(gpio_pin_config_t){kGPIO_DigitalOutput, (output)}) /*!< Enable target LED_GREEN */
#define LED_GREEN_ON() \
    GPIO_ClearPinsOutput(BOARD_LED_GREEN_GPIO, 1U << BOARD_LED_GREEN_GPIO_PIN) /*!< Turn on target LED_GREEN */
#define LED_GREEN_OFF() \
    GPIO_SetPinsOutput(BOARD_LED_GREEN_GPIO, 1U << BOARD_LED_GREEN_GPIO_PIN) /*!< Turn off target LED_GREEN */
#define LED_GREEN_TOGGLE() \
    GPIO_TogglePinsOutput(BOARD_LED_GREEN_GPIO, 1U << BOARD_LED_GREEN_GPIO_PIN) /*!< Toggle on target LED_GREEN */

#define LED_BLUE_INIT(output)                                  \
    GPIO_PinInit(BOARD_LED_BLUE_GPIO, BOARD_LED_BLUE_GPIO_PIN, \
                 &(gpio_pin_config_t){kGPIO_DigitalOutput, (output)}) /*!< Enable target LED_BLUE */
#define LED_BLUE_ON() \
    GPIO_ClearPinsOutput(BOARD_LED_BLUE_GPIO, 1U << BOARD_LED_BLUE_GPIO_PIN) /*!< Turn on target LED_BLUE */
#define LED_BLUE_OFF() \
    GPIO_SetPinsOutput(BOARD_LED_BLUE_GPIO, 1U << BOARD_LED_BLUE_GPIO_PIN) /*!< Turn off target LED_BLUE */
#define LED_BLUE_TOGGLE() \
    GPIO_TogglePinsOutput(BOARD_LED_BLUE_GPIO, 1U << BOARD_LED_BLUE_GPIO_PIN) /*!< Toggle on target LED_BLUE */

/* The SDHC instance/channel used for board */
#define BOARD_SDHC_CD_GPIO_IRQ_HANDLER PORTB_IRQHandler

/* SDHC base address, clock and card detection pin */
#define BOARD_SDHC_BASEADDR SDHC
#define BOARD_SDHC_CLKSRC kCLOCK_CoreSysClk
#define BOARD_SDHC_IRQ SDHC_IRQn
#define BOARD_SDHC_CD_GPIO_BASE GPIOE
#define BOARD_SDHC_CD_GPIO_PIN 6U
#define BOARD_SDHC_CD_PORT_BASE PORTE
#define BOARD_SDHC_CD_PORT_IRQ PORTE_IRQn
#define BOARD_SDHC_CD_PORT_IRQ_HANDLER PORTE_IRQHandler
#define BOARD_SDHC_CD_LOGIC_RISING

#define BOARD_ACCEL_I2C_BASEADDR I2C0

/* @brief FreeRTOS tickless timer configuration. */
#define vPortLptmrIsr LPTMR0_IRQHandler /*!< Timer IRQ handler. */
#define TICKLESS_LPTMR_BASE_PTR LPTMR0  /*!< Tickless timer base address. */
#define TICKLESS_LPTMR_IRQn LPTMR0_IRQn /*!< Tickless timer IRQ number. */

/* @brief pit IRQ configuration for lwip demo */
#define LWIP_TIME_ISR PIT0_IRQHandler
#define LWIP_PIT_IRQ_ID PIT0_IRQn


// LOGITECH SPECIFIC PINS ===========================================================================
#define BOARD_STEP_ENABLE_GPIO GPIOC
#define BOARD_STEP_ENABLE_GPIO_PORT PORTC
#define BOARD_STEP_ENABLE_GPIO_PIN 2U

#define BOARD_STEP_PULSE_GPIO GPIOC
#define BOARD_STEP_PULSE_GPIO_PORT PORTC
#define BOARD_STEP_PULSE_GPIO_PIN 3U

#define BOARD_STEP_DIR_GPIO GPIOC
#define BOARD_STEP_DIR_GPIO_PORT PORTC
#define BOARD_STEP_DIR_GPIO_PIN 4U

#define BOARD_STEP_INTAP_GPIO GPIOD
#define BOARD_STEP_INTAP_GPIO_PORT PORTD
#define BOARD_STEP_INTAP_GPIO_PIN 2U

#define BOARD_STEP_OUTTAP_GPIO GPIOD
#define BOARD_STEP_OUTTAP_GPIO_PORT PORTD
#define BOARD_STEP_OUTTAP_GPIO_PIN 0U

#define BOARD_DRUM_ENABLE_GPIO GPIOC
#define BOARD_DRUM_ENABLE_GPIO_PORT PORTC
#define BOARD_DRUM_ENABLE_GPIO_PIN 8U

#define BOARD_VAC_ENABLE_GPIO GPIOC
#define BOARD_VAC_ENABLE_GPIO_PORT PORTC
#define BOARD_VAC_ENABLE_GPIO_PIN 7U

#define BOARD_WG2DIR_GPIO GPIOC
#define BOARD_WG2DIR_GPIO_PORT PORTC
#define BOARD_WG2DIR_GPIO_PIN 9U

#define BOARD_WG2VEL_GPIO GPIOB
#define BOARD_WG2VEL_GPIO_PORT PORTB
#define BOARD_WG2VEL_GPIO_PIN 19U

#define BOARD_TACH_GPIO GPIOD
#define BOARD_TACH_GPIO_PORT PORTD
#define BOARD_TACH_GPIO_PIN 01U
#define BOARD_TACH_IRQ PORTD_IRQn
#define BOARD_TACH_IRQ_HANDLER PORTD_IRQHandler

#define BOARD_SERVO_CLK_GPIO GPIOC
#define BOARD_SERVO_CLK_GPIO_PORT PORTC
#define BOARD_SERVO_CLK_GPIO_PIN 0U

#define BOARD_SERVO_STATE_GPIO GPIOC
#define BOARD_SERVO_STATE_GPIO_PORT PORTC
#define BOARD_SERVO_STATE_GPIO_PIN 1U


#define STEP_ENABLE_ON() GPIO_ClearPinsOutput(BOARD_STEP_ENABLE_GPIO, 1U << BOARD_STEP_ENABLE_GPIO_PIN)
#define STEP_ENABLE_OFF() GPIO_SetPinsOutput(BOARD_STEP_ENABLE_GPIO, 1U << BOARD_STEP_ENABLE_GPIO_PIN)
#define STEP_ENABLE_TOGGLE() GPIO_TogglePinsOutput(BOARD_STEP_ENABLE_GPIO, 1U << BOARD_STEP_ENABLE_GPIO_PIN)

#define STEP_DIR_ON() GPIO_ClearPinsOutput(BOARD_STEP_DIR_GPIO, 1U << BOARD_STEP_DIR_GPIO_PIN)
#define STEP_DIR_OFF() GPIO_SetPinsOutput(BOARD_STEP_DIR_GPIO, 1U << BOARD_STEP_DIR_GPIO_PIN)
#define STEP_DIR_TOGGLE() GPIO_TogglePinsOutput(BOARD_STEP_DIR_GPIO, 1U << BOARD_STEP_DIR_GPIO_PIN)

#define STEP_PULSE_ON() GPIO_ClearPinsOutput(BOARD_STEP_PULSE_GPIO, 1U << BOARD_STEP_PULSE_GPIO_PIN)
#define STEP_PULSE_OFF() GPIO_SetPinsOutput(BOARD_STEP_PULSE_GPIO, 1U << BOARD_STEP_PULSE_GPIO_PIN)
#define STEP_PULSE_TOGGLE() GPIO_TogglePinsOutput(BOARD_STEP_PULSE_GPIO, 1U << BOARD_STEP_PULSE_GPIO_PIN)


#define SERVO_CLK_ON() GPIO_ClearPinsOutput(BOARD_SERVO_CLK_GPIO, 1U << BOARD_SERVO_CLK_GPIO_PIN)
#define SERVO_CLK_OFF() GPIO_SetPinsOutput(BOARD_SERVO_CLK_GPIO, 1U << BOARD_SERVO_CLK_GPIO_PIN)
#define SERVO_CLK_TOGGLE() GPIO_TogglePinsOutput(BOARD_SERVO_CLK_GPIO, 1U << BOARD_SERVO_CLK_GPIO_PIN)

#define SERVO_STATE_ON() GPIO_ClearPinsOutput(BOARD_SERVO_STATE_GPIO, 1U << BOARD_SERVO_STATE_GPIO_PIN)
#define SERVO_STATE_OFF() GPIO_SetPinsOutput(BOARD_SERVO_STATE_GPIO, 1U << BOARD_SERVO_STATE_GPIO_PIN)
#define SERVO_STATE_TOGGLE() GPIO_TogglePinsOutput(BOARD_SERVO_STATE_GPIO, 1U << BOARD_SERVO_STATE_GPIO_PIN)


#define DRUM_ENABLE_ON() GPIO_ClearPinsOutput(BOARD_DRUM_ENABLE_GPIO, 1U << BOARD_DRUM_ENABLE_GPIO_PIN)
#define DRUM_ENABLE_OFF() GPIO_SetPinsOutput(BOARD_DRUM_ENABLE_GPIO, 1U << BOARD_DRUM_ENABLE_GPIO_PIN)
#define DRUM_ENABLE_TOGGLE() GPIO_TogglePinsOutput(BOARD_DRUM_ENABLE_GPIO, 1U << BOARD_DRUM_ENABLE_GPIO_PIN)

#define VAC_ENABLE_ON() GPIO_ClearPinsOutput(BOARD_VAC_ENABLE_GPIO, 1U << BOARD_VAC_ENABLE_GPIO_PIN)
#define VAC_ENABLE_OFF() GPIO_SetPinsOutput(BOARD_VAC_ENABLE_GPIO, 1U << BOARD_VAC_ENABLE_GPIO_PIN)

#define BOARD_WG2DIR_ON() GPIO_ClearPinsOutput(BOARD_WG2DIR_GPIO, 1U << BOARD_WG2DIR_GPIO_PIN)
#define BOARD_WG2DIR_OFF() GPIO_SetPinsOutput(BOARD_WG2DIR_GPIO, 1U << BOARD_WG2DIR_GPIO_PIN)

#define BOARD_WG2VEL_ON() GPIO_ClearPinsOutput(BOARD_WG2VEL_GPIO, 1U << BOARD_WG2VEL_GPIO_PIN)
#define BOARD_WG2VEL_OFF() GPIO_SetPinsOutput(BOARD_WG2VEL_GPIO, 1U << BOARD_WG2VEL_GPIO_PIN)



// PROTOTYPES ==================================================================================================

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */


void BOARD_InitDebugConsole(void);
void InitBSP(void);
void setGPIOPin (GPIO_Type *base, uint32_t pin, gpio_pin_direction_t dir, uint32_t status);
void setDigitalInput (GPIO_Type *gpiobase, PORT_Type *portbase, uint32_t pin);
void setServoPWM();
void setPWM(uint8_t channel, uint8_t value);
void setupTach();


#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _BOARD_H_ */
