
#ifndef TIME_H
#define TIME_H

#include <stdlib.h>
#include <unistd.h>
 
typedef void (*time_handler)(size_t timer_id, void * user_data);
 
typedef enum
{
TIMER_SINGLE_SHOT = 0, /*Periodic Timer*/
TIMER_PERIODIC         /*Single Shot Timer*/
} t_timer;
 
struct timer_node
{
    int                 fd;
    time_handler        callback;
    void *              user_data;
    unsigned int        interval;
    t_timer             type;
    struct timer_node * next;
};
 
#ifdef __cplusplus
extern "C" {
#endif

extern int active_IoTtimer();
extern int start_IoTtimers();
extern void stop_IoTtimers();

extern size_t create_IoTtimer(unsigned int interval, time_handler handler, t_timer type, void * user_data);
extern void delete_IoTtimer(size_t timer_id);
extern void change_IoTtimer(size_t timer_id, int newperiod);
 
#ifdef __cplusplus
}
#endif

#endif

