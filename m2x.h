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

    @file          m2x.h
    @version       1.0
    @date          Sept 2017

======================================================================== */


#ifndef __M2X_H__
#define __M2X_H__

#include <stdio.h>
#include <time.h>
#include "mytimer.h"

typedef void (*m2xSensorTx)(int,int);

typedef struct _m2xfunc {
    char                *name;
    int			rate;
    m2xSensorTx		func;
    struct timer_node * tmr;
    } M2XFUNC;
 
#ifdef __cplusplus
extern "C" {
#endif

extern M2XFUNC m2xfunctions[];
extern const int _max_m2xfunctions;
extern size_t m2x_sensor_timer;

extern void do_hts2m2x(void);
extern void do_adc2m2x(void);

#ifdef __cplusplus
}
#endif

#include <nettle/nettle-stdint.h>
#include <hwlib/hwlib.h>

#endif // __M2X_H__

