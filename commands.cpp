

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
#include "lis2dw12.h"
#include "binio.h"
#include "mytimer.h"
#include "http.h"
#include "m2x.h"
#include "MAX31855.hpp"

#include "mal.hpp"

#ifdef __cplusplus
extern "C" {
#endif
int gpio_irq_callback(gpio_pin_t pin_name, gpio_irq_trig_t direction);
void set_color(char *);
void wwan_io(int);
#ifdef __cplusplus
}
#endif

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

const char *device_id        = DEFAULT_DEVICE_ID;
const char *api_key          = DEFAULT_API_KEY;
const char *adc_stream_name  = DEFAULT_ADC_API_STREAM;
const char *temp_stream_name = DEFAULT_TEMP_API_STREAM;

const cmd_entry mon_command_table[] =
  {
  max_moncommands, ";",   
     "A ';' denotes that the rest of the line is a comment and not to be executed.",            NULL,
  0, ";",           
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
  0, "MAX31855",      
     "MAX31855      Read the MAX31855 Thermocouple-to-Digital Converter (SPI BUS on PMOD)",     command_spi,
  0, "GPS",         
     "GPS           Display GPS information                                             ",      command_gps,
  0, "ADC",         
     "ADC           Read ADC                                                            ",      command_adc,
  0, "I2CPEEK",     
     "I2CPEEK <dev> <reg> <nbr_bytes> Display <nbr_bytes> returned by reading <reg> on I2C bus",command_i2cpeek,
  0, "I2CPOKE",     
     "I2CPOKE <dev> <reg> <b1> <b2> <b3> <b4> <b5> <b6> write up to 6 bytes to <reg> on I2C bus",command_i2cpoke,
  0, "DEBUG",       
     "DEBUG X V     Set or Clear a debug flag X=set or clr, V = flag to set or clear",          command_dbg,
  0, "WNC",         
     "wnc           Enters WNC testing command mode",                                           command_wnctest,
  0, "DODEMO",      
     "dodemo        Run the Demo Program (hold user key for > 3 sedonds to return to monitor)", command_demo_mode,
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
  0, "MON",         "mon           Enter interactive monitor mode",            command_iotmon,
  0, "EXIT",        "exit          End Program execution",                     command_exit
  };
#define _MAX_IOTCOMMANDS	(sizeof(fac_command_table)/sizeof(cmd_entry))

int command_help(int argc, const char * const * argv );
int command_gpio(int argc, const char * const * argv );
int command_blink(int argc, const char * const * argv );
int command_sndat(int argc, const char * const * argv );
int command_facttest(int argc, const char * const * argv );
int command_hts221(int argc __attribute__((unused)), const char * const * argv );
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

void do_hts2m2x(void);

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

//
// debug set/clr flag

int command_dbg(int argc __attribute__((unused)), const char * const * argv ) 
{
    char *action=NULL, *flag=NULL;
    int a=0, done=0, idx;

    if( argc < 3 ) {
        printf("Possible flags are:(0x%04X)\n",dbg_flag);
        printf("  CURL - display information about CURL operations\n");
        printf("  FLOW - display information about CURL operations\n");
        printf("  M2X - display informatioin about\n");
        printf("  TIMER - display informatioin about\n");
        printf("  LIS2DW12 - display informatioin about\n");
        printf("  HTS221 - display informatioin about\n");
        printf("  BINIO - display informatioin about\n");
        printf("  MAL -  display information to/from the MAL\n");
        printf("  I2C -  display information on the I2C bus\n");
        printf("  SPI -  display information on the SPI bus\n");
        return 0;
        }

    action = (char*)argv[1];
    a |= !strcmp(strupr(action),"SET")?1:0;
    a |= !strcmp(strupr(action),"1")?1:0;
    a |= !strcmp(strupr(action),"CLR")?2:0;
    a |= !strcmp(strupr(action),"0")?2:0;

    flag   = (char*)argv[(idx=2)];

    while( a && action != NULL && flag != NULL && idx < argc ) {
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
    const char *radio_mode[] = {"No service", "3G", "4G" };
    const char *radio_state[]= { 
        "No Registered", "Registered with network", 
        "searching", "registration denied", "unknown" };
    const char *xs_state[]   = {"Unknown","Attached","Detached"};
    const char *roaming[]    = {"Home", "Roaming", "Unknown"};

    printf(" WWAN Status (");
    start_data_service();

    printf("received %d key/value pairs):\n", get_wwan_status(om, 20));
    printf("             Radio Mode: %s(%s)\n", radio_mode[atoi(om[3].value)], om[3].value);
    printf("        Signal strength; %s\n", om[4].value);
    printf("           Signal Level: %s\n", om[6].value);
    printf("                  state: %s(%s)\n", radio_state[atoi(om[7].value)], om[7].value);
    printf(" Circuit-switched state: %s(%s)\n", xs_state[atoi(om[8].value)], om[8].value);
    printf("  Packet-switched state: %s(%s)\n", xs_state[atoi(om[9].value)], om[9].value);
    printf("     Registration state: %s(%s)\n", roaming[atoi(om[16].value)], om[16].value);

//    for(k=0; k<20; k++) 
//        printf("#%02d: {%s, %s}\n", k, om[k].key, om[k].value);
}

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
    printf("              WNC Model: %s\n",mySystem.model.c_str());
    mySystem.firmVer=getFirmwareVersion(om, 4);
    printf("       Firmware Version: %s\n",mySystem.firmVer.c_str());
    mySystem.malwarVer=getMALManVer(om, 4);
    printf("            MAL Version: %s\n",mySystem.malwarVer.c_str());
    mySystem.opMode = getOperatingMode(om, 4);
    printf("         Operating Mode: %s\n",om_str[atoi(mySystem.opMode.c_str())]);
    mySystem.ip=get_ipAddr(om, 4);
    printf("                     IP: %s\n",mySystem.ip.c_str());
    mySystem.iccid=getICCID(om, 4);
    printf("                  ICCID: %s\n",mySystem.iccid.c_str());
    mySystem.imei=getIMEI(om, 4);
    printf("                   IMEI: %s\n",mySystem.imei.c_str());
    mySystem.imsi=getIMSI(om, 4);
    printf("                   IMSI: %s\n",mySystem.imsi.c_str());
    mySystem.msisdn=getMSISDN(om, 4);
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
        case 92:  //GPIO_02
        case 101:  //GPIO_03
        case 102:  //GPIO_07
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
        case 92:  //GPIO_02
        case 101:  //GPIO_03
        case 102:  //GPIO_07
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


#define CTOF(x)  ((x)*(float)1.8+32) 
int command_hts221(int argc __attribute__((unused)), const char * const * argv )
{
    int repeats, delay = 0;
    float temp;

    if( argc == 3 ) {
        delay   = atoi(argv[2]);
        repeats = atoi(argv[1]);
        }
    else if( argc == 2 )
        repeats = atoi(argv[1]);
    else
        repeats = 1;

    printf("send %d mesurments with %d second delay between each measurment.\n",repeats,delay);
    do {
        temp  = hts221_getTemp();
        printf("   HTS221 Device id: 0x%02X\n", hts221_getDeviceID());
        printf(" HTS221 Temperature: %3.2fc/%3.2ff\n", temp, CTOF(temp));
        printf("    HTS221 Humidity: %2.1f\n", hts221_getHumid()/10);
        sleep(delay);
        }
    while (--repeats);
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

    command_demo_mode(argc, argv);
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
sleep(2);
    getGPSlocation(om,sizeof(om)/sizeof(char*));
    setGPSmode(4);
    setGPS_NMEAFilter( 0xff );
    getGPSconfig(om,  sizeof(om)/sizeof(char*));
    disableGPS();

}

int command_adc(int argc, const char * const * argv )
{
    adc_handle_t my_adc=(adc_handle_t)NULL;
    float val;

    adc_init(&my_adc);
    adc_read(my_adc, &val);
    printf("       ADC return value: %f\n", val);
    adc_deinit(&my_adc);
}


int command_facttest(int argc __attribute__((unused)), const char * const * argv )
{
    extern struct timespec key_press, key_release, keypress_time;
    extern volatile gpio_level_t button_press;
    extern GPIOPIN_IN gpio_input;
    extern gpio_handle_t user_key, red_led, green_led, blue_led;

    float temp  = hts221_getTemp();

//Test: Display WNC part info and SIM card info
//Test: Display LTE RSSI information
    command_WNCInfo(0,NULL);
    printf("\n");
    command_WWANStatus(0,NULL);
    printf("\n");

//Test: Read ADC: Ambient Light Sensor 
    command_adc(0,NULL);
    printf("\n");

//Test: Read/Display Temprature information
    printf("       HTS221 Device id: 0x%02X\n", hts221_getDeviceID());
    printf("     HTS221 Temperature: %3.2fc/%3.2ff\n", temp, CTOF(temp));
    printf("        HTS221 Humidity: %2.1f\n\n", hts221_getHumid()/10);

//Test: Read ID register from LIS2DW12
    printf("     LIS2DW12 Device id: 0x%02X\n\n", lis2dw12_getDeviceID());

//Test: User Push-Button/LED test
#define WAIT_4_BUTTON {while(!button_press); while(button_press);} 
    binario_io_close();
    gpio_init( GPIO_PIN_92,  &red_led );
    gpio_init( GPIO_PIN_101, &green_led );
    gpio_init( GPIO_PIN_102, &blue_led );
    gpio_init( GPIO_PIN_98,  &user_key );  //SW3

    gpio_dir(red_led,   GPIO_DIR_OUTPUT);
    gpio_dir(green_led, GPIO_DIR_OUTPUT);
    gpio_dir(blue_led,  GPIO_DIR_OUTPUT);
    
    button_press = (gpio_level_t)0;
    gpio_dir(user_key, GPIO_DIR_INPUT);
    gpio_irq_request(user_key, GPIO_IRQ_TRIG_BOTH, gpio_irq_callback);

    set_color((char*)"OFF");
    printf("Press user button to being...\n");
    while( !button_press ); /* wait for a button press */
    while( button_press ); /* wait for the user to release the button */

    printf(" Press user button to cycle through all colors:\n");
    set_color((char*)"BLUE");
    printf(" Color set to BLUE\n");
    printf( "Button pressed for %d seconds. Now GREEN\n", keypress_time.tv_sec);
    set_color((char*)"GREEN");
    WAIT_4_BUTTON;
    printf( "Button pressed for %d seconds. Now BLUE\n", keypress_time.tv_sec);
    set_color((char*)"BLUE");
    WAIT_4_BUTTON;
    printf( "Button pressed for %d seconds. Now MAGENTA\n", keypress_time.tv_sec);
    set_color((char*)"MAGENTA");
    WAIT_4_BUTTON;
    printf( "Button pressed for %d seconds. Now TURQUOISE\n", keypress_time.tv_sec);
    set_color((char*)"TURQUOISE");
    WAIT_4_BUTTON;
    printf( "Button pressed for %d seconds. Now RED\n", keypress_time.tv_sec);
    set_color((char*)"RED");
    WAIT_4_BUTTON;
    printf( "Button pressed for %d seconds. Now WHITE\n", keypress_time.tv_sec);
    set_color((char*)"WHITE");
    WAIT_4_BUTTON;
    printf( "Button pressed for %d seconds. Now OFF\n", keypress_time.tv_sec);
    set_color((char*)"OFF");
    printf( "WWAN LED now on.\n");
    wwan_io(1);
    WAIT_4_BUTTON;
    wwan_io(0);
    printf( "LED Testing completed.\n");

    gpio_deinit( &red_led);
    gpio_deinit( &green_led);
    gpio_deinit( &blue_led);
    gpio_deinit( &user_key);

    binary_io_init();

//
// The following tests are TBD at this time.
//Test: Display GPS RSSI information
//Test: Read I2C Pmod: TBD 
//Test: Read SPI Pmod: TBD 

}


//  tx2m2x Y X Z  Tx data Y times every X secs from device Z
int command_tx2m2x(int argc __attribute__((unused)), const char * const * argv )
{
    char*  sensor=NULL;
    int    i, interval;
    int    iterations;

    if( argc == 4 ) {
        sensor    = (char*)argv[3]; 
        interval  = (unsigned char)atoi(argv[2]);  //frequency in seconds
        iterations  = (unsigned char)atoi(argv[1]);   //nbr of times to send data
        }
    if( sensor != NULL ) {
        for (i=0; i<_max_m2xfunctions; i++) {
            if ( !strcmp((m2xfunctions[i]).name,strupr(sensor)) )
                (m2xfunctions[i]).func(interval, iterations);
            }
        }
    else {
        printf("Need to specify a sensor, one of:\n");
        for (i=0; i<_max_m2xfunctions-1; i++) 
            printf(" - %s\n",(m2xfunctions[i]).name);
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

    printf("Testing MAX31855.\n");
    printf("Thermocoupler Temp (c) = %5.2f\n",max.readThermo(1));
    printf("Thermocoupler Temp (F) = %5.2f\n",max.readThermo(0));
    printf("Internal Temp (c) = %5.2f\n",max.readIntern(1));
    printf("Internal Temp (F) = %5.2f\n",max.readIntern(0));
    printf("Errors encountered = 0x%02X\n",max.readError());
    return 0;
}

