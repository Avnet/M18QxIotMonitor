

#ifndef __IOT_MONITOR_H__
#define __IOT_MONITOR_H__

#include <cctype>
#include <cstddef>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <functional>
#include <string>

#define my_getc()	getc()
#define my_puts(s)	puts(s)
#define my_putc(c)	putchar(c)
#define my_printf(fmt,...)	printf(fmt, ##__VA_ARGS__)

#define MAX(a,b)	(((a)>(b))?(a):(b))
#define MIN(a,b)	(((a)<(b))?(a):(b))

#define MONITOR_PROMPT	((char*)"MON> ")
#define FACTORY_PROMPT	((char*)"FAC> ")

// struct of string for the command and pointer to function 
typedef struct {
  int max_elements;
  const char *commandp;
  const char *help_str;
  int (*func_p)(int, const char * const *);
  } cmd_entry;

typedef struct {
  std::string firmVer;
  std::string malwarVer;
  std::string ip;
  std::string model;
  std::string imei;
  std::string imsi;
  std::string iccid;
  std::string msisdn;
  std::string opMode;
  } sysinfo;
  
extern char* strupr(char* s);

extern const cmd_entry mon_command_table[];
extern const cmd_entry iot_command_table[];
extern sysinfo mySystem;

extern int command_help(int argc, const char * const * argv );
extern int command_gpio(int argc, const char * const * argv );
extern int command_blink(int argc, const char * const * argv );
extern int command_sndat(int argc, const char * const * argv );
extern int command_baud(int argc, const char * const * argv );
extern int command_hts221(int argc, const char * const * argv );
extern int command_i2cpeek(int argc, const char * const * argv );
extern int command_i2cpoke(int argc, const char * const * argv );
extern int command_peek(int argc, const char * const * argv );
extern int command_poke(int argc, const char * const * argv );
extern int command_comscmd(int argc, const char * const * argv);
extern int command_http_put(int argc, const char * const * argv);
extern int command_http_get(int argc, const char * const * argv);
extern int command_iotmon(int argc, const char * const * argv );
extern int command_factest(int argc, const char * const * argv );
extern int command_iothelp(int argc, const char * const * argv );
extern int command_WWANStatus(int, char const* const*);
extern int command_WNCInfo(int argc, const char * const * argv );
extern int command_tx2m2x(int argc, const char * const * argv );
extern int command_exit(int argc, const char * const * argv );
extern int command_gps(int argc, const char * const * argv );
extern int command_adc(int argc, const char * const * argv );
extern int command_headless(int argc, const char * const * argv );
extern int command_facttest(int argc, const char * const * argv );

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
