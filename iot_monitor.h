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

    @file          iot_monitor.h
    @version       1.0
    @date          Sept 2017

======================================================================== */


#ifndef __IOT_MONITOR_H__
#define __IOT_MONITOR_H__

#ifdef __cplusplus

#include <cctype>
#include <cstddef>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <functional>
#include <string>

#endif

#define VER             1.11
#define VER_DATE	__DATE__ " @ " __TIME__

#define my_getc()	getc()
#define my_puts(s)	puts(s)
#define my_putc(c)	putchar(c)
#define my_printf(fmt,...)	printf(fmt, ##__VA_ARGS__)

#define MAX(a,b)	(((a)>(b))?(a):(b))
#define MIN(a,b)	(((a)<(b))?(a):(b))

#define DBG_CURL	0x0001
#define DBG_FLOW	0x0002
#define DBG_M2X 	0x0004
#define DBG_MYTIMER	0x0010
#define DBG_LIS2DW12	0x0020
#define DBG_HTS221	0x0040
#define DBG_BINIO	0x0100
#define DBG_MAL         0x0200
#define DBG_DEMO	0x0400
#define DBG_I2C		0x0800
#define DBG_SPI		0x1000
#define DBG_EMISSIN	0x2000

#define MONITOR_PROMPT	((char*)"MON> ")
#define FACTORY_PROMPT	((char*)"WNC> ")

#define DEFAULT_TEMP_API_STREAM (char*)"temp"
#define DEFAULT_ADC_API_STREAM  (char*)"light_sens"

#define my_debug(x,...)	(__VA_ARGS__);

#ifdef __cplusplus
typedef struct {
  std::string firmVer;
  std::string appsVer;
  std::string malwarVer;
  std::string ip;
  std::string model;
  std::string imei;
  std::string imsi;
  std::string iccid;
  std::string msisdn;
  std::string opMode;
  } sysinfo;
extern sysinfo mySystem;
#endif
  
// struct of string for the command and pointer to function 
typedef struct {
  int max_elements;
  const char *commandp;
  const char *help_str;
  int (*func_p)(int, const char * const *);
  } cmd_entry;

extern const cmd_entry mon_command_table[];

#ifndef _DATADEF_ 
extern char device_id[];
extern char api_key[];
extern char *temp_stream_name;
extern char *adc_stream_name;
extern char* strupr(char* s);
extern const cmd_entry iot_command_table[];
extern int headless;
extern int doM2X;
extern int headless_timed;
extern unsigned int dbg_flag;
extern int ft_time;
extern int ft_mode, extendedIO;
extern unsigned int dbg_flag;
extern void *htsdev;
#endif

extern int command_help(int argc, const char * const * argv );
extern int command_gpio(int argc, const char * const * argv );
extern int command_blink(int argc, const char * const * argv );
extern int command_sndat(int argc, const char * const * argv );
extern int command_baud(int argc, const char * const * argv );
extern int command_hts221(int argc, const char * const * argv );
extern int command_lis2dw12(int argc, const char * const * argv );
extern int command_spi(int argc, const char * const * argv );
extern int command_i2cpeek(int argc, const char * const * argv );
extern int command_i2cpoke(int argc, const char * const * argv );
extern int command_peek(int argc, const char * const * argv );
extern int command_poke(int argc, const char * const * argv );
extern int command_comscmd(int argc, const char * const * argv);
extern int command_http_put(int argc, const char * const * argv);
extern int command_http_get(int argc, const char * const * argv);
extern int command_iotmon(int argc, const char * const * argv );
extern int command_wnctest(int argc, const char * const * argv );
extern int command_iothelp(int argc, const char * const * argv );
extern int command_WWANStatus(int, char const* const*);
extern int command_WWANLED(int, char const* const*);
extern int command_WNCInfo(int argc, const char * const * argv );
extern int command_tx2m2x(int argc, const char * const * argv );
extern int command_exit(int argc, const char * const * argv );
extern int command_gps(int argc, const char * const * argv );
extern int command_adc(int argc, const char * const * argv );
extern int command_headless(int argc, const char * const * argv );
extern int command_facttest(int argc, const char * const * argv );
extern int command_dbg(int argc, const char * const * argv );
//extern int command_VL53L0X(int argc, const char * const * argv );

#ifdef __cplusplus
extern "C" {
#endif
extern int command_demo_mode(int, const char * const * argv);
extern unsigned int ascii_to_epoch(char *epoch_ascii);
extern void doNewLine(void);
void *check_gps(void *);
void monitor_wwan( void );
void kill_monitor_wwan( void );

#ifdef __cplusplus
}
#endif


extern void sigint_cb (void);
extern cmd_entry *current_table;

extern int max_moncommands, max_iotcommands;

extern char * completion_array [];

void set_cmdhandler(    void (*of)(const char*),  //function to call to output text
                                   char *prompt,  //prompt to use
                                       int plen,  //length of prompt
                       void (*siginit_cb)(void),  //call back of interrupt signal (^C)
       int (*exec_cb)(int, char const * const*),  //call back to call when a input line is complete
char ** (*complete_cb)(int, const char* const*),  //call back for command line completion
                         const cmd_entry tabp[]); //what command table to use



#endif // __IOT_MONITOR_H__
