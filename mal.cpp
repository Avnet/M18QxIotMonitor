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

    @file          mal.cpp
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
#include <time.h>
#include <sys/un.h>
#include <sys/syscall.h>
#include <sys/socket.h>

#include <string>

#ifdef __cplusplus
extern "C" {
#endif

#include <hwlib/hwlib.h>

int mal_busy;   //signal that the MAL is busy incase the WWAN routines try to use it

#ifdef __cplusplus
}
#endif

#include <json-c/json.h>

#include "iot_monitor.h"
#include "mal.hpp"

#define JSON_SOCKET_ADDR	"/tmp/cgi-2-sys"

// 
// This function opens a socket to the MAL manager, then sends the JSON command to it.  It will
// then wait for a response then close the socket and return.
//
// Inputs:
//     json_cmd     : json string with command to be sent to the MAL manager
//     json_resp    : json string that will contain the response if needed
//     len_json_resp: space availble for the response
//     wait_resp    : TRUE if we should wait for a response
//
// Returns:
//      will return <0 if an error occurs, othersize 0.

int send_mal_command(char *json_cmd, char *json_resp, int len_json_resp, uint8_t wait_resp) 
{
    int client_socket;
    socklen_t addr_length;
    struct sockaddr_un addr;

    mal_busy = 1;
    strcpy(addr.sun_path, JSON_SOCKET_ADDR);    // max 108 bytes
    addr.sun_family = AF_UNIX;
    addr_length = SUN_LEN(&addr);

    if ((client_socket=socket(AF_UNIX, SOCK_STREAM, 0)) < 0)  {
        mal_busy = 0;
        return -1;
        }

    if (connect(client_socket, (struct sockaddr*) &addr, addr_length) < 0) {
        close(client_socket);
        mal_busy = 0;
        return -2;
        }

    if( dbg_flag & DBG_MAL )
        printf("-MAL: send_mal_command sent (%d)= '%s'\n",strlen(json_cmd),json_cmd);

    if (write(client_socket, json_cmd, strlen(json_cmd)) < 0) {
        close(client_socket);
        mal_busy = 0;
        return -3;
        }

    if (wait_resp) {
        char tresp[1024];                       
        int bytes_read = read(client_socket, tresp, sizeof(tresp)-1);
        if (bytes_read <= 0 || bytes_read > len_json_resp) {
            if( dbg_flag & DBG_MAL )
                printf("-MAL: socket read failed, bytes read = %d\n",bytes_read);
            close(client_socket);
            mal_busy = 0;
            return -4;
            }
        if( dbg_flag & DBG_MAL )
            printf("-MAL: send_mal_command response (%d)= '%s'\n",bytes_read,tresp);
        memcpy(json_resp, tresp, bytes_read);
        }
    close(client_socket);
    mal_busy = 0;
    return 0;
}

//
// This function starts data service in the WNC M18Qx using the MAL manager.
//
// Inputs: NONE
//
// Returns:
//      will return <0 if an error occurs, otherwise 0.

int start_data_service(void) {

    int ret = 0;
    char jcmd1[] = "{\"action\":\"set_network_connection_mode\",\"args\":{\"mode\":0,\"ondemand_timeout\":2,\"manual_mode\":1}}";
    char jcmd2[] = "{\"action\":\"set_wwan_allow_data_roaming\",\"args\":{\"enable\":1}}";
    ret  = send_mal_command(jcmd1, NULL, 0, false);
    ret |= send_mal_command(jcmd2, NULL, 0, false);

    return ret;
} 

//
// Get the IP address we are assigned to
//

char * get_ipAddr(json_keyval *kv, int kvsize) {
    int  i, k;
    char rstr[500];
    char jcmd[] = "{ \"action\" : \"get_wwan_ipv4_network_ip\" }";

    send_mal_command(jcmd, rstr, sizeof(rstr), true);
    i = parse_maljson (rstr, kv, kvsize);
    if( i < 0 )  //parse failed
        return NULL;
    else if( atoi(kv[1].value) ) // we got an error value back
        return kv[2].value;      // so return error message
    else
        return kv[3].value;    
}



//
// Get Firmware version
//

char * getFirmwareVersion(json_keyval *kv, int kvsize) {
    int  i, k;
    char rstr[100];
    char jcmd[] = "{ \"action\" : \"get_system_firmware_version\" }";

    send_mal_command(jcmd, rstr, sizeof(rstr), true);
    i = parse_maljson (rstr, kv, kvsize);
    if( i < 0 )  //parse failed
        return NULL;
    else if( atoi(kv[1].value) ) // we got an error value back
        return kv[2].value;      // so return error message
    else
        return kv[3].value;    
}


//
// Get Apps version
//
char * getAppsVersion(json_keyval *kv, int kvsize) {
    int  i;
    char rstr[400];
    char jcmd[] = "{ \"action\" : \"get_system_config\" }";

    i=send_mal_command(jcmd, rstr, sizeof(rstr), true);
    if( i<0 )
        return (char*)"";
    i = parse_maljson (rstr, kv, kvsize);
    if( i < 0 )  //parse failed
        return (char*)"";
    else if( atoi(kv[1].value) ) // we got an error value back
        return kv[2].value;      // so return error message
    else
        return kv[5].value;      // return version string
}


//
// Get MAL Manager version
//

char * getMALManVer(json_keyval *kv, int kvsize) {
    int  i, k;
    char rstr[100];
    char jcmd[] = "{ \"action\" : \"get_mal_manager_version\" }";
    json_keyval retpars[10];

    send_mal_command(jcmd, rstr, sizeof(rstr), true);
    i = parse_maljson (rstr, kv, kvsize);
    if( i < 0 )  //parse failed
        return NULL;
    else if( atoi(kv[1].value) ) // we got an error value back
        return kv[2].value;      // so return error message
    else
        return kv[3].value;    
}


char * getModelID(json_keyval *kv, int kvsize) {
    int  i, k;
    char rstr[200];
    char jcmd[] = "{ \"action\" : \"get_system_model_id\" }";

    i =send_mal_command(jcmd, rstr, sizeof(rstr), true);
    if (i<0) {
        printf("send_mal_command='%s'\n",i);
        return NULL;
        }
    else {
        i = parse_maljson (rstr, kv, kvsize);
        if( i < 0 )  //parse failed
            return NULL;
        else if( atoi(kv[1].value) ) // we got an error value back
            return kv[2].value;      // so return error message
        else
            return kv[3].value;    
        }
    return NULL;  //we should never get here...
}

char * getIMEI(json_keyval *kv, int kvsize) {
    int  i, k;
    char rstr[100];
    char jcmd[] = "{ \"action\" : \"get_system_imei\" }";

    send_mal_command(jcmd, rstr, sizeof(rstr), true);
    i = parse_maljson (rstr, kv, kvsize);
    if( i < 0 )  //parse failed
        return NULL;
    else if( atoi(kv[1].value) ) // we got an error value back
        return kv[2].value;      // so return error message
    else
        return kv[3].value;    
}

char * getIMSI(json_keyval *kv, int kvsize) {
    int  i, k;
    char rstr[100];
    char jcmd[] = "{ \"action\" : \"get_system_imsi\" }";

    send_mal_command(jcmd, rstr, sizeof(rstr), true);
    i = parse_maljson (rstr, kv, kvsize);
    if( i < 0 )  //parse failed
        return NULL;
    else if( atoi(kv[1].value) ) // we got an error value back
        return kv[2].value;      // so return error message
    else
        return kv[3].value;    
}

char * getICCID(json_keyval *kv, int kvsize) {
    int  i, k;
    char rstr[100];
    char jcmd[] = "{ \"action\" : \"get_system_iccid\" }";

    send_mal_command(jcmd, rstr, sizeof(rstr), true);
    i = parse_maljson (rstr, kv, kvsize);
    if( i < 0 )  //parse failed
        return NULL;
    else if( atoi(kv[1].value) ) // we got an error value back
        return kv[2].value;      // so return error message
    else
        return kv[3].value;    
}

char * getMSISDN(json_keyval *kv, int kvsize) {
    int  i, k;
    char rstr[100];
    char jcmd[] = "{ \"action\" : \"get_system_msisdn\" }";

    send_mal_command(jcmd, rstr, sizeof(rstr), true);
    i = parse_maljson (rstr, kv, kvsize);
    if( i < 0 )  //parse failed
        return NULL;
    else if( atoi(kv[1].value) ) // we got an error value back
        return kv[2].value;      // so return error message
    else
        return kv[3].value;    
}

char *getOperatingMode(json_keyval *kv, int kvsize) {
    int  i;
    char rstr[100];
    char jcmd[] = "{ \"action\" : \"get_operating_mode\" }";

    send_mal_command(jcmd, rstr, sizeof(rstr), true);
    i = parse_maljson (rstr, kv, kvsize);
    if( i < 0 )  //parse failed
        return NULL;
    else if( atoi(kv[1].value) ) // we got an error value back
        return kv[2].value;      // so return error message
    else
        return kv[3].value;    
}

//
// get roaming permission
//
// Input:   pointer to a key/value structure
//          size of the key/value structure
// Output:  Key/Value pairs are updated with information that is returned
//
// Returns: number of Key/Value pairs
//
int get_wwan_allow_data_roaming( json_keyval *kv, int kvsize) {
    char rstr[100];
    char jcmd[] = "{ \"action\" : \"get_wwan_allow_data_roaming\" }";

    send_mal_command(jcmd, rstr, sizeof(rstr), true);
    return parse_maljson (rstr, kv, kvsize);
}

int get_connection_status(json_keyval *kv, int kvsize) {
    char rstr[500];
    char jcmd[] = "{ \"action\" : \"get_network_connection_status\" }";

    send_mal_command(jcmd, rstr, sizeof(rstr), true);
    return parse_maljson (rstr, kv, kvsize);
}

//
// get radio mode, service provide and roaming status info
//
// Input:   pointer to a key/value structure
//          size of the key/value structure
// Output:  Key/Value pairs are updated with information that is returned
//
// Returns: number of Key/Value pairs
//
int get_wwan_status( json_keyval *kv, int kvsize) {
    char rstr[500];
    char jcmd[] = "{ \"action\" : \"get_wwan_serving_system_status\" }";

    send_mal_command(jcmd, rstr, sizeof(rstr), true);
    return parse_maljson (rstr, kv, kvsize);
}


//
// GPS functions
//

char *getGPSconfig(json_keyval *kv, int kvsize) {
    int  i;
    char rstr[300];
    char jcmd[] = "{ \"action\" : \"get_loc_config\" }";

    memset(rstr,0x00,sizeof(rstr));
    i = send_mal_command(jcmd, rstr, sizeof(rstr), true);
    if( i<0 ) {
        printf("-MAL: in get_loc_config, send_mal_command failed - %d\n",i);
        return NULL;
        }
    i=parse_maljson (rstr, kv, kvsize);
    if( i < 0 )  //parse failed
        return NULL;
    else if( atoi(kv[1].value) ) // we got an error value back
        return kv[2].value;      // so return error message
    else
        return kv[3].value;    
}


int getGPSlocation(json_keyval *kv, int kvsize) {
    int  i;
    char rstr[300];
    char jcmd[] = "{ \"action\" : \"get_loc_position_info\" }";

    memset(rstr,0x00,sizeof(rstr));
    i=send_mal_command(jcmd, rstr, sizeof(rstr), true);
    if( i< 0 ) {
        printf("-MAL: getGPSlocation error (%d)\n",i);
        return 0;
        }
    return parse_maljson (rstr, kv, kvsize);
}


int enableGPS(void) {
    char rstr[300];
    char jcmd[] = "{ \"action\": \"set_loc_config\", \"args\": { \"loc\": true } }";
    int i = send_mal_command(jcmd, rstr, sizeof(rstr), true);
    if( i<0 ) {
        printf("-MAL: enableGPS error (%d); ",i);
        printf("returned '%s'\n",rstr);
        return i;
        }
    return 0;
}

int disableGPS(void) {
    int  i;
    char rstr[300];
    char jcmd[] = "{ \"action\": \"set_loc_config\", \"args\": { \"loc\": false } }";

    i=send_mal_command(jcmd, rstr, sizeof(rstr), false);
    if( i<0 ) {
        printf("-MAL: disableGPS error (%d) returned %s\n",i,rstr);
        return i;
        }
    return 0;
}

int resetGPS(void) {
    int  i;
    char rstr[300];
    char jcmd[] = "{ \"action\": \"set_loc_relocate\" }";

    i=send_mal_command(jcmd, rstr, sizeof(rstr), false);
    if( i<0 ) {
        printf("-MAL: resetGPS error (%d) returned %s\n",i,rstr);
        return i;
        }
    return 0;
}

int setGPSmode(int m) {
    int i;
    char jcmd[100];
    char rstr[300];
    sprintf(jcmd, "{ \"action\": \"set_loc_mode\", \"args\": { \"mode\": %d } }", m);;
    i=send_mal_command(jcmd, rstr, sizeof(rstr), false);
    if( i<0 ) {
        printf("-MAL: setGPSmode error (%d) returned %s\n",i,rstr);
        return i;
        }
    return 0;
}

int setGPS_NMEAFilter( int f ) {
    int i;
    char jcmd[100];
    char rstr[300];
    sprintf(jcmd, "{ \"action\": \"set_loc_nmea_filter\", \"args\": { \"mode\": %d } }", f);;
    i=send_mal_command(jcmd, rstr, sizeof(rstr), false);
    if( i<0 ) {
        printf("-MAL: setGPS_NMEAFilter error (%d) returned %s\n",i,rstr);
        return i;
        }
    return 0;
}


