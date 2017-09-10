/**
 ******************************************************************************
 * File Name          : gpio.h
 * Date               : 28/01/2016 20:57:00
 * Description        : This file contains definitions of gpio indices
 ******************************************************************************
 *
 * COPYRIGHT(c) 2014 STMicroelectronics
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __gpio_defs_H
#define __gpio_defs_H
#ifdef __cplusplus
extern "C" {
#endif

/*
 * this enum defines the indices various GPIO arrays defined
 * for each board.
 */
typedef enum
{
//  GPIO NAME  // PCB1515A NET / PCB1515B NET / PCB1537A NET /
// ------------//--------------/--------------/--------------/
    RS_GPIO1,  //              /              / SPARE9       /
    RS_GPIO2,  //              /              / SPARE10      /
    RS_GPIO3,  // STM32_MISC0  / STM32_MISC0  / SPARE11      /
    RS_GPIO4,  // STM32_MISC1  / STM32_MISC1  /              /
    RS_GPIO15, //              / SW1          /              /
    RS_GPIO29, // STM32_SS     / STM32_SS     / STM32_CS0    /
    RS_GPIO35, //              /              / SYSTEM_CS1   /
    RS_GPIO40, //              /              / GPIO3        /
    RS_GPIO41, // GPIO_B0      / GPIO_PE10    / GPIO4        /
    RS_GPIO42, // GPIO_B1      / GPIO_PE11    / GPIO5        /
    RS_GPIO43, //              /              / GPIO6        /
    RS_GPIO44, //              /              / GPIO7        /
    RS_GPIO45, //              /              / GPIO8        /
    RS_GPIO51, //              /              / DUT_CS0      /
    RS_GPIO53, //              / DVDD_EN      /              /
    RS_GPIO54, //              / IOVDD_EN     /              /
    RS_GPIO55, // VDD_VCSEL_EN / VDD_VCSEL_EN / SPARE8       /
    RS_GPIO56, //              /              / SPARE7       /
    RS_GPIO57, //              / GPIO_PD10    / SPARE6       /
    RS_GPIO58, //              / GPIO_PD11    / SPARE5       /
    RS_GPIO59, //              /              / SPARE4       /
    RS_GPIO60, //              /              / DUT_PWR_EN   /
    RS_GPIO61, // STM32_GPIO0  / STM32_GPIO0  / DUT_XSDN     /
    RS_GPIO62, // STM32_GPIO1  / STM32_GPIO1  / DUT_INT      /
    RS_GPIO63, // USB_ENUM     / USB_ENUM     / USB_ENUM     /
    RS_GPIO64, // COMMS_ACTIVE / COMMS_ACTIVE / COMMS_ACTIVE /
    RS_GPIO65, // VDD_SENSOR_EN/ VDD_SENSOR_EN/              /
    RS_GPIO78, //              /              / SPARE14      /
    RS_GPIO79, //              /              / SPARE13      /
    RS_GPIO80, //              /              / SPARE12      /
    RS_GPIO81, // GPIO_D0      / GPIO_PD0     / GPIO13       /
    RS_GPIO82, // GPIO_D1      / GPIO_PD1     / GPIO12       /
    RS_GPIO83, //              /              / GPIO11       /
    RS_GPIO84, //              /              / GPIO10       /
    RS_GPIO85, //              /              / GPIO9        /
    RS_GPIO95, //              /              / SPARE3       /
    RS_GPIO96, //              /              / SPARE2       /
    RS_GPIO97, //              /              / SPARE1       /
    RS_GPIO98, //              /              / SPARE0       /
    GPIO_COUNT,

    // Specific Names - redefinitions of some of the items above
    RS_GPIO_XSDN=22,
    RS_GPIO_INT=23
} RS_GPIO_Pin;


typedef enum
{
	GPIO_Input  = 0,
	GPIO_OutputPP = 1,
	GPIO_OutputOD = 2,
	GPIO_Analog = 3,
	GPIO_Input_Pullup = 4,
	GPIO_Input_Pulldown = 5
} RS_GPIO_Mode;


typedef enum
{
	Pin_State_Low = 0,
	Pin_State_High,
} RS_GPIO_State;


#ifdef __cplusplus
}
#endif
#endif /*__ __gpio_defs_H */

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
