
#ifndef __ADC_H__
#define __ADC_H__

typedef struct _gpios {
    int                 nbr;
    int			rate;
    int			val;
    gpio_handle_t	hndl;
    struct timer_node * tmr;
    } GPIOPIN;
 
#ifdef __cplusplus
extern "C" {
#endif

extern void do_adc_init(void);
extern void do_adc_close(void);
extern GPIOPIN gpio[];

#ifdef __cplusplus
}
#endif

#include <nettle/nettle-stdint.h>
#include <hwlib/hwlib.h>

#endif // __ADC_H__


extern int adc_init(adc_handle_t *handle_ptr);
extern int adc_deinit(adc_handle_t *handle_ptr);
extern int adc_read(adc_handle_t handle, float *adc_value);
