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

    @file          main.cpp
    @version       1.0
    @date          Sept 2017

======================================================================== */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>
#include <sys/un.h>
#include <sys/syscall.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>
#include "mal.hpp"

#include <string>

#include "iot_monitor.h"
#include "microrl_config.h"
#include "microrl.h"
#include "HTS221.hpp"
#include "lis2dw12.h"
#include "binio.h"

static struct termios oldt, newt; //used to store and change the terminal attributes during program usage

void do_emissions_test(void);  //this test is for performing CE/FCC emissions testing
int emission_test = 0;

//
// This function is used to output data to the terminal as it is typed
//
static void my_putchar(const char *c)
{
    printf("%s",c);
}

//
// display quick usage overview of how to use iot_monitor
//
void usage (void)
{
    printf(" The 'iot_monitor' program can be started with several options:\n");
    printf(" -m  : Disable sending data to M2X.  Used during execution of Demo application\n");
    printf("       Disable GPS testing when used with factory test mode flag\n");
    printf(" -r #: Perform Factory Test Mode, ensure test runs for # seconds to observe LEDs\n");
    printf(" -e  : run the extended I/O test when expansion board is attached\n");
    printf(" -d X: Set the Device id to 'X'\n");
    printf(" -a X: Set the API key to 'X'\n");
    printf(" -l X: Set the ADC Stream name\n");
    printf(" -t X: Set the Temp Stream name\n");
    printf(" -u X: Set the Demo application URL to 'X'\n");
    printf(" -v X: Display debug info for 'X', where 'X' is a bit field composed of:\n");
    printf("          1 = CURL \n");
    printf("          2 = FLOW \n");
    printf("          4 = M2X \n");
    printf("         10 = TIMER\n");
    printf("         20 = LIS2DW12\n");
    printf("         40 = HTS221\n");
    printf("        100 = BINIO\n");
    printf("        200 = MAL \n");
    printf("        400 = DEMO\n");
    printf("        800 = I2C\n");
    printf("       1000 = SPI\n");
    printf("       NOTE: values can be combined, e.g., 23 is LIS2DW12+FLOW+CURL\n");
    printf(" -f #: Run the Demo application. Loop every # seconds\n");
    printf(" -q #: Run the Quick Start application. Delay # seconds between M2X postings.\n");
    printf(" -x #: Set the ADC Threshold for demo mode to # (defaults to 0.1)\n");
    printf(" -g #: Set a GPS timeout\n");
    printf(" -9  : Run Emissons Test \n");
    printf(" -?  : Display usage info\n");
}

static char demo_url[100];

int main(int argc, char *argv[]) 
{
    HTS221        *hts221;
    extern float  adc_threshold;
    extern int    GPS_TO;
    int           c, qsa=0;

    void print_banner(void);
    int  process_command (int argc, const char * const * argv);
    int  quickstart_app(int argc, const char * const * argv );
    void app_exit(void);

#ifdef _USE_COMPLETE
    char ** complet (int argc, const char * const * argv);
#endif

    tcgetattr( STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr( STDIN_FILENO, TCSANOW, &newt );

    strcpy(device_id,  "");
    strcpy(api_key,  "");
    memset(demo_url,0x00,sizeof(demo_url));

    while((c=getopt(argc,argv,"?q:9mex:f:d:a:t:l:v:r:u:g:s")) != -1 )
        switch(c) {
           case '9': //run in FCC mode
               emission_test = 1;
               break;
           case 'g': //set a GPS timeout value
               sscanf(optarg,"%d",&GPS_TO);
               printf("-GPS Time Out set to %d seconds\n",GPS_TO);
               break;
           case 'm': //send data to M2X
               doM2X=false;
               break;
           case 'e': //extended factory test
               extendedIO=1;
               break;
           case 'r': //factory test
               ft_mode=1;
               sscanf(optarg,"%d",&ft_time);
               break;
           case 'd': //device_id
               strcpy(device_id, optarg);
               printf("-setting Device ID to [%s]\n",device_id);
               break;
           case 'a': //api_key
               strcpy(api_key, optarg);
               printf("-setting API Key to [%s]\n",api_key);
               break;
           case 'l': //light sensor stream_name
               adc_stream_name = optarg;
               printf("-setting Stream Name to [%s]\n",adc_stream_name);
               break;
           case 't': //temp stream_name
               temp_stream_name = optarg;
               printf("-setting Stream Name to [%s]\n",temp_stream_name);
               break;
           case 'u':
               headless=1;
               strcpy(demo_url,optarg);
               printf("-using URL [%s] for demo.\n",demo_url);
               break;
           case 'v':
               sscanf(optarg,"%x",&dbg_flag);
               printf("-debug flag set to 0x%04X\n",dbg_flag);
               break;
           case 'f':
               headless=1;
               sscanf(optarg,"%d",&headless_timed);
               break;
           case 'q':
               qsa = 1;  //signal to run Quick Start Application
               sscanf(optarg,"%d",&headless_timed);
               break;
           case 'x':
               sscanf(optarg,"%f",&adc_threshold);
               break;
           case 's':
               headless = 2;
               break;
           case '?':
               if (optopt == 'a' || optopt == 'd')
                 fprintf (stderr, "Option -%c requires an argument.\n", optopt);
               else if (optopt == '\0')
                   usage();
               else
                   fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
               app_exit();

           default:
               abort();
           }

    if( ft_mode ) {
        command_facttest(1, argv);
        app_exit();
        }
    
    if( emission_test ) {
        do_emissions_test();
        app_exit();
        }

    c=start_data_service();
    while ( c < 0 ) {
        printf("WAIT: starting WNC Data Module (%d)\n",c);
        sleep(10);
        c=start_data_service();
        }

    monitor_wwan();
    binary_io_init();

    c=lis2dw12_initialize();
    if( dbg_flag & DBG_LIS2DW12 ) {
        printf("-LIS2DW12: lis2dw12_initialize() = %d\n",c);
        lis2dw12_regdump();
        }

    c=lis2dw12_getDeviceID();
    if( dbg_flag & DBG_LIS2DW12 )
        printf("-LIS2DW12: lis2dw12_getDeviceID()= 0x%02X\n",c);

    if( qsa ) {
        quickstart_app(headless_timed, (const char* const *)demo_url );
        app_exit();
        }

    hts221 = new HTS221;
    htsdev = (void*)hts221;

    c=hts221->getDeviceID();
    if( dbg_flag & DBG_HTS221 )
        printf("-HTS221: hts221_getDeviceID() = 0x%02X\n", c);

    if( headless ) {
        doM2X=false;
        command_headless(0, (const char* const *)demo_url );
        app_exit();
        }
    
    print_banner();

    set_cmdhandler(&my_putchar, (char*)MONITOR_PROMPT, strlen(MONITOR_PROMPT), 
                   app_exit, &process_command, complet, mon_command_table);
    new_line_handler(prl);

    while(1)
    {
        int c;
        c=getchar();
        microrl_insert_char(prl, c);
    }
}


//
// Because can be used by both C++ and C files, make sure to define it as a C
// function. all it does is output a BS to the terminal. Its intended usage is
// when asyncronus message text is output to the user.  This will help restore
// the prompt for the user to avoid confusion.
//
#ifdef __cplusplus
extern "C" {
#endif

void doNewLine(void)
{
    microrl_insert_char(prl, ' ');
    microrl_insert_char(prl, KEY_BS);
}

#ifdef __cplusplus
}
#endif


//
// two ways to leave this program are either with a ^C (if the IOT Application is running) or with an "EXIT" command
// from the monitor.  In both cases the following function is called...

void app_exit(void)
{
//    clean-up, free any malloc'd memory, exit
    binario_io_close();
    printf("-exiting...\n");

    tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
    exit(EXIT_SUCCESS);
}

