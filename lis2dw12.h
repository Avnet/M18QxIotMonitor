/* =====================================================================
   Copyright © 2016, Avnet (R)

   www.avnet.com 
 
   Licensed under the Apache License, Version 2.0 (the "License"); 
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, 
   software distributed under the License is distributed on an 
   "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
   either express or implied. See the License for the specific 
   language governing permissions and limitations under the License.

    @file          lis2dw12.h
    @version       1.0
    @date          Sept 2017

======================================================================== */


#ifndef __LIS2DW12_H__
#define __LIS2DW12_H__
               
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/syscall.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <nettle/nettle-stdint.h>
#include <hwlib/hwlib.h>
#include <json-c/json.h>

#define LIS2DW12_DEV_NAME		"lis2dw12"

enum {
	LIS2DW12_ACCEL = 0,
	LIS2DW12_FF,
	LIS2DW12_TAP,
	LIS2DW12_DOUBLE_TAP,
	LIS2DW12_WAKEUP,
	LIS2DW12_SENSORS_NUMB,
};


#define LIS2DW12_SAD			0x19

#ifdef __cplusplus
extern "C" {
#endif

int      lis2dw12_initialize(void);
uint8_t  lis2dw12_getDeviceID(void);
void     lis2dw12_regdump(void);
int      lis2dw12_readTemp8(void);
float    lis2dw12_readTemp12(void);
void     lis2dw12_onoff( uint8_t on );
void     lis2dw12_timer_task(size_t timer_id, void * user_data);
void     lis2dw12_ot_acc_data(void);
void     release_irqs(void);

#ifdef __cplusplus
}
#endif

#endif //__LIS2DW12_H__

