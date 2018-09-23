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

    @file          facttest.cpp
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
#include "MAX31855.hpp"

#include "mal.hpp"

#ifdef __cplusplus
extern "C" {
#endif
void set_color(char *);
void wwan_io(int);
#ifdef __cplusplus
}
#endif

float lat;
float lng;
int   alt;
int   rst_m2_detected = 0;
static int   idx = 0;
struct timeval gps_start, gps_end;  //measure duration of gps call...

int GPS_TO = 120;  //default is 120 seconds to get GPS fix
extern int emission_test;

void *check_gps(void *ptr)
{
    json_keyval om[12];
    double elapse=0;
    int k, done, i, m;

    setGPSmode(4);
    getGPSconfig(om,sizeof(om));
    enableGPS();
    resetGPS();
    m=done = 0;
    while( !done ) {
        k=getGPSlocation(om,sizeof(om));
        done = atoi(om[3].value)?1:0;
        for( i=1; i<k; i++ ) {
            if( !strcmp(om[i].key,"latitude") ) {
                sscanf( om[i].value, "%f", &lat);
                }
            else if( !strcmp(om[i].key,"longitude") ) {
                sscanf( om[i].value, "%f", &lng);
                }
            else if( !strcmp(om[i].key,"altitude") ) {
                sscanf( om[i].value, "%d", &alt);
                }
            else if( !strcmp(om[i].key,"errno") ) {
                if( atoi(om[i].value) )
                    printf("GPS ERROR! %d\n",atoi(om[i].value));
                }
            else if( !strcmp(om[i].key,"errmsg") ) {
                if( strcmp(om[i].value,"<null>") )
                    printf("GPS ERROR MESSAGE: %s\n",om[i].value);
                }
            }
        gettimeofday(&gps_end, NULL);
        elapse = (((gps_end.tv_sec - gps_start.tv_sec)*1000) + (gps_end.tv_usec/1000 - gps_start.tv_usec/1000));
        if( ((GPS_TO*1000)-round(elapse))/1000 < 0) {
            done = 1;
            if( emission_test )
                printf("Note: Satellite acquisition timeout is disabled to facilitate GPS testing\n");
            else
                printf("\rGPS Acquisiton TO (%d seconds)\n",(int)round(elapse)/1000);
            }
        else {
//            printf("\r%c",(m==0)?0x7C:(m==1)?0x2f:(m==2)?0x2d:0x5c);
            printf("\r%c",(m==0)?'O':(m==1)?'o':(m==2)?'.':' ');
            fflush(stdout);
            m++;
            m %= 4;
            usleep(250000);
            }
        }
    if( !emission_test )
        disableGPS();
    printf("\r");
    return NULL;
}


volatile int  user_press;
extern gpio_handle_t boot_key, user_key, red_led, green_led, blue_led;

#define WAIT_FOR_BPRESS(x) {while( !x ); while( x );}

int gpio_ftirq_callback(gpio_pin_t pin_name, gpio_irq_trig_t direction)
{
    int *key;
    if (pin_name == GPIO_PIN_98) 
        key= (int*)&user_press;
    else
        return 0;

    *key = ! *key;
    return 0;
}

//
// This callback is only used for the extended I/O test
//
int gpio_rst_m2_callback(gpio_pin_t pin_name, gpio_irq_trig_t direction)
{
    int *key;
    if (pin_name == GPIO_PIN_95) 
        rst_m2_detected = 1;
    return 0;
}


int command_facttest(int argc, const char *const *argv)
{
    extern GPIOPIN_IN gpio_input;
    MAX31855 max31855;
    json_keyval om[20];
    int doGPS, i;
    float val;
    pthread_t thread1;
    struct timeval start, end;          //measure duration of flow calls...
    double elapse=0;

    gpio_level_t r1=(gpio_level_t)0, r2=(gpio_level_t)0;
    gpio_handle_t rst_m1_in=0, pwm_m1_in=0, pwm_m2_in=0;
    gpio_handle_t cs_m1_out=0, int_m2_out=0, int_m1_out=0, rst_m2_out=0;


    doGPS = doM2X;

//
// if ft_mopde == 0, we have been called from within the monitor program
// so switch context to factory test mdoe and restore it when leaving
//
    if( !ft_mode ) {
        if( argc == 2 ) {
            ft_time = atoi(argv[1]);
            printf("-set factory test time to %d\n",ft_time);
            }
        binario_io_close();
        ft_mode = 2;
        }

    binary_io_init();
    gpio_deinit( &gpio_input.hndl);

    i=start_data_service();
    while ( i < 0 ) {
        printf("WAIT: starting WNC MAL Interface (%d)\n",i);
        sleep(10);
        i=start_data_service();
        }

    if( (i=gpio_init( GPIO_PIN_98,  &user_key)) != 0 )
        printf("ERROR: unable to initialize user key gpio. (%d)\n",i);
    if( (i=gpio_dir(user_key, GPIO_DIR_INPUT)) != 0 )
        printf("ERROR: can't set user key as input. (%d)\n",i);
    if( (i=gpio_irq_request(user_key, GPIO_IRQ_TRIG_BOTH, gpio_ftirq_callback)) != 0)
        printf("ERROR: can't set user key as interrupt input. (%d)\n",i);

    user_press = 0;
    printf("\n---- BTN Test 1 ------------------------------\n");
    printf("BTN: Press User key");
    fflush(stdout);
    WAIT_FOR_BPRESS(user_press);
    printf("= PASS\n");
    printf("\n---- WNC Test 2 ------------------------------\n");
    fflush(stdout);
    do
        mySystem.model=getModelID(om, sizeof(om));
    while( mySystem.model == "service is not ready");
    mySystem.firmVer=getFirmwareVersion(om, sizeof(om));
    mySystem.appsVer=getAppsVersion(om, sizeof(om));
    mySystem.malwarVer=getMALManVer(om, sizeof(om));
    mySystem.ip=get_ipAddr(om, sizeof(om));
    mySystem.iccid=getICCID(om, sizeof(om));
    mySystem.imei=getIMEI(om, sizeof(om));
    mySystem.imsi=getIMSI(om, sizeof(om));

    printf("WNC: Module   #    = %s\n", mySystem.model.c_str());
    printf("WNC: Apps Ver #    = %s\n", mySystem.appsVer.c_str());
    printf("WNC: Firmware #    = %s\n", mySystem.firmVer.c_str());
    printf("WNC: MAL Ver. #    = %s\n", mySystem.malwarVer.c_str());
    printf("WNC: IP Addr. #    = %s\n", mySystem.ip.c_str());
    printf("SIM: ICCID    #    = %s\n", mySystem.iccid.c_str());
    printf("SIM: IMEA     #    = %s\n",mySystem.imei.c_str());
    printf("SIM: IMSI     #    = %s\n",mySystem.imsi.c_str());

    get_wwan_status(om, sizeof(om));
    printf("\n---- WNC Test 3 ------------------------------\n");
    printf("LTE: Signal RSSI   = %s\n", om[4].value);
    printf("LTE: Signal Level  = %s\n", om[6].value);

    printf("\n---- ADC Test 4 ------------------------------\n");
    adc_handle_t my_adc=(adc_handle_t)NULL;

    adc_init(&my_adc);
    adc_read(my_adc, &val);
    printf("ADC: Light Sensor  = %f\n",val);
    adc_deinit(&my_adc);

    printf("\n---- I2C Test 5 ------------------------------\n");
    printf("I2C: LIS2DW12 ID   = 0x%02X (expect 0x44)\n", lis2dw12_getDeviceID());
    printf("I2C: PMOD/HTS ID   = ");
    HTS221 *hts221 = new HTS221;
    if( (i=hts221->getDeviceID()) == 0xff )
        printf("FAIL\n");
    else
        printf("0x%02X (expect 0xBC)\n",i);

    printf("\n---- SPI Test 6 ------------------------------\n");
    printf("SPI: PMOD Result   = %s\n", max31855.loopbackTest()?"PASS":"FAIL");

// start the GPS interface.  It takes a while and uses the MAL interface
// so don't start it until we are done accessing the MAL for other info

    if( doGPS ) {
        pthread_create( &thread1, NULL, check_gps, NULL);
        gettimeofday(&gps_start, NULL);
        }

    printf("\n---- LED Test 7 ------------------------------\n");
    wwan_io(1);
    gpio_init( GPIO_PIN_92,  &red_led );
    gpio_init( GPIO_PIN_101, &green_led );
    gpio_init( GPIO_PIN_102, &blue_led );

    gpio_dir(red_led,   GPIO_DIR_OUTPUT);
    gpio_dir(green_led, GPIO_DIR_OUTPUT);
    gpio_dir(blue_led,  GPIO_DIR_OUTPUT);

    printf("LED: PWR           = Green\n");
    printf("LED: WWAN          = Amber\n");
    printf("LED: USER Blnk     = R-G-B\n");
    printf("LED: CHECK LEDs\n");
    do_gpio_blink( 0, 1 );
    gettimeofday(&start, NULL);

    if( doGPS ) {
        printf("\n---- GPS Test 8 -- May take up to %4d seconds-\n", GPS_TO);
        pthread_join( thread1, NULL); //wait here for the GSP to finish

        printf("GPS: Latitude      = %8.5f\n",lat);
        printf("GPS: Longitude     = %8.5f\n",lng);
        }
    else
        printf("\n---- Skip GPS Test 8 ------------------------------\n");

    gettimeofday(&end, NULL);
    elapse += (((end.tv_sec - start.tv_sec)*1000) + (end.tv_usec/1000 - start.tv_usec/1000));
    i = ((ft_time*1000)-round(elapse))/1000;

    if( i>0 ) {
        printf("\nDelay %d seconds, check LEDs\n",i);
        fflush(stdout);
        sleep(i);
        }

    if( extendedIO ) {
        binario_io_close();

        printf("\n---- XTEND IO Test 9 ------------------------------\n");

        //
        // Extended Binary I/O configuration is as follows:
        // GPIO02 / rst_m1_in - Input
        // GPIO03 / cs_m1_out - Output
        // GPIO04 / pwm_m1_in - Input
        // GPIO94 / int_m1_out- Output
        //
        // GPIO95 / rst_m2_out- Output
        // GPIO96 / pwm_m2_in - Input
        // GPIO07 / int_m2_out- output 

        if( (i=gpio_init(GPIO_PIN_3, &cs_m1_out )) != 0 )
            printf("ERROR: unable to initialize GPIO_PIN_3. (%d)\n",i);
        if( (i=gpio_init(GPIO_PIN_7, &int_m2_out)) != 0 )
            printf("ERROR: unable to initialize GPIO_PIN_7. (%d)\n",i);
        if( (i=gpio_init(GPIO_PIN_95, &rst_m2_out)) != 0 )
            printf("ERROR: unable to initialize GPIO_PIN_95. (%d)\n",i);
        if( (i=gpio_init(GPIO_PIN_94, &int_m1_out)) != 0 )
            printf("ERROR: unable to initialize GPIO_PIN_94. (%d)\n",i);

        if( (i=gpio_dir(cs_m1_out, GPIO_DIR_OUTPUT)) != 0 )
            printf("ERROR: can't set direction on cs_m1_out. (%d)\n",i);
        if( (i=gpio_dir(int_m2_out, GPIO_DIR_OUTPUT)) != 0 )
            printf("ERROR: can't set direction on int_m2_out. (%d)\n",i);
        if( (i=gpio_dir(int_m1_out, GPIO_DIR_OUTPUT)) != 0 )
            printf("ERROR: can't set direction on int_m1_out. (%d)\n",i);
        if( (i=gpio_dir(rst_m2_out, GPIO_DIR_INPUT)) != 0 )
            printf("ERROR: can't set direction on pwm_m2_in. (%d)\n",i);

        if( (i=gpio_init(GPIO_PIN_2, &rst_m1_in)) != 0 )
            printf("ERROR: unable to initialize GPIO_PIN_2. (%d)\n",i);
        if( (i=gpio_init(GPIO_PIN_4, &pwm_m1_in)) != 0 )
            printf("ERROR: unable to initialize GPIO_PIN_4. (%d)\n",i);
        if( (i=gpio_init(GPIO_PIN_96, &pwm_m2_in)) != 0 )
            printf("ERROR: unable to initialize GPIO_PIN_96. (%d)\n",i);

        if( (i=gpio_dir(rst_m1_in, GPIO_DIR_INPUT)) != 0 )
            printf("ERROR: can't set direction on rst_m1_in. (%d)\n",i);
        if( (i=gpio_dir(pwm_m1_in, GPIO_DIR_INPUT)) != 0 )
            printf("ERROR: can't set direction on pwm_m1_in. (%d)\n",i);
        if( (i=gpio_dir(pwm_m2_in, GPIO_DIR_INPUT)) != 0 )
            printf("ERROR: can't set direction on pwm_m2_in. (%d)\n",i);

        if( (i=gpio_irq_request(rst_m2_out, GPIO_IRQ_TRIG_BOTH, gpio_rst_m2_callback)) != 0)
            printf("ERROR: can't set user key as interrupt input. (%d)\n",i);
        rst_m2_detected=0;
        max31855.loopbackTest();  //SPIM_EN_1 is set when an SPI transaction occurs so do the loopback test

        // check Socket #1
        // cs_m1_out -> rst_m1_in 
        // int_m1_out -> pwm_m1_in

        // check Socket #2
        // rst_m2_out -> cs_m2_in (no connect)
        // int_m2_out -> pwm_m2_in

        gpio_write(cs_m1_out, GPIO_LEVEL_HIGH );
        gpio_read(rst_m1_in,  &r1);
        gpio_write(cs_m1_out, GPIO_LEVEL_LOW );
        gpio_read(rst_m1_in,  &r2);
        printf("XIO: cs_m1 ->rst_m1= %s\n", (r1^r2)?"OK":"FAIL");

        gpio_write(int_m1_out, GPIO_LEVEL_HIGH );
        gpio_read(pwm_m1_in,  &r1);
        gpio_write(int_m1_out, GPIO_LEVEL_LOW );
        gpio_read(pwm_m1_in,  &r2);
        printf("XIO: int_m1->pwm_m1= %s\n", (r1^r2)?"OK":"FAIL");

        printf("XIO: cs_m2 ->rst_m2= %s\n", (rst_m2_detected)?"OK":"FAIL");
       
        gpio_write(int_m2_out, GPIO_LEVEL_HIGH );
        gpio_read(pwm_m2_in,  &r1);
        gpio_write(int_m2_out, GPIO_LEVEL_LOW );
        gpio_read(pwm_m2_in,  &r2);
        printf("XIO: int_m2->pwm_m2= %s\n", (r1^r2)?"OK":"FAIL");

        gpio_irq_free(rst_m2_out);
        gpio_deinit(&rst_m2_out);
        gpio_deinit(&cs_m1_out );
        gpio_deinit(&int_m2_out);
        gpio_deinit(&int_m1_out);
        gpio_deinit(&rst_m1_in);
        gpio_deinit(&pwm_m1_in);
        gpio_deinit(&pwm_m2_in);
        }

    printf("\n\n");
    do_gpio_blink( 0, 0 );
    gpio_deinit( &user_key);
    binario_io_close();
    if( ft_mode == 2 ) 
        ft_mode = 0;  //restore factory test mode to what it was origionally
    binary_io_init();
}


