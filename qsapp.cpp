
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
extern int button_press;
extern GPIOPIN_IN gpio_input;
extern gpio_handle_t user_key;

int user_key_callback(gpio_pin_t pin_name, gpio_irq_trig_t direction)
{
    if (pin_name != GPIO_PIN_98) 
        return 0;

    if( button_press = !button_press ) 
        clock_gettime(CLOCK_MONOTONIC, &key_press);
    else{
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
    void  wwan_io(int);
    void  do_lis2dw2m2x(void);
    float lis2dw12_readTemp12(void);

    adc_handle_t my_adc=(adc_handle_t)NULL;
    int          start_data_service(void);
    int          done=0, k=0, i;
    int          dly, delay_time;
    char         cmd[1024], resp[1024], qsa_url[500];
    char         color[10];
    char         **ptr, **lis2dw12_m2x(void);
    char         str_val[16];
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

//
// argv contains the M2X base URL to use for posting data. This
// must be provided to fhe application to work.
//
    if( argv == NULL ) {
        printf("ERROR: Must supply a M2X URL\n");
        return 0;
        }

    mySystem.model=getModelID(om, sizeof(om));
    mySystem.iccid=getICCID(om, sizeof(om));

    sprintf(qsa_url, "%s/%s/streams/", (char*)argv, mySystem.iccid.c_str());

    printf("\n\nQuick Start Application:\n");
    printf("This application will post XYZ data, Temperature data, and Light ADC data to a standard M2X\n");
    printf("account.  To access the data, browse to this URL:\n");
    printf("\n");
    printf("       %s/<stream_id>\n", qsa_url);
    printf("\n");
    printf("From this account there are three streams of data being updated:\n");
    printf("    %sXYZ/value\n", qsa_url);
    printf("    %sTEMP/value\n", qsa_url);
    printf("    %sADC/value\n", qsa_url);
    printf("\n");
    printf("These streams are updated continuously user a Delay between data postings.\n");
    printf("\n");
    printf("Intially the LED will display RED while a connection is being established\n");
    printf("and then will turn GREEN.  There after, the LED will cycle through colors\n");
    printf("after each set of sensor data is sent to M2X.\n");
    printf("\n");
    printf("To exit the Quick Start Applicatioin, press the User Button on the Global \n");
    printf("LTE IoT Starter Kit for > 3 seconds.\n\n");

    gpio_deinit( &gpio_input.hndl);
    button_press = 0;
    gpio_init( GPIO_PIN_98,  &user_key );  //SW3
    gpio_dir(user_key, GPIO_DIR_INPUT);
    gpio_irq_request(user_key, GPIO_IRQ_TRIG_BOTH, user_key_callback);

    get_wwan_status(om, sizeof(om));
    wwan_io(!strcmp(om[7].value,"1")?1:0);

    while( !done ) {
        gettimeofday(&start, NULL);
        adc_init(&my_adc);
        adc_read(my_adc, &adc_voltage);
        adc_deinit(&my_adc);
        memset(str_val, 0, sizeof(str_val));
        sprintf(str_val, "%f", adc_voltage);

        printf(">Send ADC value.\n");
        m2x_create_stream(device_id, api_key, "ADC");
        m2x_update_stream_value(device_id, api_key, "ADC", str_val);		
    
        printf(">Send TEMP value.\n");
        sprintf(str_val, "%f", lis2dw12_readTemp12());
        m2x_create_stream(device_id, api_key, "TEMP");
        m2x_update_stream_value(device_id, api_key, "TEMP", str_val);		

        ptr=lis2dw12_m2x();
        m2x_create_stream(device_id, api_key, "XVALUE");
        m2x_create_stream(device_id, api_key, "YVALUE");
        m2x_create_stream(device_id, api_key, "ZVALUE");

        printf(">Send XYZ values.\n");
        m2x_update_stream_value(device_id, api_key, "XVALUE", ptr[0]);		
        m2x_update_stream_value(device_id, api_key, "YVALUE", ptr[1]);		
        m2x_update_stream_value(device_id, api_key, "ZVALUE", ptr[2]);		

        printf(">Stream Values all sent.\n");
        if( keypress_time.tv_sec > 3 ) 
            done = 1;
        else {
            gettimeofday(&end, NULL);
            elapse = (end.tv_sec - start.tv_sec)*1000 + (end.tv_usec/1000 - start.tv_usec/1000);

            dly = ((delay_time*1000)-round(elapse))/1000;
            if( dly > 0) {
                printf("==>Pause %d seconds for delay\n",dly);
                sleep(dly);
                }
            }
        }
    printf("Exiting Quick Start Application.\n");

    gpio_deinit( &user_key);
    binario_io_close();
    binary_io_init();
}

