/**
 ******************************************************************************
 * File Name          : power_board_defs.h
 * Date               : 21/03/2016 12:00:00
 * Description        : This file contains definitions for the photonics stack
 *                      power board
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
#ifndef __power_board_defs_H
#define __power_board_defs_H
#ifdef __cplusplus
extern "C" {
#endif

#define POWER_BOARD_I2C_ADDRESS 0x60

#define ENABLE_DUT_POWER    0x00
#define ENABLE_REGULATOR    0x02
#define DISABLE_REGULATOR   0x03
#define SET_VOLTAGE_LEVEL   0x04
#define SET_FULL_VOLT_RANGE 0x06

#define NVM_REGULATOR   0x00
#define VHV_REGULATOR   0x01
#define VCSEL_REGULATOR 0x02
#define VDDIO_REGULATOR 0x03
#define AVDD3_REGULATOR 0x04
#define AVDD2_REGULATOR 0x05
#define AVDD_REGULATOR  0x06
#define DVDD_REGULATOR  0x07
     
#pragma pack(1)
typedef struct _POWER_BOARD_CMD{
    char command;
    char regulator;
    char voltage_integer;
    char voltage_fractional;
    char delay;
    char reserved0;
    char sequence;
    char reserved1;
} POWER_BOARD_CMD, *PPOWER_BOARD_CMD;
#pragma pack()

#ifdef __cplusplus
}
#endif
#endif /*__ __power_board_defs_H */

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
