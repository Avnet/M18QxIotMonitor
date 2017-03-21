/* =====================================================================
   Copyright © 2016, Avnet (R)

   Contributors:
     * James M Flynn, www.em.avnet.com 
 
   Licensed under the Apache License, Version 2.0 (the "License"); 
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, 
   software distributed under the License is distributed on an 
   "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
   either express or implied. See the License for the specific 
   language governing permissions and limitations under the License.

    @file          WNC_GMDemo1.cpp
    @version       1.0
    @date          Jan 2017

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

#include <string>

#include "MyBuffer.hpp"
#include "microrl_config.h"
#include "microrl.h"
#include "iot_monitor.h"
#include "hts221.h"
#include "lis2dw12.h"
#include "binio.h"


sysinfo mySystem;
int headless=false;
static struct termios oldt, newt;

void my_putchar(const char *c)
{
    printf("%s",c);
}

unsigned int dbg_flag = 0;

int main(int argc, char *argv[]) 
{
    extern void print_banner(void);
    int process_command (int argc, const char * const * argv);
    int c;
    void app_exit(void);
#ifdef _USE_COMPLETE
    char ** complet (int argc, const char * const * argv);
#endif

    tcgetattr( STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr( STDIN_FILENO, TCSANOW, &newt );

    while((c=getopt(argc,argv,"fd:a:t:l:v:")) != -1 )
        switch(c) {
           case 'd': //device_id
               device_id = optarg;
               printf("-setting Device ID to [%s]\n",device_id);
               break;
           case 'a': //api_key
               api_key = optarg;
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
           case 'v':
               sscanf(optarg,"%x",&dbg_flag);
               printf("-debug flag set to 0x%04X\n",dbg_flag);
               break;
           case 'f':
               headless=true;
               break;
           case '?':
               if (optopt == 'a' || optopt == 's' ||optopt == 'd')
                 fprintf (stderr, "Option -%c requires an argument.\n", optopt);
               else
                 fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
               app_exit();

           default:
               abort();
           }

    
    binary_io_init();

    c=lis2dw12_initialize();
    if( dbg_flag & DBG_LIS2DW12 )
        printf("-LIS2DW12: lis2dw12_initialize() = %d\n",c);

    c=lis2dw12_getDeviceID();
    if( dbg_flag & DBG_LIS2DW12 )
        printf("-LIS2DW12: lis2dw12_getDeviceID()= 0x%02X\n",c);

    c=hts221_initialize();
    if( dbg_flag & DBG_HTS221 )
        printf("-HTS221: hts221_initialize() = %d\n", c);

    c=hts221_getDeviceID();
    if( dbg_flag & DBG_HTS221 )
        printf("-HTS221: hts221_getDeviceID() = 0x%02X\n", c);

    if( headless ){
        printf("-running DEMO mode\n");
        command_headless(argc, argv );
        }
    
    print_banner();
    set_cmdhandler(&my_putchar, (char*)MONITOR_PROMPT, strlen(MONITOR_PROMPT), app_exit, &process_command, complet, mon_command_table);
    new_line_handler(prl);

    while(1)
    {
        int c;
        c=getchar();
        microrl_insert_char(prl, c);
    }
 }


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

