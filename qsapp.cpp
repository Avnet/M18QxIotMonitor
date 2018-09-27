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

    @file          qsapp.cpp
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
void wwan_io(int);
char  **lis2dw12_m2x(void);

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

struct timespec key_press, key_release, keypress_time;
extern GPIOPIN_IN gpio_input;
extern gpio_handle_t user_key;

static volatile int bpress;

const char *current_color=NULL;
static const char *colors[] = {
    "BLUE",
    "GREEN",
    "BLUE",
    "MAGENTA",
    "TURQUOISE",
    "RED",
    "WHITE",
    "YELLOW",
    "CYAN"
    };

#define red_led		gpios[0].hndl
#define green_led	gpios[1].hndl
#define blue_led	gpios[2].hndl

#define RED_LED        1  //0001 RED
#define GREEN_LED      2  //0010 GREEN
#define YELLOW_LED     3  //0011 RED + GREEN
#define BLUE_LED       4  //0100
#define MAGENTA_LED    5  //0101 BLUE + RED
#define CYAN_LED       6  //0110 GREEN+ BLUE 
#define WHITE_LED      7  //0111 GREEN+BLUE+RED

#define WAIT_FOR_BPRESS(x) {while( !x ); while( x );}

void do_color( const char *color )
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

    gpio_write( red_led, (val&RED_LED)?GPIO_LEVEL_HIGH:GPIO_LEVEL_LOW );
    gpio_write( green_led, (val&GREEN_LED)?GPIO_LEVEL_HIGH:GPIO_LEVEL_LOW );
    gpio_write( blue_led, (val&BLUE_LED)?GPIO_LEVEL_HIGH:GPIO_LEVEL_LOW );
}

int qsa_irq_callback(gpio_pin_t pin_name, gpio_irq_trig_t direction)
{
    if (pin_name != GPIO_PIN_98) 
        return 0;

    if( bpress = !bpress ) {
        do_color("WHITE");
        clock_gettime(CLOCK_MONOTONIC, &key_press);
        }
    else{
        do_color(current_color);
        clock_gettime(CLOCK_MONOTONIC, &key_release);
        if ((key_release.tv_nsec-key_press.tv_nsec)<0) 
            keypress_time.tv_sec = key_release.tv_sec-key_press.tv_sec-1;
        else 
            keypress_time.tv_sec = key_release.tv_sec-key_press.tv_sec;
        }
    return 0;
}

int quickstart_app(int argc, const char * const * argv )
{
    void   wwan_io(int);
    void   do_lis2dw2m2x(void);
    float  lis2dw12_readTemp12(void);
    pthread_t thread1;
    float  last_lat=0.0, last_lng=0.0;
    int    last_alt=0;
    bool   using_masterkey = false;
    bool   have_deviceid   = false;
    extern struct timeval gps_start, gps_end;  //measure duration of gps call...
    extern float lat;
    extern float lng;
    extern int   alt, GPS_TO;

    adc_handle_t my_adc=(adc_handle_t)NULL;
    int          start_data_service(void);
    int          done=0, k=0, i;
    int          dly, delay_time;
    char         resp[1024], qsa_url[100];
    char         color[10];
    char         **ptr, **lis2dw12_m2x(void);
    char         str_val[16];
    char         str_lat[16];
    char         str_long[16];
    char         str_elev[16];
    char*        strptr;
    double       elapse=0;
    float        adc_voltage;

    json_keyval    om[20];
    struct timeval start, end;  //measure duration of flow calls...

//
// argc contains the delay between posting messages to M2X
// if no delay time is provided, it is assumed to be 5 seconds
//

    if( argc < 5 )
        delay_time = 5;
    else
        delay_time = argc;

    do_color(current_color="RED");
    mySystem.iccid=getICCID(om, sizeof(om));
    printf("\n\nQuick Start Application:\n");
    gpio_deinit( &gpio_input.hndl);
    bpress = 0;
    gpio_init( GPIO_PIN_98,  &user_key );  //SW3
    gpio_dir(user_key, GPIO_DIR_INPUT);
    gpio_irq_request(user_key, GPIO_IRQ_TRIG_BOTH, qsa_irq_callback);

    get_wwan_status(om, sizeof(om));
    wwan_io(!strcmp(om[7].value,"1")?1:0);

    if( !strlen(api_key) ) {
        printf("\nERROR: Must provide API KEY!\n");
        gpio_deinit( &user_key);
        binario_io_close();
        binary_io_init();
        return 0;
        }

    m2x_getkeys(1, api_key,resp);           // see if we are using the Master KEY
    strptr = strstr(resp,"\"Master Key\"");
    using_masterkey = (strptr)? true:false;
    have_deviceid = (strlen(device_id)!=0);

    if (using_masterkey ) {        // verify api_key and device_id
        m2x_getkeys(0, api_key, resp);
        strptr = strstr(resp,mySystem.iccid.c_str());
        if( strptr ) {
            char *tptr1 = strstr(strptr,"\"id\":");
            char *tptr2 = strstr(tptr1+6,"\"");
            strncpy(device_id,tptr1+6,(tptr2-tptr1)-6);
            tptr1 = strstr(strptr,"\"key\":");
            tptr2 = strstr(tptr1+7,"\"");
            strncpy(api_key,tptr1+7,(tptr2-tptr1)-7);
            printf("Primary API Key and Device already exist, using them.\n");
            }
        else{
            printf("Using ICCID for device id to new create Device.\n");
            strcpy(device_id,  mySystem.iccid.c_str());
            }
        }
    else if (!have_deviceid){
        printf("ERROR: You must provide device id for primary api key!\n");
        gpio_deinit( &user_key);
        binario_io_close();
        binary_io_init();
        return 0;
        }

    printf("-Validating API Key (%s) and Device ID (%s)...\n",api_key,device_id);
    m2x_device_info(api_key, device_id,  resp);
    strptr = strstr(resp,"\"name\":\"Global Starter Kit\",");
    if (strptr) {
        printf("device already present.\n");
        }
    else{
        printf("Create a new device.\n");
        if( !using_masterkey ) {
            printf("ERROR: must use MASTER KEY to create device!\n");
            gpio_deinit( &user_key);
            binario_io_close();
            binary_io_init();
            return 0;
            }

        m2x_create_device(api_key, device_id, resp);
        i = parse_maljson(resp, om, sizeof(om));
        strcpy(device_id, om[11].value);
        printf("Now using Device ID: %s\n-Creating the data streams...\n",device_id);
        m2x_create_stream(device_id, api_key, "ADC");
        m2x_create_stream(device_id, api_key, "TEMP");
        m2x_create_stream(device_id, api_key, "XVALUE");
        m2x_create_stream(device_id, api_key, "YVALUE");
        m2x_create_stream(device_id, api_key, "ZVALUE");
        }

    sprintf(qsa_url, "https://m2x.att.com/devices/%s", device_id);
    printf("Using API Key = %s, Device Key = %s\n",api_key, device_id);
    printf("This application will post XYZ data, Temperature data, Light ADC data, and GPS location to a standard M2X\n");
    printf("account.  To access the data, browse to this URL:\n");
    printf("\n");
    printf("       %s\n", qsa_url);
    printf("\n");
    printf("These streams are updated each time the user presses the USER button. To exit the program, press and hold\n");
    printf("the User Button for > 3 seconds.\n\n");
    printf("LED colors will display a different colors for waiting, button press, and sensor data being sent to M2X.\n");
    printf("\n");
    i=1;
    while( !done ) {
        do_color(current_color="GREEN");
        WAIT_FOR_BPRESS(bpress);
        if( keypress_time.tv_sec > 3 ) {
            done = 1;
            continue;
            }
        do_color(current_color="BLUE");
    
        gettimeofday(&start, NULL);
        adc_init(&my_adc);
        adc_read(my_adc, &adc_voltage);
        adc_deinit(&my_adc);
        memset(str_val, 0, sizeof(str_val));
        sprintf(str_val, "%f", adc_voltage);

        printf("%2d. Sending ADC value",i++);
        fflush(stdout);
        m2x_update_stream_value(device_id, api_key, "ADC", str_val);		
    
        printf(", TEMP value");
        fflush(stdout);
        sprintf(str_val, "%f", lis2dw12_readTemp12());
        m2x_update_stream_value(device_id, api_key, "TEMP", str_val);		

        ptr=lis2dw12_m2x();

        printf(", XYZ values...");
        fflush(stdout);
        m2x_update_stream_value(device_id, api_key, "XVALUE", ptr[0]);		
        m2x_update_stream_value(device_id, api_key, "YVALUE", ptr[1]);		
        m2x_update_stream_value(device_id, api_key, "ZVALUE", ptr[2]);		

        printf(" now get GPS Coordinates (this may take up to %d sec).\n",GPS_TO);
        gettimeofday(&gps_start, NULL);
        pthread_create( &thread1, NULL, check_gps, NULL);
        pthread_join( thread1, NULL); //wait here for the GSP to finish

        if( lat == 0.0 && lng == 0.0 && alt == 0 ) {
            printf(" ** Didn't get GPS info, using last know values ** ");
            lat = last_lat;
            lng = last_lng;
            alt = last_alt;
            }

        sprintf(str_lat, "%f", lat);
        sprintf(str_long, "%f", lng);
        sprintf(str_elev, "%f", alt);

        m2x_update_location_value ( device_id, api_key, device_id, str_lat, str_long, str_elev);

        printf("All Values sent.\r");
        fflush(stdout);

        gettimeofday(&end, NULL);
        elapse = (end.tv_sec - start.tv_sec)*1000 + (end.tv_usec/1000 - start.tv_usec/1000);
        dly = ((delay_time*1000)-round(elapse))/1000;
        if( dly > 0) {
            printf(" (delay %d seconds)\n",dly);
            sleep(dly);
            }
        }
    printf("\n\nExiting Quick Start Application.\n");

    gpio_deinit( &user_key);
    binario_io_close();
    binary_io_init();
    return 1;
}

