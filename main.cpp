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
#include "binio.h"


sysinfo mySystem;

void my_putchar(const char *c)
{
    printf("%s",c);
}

int main(int argc, char *argv[]) 
{
    extern void print_banner(void);
    int process_command (int argc, const char * const * argv);
    int c;
    void app_exit(void);

    int headless=false;

    while((c=getopt(argc,argv,"f")) != -1 )
        switch(c) {
           case 'f':
               headless=true;
               break;
           case '?':
               if (optopt == 'c')
                 fprintf (stderr, "Option -%c requires an argument.\n", optopt);
               else if (isprint (optopt))
                 fprintf (stderr, "Unknown option `-%c'.\n", optopt);
               else
                 fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
               app_exit();

           default:
               abort();
           }

    
    binary_io_init();

    if (hts221_initialize() < 0) {
        fprintf( stderr, "HTS221 initialization failed\n");
        app_exit();
        }

    if( headless ){
        command_headless(argc, argv );
        exit(0);
        }
    
    print_banner();
    set_cmdhandler(&my_putchar, (char*)MONITOR_PROMPT, strlen(MONITOR_PROMPT), app_exit, &process_command, NULL, mon_command_table);
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
printf("exiting...\n");
    exit(EXIT_SUCCESS);
}

