#include <stdio.h>
#include <string.h>

#include <nettle/nettle-stdint.h>
#include <hwlib/hwlib.h>

#include "adc.h"

//
//  initialize all the binary i/o pins in the system
//
void do_adc_init(void)
{
    gpio_init( GPIO_PIN_2,  &gpio[0].hndl );
    gpio_init( GPIO_PIN_7,  &gpio[1].hndl );
    gpio_init( GPIO_PIN_3,  &gpio[2].hndl );
    gpio_init( GPIO_PIN_95, &gpio[3].hndl );

    gpio_dir(gpio[0].hndl, GPIO_DIR_OUTPUT);
    gpio_dir(gpio[1].hndl, GPIO_DIR_OUTPUT);
    gpio_dir(gpio[2].hndl, GPIO_DIR_OUTPUT);
    gpio_dir(gpio[3].hndl, GPIO_DIR_OUTPUT);
}

void do_adc_close(void)
{
    gpio_deinit( &gpio[0].hndl);
    gpio_deinit( &gpio[1].hndl);
    gpio_deinit( &gpio[2].hndl);
    gpio_deinit( &gpio[3].hndl);
}
