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

    @file          binio.c
    @version       1.0
    @date          Sept 2017

======================================================================== */


#include <stdio.h>
#include <string.h>

#include <time.h>
#include <sys/types.h>
#include <nettle/nettle-stdint.h>
#include <hwlib/hwlib.h>

#include "iot_monitor.h"
#include "binio.h"
#include "mytimer.h"
#include "m2x.h"
#include "lis2dw12.h"

GPIOPIN_IN gpio_input;
size_t     gpio_input_timer;

GPIOPIN gpios[] = {
//  nbr, rate, val, hndl, timr
     92,  0,   0,    0,   0,   //RED LED
    101,  0,   0,    0,   0,   //GREEN LED
    102,  0,   0,    0,   0,   //BLUE LED
    };
#define _MAX_GPIO	(sizeof(gpio)/sizeof(GPIOPIN))

#define _MAX_GPIOPINS	(sizeof(gpios)/sizeof(GPIOPIN))
const int _max_gpiopins = _MAX_GPIOPINS;


//
//  initialize all the binary i/o pins in the system
//
void binary_io_init(void)
{
    gpio_init( GPIO_PIN_92,   &gpios[0].hndl );
    gpio_init( GPIO_PIN_101,  &gpios[1].hndl );
    gpio_init( GPIO_PIN_102,  &gpios[2].hndl );

    gpio_dir(gpios[0].hndl, GPIO_DIR_OUTPUT);
    gpio_dir(gpios[1].hndl, GPIO_DIR_OUTPUT);
    gpio_dir(gpios[2].hndl, GPIO_DIR_OUTPUT);

    gpio_init( GPIO_PIN_98,  &gpio_input.hndl );  //SW3
    gpio_dir(gpio_input.hndl, GPIO_DIR_INPUT);
}

void binario_io_close(void)
{
    gpio_deinit( &gpios[0].hndl);
    gpio_deinit( &gpios[1].hndl);
    gpio_deinit( &gpios[2].hndl);
    gpio_deinit( &gpio_input.hndl);
    release_irqs();
}



void gpio_timer_task(size_t timer_id, void * user_data)
{
    void lis2dw12_timer_task(size_t timer_id, void * user_data);
    struct timer_node * node = (struct timer_node *)timer_id;
    GPIOPIN *mytimer = (GPIOPIN*)node->user_data;
    extern int ft_mode;  //factory test mode flag
    int done,i=0;

    if( ft_mode ) {
        switch(ft_mode) {
            case 1:
                gpio_write( mytimer[1].hndl, GPIO_LEVEL_LOW );
                gpio_write( mytimer[0].hndl, GPIO_LEVEL_HIGH );
                break;
            case 2:
                gpio_write( mytimer[0].hndl, GPIO_LEVEL_LOW );
                gpio_write( mytimer[2].hndl, GPIO_LEVEL_HIGH );
                break;
            case 3:
                gpio_write( mytimer[2].hndl, GPIO_LEVEL_LOW );
                gpio_write( mytimer[1].hndl, GPIO_LEVEL_HIGH );
                break;
            }
        if( ++ft_mode > 3)
            ft_mode = 1;
        return;
        }

    do {
        done = ((mytimer[i]).timr == timer_id);
        i++;
        }
    while (!done && i<_max_gpiopins);
    --i;

    const time_t t = time(0);

    mytimer[i].val = !mytimer[i].val;  //toggle the value
    done=gpio_write( mytimer[i].hndl, mytimer[i].val );
    if( dbg_flag & DBG_BINIO ) {
        printf("-BINIO: (%d) Toggle GPIO pin GPIO_PIN_",mytimer[i].timr);
        printf("%d/%d, Rate=%d, Value=%d(%d) : %s",mytimer[i].nbr,i,mytimer[i].rate,mytimer[i].val,done,asctime(localtime(&t)));
        }
    lis2dw12_timer_task(0,NULL);
}


void do_gpio_blink( int i, int interval ) 
{
    if( gpios[i].timr != 0 ) {  //currently have a timer running
        if( interval ) { //just want to change the rate of samples
            delete_IoTtimer(gpios[i].timr);
            gpios[i].timr = create_IoTtimer(interval, gpio_timer_task, TIMER_PERIODIC, (void*)&gpios);
            gpios[i].rate = interval;
            if( dbg_flag & DBG_BINIO ) 
                printf("-BINIO: changed timer GPIO_PIN_%d/%d (%d)\n",gpios[i].nbr,i,gpios[i].timr);
            }
         else {  //want to kill the timer
            delete_IoTtimer(gpios[i].timr);
            stop_IoTtimers();
            gpios[i].timr = gpios[i].rate = gpios[i].val  = 0;
            if( dbg_flag & DBG_BINIO ) 
                printf("-BINIO: stopped timer GPIO_PIN_%d/%d (%d) - [%s]\n", 
                        gpios[i].nbr,i,gpios[i].timr, active_IoTtimer()?"RUNNING":"STOPPED");
            }
        }
    else { //don't have a timer currently running, start one up
        start_IoTtimers();
        gpios[i].timr = create_IoTtimer(interval, gpio_timer_task, TIMER_PERIODIC, (void*)&gpios);
        gpios[i].rate = interval;
        gpios[i].val  = 1;
        gpio_write( gpios[i].hndl, gpios[i].val );
        if( dbg_flag & DBG_BINIO ) 
            printf("-BINIO: started timer GPIO_PIN_%d/%d (%d) - [%s]\n", 
                    gpios[i].nbr,i,gpios[i].timr,active_IoTtimer()?"RUNNING":"STOPPED");
        }
}

void my_gpio_cb( size_t val )
{
    static gpio_level_t last_val=0;

    if (val != last_val) {
        if( !val ) {
            do_hts2m2x();
            if( dbg_flag & DBG_BINIO ) 
                printf("-BINIO: detected a SW2 Press\n");
            }
        else if( dbg_flag & DBG_BINIO ) 
            printf("-BINIO: detected a SW2 Release\n");
        last_val=val;
        }
}

void gpio_input_timer_task(size_t timer_id, void * user_data)
{
    gpio_deinit( &gpio_input.hndl);
    gpio_init(GPIO_PIN_98,  &gpio_input.hndl);  //SW3
    gpio_dir(gpio_input.hndl, GPIO_DIR_INPUT);

    gpio_read(gpio_input.hndl, &gpio_input.val);
    if( dbg_flag & DBG_BINIO )
        printf("-BINIO: timer task read (%d)\n",gpio_input.val);
    gpio_input.func(gpio_input.val);
}

void monitor_gpios( void )
{
    gpio_input.nbr=4;
    gpio_input.rate=0;
    gpio_input.val=0;
    if( dbg_flag & DBG_BINIO )
        printf("-BINIO: initial read (%d)",gpio_read(gpio_input.hndl, &gpio_input.val));
    gpio_input.func = my_gpio_cb;

    start_IoTtimers();
    gpio_input.timr = create_IoTtimer(1, gpio_input_timer_task, TIMER_PERIODIC, NULL);
}


