
/*
* This file is part of VL53L1 Platform
*
* Copyright (c) 2016, STMicroelectronics - All Rights Reserved
*
* License terms: BSD 3-clause "New" or "Revised" License.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.
*
* 3. Neither the name of the copyright holder nor the names of its contributors
* may be used to endorse or promote products derived from this software
* without specific prior written permission.
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
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include <hwlib/hwlib.h>

#include "vl53l1_platform.h"
#include "vl53l1_platform_log.h"

#define VL53L1_get_register_name(VL53L1_PRM_00002,VL53L1_PRM_00032)

#include "ranging_sensor_comms.h"
#include "comms_platform.h"
#include "power_board_defs.h"

const uint32_t _power_board_in_use = 0;

uint32_t _power_board_extended = 0;
uint8_t global_comms_type = 0;

#define  VL53L1_COMMS_CHUNK_SIZE  56
#define  VL53L1_COMMS_BUFFER_SIZE 64

#define GPIO_INTERRUPT          RS_GPIO62
#define GPIO_POWER_ENABLE       RS_GPIO60
#define GPIO_XSHUTDOWN          RS_GPIO61
#define GPIO_SPI_CHIP_SELECT    RS_GPIO51

#define DBGP			1 //or 0 to disable

#define TOSTR(x)		#x
#define INTSTR(x)		TOSTR(x)
#define DBGOUT(fmt, ...)	printf("DEBUG:"  __FILE__  "@" INTSTR(__LINE__) ":" fmt, ##__VA_ARGS__);
#define DBG(fmt, ...)	if(DBGP)DBGOUT(fmt, ##__VA_ARGS__);

VL53L1_Error VL53L1_CommsInitialise( VL53L1_Dev_t *pdev, uint8_t comms_type, uint16_t comms_speed_khz)
{
    DBG("VL53L1_CommsInitialise enter\n");
    SUPPRESS_UNUSED_WARNING(pdev);
    SUPPRESS_UNUSED_WARNING(comms_speed_khz);

    global_comms_type = comms_type;

    DBG("VL53L1_CommsInitialise exit\n");
    return VL53L1_ERROR_NONE;
}

VL53L1_Error VL53L1_CommsClose( VL53L1_Dev_t *pdev)
{
    DBG("VL53L1_CommsClose enter\n");
    SUPPRESS_UNUSED_WARNING(pdev);
    DBG("VL53L1_CommsClose exit\n");
    return VL53L1_ERROR_NONE;
}

VL53L1_Error VL53L1_WriteMulti( VL53L1_Dev_t *pdev, uint16_t index, uint8_t *pdata, uint32_t count)
{
    VL53L1_Error status         = VL53L1_ERROR_NONE;
    uint32_t     position       = 0;

    DBG("VL53L1_WriteMulti enter\n");

    if(global_comms_type == VL53L1_I2C) {
        for(position=0; position<count; position+=VL53L1_COMMS_CHUNK_SIZE) {
//            if (count > VL53L1_COMMS_CHUNK_SIZE) {
//                if((position + VL53L1_COMMS_CHUNK_SIZE) > count) {
//                    data_size = count - position;
//                    }
//                else
//                    data_size = VL53L1_COMMS_CHUNK_SIZE;
//                }
//            else
//                data_size = count;

//            if (status == VL53L1_ERROR_NONE) {
//                if( RANGING_SENSOR_COMMS_Write_CCI(
//                                pdev->i2c_slave_address,
//                                0,
//                                index+position,
//                                pdata+position,
//                                data_size) != 0 ) {
//                    status = VL53L1_ERROR_CONTROL_INTERFACE;
//                    }
//                }
            }

//        if(status != VL53L1_ERROR_NONE) {
//            RANGING_SENSOR_COMMS_Get_Error_Text(comms_error_string);
//
//            trace_i2c("VL53L1_WriteMulti RANGING_SENSOR_COMMS_Write_CCI() failed\n");
//            trace_i2c(comms_error_string);
//            }
        }
    else {
//        trace_i2c("VL53L1_WriteMulti: Comms must be VL53L1_I2C \n");
        status = VL53L1_ERROR_CONTROL_INTERFACE;
        }

    return status;
}


VL53L1_Error VL53L1_ReadMulti(
    VL53L1_Dev_t *pdev,
    uint16_t      index,
    uint8_t      *pdata,
    uint32_t      count)
{
    VL53L1_Error status         = VL53L1_ERROR_NONE;
//    uint32_t     position       = 0;
    //uint32_t     data_size;

//    char   comms_error_string[ERROR_TEXT_LENGTH];

    _LOG_STRING_BUFFER(register_name);
    _LOG_STRING_BUFFER(value_as_str);

    if(global_comms_type == VL53L1_I2C) {
//        for(position=0; position<count; position+=VL53L1_COMMS_CHUNK_SIZE) {
//            if(count > VL53L1_COMMS_CHUNK_SIZE) {
//                if((position + VL53L1_COMMS_CHUNK_SIZE) > count) {
//                    //data_size = count - position;
//                    }
//                else
//                    //data_size = VL53L1_COMMS_CHUNK_SIZE;
//                }
//            else
//                //data_size = count;
//
//            if(status == VL53L1_ERROR_NONE) {
//                if( RANGING_SENSOR_COMMS_Read_CCI(
//                                pdev->i2c_slave_address,
//                                0,
//                                index+position,
//                                pdata+position,
//                                data_size) != 0 ) {
//                    status = VL53L1_ERROR_CONTROL_INTERFACE;
//                    }
//                }
//            }

//        if(status != VL53L1_ERROR_NONE) {
//            RANGING_SENSOR_COMMS_Get_Error_Text(comms_error_string);
//
//            trace_i2c("VL53L1_ReadMulti: RANGING_SENSOR_COMMS_Read_CCI() failed\n");
//            trace_i2c(comms_error_string);
//            }
        }
    else {
//        trace_i2c("VL53L1_ReadMulti: Comms must be VL53L1_I2C \n");
        status = VL53L1_ERROR_CONTROL_INTERFACE;
        }

    return status;
}


VL53L1_Error VL53L1_WrByte( VL53L1_Dev_t *pdev, uint16_t index, uint8_t VL53L1_PRM_00005)
{
    VL53L1_Error status = VL53L1_ERROR_NONE;
    uint8_t      buffer[2];
    int          e;

    buffer[0] = index;
    buffer[1] = (uint8_t)(VL53L1_PRM_00005);

    e=i2c_bus_init(I2C_BUS_I, &pdev->i2c);
    e+=i2c_write(pdev->i2c, pdev->i2c_slave_address, buffer, 2, I2C_STOP);
    e+=i2c_bus_deinit(&pdev->i2c);

    if( e )
        status = VL53L1_ERROR_CONTROL_INTERFACE;

    return status;
}


VL53L1_Error VL53L1_WrWord( VL53L1_Dev_t *pdev, uint16_t index, uint16_t VL53L1_PRM_00005)
{
    VL53L1_Error status         = VL53L1_ERROR_NONE;
    uint8_t  buffer[3];
    int e;

    buffer[0] = index;
    buffer[1] = (uint8_t)(VL53L1_PRM_00005 >> 8);
    buffer[2] = (uint8_t)(VL53L1_PRM_00005 &  0x00FF);

    e=i2c_bus_init(I2C_BUS_I, &pdev->i2c);
    e+=i2c_write(pdev->i2c, pdev->i2c_slave_address, buffer, VL53L1_BYTES_PER_WORD+1, I2C_STOP);
    e+=i2c_bus_deinit(&pdev->i2c);

    if( e )
        status = VL53L1_ERROR_CONTROL_INTERFACE;

    return status;
}


VL53L1_Error VL53L1_WrDWord( VL53L1_Dev_t *pdev, uint16_t index, uint32_t VL53L1_PRM_00005)
{
    VL53L1_Error status = VL53L1_ERROR_NONE;
    uint8_t  buffer[5];
    int e;

    buffer[0] = index;
    buffer[1] = (uint8_t) (VL53L1_PRM_00005 >> 24);
    buffer[2] = (uint8_t)((VL53L1_PRM_00005 &  0x00FF0000) >> 16);
    buffer[3] = (uint8_t)((VL53L1_PRM_00005 &  0x0000FF00) >> 8);
    buffer[4] = (uint8_t) (VL53L1_PRM_00005 &  0x000000FF);

    e=i2c_bus_init(I2C_BUS_I, &pdev->i2c);
    e+=i2c_write(pdev->i2c, pdev->i2c_slave_address, buffer, VL53L1_BYTES_PER_DWORD+1, I2C_STOP);
    e+=i2c_bus_deinit(&pdev->i2c);

    if( e )
        status = VL53L1_ERROR_CONTROL_INTERFACE;

    return status;
}


VL53L1_Error VL53L1_RdByte( VL53L1_Dev_t *pdev, uint16_t index, uint8_t *pdata)
{
    VL53L1_Error status = VL53L1_ERROR_NONE;
    uint8_t  buffer[1];
    uint8_t  reg = (uint8_t)index;
    uint16_t addr=pdev->i2c_slave_address;
    i2c_handle_t handle=0;

    int e=i2c_bus_init(I2C_BUS_I, &handle);
DBG("i2c_bus_init=%d\n",e);

    e+=i2c_write(handle, addr, &reg, 1, I2C_NO_STOP);
DBG("i2c_write(x,0x%02X,0x%02X...)=%d\n",pdev->i2c_slave_address,index,e);

//    e+=i2c_read(handle, addr, buffer, 1);
//DBG("i2c_read(x,0x%02X,..)=%d\n",addr,e);

    e+=i2c_bus_deinit(&handle);
DBG("i2c_bus_deinit=%d\n",e);

    *pdata = buffer[0];
    if( e )
        status = VL53L1_ERROR_CONTROL_INTERFACE;
    return status;
}


VL53L1_Error VL53L1_RdWord( VL53L1_Dev_t *pdev, uint16_t index, uint16_t *pdata)
{
    VL53L1_Error status = VL53L1_ERROR_NONE;
    uint8_t  buffer[2], i=(uint8_t)index;
    int e=i2c_bus_init(I2C_BUS_I, &pdev->i2c);
    e+=i2c_write(pdev->i2c, pdev->i2c_slave_address, &i, 1, I2C_NO_STOP);
    e+=i2c_read(pdev->i2c, index, buffer, 2);
    e+=i2c_bus_deinit(&pdev->i2c);

    *pdata = (uint16_t)(((uint16_t)(buffer[0])<<8) + (uint16_t)buffer[1]);

    if( e )
        status = VL53L1_ERROR_CONTROL_INTERFACE;

    return status;
}


VL53L1_Error VL53L1_RdDWord( VL53L1_Dev_t *pdev, uint16_t index, uint32_t *pdata)
{
    VL53L1_Error status = VL53L1_ERROR_NONE;
    uint8_t  buffer[4], i=(uint8_t)index;
    int e=i2c_bus_init(I2C_BUS_I, &pdev->i2c);
    e+=i2c_write(pdev->i2c, pdev->i2c_slave_address, &i, 1, I2C_NO_STOP);
    e+=i2c_read(pdev->i2c, index, buffer, VL53L1_BYTES_PER_DWORD);
    e+=i2c_bus_deinit(&pdev->i2c);

    *pdata = ((uint32_t)buffer[0]<<24) + ((uint32_t)buffer[1]<<16) + ((uint32_t)buffer[2]<<8) + (uint32_t)buffer[3];

    if( e )
        status = VL53L1_ERROR_CONTROL_INTERFACE;

    return status;
}

long long timeval_diff(struct timeval *difference, struct timeval *end_time, struct timeval *start_time)
{
  struct timeval temp_diff;

  if(difference==NULL) {
    difference=&temp_diff;
  }

  difference->tv_sec =end_time->tv_sec -start_time->tv_sec ;
  difference->tv_usec=end_time->tv_usec-start_time->tv_usec;

  /* Using while instead of if below makes the code slightly more robust. */

  while(difference->tv_usec<0) {
    difference->tv_usec+=1000000;
    difference->tv_sec -=1;
  }

  return 1000000LL*difference->tv_sec+ difference->tv_usec;

} /* timeval_diff() */


VL53L1_Error VL53L1_WaitUs( VL53L1_Dev_t *pdev, int32_t wait_us)
{
    struct timeval start, stop;
    long long diff;
    SUPPRESS_UNUSED_WARNING(pdev);
    gettimeofday(&start,NULL);
    gettimeofday(&stop,NULL);
    while( (diff=timeval_diff(NULL,&stop,&start))<wait_us ) 
        gettimeofday(&stop,NULL);
    return VL53L1_ERROR_NONE;
}


VL53L1_Error VL53L1_WaitMs( VL53L1_Dev_t *pdev, int32_t wait_ms)
{
    return VL53L1_WaitUs(pdev, wait_ms * 1000);
}

VL53L1_Error VL53L1_GetTimerFrequency(int32_t *ptimer_freq_hz)
{
    *ptimer_freq_hz = 0;

//    trace_print(VL53L1_TRACE_LEVEL_INFO, "VL53L1_GetTimerFrequency: Freq : %dHz\n", *ptimer_freq_hz);
    return VL53L1_ERROR_NONE;
}

VL53L1_Error VL53L1_GetTimerValue(int32_t *ptimer_count)
{
    *ptimer_count = 0;

//    trace_print(VL53L1_TRACE_LEVEL_INFO, "VL53L1_GetTimerValue: Freq : %dHz\n", *ptimer_count);
    return VL53L1_ERROR_NONE;
}

VL53L1_Error VL53L1_GpioSetMode(uint8_t pin, uint8_t mode)
{
    VL53L1_Error status         = VL53L1_ERROR_NONE;

//    if(RANGING_SENSOR_COMMS_GPIO_Set_Mode((RS_GPIO_Pin)pin, (RS_GPIO_Mode)mode) != CP_STATUS_OK)
//        status = VL53L1_ERROR_CONTROL_INTERFACE;

//    trace_print(VL53L1_TRACE_LEVEL_INFO, "VL53L1_GpioSetMode: Status %d. Pin %d, Mode %d\n", status, pin, mode);
    return status;
}


VL53L1_Error  VL53L1_GpioSetValue(uint8_t pin, uint8_t value)
{
    VL53L1_Error status         = VL53L1_ERROR_NONE;

//    if(RANGING_SENSOR_COMMS_GPIO_Set_Value((RS_GPIO_Pin)pin, value) != CP_STATUS_OK)
//        status = VL53L1_ERROR_CONTROL_INTERFACE;

//    trace_print(VL53L1_TRACE_LEVEL_INFO, "VL53L1_GpioSetValue: Status %d. Pin %d, Mode %d\n", status, pin, value);
    return status;
}


VL53L1_Error  VL53L1_GpioGetValue(uint8_t pin, uint8_t *pvalue)
{
    VL53L1_Error status         = VL53L1_ERROR_NONE;

    DWORD value = 0;

//   if(RANGING_SENSOR_COMMS_GPIO_Get_Value((RS_GPIO_Pin)pin, &value) != CP_STATUS_OK)
//        status = VL53L1_ERROR_CONTROL_INTERFACE;
//    else
        *pvalue = (uint8_t)value;

//    trace_print(VL53L1_TRACE_LEVEL_INFO, "VL53L1_GpioGetValue: Status %d. Pin %d, Mode %d\n", status, pin, *pvalue);
    return status;
}

VL53L1_Error  VL53L1_GpioXshutdown(uint8_t value)
{
    VL53L1_Error status         = VL53L1_ERROR_NONE;

    if(status == VL53L1_ERROR_NONE)
        status = VL53L1_GpioSetMode((uint8_t)GPIO_XSHUTDOWN, (uint8_t)GPIO_OutputPP);

    if(status == VL53L1_ERROR_NONE) {
        if(value) {
//            if (RANGING_SENSOR_COMMS_GPIO_Set_Value(GPIO_XSHUTDOWN, (DWORD)Pin_State_High) != CP_STATUS_OK)
//            status = VL53L1_ERROR_CONTROL_INTERFACE;
            }
        else {
//           if (RANGING_SENSOR_COMMS_GPIO_Set_Value(GPIO_XSHUTDOWN, (DWORD)Pin_State_Low) != CP_STATUS_OK)
//                status = VL53L1_ERROR_CONTROL_INTERFACE;
            }
        }

//    trace_print(VL53L1_TRACE_LEVEL_INFO, "VL53L1_GpioXShutdown: Status %d. Value %d\n", status, value);
    return status;
}


VL53L1_Error  VL53L1_GpioCommsSelect(uint8_t value)
{
    VL53L1_Error status         = VL53L1_ERROR_NONE;

    if(status == VL53L1_ERROR_NONE)
        status = VL53L1_GpioSetMode((uint8_t)GPIO_SPI_CHIP_SELECT, (uint8_t)GPIO_OutputPP);

    if(status == VL53L1_ERROR_NONE) {
        if(value) {
//            if(RANGING_SENSOR_COMMS_GPIO_Set_Value(GPIO_SPI_CHIP_SELECT, (DWORD)Pin_State_High) != CP_STATUS_OK)
//                status = VL53L1_ERROR_CONTROL_INTERFACE;
            }
        else {
//            if(RANGING_SENSOR_COMMS_GPIO_Set_Value(GPIO_SPI_CHIP_SELECT, (DWORD)Pin_State_Low) != CP_STATUS_OK)
//                status = VL53L1_ERROR_CONTROL_INTERFACE;
            }
        }

//    trace_print(VL53L1_TRACE_LEVEL_INFO, "VL53L1_GpioCommsSelect: Status %d. Value %d\n", status, value);
    return status;
}


VL53L1_Error  VL53L1_GpioPowerEnable(uint8_t value)
{
    VL53L1_Error status         = VL53L1_ERROR_NONE;
    POWER_BOARD_CMD power_cmd;

    if(status == VL53L1_ERROR_NONE)
        status = VL53L1_GpioSetMode((uint8_t)GPIO_POWER_ENABLE, (uint8_t)GPIO_OutputPP);

    if(status == VL53L1_ERROR_NONE) {
        if(value) {
//            if(RANGING_SENSOR_COMMS_GPIO_Set_Value(GPIO_POWER_ENABLE, (DWORD)Pin_State_High) != CP_STATUS_OK)
//                status = VL53L1_ERROR_CONTROL_INTERFACE;
            }
        else {
//            if(RANGING_SENSOR_COMMS_GPIO_Set_Value(GPIO_POWER_ENABLE, (DWORD)Pin_State_Low) != CP_STATUS_OK)
//                status = VL53L1_ERROR_CONTROL_INTERFACE;
            }
        }

    if(status == VL53L1_ERROR_NONE && _power_board_in_use == 1 && value) {
        memset(&power_cmd, 0, sizeof(POWER_BOARD_CMD));
        power_cmd.command = ENABLE_DUT_POWER;

//        if(RANGING_SENSOR_COMMS_Write_System_I2C( POWER_BOARD_I2C_ADDRESS, sizeof(POWER_BOARD_CMD), (uint8_t*)&power_cmd) != CP_STATUS_OK)
//            status = VL53L1_ERROR_CONTROL_INTERFACE;
        }

//    trace_print(VL53L1_TRACE_LEVEL_INFO, "VL53L1_GpioPowerEnable: Status %d. Value %d\n", status, value);
    return status;
}


VL53L1_Error  VL53L1_GpioInterruptEnable(void (*function)(void), uint8_t edge_type)
{
    SUPPRESS_UNUSED_WARNING(function);
    SUPPRESS_UNUSED_WARNING(edge_type);

    return VL53L1_ERROR_NONE;
}


VL53L1_Error  VL53L1_GpioInterruptDisable(void)
{
    VL53L1_Error status         = VL53L1_ERROR_NONE;
    return status;
}


VL53L1_Error VL53L1_GetTickCount( uint32_t *ptick_count_ms)
{
    struct timeval tm;
    VL53L1_Error status  = VL53L1_ERROR_NONE;
DBG("JMF:enter VL53L1_GetTickCount\n");
    gettimeofday(&tm,NULL);
    *ptick_count_ms = (uint32_t)(tm.tv_usec/1000);  

DBG("JMF:exit VL53L1_GetTickCount\n");
    return status;
}


VL53L1_Error VL53L1_WaitValueMaskEx(
    VL53L1_Dev_t *pdev,
    uint32_t      timeout_ms,
    uint16_t      index,
    uint8_t       value,
    uint8_t       mask,
    uint32_t      poll_delay_ms)
{
    VL53L1_Error status         = VL53L1_ERROR_NONE;
    uint32_t     start_time_ms   = 0;
    uint32_t     current_time_ms = 0;
    uint8_t      byte_value      = 0;
    uint8_t      found           = 0;

    SUPPRESS_UNUSED_WARNING(poll_delay_ms);

    VL53L1_GetTickCount(&start_time_ms);
    pdev->new_data_ready_poll_duration_ms = 0;

    while ((status == VL53L1_ERROR_NONE) &&
           (pdev->new_data_ready_poll_duration_ms < timeout_ms) &&
           (found == 0)) {
DBG("Call VL53L1_RdByte\n");
        status = VL53L1_RdByte( pdev, index, &byte_value);
DBG("returned from VL53L1_RdByte\n");

        if ((byte_value & mask) == value)
            found = 1;

        VL53L1_GetTickCount(&current_time_ms);
        pdev->new_data_ready_poll_duration_ms = current_time_ms - start_time_ms;
        }

    _LOG_SET_TRACE_FUNCTIONS(trace_functions);

    if (found == 0 && status == VL53L1_ERROR_NONE)
        status = VL53L1_ERROR_TIME_OUT;

    return status;
}


