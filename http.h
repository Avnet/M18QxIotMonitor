
#ifndef __HTTP_H__
#define __HTTP_H__

#include "mytimer.h"

typedef void (*m2xSensorTx)(int);

typedef struct _m2xfunc {
    char                *name;
    int			rate;
    m2xSensorTx		func;
    struct timer_node * tmr;
    } M2XFUNC;
 
#ifdef __cplusplus
extern "C" {
#endif

extern M2XFUNC m2xfunctions[];
extern const int _max_m2xfunctions;
extern size_t m2x_sensor_timer;

extern void m2x_htts221(int interval);

#ifdef __cplusplus
}
#endif

#include <nettle/nettle-stdint.h>
#include <hwlib/hwlib.h>

#endif // __HTTP_H__
