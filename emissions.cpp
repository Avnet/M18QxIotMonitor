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

    @file          emissions.cpp
    @version       1.0
    @date          Sept 2017

======================================================================== */


#include <unistd.h>
#include <cctype>
#include <cstddef>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <functional>
#include <math.h>
#include <time.h>
#include <sys/time.h>

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <hwlib/hwlib.h>

#ifdef __cplusplus
}
#endif

#include "iot_monitor.h"
#include "microrl_config.h"
#include "microrl.h"
#include "HTS221.hpp"
#include "lis2dw12.h"
#include "binio.h"
#include "mytimer.h"
#include "http.h"
#include "m2x.h"

#include "mal.hpp"

#ifdef __cplusplus
extern "C" {
#endif
void wwan_io(int);
#ifdef __cplusplus
}
#endif

extern volatile int  user_press;
extern gpio_handle_t boot_key, user_key, red_led, green_led, blue_led;

extern void print_banner(void);
extern int gpio_ftirq_callback(gpio_pin_t pin_name, gpio_irq_trig_t direction);

#define WAIT_FOR_BPRESS(x) {while( !x ); while( x );}

#define red_led		gpios[0].hndl
#define green_led	gpios[1].hndl
#define blue_led	gpios[2].hndl

void do_emissions_test(void)
{
    extern GPIOPIN_IN gpio_input;
    int wwan_sig, i;
    json_keyval om[20];
    struct timeval start, end;          //measure duration of flow calls...
    double elapse=0;

    binary_io_init();
    gpio_deinit( &gpio_input.hndl);

    if( (i=gpio_init( GPIO_PIN_98,  &user_key)) != 0 )
        printf("ERROR: unable to initialize user key gpio. (%d)\n",i);
    if( (i=gpio_dir(user_key, GPIO_DIR_INPUT)) != 0 )
        printf("ERROR: can't set user key as input. (%d)\n",i);
    if( (i=gpio_irq_request(user_key, GPIO_IRQ_TRIG_BOTH, gpio_ftirq_callback)) != 0)
        printf("ERROR: can't set user key as interrupt input. (%d)\n",i);
    user_press = 0;

    gpio_write( red_led,    GPIO_LEVEL_HIGH );
    gpio_write( green_led,  GPIO_LEVEL_HIGH );
    gpio_write( blue_led,   GPIO_LEVEL_HIGH );

    print_banner();
    printf("\n>>> Press the User Key to Continue  <<<\n");
    fflush(stdout);
    user_press = 0;
    WAIT_FOR_BPRESS(user_press);

    i=start_data_service();
    while ( i < 0 ) {
        printf("WAIT: starting WNC Data Module (%d)\n",i);
        sleep(10);
        i=start_data_service();
        }

    gpio_write( red_led,    GPIO_LEVEL_LOW );
    gpio_write( green_led,  GPIO_LEVEL_LOW );
    gpio_write( blue_led,   GPIO_LEVEL_LOW );

    if( dbg_flag & DBG_EMISSIN ) 
        printf("-EMISSIONS: Data Service started...\n");

    i=lis2dw12_initialize();
    i=lis2dw12_getDeviceID();
    if( dbg_flag & DBG_EMISSIN ) 
        printf("-EMISSIONS: lis2dw12_getDeviceID()= 0x%02X\n",i);

    ft_mode = 1;  //use factory test mode to cycle through the LED colors
    do_gpio_blink( 0, 1 );
    if( dbg_flag & DBG_EMISSIN ) 
        printf("-EMISSIONS: LEDs blinking...\n");

    printf("\n>>> Press the User Key to TERMINATE test. <<<\n");
    printf("\n>>> WWAN LED indicates presence of signal <<<\n");

    check_gps(NULL);
    while( !user_press ) {
        get_wwan_status(om, sizeof(om));
        sscanf(om[4].value,"%d",&wwan_sig);
        if( dbg_flag & DBG_EMISSIN ) 
            printf("EMISSIONS: Signal RSSI   = %s (%d)\r", om[4].value, wwan_sig);
        wwan_io((wwan_sig<0)?1:0);  //light or extinguish WWAN depending on RSSI
    }
    while( user_press );  //wait for user to release the user button
    printf("\n>>> Done... <<<\n");
    disableGPS();

    wwan_io(0);
    ft_mode = 0;
    do_gpio_blink( 0, 0 );
    gpio_write( red_led,    GPIO_LEVEL_LOW );
    gpio_write( green_led,  GPIO_LEVEL_LOW );
    gpio_write( blue_led,   GPIO_LEVEL_LOW );
    gpio_deinit( &user_key);
    binario_io_close();
}



