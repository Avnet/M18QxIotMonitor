

#include <unistd.h>
#include <cctype>
#include <cstddef>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <functional>


#ifdef __cplusplus
extern "C" {
#endif

#include <hwlib/hwlib.h>

#ifdef __cplusplus
}
#endif

#include "iot_monitor.h"
#include "microrl_config.h"
#include "microrl.h"
#include "hts221.h"
#include "binio.h"
#include "mytimer.h"
#include "http.h"

#include "mal.hpp"

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

const cmd_entry mon_command_table[] =
  {
  max_moncommands, ";",           "A ';' denotes that the rest of the line is a comment and not to be executed.",NULL,
  0,               ";",           "GPIO's are: GPIO_PIN_95, GPIO_PIN_2, GPIO_PIN_7, GPIO_PIN_3 currently supported.\n",  NULL,
  0, "?",           "?             Displays this help screen",                                              command_help,
  0, "HELP",        "help          Displays this help screen",                                              command_help,
  0, "GPIO",        "gpion # # #   Drives GPIO pin high/low as desired (GPIO GPIOx 0/1)",                    command_gpio,
  0, "BLINK",       "blink # #     Alternates GPIO between 0/1 @ requested rate (secs)<interval=0 to stop>",command_blink, 
  0, "HTS221",      "HTS221        Display information about the HTS221 sensor.",                           command_hts221,
  0, "GPS",         "GPS           Display GPS information                                             ",   command_gps,
  0, "ADC",         "ADC #         Read ADC #                                                          ",   command_adc,
  0, "I2CPEEK",     "I2CPEEK <reg> <nbr_bytes> Display <nbr_bytes> returned by reading <reg> on I2C bus",   command_i2cpeek,
  0, "I2CPOKE",     "I2CPOKE <reg> <b1> <b2> <b3> <b4> <b5> <b6> write up to 6 bytes to <reg> on I2C bus",  command_i2cpoke,
  0, "FACT",        "fact          Performs a connect test with the WNC Module & SIM card",                 command_factest,
  0, "EXIT",        "exit          End Program execution",                                                  command_exit,
  };
#define _MAX_MONCOMMANDS	(sizeof(mon_command_table)/sizeof(cmd_entry))

const cmd_entry fac_command_table[] =
  {
  max_iotcommands, ";",           "A ';' denotes that the rest of the line is a comment and not to be executed.",NULL,
  0,               ";",           "\n",                                                                          NULL,
  0, "?",           "?             Displays this help screen",                 command_help,
  0, "HELP",        "help          Displays this help screen",                 command_help,
  0, "WNCINFO",     "wncinfo       Displays the WNC module information",       command_WNCInfo, 
  0, "TX2M2X",      "tx2m2x ## X   Tx data from X at ## sec intervals to M2X", command_tx2m2x, 
  0, "MON",         "mon           Enter interactive monitor mode",            command_iotmon,
  0, "EXIT",        "exit          End Program execution",                     command_exit,
  };
#define _MAX_IOTCOMMANDS	(sizeof(fac_command_table)/sizeof(cmd_entry))

int command_help(int argc, const char * const * argv );
int command_gpio(int argc, const char * const * argv );
int command_blink(int argc, const char * const * argv );
int command_sndat(int argc, const char * const * argv );
int command_baud(int argc, const char * const * argv );
int command_hts221(int argc __attribute__((unused)), const char * const * argv );
int command_i2cpeek(int argc, const char * const * argv );
int command_i2cpoke(int argc, const char * const * argv );
int command_peek(int argc, const char * const * argv );
int command_poke(int argc, const char * const * argv );
int command_comscmd(int argc, const char * const * argv);
int command_http_put(int argc, const char * const * argv);
int command_http_get(int argc, const char * const * argv);
int command_iotmon(int argc, const char * const * argv );
int command_factest(int argc, const char * const * argv );
int command_tx2m2x(int argc, const char * const * argv );
int command_WNCInfo(int argc, const char * const * argv );
int command_gps(int argc, const char * const * argv );
int command_adc(int argc, const char * const * argv );
int command_exit(int, char const* const*);

void sigint_cb (void);

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
    long max = (long)current_table->func_p;
    cmd_entry **tptr = &current_table;

    completion_array[0] = NULL;

    // if there is token in cmdline
    if (argc == 1) {
        // get last entered token
        char * bit = (char*)argv [argc-1];
        for (int i = 0; i < max; i++) {                     // iterate through available cmds and match it
          if (strstr((tptr[i])->commandp, strupr(bit)) == mon_command_table[i].commandp) {   // if token matches 
            completion_array[j++] = (char*)(tptr[i])->commandp;                 // add it to completion set
            }
        }
      }
  else { // if there is no token in cmdline, just print all available commands
     for (; j < max; j++) {
       completion_array[j] = (char*)(tptr[j])->commandp;
       }
  }

  // note! last ptr in array always must be NULL!!!
  completion_array[j] = NULL;       // return set of variants
  return completion_array;
  }
#endif


void print_banner(void) {

//  my_puts("\033[2J");
  my_puts("----------------------------------------------------------------------------");
  my_puts("      _____           Copyright (c) 2017 Avnet");
  my_puts("     *     *");
  my_puts("    *____   *");
  my_puts("   * *===*   *==*");
  my_puts("  *___*===*___**");
  my_puts("       *======*");
  my_puts("        *====*");
  my_puts(" AVNET - AT&T Global Module IoT Monitor");
  my_puts(" Version 00.99 // 01-31/2017");
  my_puts(" Hardware Supported: WNC M18Qx Cellular Data Module");
  my_puts("----------------------------------------------------------------------------\n");
  
  return ;
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
        long max = current_table->max_elements;
        cmd_entry *tptr = current_table;

        if (*argv[0] != ';') {
          if (*argv[0] != '?') {
            while ((strcmp(strupr((char*)argv[0]), (tptr[i]).commandp) != 0) && ++i < max) 
              /* check next */;

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
    my_puts(((tptr[1]).help_str));
    my_puts("Command          Description");
    my_puts("============= ========================================================================\n");

    for( i=2; i<max; i++ )
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
    while (*p=toupper(*p)) p++;
    return s;
}



int command_exit(int, char const* const*)
{
    void app_exit(void);

    app_exit();
}

//for(k=0; k<3; k++) printf("#%02d: {%s, %s}\n", k,om[k].key,om[k].value);
int command_WNCInfo(int, char const* const*)
{
    json_keyval om[4];
    const char *om_str[] = {
        "0-Online",
        "1-Low Power",
        "2-Factory Test mode",
        "3-Offline",
        "4-Resetting",
        "5-Shutting down" };

    printf(" WNC Information:\n");
    start_data_service();

    mySystem.model=getModelID(om, 4);
    printf("       WNC Model: %s\n",mySystem.model.c_str());
    mySystem.firmVer=getFirmwareVersion(om, 4);
    printf("Firmware Version: %s\n",mySystem.firmVer.c_str());
    mySystem.malwarVer=getMALManVer(om, 4);
    printf("     MAL Version: %s\n",mySystem.malwarVer.c_str());
    mySystem.opMode = getOperatingMode(om, 4);
    printf("  Operating Mode: %s\n",om_str[atoi(mySystem.opMode.c_str())]);
    mySystem.ip=get_ipAddr(om, 4);
    printf("              IP: %s\n",mySystem.ip.c_str());
    mySystem.iccid=getICCID(om, 4);
    printf("           ICCID: %s\n",mySystem.iccid.c_str());
    mySystem.imei=getIMEI(om, 4);
    printf("            IMEI: %s\n",mySystem.imei.c_str());
    mySystem.imsi=getIMSI(om, 4);
    printf("            IMSI: %s\n",mySystem.imsi.c_str());
    mySystem.msisdn=getMSISDN(om, 4);
    printf("          MSISDN: %s\n",mySystem.msisdn.c_str());
}

int command_gpio(int argc __attribute__((unused)), const char * const * argv )
{
    int indx   = atoi(&argv[1][9]);
    int state  = atoi(argv[2]);
    int k=0, done;

    switch(indx) {
        case 2:  //GPIO_02
        case 3:  //GPIO_03
        case 7:  //GPIO_07
        case 95: //GPIO_95
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
     }
}

int command_blink(int argc __attribute__((unused)), const char * const * argv )
{
    int indx = atoi(&argv[1][9]);
    int rate = atoi(argv[2]);
    int k=0, done, state;
    
 //   do_gpio_blink( 0, 8);
//    do_gpio_blink( 1, 4);
//    do_gpio_blink( 2, 2);
//    sleep(10);
//    do_gpio_blink( 0, 0 );
//    sleep(10);
//    do_gpio_blink( 1, 0 );
//    sleep(10);
//    do_gpio_blink( 2, 0 );
//    return 0;

    switch(indx) {
        case 2:  //GPIO_02
        case 3:  //GPIO_03
        case 7:  //GPIO_07
        case 95: //GPIO_95
            do {
                done = (gpios[k].nbr == indx);
                k++;
                }
            while( k < _max_gpiopins && !done);
            k--; //k will always be 1 more than it should be, but done will be TRUE if found
//printf("blinking gpio_pin_%d\n",gpios[k].nbr);
                
            if (done) {  //ok found a GPIO_PIN_x to toggle
                do_gpio_blink( k, rate );
                break;
                }

        default:
            printf("ERROR: unknown binary i/o GPIO_PIN_%s\n",argv[1][9]);
            break;
     }
}


#define CTOF(x)  ((x)*(float)1.8+32) 
int command_hts221(int argc __attribute__((unused)), const char * const * argv )
{
    int repeats = atoi(argv[1])+1;
    int delay   = atoi(argv[2]);
    float temp  = hts221_getTemp();

    do {
        printf(" HTS221 Temperature: %3.2fc/%3.2ff\n", temp, CTOF(temp));
        printf("    HTS221 Humidity: %2.1f\n", hts221_getHumid());
        sleep(delay);
        }
    while (--repeats);
}

int command_i2cpeek(int argc __attribute__((unused)), const char * const * argv )
{
  unsigned char nbr  = (unsigned char)atoi(argv[2]);  //get number of bytes to read
  unsigned char buf[100];                 //use a 100 byte working buffer
  char    reg;
  int     i;

  memset(buf,0x00,sizeof(buf));
  sscanf(argv[1],"%x",(int)&reg);

  hts221_read(reg, buf, nbr);

  for (i=0; i<nbr; i+=8) 
    printf("%04X: %02X %02X %02X %02X %02X %02X %02X %02X %2c %2c %2c %2c %2c %2c %2c %2c\n\r",
               reg,buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7],
               buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7]);
  return 2;
}


int command_tx2m2x(int argc __attribute__((unused)), const char * const * argv )
{
    char*  sensor    = (char*)argv[1]; 
    int    i, interval  = (unsigned char)atoi(argv[2]);  //frequency in seconds

    for (i=0; i<_max_m2xfunctions; i++) {
        if ( !strcmp((m2xfunctions[i]).name,strupr(sensor)) )
            (m2xfunctions[i]).func(interval);
        }
}

int command_i2cpoke(int, char const* const*)
{
    printf("called i2cpoke\n");
}

int command_factest(int, char const* const*)
{
    current_table = (cmd_entry*)fac_command_table;
    microrl_setprompt (prl, FACTORY_PROMPT, strlen(FACTORY_PROMPT)) ;
    printf("called factest\n");
}

int command_headless(int argc, const char * const * argv )
{
    void app_exit();

    do_gpio_blink( 1, 1 );
    sleep(60);
    app_exit();
}


int command_gps(int argc, const char * const * argv )
{
    json_keyval om[4];
    const char *mode[] = {
        "2-MS-based mode",
        "3-MS assisted mode",
        "4-Standalone mode" };

    printf("calling gps stuff\n");

    enableGPS();
sleep(5);
    getGPSlocation(om,sizeof(om)/sizeof(char*));
    setGPSmode(4);
    setGPS_NMEAFilter( 0xff );
    getGPSconfig(om,  sizeof(om)/sizeof(char*));
    disableGPS();

}

int command_adc(int argc, const char * const * argv )
{
    adc_handle_t my_adc;
    float val;

    printf("called for adc\n");
    printf("adc_init return: %d\n", adc_init(&my_adc));

    printf("adc_read return: %d\n", adc_read(my_adc, &val));
    printf("value return: %f\n", val);

    printf("adc_deinit return: %d\n", adc_deinit(&my_adc));

}

