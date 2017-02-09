#ifndef TIME_H
#define TIME_H

#include <stdlib.h>
#include <unistd.h>
 
typedef enum
{
TIMER_SINGLE_SHOT = 0, /*Periodic Timer*/
TIMER_PERIODIC         /*Single Shot Timer*/
} t_timer;
 
typedef void (*time_handler)(size_t timer_id, void * user_data);
 
extern int start_IoTtimers();
extern void stop_IoTtimers();

extern size_t create_IoTtimer(unsigned int interval, time_handler handler, t_timer type, void * user_data);
extern void delete_IoTtimer(size_t timer_id);
 
#endif

