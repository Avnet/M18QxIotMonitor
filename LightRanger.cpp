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

    @file          LightRanger.cpp
    @version       1.0
    @date          Sept 2017

======================================================================== */


#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <math.h>

extern "C" {
#include <hwlib/hwlib.h>
}

#include "iot_monitor.h"

extern "C" int test_vl53l0x();

#define VL53L1_SAD	(29<<1)

int command_VL53L0X(int argc, const char * const * argv )
{
    int     dur, range=0;
    double  lux=0.0, elapse=0;
    struct  timeval start_time, stop_time;  

    if( argc == 2 )
        dur = atoi(argv[1]);
    else
        dur = 1;
 
//jmf    test_vl53l0x();

#if 0
    if (tof.chkVL53L0X()) {
        printf("run VL53L0X for %d seconds\n",dur);
        tof.init(0);
    
        gettimeofday(&start_time, NULL);
        stop_time = start_time;
        printf("sending mesurments for %d seconds.\n   Range [mm]:\r",dur);
        tof.startContinuous(0);
        while( ((dur*1000)-round(elapse))/1000 > 0) {
            printf("   Range [mm]: %-5d\r", tof.readRangeContinuousMillimeters());
            fflush(stdout);
            gettimeofday(&stop_time, NULL);
            elapse = (((stop_time.tv_sec - start_time.tv_sec)*1000) + (stop_time.tv_usec/1000 - start_time.tv_usec/1000));
            }
        tof.stopContinuous();
        printf("\n");
        }
    else
        printf("No VL53L0X detected. (%d)\n",tof.getLastStatus());
#endif
}
