

#ifndef __BINIO_H__
#define __BINIO_H__

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

extern void binary_io_init(void);
extern void binario_io_close(void);
extern GPIOPIN gpio[];

#ifdef __cplusplus
}
#endif

#include <nettle/nettle-stdint.h>
#include <hwlib/hwlib.h>

#endif // __BINIO_H__
