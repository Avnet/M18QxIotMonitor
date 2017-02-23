
#include <stdint.h>
#include <string.h>

#include <nettle/nettle-stdint.h>

#include "iot_monitor.h"
#include "http.h"
#include "m2x.h"
#include "hts221.h"
#include "mytimer.h"

#include "mal.hpp"

size_t m2x_sensor_timer;
int hts221_iterations;
int lis2dw12_iterations;

void sensor_hts221(int interval, int iterations);
void sensor_lis2dw12(int interval, int iterations);

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
    const time_t t = time(0);
    if( hts221_iterations ) {
        printf("\n-Read HTS221 sensor and send data %d more times (%s)\n",hts221_iterations, asctime(localtime(&t)));
        do_hts2m2x();
        --hts221_iterations;
        if( !hts221_iterations )
            sensor_hts221(0,0);
        }
    if( lis2dw12_iterations ) {
        printf("\n-Read LSW12D2 sensor and send data %d more times (%s)\n",lis2dw12_iterations, asctime(localtime(&t)));
        do_adc2m2x();
        --lis2dw12_iterations;
        if( !lis2dw12_iterations )
            sensor_lis2dw12(0,0);
        }
}
 
 
void do_hts2m2x(void)
{
    float adc_voltage;
    char str_adc_voltage[16];
    
    printf("-Tx HTS221 M2X data to:\n-DeviceID = [%s]\n-API Key=[%s]\nStream Name=[%s]\n",
            device_id, api_key, temp_stream_name);

    start_data_service();
    m2x_create_stream(device_id, api_key, "humid");

    adc_voltage = hts221_getTemp();
    memset(str_adc_voltage, 0, sizeof(str_adc_voltage));
    sprintf(str_adc_voltage, "%f", adc_voltage);
    m2x_update_stream_value(device_id, api_key, temp_stream_name, str_adc_voltage);		


    printf("-Tx HTS221 M2X data to:\n-DeviceID = [%s]\n-API Key=[%s]\nStream Name=[%s]\n",
            device_id, api_key, "humid");
    adc_voltage = hts221_getHumid()/10;
    memset(str_adc_voltage, 0, sizeof(str_adc_voltage));
    sprintf(str_adc_voltage, "%f", adc_voltage);
    m2x_update_stream_value(device_id, api_key, "humid", str_adc_voltage);		

}

void sensor_hts221(int interval, int iterations)
{
printf("-sending data %d times, every %d seconds.\n",iterations, interval);

    if( m2x_sensor_timer != 0 ) {  //currently have a timer running
        if( interval ) { //just want to change the rate of samples
            hts221_iterations = iterations;
            delete_IoTtimer(m2x_sensor_timer);
            m2x_sensor_timer = create_IoTtimer(interval, tx2m2x_timer, TIMER_PERIODIC, NULL);
printf("-changed timer\n");
            }
         else {  //want to kill the timer
            hts221_iterations = 0;
            delete_IoTtimer(m2x_sensor_timer);
            stop_IoTtimers();
            m2x_sensor_timer = 0;
printf("-stopped timer\n");
            }
        }
    else { //don't have a timer currently running, start one up
        hts221_iterations = iterations;
        start_IoTtimers();
        m2x_sensor_timer = create_IoTtimer(interval, tx2m2x_timer, TIMER_PERIODIC, NULL);
printf("-started timer\n");
        }
}


void sensor_lis2dw12(int interval, int iterations)
{
printf("-sending ADC data %d times, every %d seconds.\n",iterations, interval);

    if( m2x_sensor_timer != 0 ) {  //currently have a timer running
        if( interval ) { //just want to change the rate of samples
            lis2dw12_iterations = iterations;
            delete_IoTtimer(m2x_sensor_timer);
            m2x_sensor_timer = create_IoTtimer(interval, tx2m2x_timer, TIMER_PERIODIC, NULL);
printf("-changed timer\n");
            }
         else {  //want to kill the timer
            lis2dw12_iterations = 0;
            delete_IoTtimer(m2x_sensor_timer);
            stop_IoTtimers();
            m2x_sensor_timer = 0;
printf("-stopped timer\n");
            }
        }
    else { //don't have a timer currently running, start one up
        lis2dw12_iterations = iterations;
        start_IoTtimers();
        m2x_sensor_timer = create_IoTtimer(interval, tx2m2x_timer, TIMER_PERIODIC, NULL);
printf("-started timer\n");
        }
}

void do_adc2m2x(void)
{
    adc_handle_t my_adc=(adc_handle_t)NULL;
    float adc_voltage;
    char str_adc_voltage[16];
    
    printf("-Tx M2X data to:\n-DeviceID = [%s]\n-API Key=[%s]\nStream Name=[%s]\n",
            device_id, api_key, adc_stream_name);

    start_data_service();
    m2x_create_stream(device_id, api_key, adc_stream_name);

    adc_init(&my_adc);
    adc_read(my_adc, &adc_voltage);

    memset(str_adc_voltage, 0, sizeof(str_adc_voltage));
    sprintf(str_adc_voltage, "%f", adc_voltage);
    m2x_update_stream_value(device_id, api_key, adc_stream_name, str_adc_voltage);		

    adc_deinit(&my_adc);
    printf("\n");
}

