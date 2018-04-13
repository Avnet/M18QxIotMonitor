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

    @file          commands.cpp
    @version       1.0
    @date          Sept 2017

======================================================================== */



#define _DATADEF_ 1

#include <unistd.h>
#include <cctype>
#include <cstddef>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <functional>
#include <math.h>

extern "C" {
#include <hwlib/hwlib.h>
}

#include "iot_monitor.h"
#include "microrl_config.h"
#include "microrl.h"
#include "HTS221.hpp"
#include "lis2dw12.h"
#include "binio.h"
#include "mytimer.h"
#include "http.h"
#include "m2x.h"
#include "MAX31855.hpp"
#include "MS5637.hpp"
#include "HTU21D.hpp"
#include "KMA36.hpp"

#include "mal.hpp"

extern "C" {
  int gpio_irq_callback(gpio_pin_t pin_name, gpio_irq_trig_t direction);
  void set_color(char *);
  void wwan_io(int);
}

#define my_getc()	getc()
#define my_puts(s)	puts(s)
#define my_putc(c)	putchar(c)
#define my_printf(fmt,...)	printf(fmt, ##__VA_ARGS__)

#define MAX(a,b)	(((a)>(b))?(a):(b))
#define MIN(a,b)	(((a)<(b))?(a):(b))

extern char* strupr(char* s);

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

microrl_t rl;
microrl_t *prl = &rl;

char device_id[50];
char api_key[50];
char *adc_stream_name  = (char*)DEFAULT_ADC_API_STREAM;
char *temp_stream_name = (char*)DEFAULT_TEMP_API_STREAM;
sysinfo mySystem;
int headless=0;
int headless_timed=0;
int ft_mode=0, extendedIO=0;
int ft_time=0;
int doM2X=true;
unsigned int dbg_flag = 0;
void *htsdev;
float adc_threshold=0;

int command_help(int argc, const char * const * argv );
int command_gpio(int argc, const char * const * argv );
int command_blink(int argc, const char * const * argv );
int command_sndat(int argc, const char * const * argv );
int command_facttest(int argc, const char * const * argv );
int command_hts221(int argc __attribute__((unused)), const char * const * argv );
int command_lis2dw12(int argc __attribute__((unused)), const char * const * argv );
int command_lis2dw(int argc __attribute__((unused)), const char * const * argv );
int command_spi(int argc __attribute__((unused)), const char * const * argv );
int command_i2cpeek(int argc, const char * const * argv );
int command_i2cpoke(int argc, const char * const * argv );
int command_peek(int argc, const char * const * argv );
int command_poke(int argc, const char * const * argv );
int command_comscmd(int argc, const char * const * argv);
int command_http_put(int argc, const char * const * argv);
int command_http_get(int argc, const char * const * argv);
int command_iotmon(int argc, const char * const * argv );
int command_wnctest(int argc, const char * const * argv );
int command_tx2m2x(int argc, const char * const * argv );
int command_WNCInfo(int argc, const char * const * argv );
int command_WWANStatus(int, char const* const*);
int command_WWANLED(int, char const* const*);
int command_gps(int argc, const char * const * argv );
int command_adc(int argc, const char * const * argv );
int command_exit(int, char const* const*);
int command_dbg(int argc, const char * const * argv );
int command_devid(int argc, const char * const * argv );
int command_apikey(int argc, const char * const * argv );
int command_pause(int argc, const char * const * argv );
int command_ms5637(int argc, const char * const * argv );
int command_htu21d(int argc, const char * const * argv );
int command_tsys02d(int argc, const char * const * argv );
int command_tsys01(int argc, const char * const * argv );
int command_kma36(int argc, const char * const * argv );
//jmf int command_VL53L0X(int argc, const char * const * argv );
void do_hts2m2x(void);

void sigint_cb (void);

const cmd_entry mon_command_table[] =
  {
  max_moncommands, ";",           
     "GPIO's currently supported: GPIO_PIN_92, GPIO_PIN_101, GPIO_PIN_102\n",                   NULL,
  0, "?",           
     "?             Displays this help screen",                                                 command_help,
  0, "HELP",        
     "help          Displays this help screen",                                                 command_help,
  0, "FTEST",       
     "ftest         Perform series of factory pass/fail tests",                                 command_facttest,
  0, "GPIO",        
     "gpio # #      Drives GPIO pin high/low as desired (GPIO GPIx 0/1)",                       command_gpio,
  0, "BLINK",       
     "blink # #     Alternates GPIO between 0/1 @ requested rate (secs)<interval=0 to stop>",   command_blink, 
  0, "HTS221",      
     "HTS221 X Y    Display information about the HTS221 sensor, send X msgs with Y sec delay", command_hts221,
  0, "LIS2DW12",      
     "LIS2DW12      Display information about the LIS2DW12 sensor, send X msgs with Y sec delay", command_lis2dw12,
  0, "MAX31855",      
     "MAX31855      Read the MAX31855 Thermocouple-to-Digital Converter (SPI BUS on PMOD)",     command_spi,
  0, "MS5637",      
     "MS5637        Read the MS5637-30BA Pressure/Tempertaure sensor (if pressent)",            command_ms5637,
  0, "HTU21D",      
     "HTU21D        Read the HTU21D Temperature/Humidity sensor (if present)",                  command_htu21d,
  0, "TSYS02D",      
     "TSYS02D       Read the TSYS02D Temperature sensor (if present)",                          command_tsys02d,
  0, "TSYS01",      
     "TSYS01        Read the TSYS01 Temperature sensor (if present)",                           command_tsys01,
//jmf  0, "VL53L0X",      
//jmf     "VL53L0X #     Read the LightRanger/VL53L0X sensor (if present); #=duration",              command_VL53L0X,
  0, "KMA36",      
     "KMA36         Read the KMA36 sensor (if present)",                                        command_kma36,
  0, "GPS",         
     "GPS           Display GPS information                                             ",      command_gps,
  0, "ADC",         
     "ADC #         Read ADC (if a # is present, it is set as a threshold value)        ",      command_adc, 
  0, "I2CPEEK",     
     "I2CPEEK <dev> <reg> <nbr_bytes> Display <nbr_bytes> returned by reading <reg> on I2C bus\n"
     "             LIS2DW12 <dev> =19; HTS221 <dev>=5F;                                         ",command_i2cpeek,
  0, "I2CPOKE",     
     "I2CPOKE <dev> <reg> <b1> <b2> <b3> <b4> <b5> <b6> write up to 6 bytes to <reg> on I2C bus\n"
     "             LIS2DW12 <dev> =19; HTS221 <dev>=5F;                                         ",command_i2cpoke,
  0, "DEBUG",       
     "DEBUG X V     Set or Clear a debug flag X=set or clr, V = flag to set or clear",          command_dbg,
  0, "WNC",         
     "wnc           Enters WNC testing command mode",                                           command_wnctest,
  0, "DODEMO",      
     "dodemo        Run the Demo Program (hold user key for > 3 sedonds to return to monitor)", command_headless,
  0, "PAUSE",      
     "pause X       Pause program execution for # seconds",                                     command_pause,
  0, "EXIT",        
     "exit          End Program execution",                                                     command_exit
  };
#define _MAX_MONCOMMANDS	(sizeof(mon_command_table)/sizeof(cmd_entry))

const cmd_entry fac_command_table[] =
  {
  max_iotcommands, ";",           "A ';' denotes that the rest of the line is a comment and not to be executed.",NULL,
  0,               ";",           "\n",                                                                          NULL,
  0, "?",           "?             Displays this help screen",                 command_help,
  0, "HELP",        "help          Displays this help screen",                 command_help,
  0, "WNCINFO",     "wncinfo       Displays the WNC module information",       command_WNCInfo, 
  0, "WWANSTAT",    "wwanstat      Displays the WWAN status information",      command_WWANStatus,
  0, "WWAN_LED",    "wwan_led 0/1  Disable/Enable the WWAN LED",               command_WWANLED,
  0, "TX2M2X",      "tx2m2x Y X Z  Tx data Y times every X secs from device Z",command_tx2m2x, 
  0, "DEVID",       "devid <ID>    Set Device id to <ID>",                     command_devid,
  0, "APIKEY",      "apikey <KEY>  Set a Primary API Key to <KEY>",            command_apikey,
  0, "MON",         "mon           Enter interactive monitor mode",            command_iotmon,
  0, "DEBUG",       "DEBUG X V     Set or Clear debug flag ",                  command_dbg,
  0, "PAUSE",       "Pause #       program execution for # seconds ",          command_pause,
  0, "EXIT",        "exit          End Program execution",                     command_exit
  };
#define _MAX_IOTCOMMANDS	(sizeof(fac_command_table)/sizeof(cmd_entry))

cmd_entry *current_table = NULL;		//maintain a pointer to the active commands

int max_moncommands = _MAX_MONCOMMANDS;
int max_iotcommands = _MAX_IOTCOMMANDS;

char * completion_array [MAX(_MAX_MONCOMMANDS,_MAX_IOTCOMMANDS) + 1];   	// array for command comletion

// command function prototypes are:
//   int command (int argc, const char * const * argv)
// and the command has to return the number of arguments it processed
//

#ifdef _USE_COMPLETE
//*****************************************************************************
// completion callback for microrl library
char ** complet (int argc, const char * const * argv)
{
    int j = 0;
    int max = current_table->max_elements;
    cmd_entry *tptr = current_table;
    completion_array[0] = NULL;

    // if there is token in cmdline
    if (argc == 1) {
        // get last entered token
        char * bit = (char*)argv [argc-1];
        for (int i = 0; i < max; i++) {                          // iterate through available cmds and match it
          if (strstr((tptr[i]).commandp, strupr(bit))) {         // if token matches 
            completion_array[j++] = (char*)(tptr[i]).commandp;   // add it to completion set
            }
        }
      }
  else { // if there is no token in cmdline, just print all available commands
     for (; j < max; j++) {
       completion_array[j] = (char*)(tptr[j]).commandp;
       }
  }

  // note! last ptr in array always must be NULL!!!
  completion_array[j] = NULL;       // return set of variants
  return completion_array;
  }
#endif


void print_banner(void) {

  printf("----------------------------------------------------------------------------\n");
  printf("\n");
  printf("       ** **        **          **  ****      **  **********  ********** ®\n");
  printf("      **   **        **        **   ** **     **  **              **\n");
  printf("     **     **        **      **    **  **    **  **              **\n");
  printf("    **       **        **    **     **   **   **  *********       **\n");
  printf("   **         **        **  **      **    **  **  **              **\n");
  printf("  **           **        ****       **     ** **  **              **\n");
  printf(" **  .........  **        **        **      ****  **********      **\n");
  printf("    ...........\n");
  printf("                                    Reach Further\n");
  printf("\n");
  printf(" AVNET - AT&T Global Module IoT Monitor\n");
  printf(" Version %5.2f // %s \n",VER,VER_DATE);
  printf(" Hardware Supported: WNC M18Qx Cellular Data Module\n");
  printf("----------------------------------------------------------------------------\n");
  
  return ;
}

//
// debug set/clr flag

int command_dbg(int argc __attribute__((unused)), const char * const * argv ) 
{
    char *action=NULL, *flag=NULL;
    int a=0, done=0, idx;

    printf("Debug flag currently = 0x%04X.\n", dbg_flag);

    if( argc < 3 ) {
        printf("Possible debug flags and their current setting is:\n",dbg_flag);
        printf("  CURL     (%s) - display information about CURL operations\n",dbg_flag&DBG_CURL?"  SET  ":"NOT SET");
        printf("  FLOW     (%s) - display information about CURL operations\n",dbg_flag&DBG_FLOW?"  SET  ":"NOT SET");
        printf("  M2X      (%s) - display informatioin about\n",dbg_flag&DBG_M2X?"  SET  ":"NOT SET");
        printf("  TIMER    (%s) - display informatioin about\n",dbg_flag&DBG_MYTIMER?"  SET  ":"NOT SET");;
        printf("  LIS2DW12 (%s) - display informatioin about\n",dbg_flag&DBG_LIS2DW12?"  SET  ":"NOT SET");
        printf("  HTS221   (%s) - display informatioin about\n",dbg_flag&DBG_HTS221?"  SET  ":"NOT SET");;
        printf("  BINIO    (%s) - display informatioin about\n",dbg_flag&DBG_BINIO?"  SET  ":"NOT SET");
        printf("  MAL      (%s) - display information to/from the MAL\n",dbg_flag&DBG_MAL?"  SET  ":"NOT SET");
        printf("  I2C      (%s) - display information on the I2C bus\n",dbg_flag&DBG_I2C?"  SET  ":"NOT SET");
        printf("  SPI      (%s) - display information on the SPI bus\n",dbg_flag&DBG_SPI?"  SET  ":"NOT SET");
        return 0;
        }

    action = (char*)argv[1];
    a |= !strcmp(strupr(action),"SET")?1:0;
    a |= !strcmp(strupr(action),"1")?1:0;
    a |= !strcmp(strupr(action),"CLR")?2:0;
    a |= !strcmp(strupr(action),"0")?2:0;

    flag   = (char*)argv[(idx=2)];

    while( a && action != NULL && flag != NULL && idx < argc ) {
        if( !strcmp(strupr(flag),"DBG_DEMO") ) {
            done=1;
            if( a == 1)
                dbg_flag |= DBG_DEMO;
            else
                dbg_flag &= ~DBG_DEMO;
            }
        if( !strcmp(strupr(flag),"CURL") ) {
            done=1;
            if( a == 1)
                dbg_flag |= DBG_CURL;
            else
                dbg_flag &= ~DBG_CURL;
            }
        if( !strcmp(strupr(flag),"FLOW") ) {
            done=1;
            if( a == 1)
                dbg_flag |= DBG_FLOW;
            else
                dbg_flag &= ~DBG_FLOW;
            }
        if( !strcmp(strupr(flag),"M2X") ) {
            done=1;
            if( a == 1)
                dbg_flag |= DBG_M2X;
            else
                dbg_flag &= ~DBG_M2X;
            }
        if( !strcmp(strupr(flag),"TIMER") ) {
            done=1;
            if( a == 1)
                dbg_flag |= DBG_MYTIMER;
            else
                dbg_flag &= ~DBG_MYTIMER;
            }
        if( !strcmp(strupr(flag),"LIS2DW12") ) {
            done=1;
            if( a == 1)
                dbg_flag |= DBG_LIS2DW12;
            else
                dbg_flag &= ~DBG_LIS2DW12;
            }
        if( !strcmp(strupr(flag),"HTS221") ) {
            done=1;
            if( a == 1)
                dbg_flag |= DBG_HTS221;
            else
                dbg_flag &= ~DBG_HTS221;
            }
        if( !strcmp(strupr(flag),"BINIO") ) {
            done=1;
            if( a == 1)
                dbg_flag |= DBG_BINIO;
            else
                dbg_flag &= ~DBG_BINIO;
            }
        if( !strcmp(strupr(flag),"MAL") ) {
            done=1;
            if( a == 1)
                dbg_flag |= DBG_MAL;
            else
                dbg_flag &= ~DBG_MAL;
            }
        if( !strcmp(strupr(flag),"I2C") ) {
            done=1;
            if( a == 1)
                dbg_flag |= DBG_I2C;
            else
                dbg_flag &= ~DBG_I2C;
            }
        if( !strcmp(strupr(flag),"SPI") ) {
            done=1;
            if( a == 1)
                dbg_flag |= DBG_SPI;
            else
                dbg_flag &= ~DBG_SPI;
            }
        flag   = (char*)argv[++idx];
        }

    printf("Debug flag set to: 0x%04X\n",dbg_flag);
}

int command_iotmon(int argc __attribute__((unused)), const char * const * argv __attribute__((unused))) {

    current_table = (cmd_entry*)mon_command_table;
    my_printf("RUN IOT APPLICATION.\n");
    microrl_setprompt (prl, MONITOR_PROMPT, strlen(MONITOR_PROMPT)) ;
    return 1;
}

// ----------------------------------------------------------------------------------------------
// This function set the call-back functions for the different modes or operation.  In normal mode 
// or a sub-mode.  It takes the following arguments:
void set_cmdhandler(   void (*of)(const char *),  //function to call to output text
                                   char *prompt,  //prompt to use
                                       int plen,  //length of prompt
                       void (*siginit_cb)(void),  //call back of interrupt signal (^C)
       int (*exec_cb)(int, char const * const*),  //call back to call when a input line is complete
char ** (*complete_cb)(int, const char* const*),  //call back for command line completion
                         const cmd_entry tabp[])  //what command table to use
{
    microrl_init (prl, (void (*)(const char *))of, prompt, plen);
    microrl_set_execute_callback (prl, exec_cb);      // set callback for execute
    current_table = (cmd_entry*)tabp;
#ifdef _USE_CTLR_C
    microrl_set_sigint_callback (prl, sigint_cb);     // set callback for Ctrl+C
#endif
#ifdef _USE_COMPLETE
    microrl_set_complete_callback (prl, complete_cb); // set callback for completion
#endif
}


// ----------------------------------------------------------------------------------------------
int process_command (int argc, const char * const * argv)
{
    int i = 1;
    int max = current_table->max_elements;
    cmd_entry *tptr = current_table;

    if (*argv[0] != ';') {
        if (*argv[0] != '?') {
            while( strcmp(strupr((char*)argv[0]), (tptr[i]).commandp) && ++i < max );
            if (i < max)
                (tptr[i]).func_p(argc,argv);
            else
                my_printf("\nunknown command: %s\n",argv[0]);
            }
        else
            command_help(argc,argv);
        }
    else
        my_printf("\n");
	
    return 0;
}


// ----------------------------------------------------------------------------------------------
int command_help(int argc, const char * const * argv )
{
    int i;
    long max = current_table->max_elements;
    cmd_entry *tptr = current_table;

    my_puts(((tptr[0]).help_str));
    my_puts("Command          Description");
    my_puts("============= ========================================================================\n");

    for( i=1; i<max; i++ )
        my_puts(((tptr[i]).help_str));

    my_puts(" ");
    return 0;
}



//*****************************************************************************
void sigint_cb (void)
{
  my_printf("\n\r^C detected!   **Stopping all periodic activities.**\n\r");

  return ;
}

char* strupr(char* s)
{
    char *p = s;
    while (*p){
        *p=toupper(*p);
        p++;
        }
    return s;
}



int command_exit(int, char const* const*)
{
    void app_exit(void);

    app_exit();
}

//for(k=0; k<3; k++) printf("#%02d: {%s, %s}\n", k,om[k].key,om[k].value);

int command_WWANStatus(int, char const* const*)
{
    json_keyval om[20];
    int k;
    const char *radio_mode[] = {
        "No service", // 0
        "No service", // 1
        "2G",         // 2
        "3G",         // 3
        "4G",         // 4
        "No service", // 5
        "4G+"         // 6
        };
    const char *radio_state[]= { 
        "No Registered", "Registered with network", 
        "searching", "registration denied", "unknown" };
    const char *xs_state[]   = {"Unknown","Attached","Detached"};
    const char *roaming[]    = {"Home", "Roaming", "Unknown"};

    start_data_service();

    printf("\n->WWAN Status \n");
    printf("received %d key/value pairs:\n", get_wwan_status(om, sizeof(om)));
    printf("             Radio Mode: %s(%s)\n", radio_mode[atoi(om[3].value)], om[3].value);
    printf("        Signal strength; %s\n", om[4].value);
    printf("           Signal Level: %s\n", om[6].value);
    printf("                  state: %s(%s)\n", radio_state[atoi(om[7].value)], om[7].value);
    printf(" Circuit-switched state: %s(%s)\n", xs_state[atoi(om[8].value)], om[8].value);
    printf("  Packet-switched state: %s(%s)\n", xs_state[atoi(om[9].value)], om[9].value);
    printf("     Registration state: %s(%s)\n\n", roaming[atoi(om[16].value)], om[16].value);

    const char * type[] = {"3G/LTE", "Ethernet", "WiFi" };
    const char * state[] ={ "Disconnected", "Disconnecting", "Connecting", "Connected", 
                            "Disconnected, and PIN locked", "Disconnected and SIM removed" };

    printf("->Network Connection Status \n");
    printf("received %d key/value pairs:\n", get_connection_status(om, sizeof(om)));
    printf("   connection type: %s(%s)\n", type[atoi(om[3].value)], om[3].value);
    printf("  connection state: %s(%s)\n", state[atoi(om[4].value)], om[4].value);
    printf("   connection time: %s\n", om[5].value);
    printf("          provider: %s\n", om[6].value);
    printf("        radio mode: %s\n", om[7].value);
    printf("  data_bearer_tech: %s\n", om[8].value);
    printf("    roaming status: %s\n", om[9].value);
    printf("   signal strength: %s\n", om[10].value);
    printf("      signal level: %s\n", om[11].value);
    printf("          LTE rsrp: %s\n", om[12].value);
    printf("        WCDMA RSCP: %s\n", om[13].value);
    printf("      ipv6 address: %s\n\n", om[14].value);

    printf("received %d key/value pairs:\n", get_wwan_allow_data_roaming(om, sizeof(om)));
    printf("Allow Data Roaming: %s (%s)\n\n", atoi(om[3].value)? "YES":"NO", om[3].value);

}


void dump_keyvalues(json_keyval *pkv, int siz)
{
    for(int k=0; k<siz/sizeof(json_keyval); k++) 
        printf("#%02d: {%s, %s}\n", k, pkv[k].key, pkv[k].value);
}

int command_WNCInfo(int x, char const* const* p)
{
    int i;
    json_keyval om[20];
    const char *om_str[] = {
        "0-Online",
        "1-Low Power",
        "2-Factory Test mode",
        "3-Offline",
        "4-Resetting",
        "5-Shutting down" };

    i=start_data_service();
    while ( i < 0 ) {
        printf("WAIT: starting WNC MAL Interface (%d)\n",i);
        sleep(10);
        i=start_data_service();
        }

    printf(" WNC Information\n");
    fflush(stdout);
    do
        mySystem.model=getModelID(om, sizeof(om));
    while( mySystem.model == "service is not ready");

    printf("              WNC Model: %s\n",mySystem.model.c_str());
    mySystem.appsVer=getAppsVersion(om, sizeof(om));
    printf("           Apps Version: %s\n",mySystem.appsVer.c_str());
    mySystem.firmVer=getFirmwareVersion(om, sizeof(om));
    printf("       Firmware Version: %s\n",mySystem.firmVer.c_str());
    mySystem.malwarVer=getMALManVer(om, sizeof(om));
    printf("            MAL Version: %s\n",mySystem.malwarVer.c_str());
    mySystem.opMode = getOperatingMode(om, 4);
    printf("         Operating Mode: %s\n",om_str[atoi(mySystem.opMode.c_str())]);
    mySystem.ip=get_ipAddr(om, sizeof(om));
    printf("                     IP: %s\n",mySystem.ip.c_str());
    mySystem.iccid=getICCID(om, sizeof(om));
    printf("                  ICCID: %s\n",mySystem.iccid.c_str());
    mySystem.imei=getIMEI(om, sizeof(om));
    printf("                   IMEI: %s\n",mySystem.imei.c_str());
    mySystem.imsi=getIMSI(om, sizeof(om));
    printf("                   IMSI: %s\n",mySystem.imsi.c_str());
    mySystem.msisdn=getMSISDN(om, sizeof(om));
    printf("                 MSISDN: %s\n",mySystem.msisdn.c_str());
}

int command_gpio(int argc __attribute__((unused)), const char * const * argv )
{
    int indx, state;
    int k=0, done;

    if( argc != 3 ) 
        return 0;

    indx   = atoi(&argv[1][9]);
    state  = atoi(argv[2]);

    switch(indx) {
        case 92:  //GPIO_PIN_2
        case 101:  //GPIO_PIN_3
        case 102:  //GPIO_PIN_7
            do {
                done = (gpios[k].nbr == indx);
                k++;
                }
            while( k < _max_gpiopins && !done);
            k--; //k will always be 1 more than it should be, but done will be TRUE if found

            if (done) {
                printf("Setting GPIO_PIN_%d to %s\n",gpios[k].nbr,(state)?"1":"0");
                gpio_write( gpios[k].hndl, (state)?GPIO_LEVEL_HIGH:GPIO_LEVEL_LOW);
                break;
                }
        default:
            printf("ERROR: unknown binary i/o GPIO_PIN_%s\n",argv[1][9]);
            break;

        case 0:  
            printf("Invalid command syntax\n");
            break;
     }
}

//
// blink gpio_pin_XXX 1
int command_blink(int argc __attribute__((unused)), const char * const * argv )
{
    int indx=0, rate=0;
    int k=0, done, state;
    
    if( argc != 3 ) 
        return 0;

    indx = atoi(&argv[1][9]);
    rate = atoi(argv[2]);

    switch(indx) {
        case 92:   //GPIO_PIN_2
        case 101:  //GPIO_PIN_3
        case 102:  //GPIO_PIN_7
            do {
                done = (gpios[k].nbr == indx);
                k++;
                }
            while( k < _max_gpiopins && !done);
            k--; //k will always be 1 more than it should be, but done will be TRUE if found
                
            if (done) {  //ok found a GPIO_PIN_x to toggle
                do_gpio_blink( k, rate );
                break;
                }
        default:
            printf("ERROR: unknown binary i/o GPIO_PIN_%s\n",argv[1][9]);
            break;
        case 0:
            printf("Invalid command syntax\n");
            break;
     }
}


#define CTOF(x)  ((x)*(float)1.8+32) //celcius to Farenheight
#define MTOI(x)  ((x)*0.0295301)    //millibars to inches

int command_ms5637(int argc __attribute__((unused)), const char * const * argv )
{
    MS5637 ms5637;
    float *val;

    //lets use the highest resolution possible...
    ms5637.setMode(D1MODE_OSR8192, D2MODE_OSR8192);
    val = ms5637.getPT();
    if( ms5637.getErr() )
        printf("No MS5637 detected. (%d)\n",ms5637.getErr());
    else {
        printf("   MS5637 Pressure/Temperature reading:\n");
        printf("                            Temp (C/F): %4.2f/%4.2f\n", val[1],CTOF(val[1]));
        printf("                      Pressume (mb/in): %5.2f/%5.2f\n", val[0],MTOI(val[0]));
        }

    return 1;
}

int command_htu21d(int argc, const char * const * argv )
{
    HTU21D htu21d;

    if( htu21d.reset() )
        printf("No HTU21D detected. (%d)\n",htu21d.getErr());
    else {
        float t = htu21d.readTemperature(TRIGGER_TEMP_MEASURE_NOHOLD);
        int te = htu21d.getErr();
        float rh= htu21d.readHumidity(TRIGGER_HUMD_MEASURE_NOHOLD);
        int he = htu21d.getErr();

        printf("   HTU21D Humidity/Temperature reading [Config=0x%02X, HE=0x%02X, TE=0x%02X]:\n",htu21d.getOpMode(),he,te);
        printf("          Temp (C/F): %4.2f/%4.2f\n", t,CTOF(t));
        printf("   Relative Humidity: %5.2f\%\n", rh);
        printf("      Dewpoint (C/F): %4.2f/%4.2f\n",DEW(t,rh),CTOF(DEW(t,rh)));
        }
}

int command_hts221(int argc __attribute__((unused)), const char * const * argv )
{
    int k=1, repeats, delay = 0;
    float temp;
    HTS221 *hts221 = new HTS221;


    if( argc == 3 ) {
        delay   = atoi(argv[2]);
        repeats = atoi(argv[1]);
        }
    else if( argc == 2 )
        repeats = atoi(argv[1]);
    else
        repeats = 1;

    k = hts221->getDeviceID();

    printf("   HTS221 Device id: 0x%02X\n", k);

    if( repeats > 1 ) {
        printf("send %d mesurments with %d second delay between each measurment.\n",repeats,delay);
        do {
           /* keep trying to update temp & humidity until successful */
            printf("\nReading #%d:\n",k++);

            while( !hts221->getHumidity() ) ;
            while( !hts221->getTemperature() ) ;

            temp  = hts221->readHumidity();
            printf(" HTS221 Temperature: %3.2fc/%3.2ff\n", temp, CTOF(temp));
            printf("    HTS221 Humidity: %2.1f\n", hts221->readHumidity());
            sleep(delay);
            }
        while (--repeats);
        }
    else {
        /* get updated temp & humidity values */
        while( !hts221->getTemperature() ) ;
        while( !hts221->getHumidity() ) ;
        temp  = hts221->readTemperature();
        printf("\n HTS221 Temperature: %3.2fc/%3.2ff\n", temp, CTOF(temp));
        printf("    HTS221 Humidity: %2.1f\n", hts221->readHumidity());
        }
    return 1;
}

//dev reg nbr_of_bytes
int command_i2cpeek(int argc, const char * const * argv )
{
  i2c_handle_t my_handle=(i2c_handle_t)NULL;
  int nbr = 0x00;
  unsigned char buf[100];                 //use a 100 byte working buffer
  unsigned char reg;
  uint16_t dev;
  int     i;

    if( argc == 4 ) {
        memset(buf,0x00,sizeof(buf));
        nbr = atoi(argv[3]); //number of bytes to read
        sscanf(argv[2],"%x",(int*)&reg);      //register to read from
        sscanf(argv[1],"%x",(int*)&dev);      //device ID

        i=i2c_bus_init(I2C_BUS_I, &my_handle);
        if( dbg_flag & DBG_I2C )
            printf("-I2C:i2c_bus_init=%d\n",i);

        i=i2c_write(my_handle, dev, &reg, 1, I2C_NO_STOP);
        if( dbg_flag & DBG_I2C )
            printf("-I2C:i2c_write(handle,0x%02X,%d,1,I2C_NO_STOP)=%d\n",dev,reg,i);

        i=i2c_read(my_handle, dev, buf, nbr);
        if( dbg_flag & DBG_I2C )
            printf("-I2C:i2c_read(handle,0x%02X,buf,%d)=%d\n",dev,nbr,i);

        i=i2c_bus_deinit(&my_handle);
        if( dbg_flag & DBG_I2C )
            printf("-I2C:i2c_bus_deinit=%d\n",i);

        for (i=0; i<nbr; i+=8) 
            printf("%04X: %02X %02X %02X %02X %02X %02X %02X %02X %2c %2c %2c %2c %2c %2c %2c %2c\n\r",
                     reg,buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7],
                     buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7]);
        }
    else
        printf("ERROR: argc=%d\n",argc);
  return 1;
}

int command_i2cpoke(int, char const* const*)
{
    printf("called i2cpoke\n");
}

int command_wnctest(int, char const* const*)
{
    current_table = (cmd_entry*)fac_command_table;
    microrl_setprompt (prl, FACTORY_PROMPT, strlen(FACTORY_PROMPT)) ;
    printf("Entered WNC command mode.\n");
}

int command_headless(int argc, const char * const * argv )
{
    void app_exit(void);
    int i=0;
    const char * const * ptr=NULL;

    if( argc > 2 ) {
        i = atoi(argv[2]);
        printf("-loop every %d seconds\n",i);
        }

    if( argc > 1 ) 
        ptr = (const char * const *)argv[1];

    if( !argc ) 
        ptr = argv;
    command_demo_mode(i, ptr);
}


int command_gps(int argc, const char * const * argv )
{   
    struct tm gps_time;
    json_keyval om[12];
    int k, done, i, intfmt, strfmt;
    const char *mode[] = {
        "2-MS-based mode",
        "3-MS assisted mode",
        "4-Standalone mode" };

    printf("Getting GPS information...\n");
    setGPSmode(4);
//    setGPS_NMEAFilter( 0xffff );
    getGPSconfig(om,sizeof(om));
    enableGPS();
    done = 5;
    while( done ) {
        k=i=getGPSlocation(om,sizeof(om));
        done = atoi(om[3].value)?0:5;
        for( i=1, intfmt=strfmt=0; i<k; i++ ) {
            if( !strcmp(om[i].key,"loc_status") )
                printf("Status: %s\n",atoi(om[i].value)?"COMPLETED\n":"IN PROGRESS");
            else if( !strcmp(om[i].key,"latitude") )
                printf(" latitude: %f\n",atof(om[i].value));
            else if( !strcmp(om[i].key,"longitude") )
                printf("longitude: %f\n",atof(om[i].value));
            else if( !strcmp(om[i].key,"timestamp") ) {
                char buf[80];
                time_t rawtime = ascii_to_epoch(om[i].value);
                struct tm *ts = localtime(&rawtime);
                strftime(buf, sizeof(buf), "  Time is: %a %d-%m-%Y %H:%M:%S %Z", ts);
                puts(buf);
                }
            else if( !strcmp(om[i].key,"altitude") )
                printf(" altitude: %d\n",atoi(om[i].value));
            else if( !strcmp(om[i].key,"speed") )
                printf("    speed: %d\n",atoi(om[i].value));
            else if( !strcmp(om[i].key,"accuracy") )
                printf(" accuracy: %d\n",atoi(om[i].value));
            else if( !strcmp(om[i].key,"errno") ) {
                if( atoi(om[i].value) )
                    printf("GPS ERROR! %d\n",atoi(om[i].value));
                }
            else if( !strcmp(om[i].key,"errmsg") ) {
                if( strcmp(om[i].value,"<null>") )
                    printf("GPS ERROR MESSAGE: %s\n",om[i].value);
                }
            else
                printf("(%2d) KEY=%s ; VALUE=%s\n",i,om[i].key,om[i].value);
            }
        sleep(done);
        }
    disableGPS();

}

int command_adc(int argc, const char * const * argv )
{
    adc_handle_t my_adc=(adc_handle_t)NULL;
    float val;
    int i;

    if( argc == 2 ) {
        adc_threshold = atof(argv[1]);
        printf("-set threshold value to %f\n",adc_threshold);
        }

    i=adc_init(&my_adc);
    i=adc_read(my_adc, &val);
    printf("       ADC return value: %f\n", val);
    adc_deinit(&my_adc);
}

//  tx2m2x Y X Z  Tx data Y times every X secs from device Z
int command_tx2m2x(int argc __attribute__((unused)), const char * const * argv )
{
    char*  sensor=NULL;
    int    i, interval;
    int    done, iterations;

    done=0;
    if( argc == 4 ) {
        sensor    = (char*)argv[3]; 
        interval  = (unsigned char)atoi(argv[2]);     //frequency in seconds
        iterations  = (unsigned char)atoi(argv[1]);   //nbr of times to send data
        }
    if( sensor != NULL ) {
        for (i=0; i<_max_m2xfunctions; i++) {
            if ( !strcmp((m2xfunctions[i]).name,strupr(sensor)) ) {
                (m2xfunctions[i]).func(interval, iterations);
                done=1;
                }
            }
        }
    if( !done ){
        printf("Need to specify a sensor, one of:\n");
        for (i=0; i<_max_m2xfunctions-1; i++) 
            printf(" - %s\n",(m2xfunctions[i]).name);
        printf("Command Format: tx2m2x <count> <frequency> <sensor>\n");
        }
}

int command_devid(int argc, const char * const * argv )
{
    if( strlen(argv[1]) )
        strcpy(device_id, (char*)argv[1]);
    printf("Set Device ID to: %s\n",device_id);
}

int command_apikey(int argc, const char * const * argv )
{
    if( strlen(argv[1]) )
        strcpy(api_key, (char*)argv[1]);
    printf("  Set API KEY to: %s\n",api_key);
}

int command_pause(int argc, const char * const * argv )
{
    if( strlen(argv[1]) ) {
        int p = atoi(argv[1]);
        printf("pausing execution for %d seconds.\n",p);
        sleep(p);
        }
}

int command_WWANLED(int argc __attribute__((unused)), const char * const * argv )
{

    int enable=-1;

    if( argc == 2 ) 
        enable = atoi(argv[1]);

    if( enable>=0 )
        wwan_io(enable);
}


int command_spi(int argc __attribute__((unused)), const char * const * argv )
{
    MAX31855 max;
    int err;

    printf("     Internal Temp (c) = %5.2f\n", max.readIntern(true));
    if( err = max.readError() )
        printf("    Errors encountered = 0x%02X\n\n",err);

    printf("Thermocoupler Temp (c) = %5.2f\n", max.readThermo(true));
    if( err = max.readError() )
        printf("    Errors encountered = 0x%02X\n\n",err);

    printf("     Internal Temp (F) = %5.2f\n", max.readIntern(false));
    if( err = max.readError() )
        printf("    Errors encountered = 0x%02X\n\n",err);

    printf("Thermocoupler Temp (F) = %5.2f\n", max.readThermo(false));
    if( err = max.readError() )
        printf("    Errors encountered = 0x%02X\n\n",err);

    return 0;
}


int command_lis2dw12(int argc __attribute__((unused)), const char * const * argv )
{
    void lis2dw12_timer_task(size_t timer_id, void * user_data);
    int k=0, repeats, delay = 0;
    float temp;

    printf("     LIS2DW12 Device id: 0x%02X\n", lis2dw12_getDeviceID());
    printf("   LIS2DW12 12-bit temp: %5.2f\n", lis2dw12_readTemp12());
    printf("   LIS2DW12  8-bit temp: %d\n\n\n", lis2dw12_readTemp8());

    if( argc == 3 ) {
        delay   = atoi(argv[2]);
        repeats = atoi(argv[1]);
        }
    else if( argc == 2 )
        repeats = atoi(argv[1]);
    else
        repeats = 1;

    if (repeats > 1 ) {
        k = 1;
        printf("send %d mesurments with %d second delay between each measurment.",repeats,delay);
        do {
            printf("\nReading #%d:\n",k++);
            printf("LIS2DW12 12-bit temp: %5.2f\n", lis2dw12_readTemp12());
            printf("LIS2DW12  8-bit temp: %d\n\n\n", lis2dw12_readTemp8());
            lis2dw12_ot_acc_data();
            sleep(delay);
            }
        while (--repeats);
        }
    else
       lis2dw12_ot_acc_data();
}

#include "TSYS02D.hpp"
int command_tsys02d(int argc, const char * const * argv )
{
    TSYS02D tsys02;

    if( tsys02.getErr() )
        printf("No TSYS02D detected. (%d)\n",tsys02.getErr());
    else {
        uint8_t *sn = tsys02.getSN();
        int     sne = tsys02.getErr();
        float     t = tsys02.getTemperature(READT_NOHOLD);
        int      te = tsys02.getErr();

        printf("   TSYS02D Temperature reading [SNE=0x%02X, TE=0x%02X]:\n",sne, te);
        printf("     TSYS02D Serial Number: ");
        for( int i=0; i<8; i++ )
            printf("0x%02X ",sn[i]);
        printf("\n                Temp (C/F): %4.2f/%4.2f\n", t,CTOF(t));
        }
}

#include "TSYS01.hpp"
int command_tsys01(int argc, const char * const * argv )
{
    TSYS01 tsys01;

    if( tsys01.getErr() )
        printf("No TSYS01 detected. (%d)\n",tsys01.getErr());
    else {
        float    t  = tsys01.getTemperature();
        int      te = tsys01.getErr();

        printf("   TSYS01 Temperature reading [TE=0x%02X]:\n",te);
        printf("          Temp (C/F): %4.2f/%4.2f\n", t,CTOF(t));
        }
}

int command_kma36(int argc, const char * const * argv )
{
    uint8_t addr = atoi(argv[1]);
    sscanf(argv[1],"%x",&addr);
    KMA36 kma(addr);

    if( kma.getErr() )
        printf("No KMA36 detected. (%d)\n",kma.getErr());
    else {
//        float    t  = tsys01.getTemperature();
//        int      te = tsys01.getErr();
//
//        printf("   TSYS01 Temperature reading [TE=0x%02X]:\n",te);
//        printf("          Temp (C/F): %4.2f/%4.2f\n", t,CTOF(t));
        }
}

