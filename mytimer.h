/* =====================================================================
   Copyright © 2016, Avnet (R)

   www.avnet.com 
 
   Licensed under the Apache License, Version 2.0 (the "License"); 
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, 
   software distributed under the License is distributed on an 
   "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
   either express or implied. See the License for the specific 
   language governing permissions and limitations under the License.

    @file          mytimer.h
    @version       1.0
    @date          Sept 2017

======================================================================== */


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
extern size_t create_ms_IoTtimer(unsigned int interval, time_handler handler, t_timer type, void * user_data);
extern void delete_IoTtimer(size_t timer_id);
extern void change_IoTtimer(size_t timer_id, int newperiod);
 
#ifdef __cplusplus
}
#endif

#endif

