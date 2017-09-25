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

    @file          wwan.c
    @version       1.0
    @date          Sept 2017

======================================================================== */

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include <pthread.h>
#include <nettle/nettle-stdint.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "mal.hpp"
#include "mytimer.h"

void wwan_blink( int interval ) ;
void wwan_timer_task(size_t timer_id, void * user_data);
void wwan_mon_task(size_t timer_id, void * user_data);

size_t wwan_timr=0, wwan_mon=0;
int    wwan_rate=0, wwan_val=0;
int    wwan_state=0, last_wwan_state;
extern int mal_busy;

void drive_wwan_io(void);

void wwan_io(int onoff) {

    int fd;
    char buf[80]; 

    fd = open("/sys/class/leds/wwan/brightness", O_WRONLY);

    write(fd, onoff?"1":"0", 1);

    close(fd);
}

void monitor_wwan( void )
{
    if( wwan_mon )
        return;
    wwan_state=0;
    start_IoTtimers();
    wwan_mon = create_ms_IoTtimer(125, wwan_mon_task, TIMER_PERIODIC, NULL);
}

void kill_monitor_wwan( void )
{
    wwan_state=0;
    delete_IoTtimer(wwan_timr);
    delete_IoTtimer(wwan_mon);
    stop_IoTtimers();
}

void wwan_mon_task(size_t timer_id, void * user_data)
{
    drive_wwan_io();
}

void drive_wwan_io(void) 
{
    json_keyval om[20];
    char *ptr;

    if( mal_busy ) {
        return;
        }

    ptr=get_ipAddr(om, sizeof(om));
    if( strcmp(ptr,"0.0.0.0") ) {  //if we have an ip address turn wwan_io on all the time
        wwan_state=4;
        wwan_blink(0);
        wwan_io(1);
        }
    else{
        // NO IP address, are we on-line.  If so, blink the wwan_io LED
        ptr=getOperatingMode(om, sizeof(om));
        if( !atoi(ptr) ) {
            wwan_state=3;
            wwan_blink(250);          //we are on-line, so blink the wwan_io LED fast
            }
        else{
            //No IP address and we are not on-line, do we have Signal, if so blink wwan_io LED slow
            get_wwan_status(om, sizeof(om));
            if( atoi(om[6].value) ){
                wwan_state=2;
                wwan_blink(500);
                }
            else {
                wwan_state=1;
                wwan_blink(0);
                wwan_io(0);
            }
        }
    }
}


void wwan_blink( int interval ) 
{

    if( last_wwan_state == wwan_state ) 
        return; 

    if( !wwan_timr && interval ) {
        wwan_timr = create_ms_IoTtimer(interval, wwan_timer_task, TIMER_PERIODIC, NULL);
        wwan_rate = interval;
        wwan_val  = 0;
        }
    else {
        if( interval ) { //wwan is already running, just change the interval
            delete_IoTtimer(wwan_timr);
            wwan_timr = create_ms_IoTtimer(interval, wwan_timer_task, TIMER_PERIODIC, NULL);
            wwan_rate = interval;
            }
         else {          //want to kill the timer
            delete_IoTtimer(wwan_timr);
            wwan_timr = wwan_rate = wwan_val  = 0;
            wwan_io(wwan_val);
            }
        }
    last_wwan_state=wwan_state;
}

void wwan_timer_task(size_t timer_id, void * user_data)
{
    timer_id = timer_id;
    user_data= user_data;
    wwan_val = !wwan_val;  //toggle the value
    wwan_io(wwan_val);
}

