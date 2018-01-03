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

    @file          demo.c
    @version       1.0
    @date          Sept 2017

======================================================================== */

#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <nettle/nettle-stdint.h>
#include <hwlib/hwlib.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>

#include "iot_monitor.h"
#include "binio.h"
#include "mytimer.h"
#include "m2x.h"
#include "http.h"
#include "lis2dw12.h"
#include "HTS221.hpp"
#include "mal.hpp"

typedef struct led_val_t {
    float temp;
    float humid;
    float myAccelZ;
    float myAccelY;
    } LED_VAL;

#define RBG_LED_RED    GPIO_PIN_92
#define RBG_LED_GREEN  GPIO_PIN_101
#define RBG_LED_BLUE   GPIO_PIN_102

#define RED_LED        1  //0001 RED
#define GREEN_LED      2  //0010 GREEN
#define YELLOW_LED     3  //0011 RED + GREEN
#define BLUE_LED       4  //0100
#define MAGENTA_LED    5  //0101 BLUE + RED
#define CYAN_LED       6  //0110 GREEN+ BLUE 
#define WHITE_LED      7  //0111 GREEN+BLUE+RED

#define HGREEN_LED      {80.0, 80.0, 0.6, 0.6} 
#define HMAGENTA_LED    {80.0, 80.0, 0.7, 0.4} 
#define HYELLOW_LED     {80.0, 80.0, 0.3, 0.8} 
#define HTURQUOISE_LED  {80.0, 80.0, 0.2, 0.2} 

#define LRED_LED        {1.0, 1.0, 0.6, 0.6} 
#define LWHITE_LED      {1.0, 1.0, 0.7, 0.4} 
#define LCYAN_LED       {1.0, 1.0, 0.3, 0.8} 
#define LBLUE_LED       {1.0, 1.0, 0.2, 0.2} 



#define FLOW_BASE_URL    "https://runm-east.att.io/cfb0a90848c28/eb2514c29597/c94b14f417ae42a/in/flow"
#define FLOW_INPUT_NAME  "climate"
#define FLOW_DEVICE_NAME "vstarterkit001"
#define FLOW_SERVER      "run-west.att.io"

gpio_handle_t adctmr_hndl=0, relay2=0, user_key=0, red_led=0, green_led=0, blue_led=0;
volatile int button_press=0;
volatile int relay2_val=0;
struct timespec key_press, key_release, keypress_time;

adc_handle_t my_adc=(adc_handle_t)NULL;
float        adc_voltage;
volatile int valid_adc_value = 0;

static char gps_cmd[512];

LED_VAL led_demo[] = {
    HGREEN_LED,
    HMAGENTA_LED,
    HYELLOW_LED,
    HTURQUOISE_LED,
    LWHITE_LED,
    LRED_LED,
    LBLUE_LED,
    LCYAN_LED,
};

void set_color( char *color )
{
    int val=0;

    if( !strcmp(color, "BLUE") )
        val=BLUE_LED;
    else if( !strcmp(color, "GREEN") )
        val=GREEN_LED;
    else if( !strcmp(color, "BLUE") )
        val=BLUE_LED;
    else if( !strcmp(color, "MAGENTA") )
        val=MAGENTA_LED;
    else if( !strcmp(color, "TURQUOISE") )
        val=CYAN_LED;
    else if( !strcmp(color, "RED") )
        val=RED_LED;
    else if( !strcmp(color, "WHITE") )
        val=WHITE_LED;
    else if( !strcmp(color, "YELLOW") )
        val=YELLOW_LED;
    else if( !strcmp(color, "CYAN") )
        val=CYAN_LED;
    else
        val=0;
    if( dbg_flag & DBG_DEMO )
        printf("-DEMO: Set LED %s\n",color);
    gpio_write( red_led, (val&RED_LED)?GPIO_LEVEL_HIGH:GPIO_LEVEL_LOW );
    gpio_write( green_led, (val&GREEN_LED)?GPIO_LEVEL_HIGH:GPIO_LEVEL_LOW );
    gpio_write( blue_led, (val&BLUE_LED)?GPIO_LEVEL_HIGH:GPIO_LEVEL_LOW );

}


int gpio_irq_callback(gpio_pin_t pin_name, gpio_irq_trig_t direction)
{
    gpio_level_t the_val=0;
	if (pin_name != GPIO_PIN_98) {
            return 0;
            }

        if( !button_press ) {
//
// toggle GPIO_PIN_3
//
            relay2_val = !relay2_val;
            button_press = 1;
            gpio_write( relay2, (relay2_val)?GPIO_LEVEL_HIGH:GPIO_LEVEL_LOW );

            clock_gettime(CLOCK_MONOTONIC, &key_press);
            if( dbg_flag & DBG_DEMO )
                printf("-DEMO: KEY PRESS detected\n-DEMO: RELAY2 is %d\n",relay2_val);
            }
        else {
            button_press = 0;
            clock_gettime(CLOCK_MONOTONIC, &key_release);
            if ((key_release.tv_nsec-key_press.tv_nsec)<0) {
		keypress_time.tv_sec = key_release.tv_sec-key_press.tv_sec-1;
	        } 
            else {
		keypress_time.tv_sec = key_release.tv_sec-key_press.tv_sec;
	        }
            if( dbg_flag & DBG_DEMO )
                printf("-DEMO: KEY RELEASE detected, pressed for %ld secs\n",(keypress_time.tv_sec));
            }

	return 0;
}

unsigned int ascii_to_epoch(char *epoch_ascii)
{
    unsigned long long int lepoch=0;
    unsigned long long int tens=1;
    unsigned int epoch;
    int asciilen, nbr;
    for( asciilen=strlen(epoch_ascii)-1; asciilen>=0; asciilen-- ) {
        nbr = epoch_ascii[asciilen] - 0x30;
        lepoch += (tens * nbr);
        tens *= 10;
        }
    return lepoch/1000;
}

void sendGPS(void)
{
    memset(gps_cmd, 0x00, sizeof(gps_cmd));
    sprintf(gps_cmd,"&LAT=51.04427&LONG=-114.062019&ALT=3438");

    if (dbg_flag & DBG_DEMO)
        printf("-DEMO: GPS command set to (%s)\n",gps_cmd);
}

//
// Following routine are for monitoring the ADC during demo mode
//

void gpio_adc_timer_task(size_t timer_id, void * user_data)
{
    int i;
    float adcv;
    extern float adc_threshold;

    valid_adc_value = adc_read(my_adc, &adcv); //read the ADC value, if read successful
                                               //i = 0,otherwise it is 1
    if( valid_adc_value )
        return;

    adc_voltage = adcv;
    if( dbg_flag & DBG_DEMO )
        printf("-DEMO: ADC Voltage Read: %f\n",adc_voltage);

    gpio_write( adctmr_hndl, (adc_voltage>adc_threshold)?GPIO_LEVEL_HIGH:GPIO_LEVEL_LOW );
}


int command_demo_mode(int argc, const char * const * argv )
{
    char *url;
    int   start_data_service(void);
    void  wwan_io(int);
    char  cmd[1024], resp[1024];
    char  color[10];
    int   done=0, k=0, i;
    float hts221_temp, hts221_humid;

    if (dbg_flag & DBG_DEMO)
        printf("-Demo: Starting Demo Mode.\n");
    printf("The LED will be red while establishing a connection to FLOW\n");
    printf("It will turn GREEN once connected.  After that pressing the\n");
    printf("USER button will cause the program to send 2 messages to M2X\n");
    printf("and 1 message to FLOW.  If you hold the button down for >3 \n");
    printf("seconds, it will exit demo mode and re-enter the monitor.\n\nconnecting...\n");


    if( argc > 0 )
        headless_timed = argc;

    if( argv == NULL ) {
        printf("ERROR: Must supply a FLOW URL\n");
        return 0;
        }

    url = (char*)argv;

     if (dbg_flag & DBG_DEMO)
        printf("using FLOW_BASE_URL of %s\n",url);

    binario_io_close();

    gpio_init( GPIO_PIN_92,  &red_led );
    gpio_init( GPIO_PIN_101, &green_led );
    gpio_init( GPIO_PIN_102, &blue_led );
    gpio_init( GPIO_PIN_3,   &relay2 );
    gpio_init( GPIO_PIN_4,   &adctmr_hndl );

    gpio_dir(red_led,     GPIO_DIR_OUTPUT);
    gpio_dir(green_led,   GPIO_DIR_OUTPUT);
    gpio_dir(blue_led,    GPIO_DIR_OUTPUT);
    gpio_dir(relay2,      GPIO_DIR_OUTPUT);
    gpio_dir(adctmr_hndl, GPIO_DIR_OUTPUT);

    gpio_init( GPIO_PIN_98,  &user_key );  //SW3
    gpio_dir(user_key, GPIO_DIR_INPUT);
    gpio_irq_request(user_key, GPIO_IRQ_TRIG_BOTH, gpio_irq_callback);
    button_press = 0;

    gpio_write( adctmr_hndl, GPIO_LEVEL_LOW );

    start_IoTtimers();
    adc_init(&my_adc);
    create_IoTtimer(1, gpio_adc_timer_task, TIMER_PERIODIC, NULL);


    if (dbg_flag & DBG_DEMO)
        printf("-Demo: Set LED RED\n");
    // while we are waiting for a data connection, make the LED RED...
    gpio_write( red_led, GPIO_LEVEL_HIGH);
    gpio_write( relay2, GPIO_LEVEL_LOW);  //startup with relay2 off
    if (dbg_flag & DBG_DEMO)
        printf("-DEMO: RELAY2 is %d\n",relay2_val);

    sendGPS();
    while( headless || !done ) {
        struct timeval start, end;  //measure duration of flow calls...
        double elapse=0;
        int dly;
        json_keyval om[20];
        char sstrength[20]; 

        get_wwan_status(om, sizeof(om));
        strncpy(sstrength,om[4].value, 10);
        if( !strcmp(om[7].value,"1") )
            wwan_io(1);
        else
            wwan_io(0);

        memset(cmd, 0x00, sizeof(cmd));
        sprintf(cmd,"&temp=%4.2f&humidity=%4.2f&accelX=0.0&accelY=%3.1f&accelZ=%3.1f", 
                     led_demo[k].temp, led_demo[k].humid, led_demo[k].myAccelY, led_demo[k].myAccelZ);
        gettimeofday(&start, NULL);
        if (dbg_flag & DBG_DEMO) {
            time_t t = time(NULL);
            struct tm *tmp = localtime(&t);
            strftime(resp, sizeof(resp), "%H-%M-%S", tmp);
            printf("==>-DEMO LOOP BEGIN AT: %s\n",resp);
            }
        flow_get ( url, FLOW_INPUT_NAME, FLOW_DEVICE_NAME, FLOW_SERVER, cmd, resp, sizeof(resp));
        gettimeofday(&end, NULL);
        sscanf(resp, "{\"status\":\"accepted\",\"LED\":\"%s", color);

        if (dbg_flag & DBG_DEMO)
            printf("-Demo: flow said: %s\n",resp);
        color[strlen(color)-2] = 0x00;
        set_color("OFF");
        set_color(color);

        if (!headless_timed)
            while( !button_press ); /* wait for a button press */

        if (doM2X) {
            if (dbg_flag & DBG_DEMO)
                printf("-DEMO: HTS221 data to M2X\n");
            do_hts2m2x();
            }

//----
        {
        i = hts221_getDeviceID();
        if( !i ) 
            printf("WARN: No HTS221 detected! Temp & Humidity value will be 0\n");
        else {
            while( !hts221_getHumidity() );
            while( !hts221_getTemperature() );
            hts221_temp = hts221_readTemperature();
            hts221_humid= hts221_readHumidity();
            }
        float x, y, z;
        int   lis2dw12_readTemp8(void);
        float lis2dw12_readTemp12(void);
        char  **ptr, **lis2dw12_m2x(void);

        float bit12_temp =  lis2dw12_readTemp12();
        int bit8_temp    =  lis2dw12_readTemp8();

        ptr=lis2dw12_m2x();
        sscanf(ptr[0],"%f",&x);
        sscanf(ptr[1],"%f",&y);
        sscanf(ptr[2],"%f",&z);

        if (dbg_flag & DBG_DEMO) {
            printf("-DEMO: to PubNub, X=%6.2f Y=%6.2f Z=%6.2f, Signal Strength=%s\n",x,y,z,sstrength);
            printf("-DEMO: to PubNub, A2D=%6.4f HTS221_temp= %4.2f THS221_humid= %4.2f\n",adc_voltage, hts221_temp, hts221_humid);
            }
    
        memset(cmd, 0x00, sizeof(cmd));
        sprintf(cmd,"&LIS2DW12_x=%06.2f&LIS2DW12_y=%06.2f&LIS2DW12_z=%06.2f"
                    "&SSIG=%s&HTS221_TEMP=%4.2f&HTS221_HUMID=%3.1f&ADC=%4.3f%s",
                     x, y, z, sstrength, hts221_temp, hts221_humid, adc_voltage, gps_cmd);
    
        if (dbg_flag & DBG_DEMO)
            printf("-DEMO: data command to PUBNUB (%s)\n",cmd);
    
        flow_get ( url, "pubnub", FLOW_DEVICE_NAME, FLOW_SERVER, cmd, resp, sizeof(resp));
        gettimeofday(&end, NULL);
        elapse = (end.tv_sec - start.tv_sec)*1000 + (end.tv_usec/1000 - start.tv_usec/1000);
        if (dbg_flag & DBG_DEMO) {
            time_t t = time(NULL);
            struct tm *tmp = localtime(&t);
            strftime(resp, sizeof(resp), "%H-%M-%S", tmp);
            printf("==>-DEMO LOOP END AT: %s\n",resp);
            printf("==>-DEMO LOOP TOOK %5.2f seconds\n",elapse/1000);
            }
        }

        if (doM2X)
            do_adc2m2x();

        dly = ((headless_timed*1000)-round(elapse))/1000;
        if( dly > 0) {
            if (dbg_flag & DBG_DEMO) 
                printf("==>-DEMO: delay %d seconds\n",dly);
            sleep(dly);
            }

        k++;
        if( k > (sizeof(led_demo)/sizeof(LED_VAL)-1) ) 
            k = 0;
        while( button_press ); /* wait for the user to release the button */
        if( keypress_time.tv_sec > 3 ) {
            if( headless == 1 ) headless = 0;
            done = 1;
            }
        }

    gpio_deinit( &red_led);
    gpio_deinit( &green_led);
    gpio_deinit( &blue_led);
    gpio_deinit( &user_key);
    gpio_deinit( &adctmr_hndl);

    stop_IoTtimers();
    adc_deinit(&my_adc);
    if (dbg_flag & DBG_DEMO)
        printf("Restarting the Monitor...\n");
    binary_io_init();
}

