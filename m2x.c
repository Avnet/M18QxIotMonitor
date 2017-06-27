
#include <stdint.h>
#include <string.h>

#include <nettle/nettle-stdint.h>

#include "iot_monitor.h"
#include "http.h"
#include "m2x.h"
#include "hts221.h"
#include "mytimer.h"
#include "lis2dw12.h"

#include "mal.hpp"

size_t m2x_sensor_timer;
int hts221_iterations;
int lis2dw12_iterations;

void sensor_hts221(int interval, int iterations);
void sensor_lis2dw12(int interval, int iterations);
void do_lis2dw_temp(int f, void *val);
void do_lis2dw2m2x(void);

M2XFUNC m2xfunctions[] = {
//  name,     rate,     func(), *tmr
    "HTS221",    0, sensor_hts221,  NULL,
    "LIS2DW12",  0, sensor_lis2dw12, NULL,
    "ENDTAB",    0,       NULL, NULL,
};
#define _MAX_M2XFUNCTIONS	(sizeof(m2xfunctions)/sizeof(M2XFUNC))

const int _max_m2xfunctions = _MAX_M2XFUNCTIONS;

void tx2m2x_timer(size_t timer_id, void * user_data)
{
    float adc_voltage;;
    char str_adc_voltage[16];
    const time_t t = time(0);

    if( hts221_iterations ) {
        if( dbg_flag & DBG_MYTIMER )
            printf("\n-MYTIMER: Read HTS221 sensor and send data %d more times (%s)\n",
                     hts221_iterations, asctime(localtime(&t)));
        do_hts2m2x();
        --hts221_iterations;
        if( !hts221_iterations )
            sensor_hts221(0,0);  //cancel the calls, we are done.
        }
    if( lis2dw12_iterations ) {
        if( dbg_flag & DBG_MYTIMER )
            printf("\n-MYTIMER: Read LSW12D2 sensor and send data %d more times (%s)\n",
                     lis2dw12_iterations, asctime(localtime(&t)));
        do_lis2dw2m2x();
        --lis2dw12_iterations;
        if( !lis2dw12_iterations )
            sensor_lis2dw12(0,0);  //cancel the calls, we are done.
        }
}

void do_hts2m2x(void)
{
    float adc_voltage;
    char str_adc_voltage[16];
    
    if( dbg_flag & DBG_M2X )
        printf("-M2X: Tx HTS221 M2X data to:\n-DeviceID = [%s]\n-API Key=[%s]\n-Stream Name=[%s]\n", 
                device_id, api_key, temp_stream_name);

    m2x_create_stream(device_id, api_key, "humid");

    adc_voltage = hts221_getTemp();
    memset(str_adc_voltage, 0, sizeof(str_adc_voltage));
    sprintf(str_adc_voltage, "%f", adc_voltage);
    m2x_update_stream_value(device_id, api_key, temp_stream_name, str_adc_voltage);		


    if( dbg_flag & DBG_M2X )
        printf("-M2X: Tx HTS221 M2X data to:\n-DeviceID = [%s]\n-API Key=[%s]\n-Stream Name=[%s]\n",
                device_id, api_key, "humid");
    adc_voltage = hts221_getHumid()/10;
    memset(str_adc_voltage, 0, sizeof(str_adc_voltage));
    sprintf(str_adc_voltage, "%f", adc_voltage);
    m2x_update_stream_value(device_id, api_key, "humid", str_adc_voltage);		

}


void sensor_hts221(int interval, int iterations)
{
        if( dbg_flag & DBG_HTS221 )
            printf("-HTS221: sending data %d times, every %d seconds.\n",iterations, interval);

    if( m2x_sensor_timer != 0 ) {  //currently have a timer running
        if( interval ) { //just want to change the rate of samples
            hts221_iterations = iterations;
            delete_IoTtimer(m2x_sensor_timer);
            m2x_sensor_timer = create_IoTtimer(interval, tx2m2x_timer, TIMER_PERIODIC, NULL);
        if( dbg_flag & DBG_HTS221 )
            printf("-HTS221: changed timer\n");
            }
         else {  //want to kill the timer
            hts221_iterations = 0;
            delete_IoTtimer(m2x_sensor_timer);
            stop_IoTtimers();
            m2x_sensor_timer = 0;
        if( dbg_flag & DBG_HTS221 )
            printf("-HTS221: stopped timer\n");
            }
        }
    else { //don't have a timer currently running, start one up
        hts221_iterations = iterations;
        start_IoTtimers();
        m2x_sensor_timer = create_IoTtimer(interval, tx2m2x_timer, TIMER_PERIODIC, NULL);
        if( dbg_flag & DBG_HTS221 )
            printf("-HTS221: started timer\n");
        start_data_service();
        }
}


void sensor_lis2dw12(int interval, int iterations)
{
    if( dbg_flag & DBG_LIS2DW12 )
        printf("-LIS2DW12: sending ADC data %d times, every %d seconds.\n",iterations, interval);

    if( m2x_sensor_timer != 0 ) {  //currently have a timer running
        if( interval ) { //just want to change the rate of samples
            lis2dw12_iterations = iterations;
            delete_IoTtimer(m2x_sensor_timer);
            m2x_sensor_timer = create_IoTtimer(interval, tx2m2x_timer, TIMER_PERIODIC, NULL);
        if( dbg_flag & DBG_LIS2DW12 )
            printf("-LIS2DW12: changed timer\n");
            }
         else {  //want to kill the timer
            lis2dw12_iterations = 0;
            delete_IoTtimer(m2x_sensor_timer);
            stop_IoTtimers();
            m2x_sensor_timer = 0;
        if( dbg_flag & DBG_LIS2DW12 )
            printf("-LIS2DW12: stopped timer\n");
            }
        }
    else { //don't have a timer currently running, start one up
        lis2dw12_iterations = iterations;
        start_IoTtimers();
        m2x_sensor_timer = create_IoTtimer(interval, tx2m2x_timer, TIMER_PERIODIC, NULL);
        if( dbg_flag & DBG_LIS2DW12 )
            printf("-LIS2DW12: started timer\n");
        }
}

void do_adc2m2x(void)
{
    adc_handle_t my_adc=(adc_handle_t)NULL;
    float adc_voltage;
    char str_adc_voltage[16];
    
        if( dbg_flag & DBG_M2X )
                printf("-M2x: Tx (adc) M2X data to:\n-DeviceID = [%s]\n-API Key=[%s]\n-Stream Name=[%s]\n",
            device_id, api_key, adc_stream_name);

    m2x_create_stream(device_id, api_key, adc_stream_name);

    adc_init(&my_adc);
    adc_read(my_adc, &adc_voltage);

    memset(str_adc_voltage, 0, sizeof(str_adc_voltage));
    sprintf(str_adc_voltage, "%f", adc_voltage);
    m2x_update_stream_value(device_id, api_key, adc_stream_name, str_adc_voltage);		

    adc_deinit(&my_adc);
    printf("\n");
}

//
// curl -i -X PUT http://api-m2x.att.com/v2/devices/2ac9dc89132469eb809bea6e3a95d675/streaX-KEY: 6cd9c60f4a4e5d91d0ec4cc79536c661" 
//    -H "Content-Type: application/json" -d "{ \"value\": \"RED\" }"
//
void set_m2xColor(char *color)
{
    
    if( dbg_flag & DBG_M2X )
        printf("-M2x: Tx M2X (color) data to:\n-DeviceID = [%s]\n-API Key=[%s]\n-Stream Name=[%s]\n",
                device_id, api_key, "rgb");

    m2x_create_stream(device_id, api_key, "rgb");

    m2x_update_color_value (device_id, api_key, "rgb", color);		

    printf("\n");
}

void do_lis2dw2m2x(void)
{
    float adc_voltage = lis2dw12_readTemp12();
    do_lis2dw_temp(1,(void*)&adc_voltage);
}

void do_lis2dw_temp(int f, void *val)
{
    adc_handle_t my_adc=(adc_handle_t)NULL;
    float adc_voltage;
    char str_val[16];
    char * float_stream_name = "LIS2DW12_TEMP";
    char * int_stream_name = "LIS2DW8";
    
    memset(str_val, 0, sizeof(str_val));

    if( f ) {                 //is the 12 bit floating value
        if( dbg_flag & DBG_M2X ) 
            printf("-M2x: Tx (lis2dw temp12)M2X data to:\n-DeviceID = [%s]\n-API Key=[%s]\n-Stream Name=[%s]\n",
                    device_id, api_key, float_stream_name);
        m2x_create_stream(device_id, api_key, float_stream_name);
        sprintf(str_val, "%f", *((float*)val));
        m2x_update_stream_value(device_id, api_key, float_stream_name, str_val);		
        }
    else {
        if( dbg_flag & DBG_M2X ) 
            printf("-M2x: (lis2dw temp8)Tx M2X data to:\n-DeviceID = [%s]\n-API Key=[%s]\n-Stream Name=[%s]\n",
                    device_id, api_key, int_stream_name);
        m2x_create_stream(device_id, api_key, int_stream_name);
        sprintf(str_val, "%d", *((int*)val));
        m2x_update_stream_value(device_id, api_key, int_stream_name, str_val);		
        }

    printf("\n");
}

void do_lis2dw_xyz(char **ptr)
{
    char * Xstream_name = "LIS2DW12_x";
    char * Ystream_name = "LIS2DW12_y";
    char * Zstream_name = "LIS2DW12_z";

    if( dbg_flag & DBG_M2X ) {
        printf("-M2x: (xyz) Tx M2X data to:\n-DeviceID = [%s]\n-API Key=[%s]\n-Stream Name=[%s]\n",
                device_id, api_key, Xstream_name);
        printf("-M2x: (xyz) Tx M2X data to:\n-DeviceID = [%s]\n-API Key=[%s]\n-Stream Name=[%s]\n",
                device_id, api_key, Ystream_name);
        printf("-M2x: (xyz) Tx M2X data to:\n-DeviceID = [%s]\n-API Key=[%s]\n-Stream Name=[%s]\n",
                device_id, api_key, Zstream_name);
        }

    m2x_create_stream(device_id, api_key, Xstream_name);
    m2x_create_stream(device_id, api_key, Ystream_name);
    m2x_create_stream(device_id, api_key, Zstream_name);

    m2x_update_stream_value(device_id, api_key, Xstream_name, ptr[0]);		
    m2x_update_stream_value(device_id, api_key, Ystream_name, ptr[1]);		
    m2x_update_stream_value(device_id, api_key, Zstream_name, ptr[2]);		

    printf("\n");
}


