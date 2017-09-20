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

    @file          binio.h
    @version       1.0
    @date          Sept 2017

======================================================================== */


#ifndef __BINIO_H__
#define __BINIO_H__

typedef	void (*gpio_in_cb_fn_t)(size_t);

typedef struct _gpios {
    int           nbr;
    int		  rate;
    int           val;
    gpio_handle_t hndl;
    size_t        timr;
    } GPIOPIN;
 
typedef struct _gpio_in {
    int             nbr;  //GPIO pin #
    int		    rate; // TBD
    gpio_level_t    val;
    gpio_in_cb_fn_t func; //call back function on change
    gpio_handle_t   hndl; //GPIO handle
    size_t          timr; //timer handle
    } GPIOPIN_IN;
 
#ifdef __cplusplus
extern "C" {
#endif

void gpio_timer(size_t timer_id, void * user_data);

extern size_t m2x_sensor_timer;
extern const int _max_gpiopins;
extern GPIOPIN gpios[];

void sensor_hts221(int interval);
void do_gpio_blink( int i, int interval );

void monitor_gpios(void);
void binary_io_init(void);
void binario_io_close(void);

#ifdef __cplusplus
}
#endif


#include <sys/types.h>
#include <stdint.h>
#include <nettle/nettle-stdint.h>
#include <hwlib/hwlib.h>

#endif // __BINIO_H__

