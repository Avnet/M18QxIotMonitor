
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
int m2x_iterations;

void sensor_hts221(int interval, int iterations);

M2XFUNC m2xfunctions[] = {
//  name,     rate,     func(), *tmr
    "HTS221",    0, sensor_hts221, NULL,
    "ENDTAB",    0,       NULL, NULL,
};
#define _MAX_M2XFUNCTIONS	(sizeof(m2xfunctions)/sizeof(M2XFUNC))

const int _max_m2xfunctions = _MAX_M2XFUNCTIONS;

void tx2m2x_timer(size_t timer_id, void * user_data)
{
    const time_t t = time(0);
    printf("\nRead sensor and send data %d more times (%s)\n",m2x_iterations, asctime(localtime(&t)));
    do_hts2m2x();
    --m2x_iterations;
    if( !m2x_iterations )
        sensor_hts221(0,0);
}
 
 
void sensor_hts221(int interval, int iterations)
{
printf("sending data %d times, every %d seconds.\n",iterations, interval);

    if( m2x_sensor_timer != 0 ) {  //currently have a timer running
        if( interval ) { //just want to change the rate of samples
            m2x_iterations = iterations;
            delete_IoTtimer(m2x_sensor_timer);
            m2x_sensor_timer = create_IoTtimer(interval, tx2m2x_timer, TIMER_PERIODIC, NULL);
printf("changed timer\n");
            }
         else {  //want to kill the timer
            m2x_iterations = 0;
            delete_IoTtimer(m2x_sensor_timer);
            stop_IoTtimers();
            m2x_sensor_timer = 0;
printf("stopped timer\n");
            }
        }
    else { //don't have a timer currently running, start one up
        m2x_iterations = iterations;
        start_IoTtimers();
        m2x_sensor_timer = create_IoTtimer(interval, tx2m2x_timer, TIMER_PERIODIC, NULL);
printf("started timer\n");
        }
}

void do_hts2m2x(void)
{
    float adc_voltage;
    char str_adc_voltage[16];
    
    printf("Tx M2X data to:\nDeviceID = [%s]\nAPI Key=[%s]\nStream Name=[%s]\n",
            device_id, api_key, stream_name);

    start_data_service();
    m2x_create_stream(device_id, api_key, stream_name);

    adc_voltage = hts221_getTemp();
    memset(str_adc_voltage, 0, sizeof(str_adc_voltage));
    sprintf(str_adc_voltage, "%f", adc_voltage);
    m2x_update_stream_value(device_id, api_key, stream_name, str_adc_voltage);		
    printf("\n");
}




