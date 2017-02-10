
#include <stdio.h>
#include <string.h>

#include <nettle/nettle-stdint.h>
#include <hwlib/hwlib.h>

#include "binio.h"
#include "mytimer.h"

GPIOPIN gpios[] = {
//  nbr, rate, val, hndl, timr
    2,    0,   0,    0,   0,
    7,    0,   0,    0,   0,
    3,    0,   0,    0,   0,
    95,   0,   0,    0,   0,
    };
#define _MAX_GPIO	(sizeof(gpio)/sizeof(GPIOPIN))

#define _MAX_GPIOPINS	(sizeof(gpios)/sizeof(GPIOPIN))
const int _max_gpiopins = _MAX_GPIOPINS;

//
//  initialize all the binary i/o pins in the system
//
void binary_io_init(void)
{
    gpio_init( GPIO_PIN_2,  &gpios[0].hndl );
    gpio_init( GPIO_PIN_7,  &gpios[1].hndl );
    gpio_init( GPIO_PIN_3,  &gpios[2].hndl );
    gpio_init( GPIO_PIN_95, &gpios[3].hndl );

    gpio_dir(gpios[0].hndl, GPIO_DIR_OUTPUT);
    gpio_dir(gpios[1].hndl, GPIO_DIR_OUTPUT);
    gpio_dir(gpios[2].hndl, GPIO_DIR_OUTPUT);
    gpio_dir(gpios[3].hndl, GPIO_DIR_OUTPUT);
}

void binario_io_close(void)
{
    gpio_deinit( &gpios[0].hndl);
    gpio_deinit( &gpios[1].hndl);
    gpio_deinit( &gpios[2].hndl);
    gpio_deinit( &gpios[3].hndl);
}


#include <time.h>
void gpio_timer_task(size_t timer_id, void * user_data)
{
    struct timer_node * node = (struct timer_node *)timer_id;
    GPIOPIN *mytimer = (GPIOPIN*)node->user_data;
    int done,i=0;

    do {
        done = ((mytimer[i]).timr == timer_id);
        i++;
        }
    while (!done && i<_max_gpiopins);
    --i;

//    if (!done)
//        printf("we got a problem\n");

    const time_t t = time(0);

//    printf(">>(%d) Toggle GPIO pin GPIO_PIN_",mytimer[i].timr);
//    printf("%d/%d, Rate=%d, Value=%d : %s",mytimer[i].nbr,i,mytimer[i].rate,mytimer[i].val,asctime(localtime(&t)));
    (mytimer[i]).val = !(mytimer[i]).val;  //toggle the value
    gpio_write( (mytimer[i]).hndl, (mytimer[i]).val );
}


void do_gpio_blink( int i, int interval ) 
{
    if( gpios[i].timr != 0 ) {  //currently have a timer running
        if( interval ) { //just want to change the rate of samples
            delete_IoTtimer(gpios[i].timr);
            gpios[i].timr = create_IoTtimer(interval, gpio_timer_task, TIMER_PERIODIC, (void*)&gpios);
            gpios[i].rate = interval;
//printf("changed timer GPIO_PIN_%d/%d (%d)\n",gpios[i].nbr,i,gpios[i].timr);
            }
         else {  //want to kill the timer
            delete_IoTtimer(gpios[i].timr);
            stop_IoTtimers();
            gpios[i].timr = gpios[i].rate = gpios[i].val  = 0;
//printf("stopped timer GPIO_PIN_%d/%d (%d) - [%s]\n", gpios[i].nbr,i,gpios[i].timr, active_IoTtimer()?"RUNNING":"STOPPED");
            }
        }
    else { //don't have a timer currently running, start one up
        start_IoTtimers();
        gpios[i].timr = create_IoTtimer(interval, gpio_timer_task, TIMER_PERIODIC, (void*)&gpios);
        gpios[i].rate = interval;
        gpios[i].val  = 1;
        gpio_write( gpios[i].hndl, gpios[i].val );
//printf("started timer GPIO_PIN_%d/%d (%d) - [%s]\n", gpios[i].nbr,i,gpios[i].timr,active_IoTtimer()?"RUNNING":"STOPPED");
        }
}
