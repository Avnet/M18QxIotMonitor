

#ifndef __BINIO_H__
#define __BINIO_H__

typedef struct _gpios {
    int           nbr;
    int		  rate;
    int		  val;
    gpio_handle_t hndl;
    size_t        timr;
    } GPIOPIN;
 
#ifdef __cplusplus
extern "C" {
#endif

void gpio_timer(size_t timer_id, void * user_data);

extern size_t m2x_sensor_timer;
extern const int _max_gpiopins;
extern GPIOPIN gpios[];

void sensor_hts221(int interval);
void do_gpio_blink( int i, int interval );

void binary_io_init(void);
void binario_io_close(void);

#ifdef __cplusplus
}
#endif


#include <nettle/nettle-stdint.h>
#include <hwlib/hwlib.h>

#endif // __BINIO_H__

