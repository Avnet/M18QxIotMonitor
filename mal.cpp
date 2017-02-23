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

#ifdef __cplusplus
extern "C" {
#endif

#include <hwlib/hwlib.h>
#ifdef __cplusplus
}
#endif

#include <json-c/json.h>

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

    strcpy(addr.sun_path, JSON_SOCKET_ADDR);    // max 108 bytes
    addr.sun_family = AF_UNIX;
    addr_length = SUN_LEN(&addr);

    if ((client_socket=socket(AF_UNIX, SOCK_STREAM, 0)) < 0) 
        return -1;

    if (connect(client_socket, (struct sockaddr*) &addr, addr_length) < 0) {
        close(client_socket);
        return -2;
        }

    if (write(client_socket, json_cmd, strlen(json_cmd)) < 0) {
        return -3;
        }

    if (wait_resp) {
        char tresp[1024];                       
        int bytes_read = read(client_socket, tresp, sizeof(tresp)-1);
        if (bytes_read <= 0 || bytes_read > len_json_resp)
            return -4;
        memcpy(json_resp, tresp, bytes_read);
        }
    close(client_socket);
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
    char jcmd[] = "{ \"action\": \"set_network_connection_mode\", \"args\": { \"manual_mode\": 0 } }";
    send_mal_command(jcmd, NULL, 0, false);
    return 0;


//    struct json_object *jobj_cmd = json_object_new_object(), 
//        *jobj_manual_mode = json_object_new_object(),
//        *jstr_action = json_object_new_string("set_network_connection_mode"),
//        *jint_manual_mode_value = json_object_new_int(0);	
//
//    json_object_object_add(jobj_manual_mode, "manual_mode", jint_manual_mode_value); 
//    json_object_object_add(jobj_cmd, "action", jstr_action); 
//    json_object_object_add(jobj_cmd, "args", jobj_manual_mode);
//    r=send_mal_command((char*)json_object_to_json_string(jobj_cmd), NULL, 0, 0);
//
//
//    if (r < 0)
//        return r;
//
//    /* JMF TODO: need to issue a 'get_network_connection_status' to ensure we are connected...  */
//    sleep(5); //wait for setting up data service.
//    return 0;
} 

//
// Get the IP address we are assigned to
//

char * get_ipAddr(json_keyval *kv, int kvsize) {
    int  i, k;
    char rstr[100];
    char jcmd[] = "{ \"action\" : \"get_wwan_ipv4_network_ip\" }";

    send_mal_command(jcmd, rstr, sizeof(rstr), true);
    i = parse_maljson (rstr, kv, kvsize);
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
    return kv[3].value;    
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
    return kv[3].value;    
}


char * getModelID(json_keyval *kv, int kvsize) {
    int  i, k;
    char rstr[100];
    char jcmd[] = "{ \"action\" : \"get_system_model_id\" }";

    send_mal_command(jcmd, rstr, sizeof(rstr), true);
    i = parse_maljson (rstr, kv, kvsize);
    return kv[3].value;    
}

char * getIMEI(json_keyval *kv, int kvsize) {
    int  i, k;
    char rstr[100];
    char jcmd[] = "{ \"action\" : \"get_system_imei\" }";

    send_mal_command(jcmd, rstr, sizeof(rstr), true);
    i = parse_maljson (rstr, kv, kvsize);
    return kv[3].value;    
}

char * getIMSI(json_keyval *kv, int kvsize) {
    int  i, k;
    char rstr[100];
    char jcmd[] = "{ \"action\" : \"get_system_imsi\" }";

    send_mal_command(jcmd, rstr, sizeof(rstr), true);
    i = parse_maljson (rstr, kv, kvsize);
    return kv[3].value;    
}

char * getICCID(json_keyval *kv, int kvsize) {
    int  i, k;
    char rstr[100];
    char jcmd[] = "{ \"action\" : \"get_system_iccid\" }";

    send_mal_command(jcmd, rstr, sizeof(rstr), true);
    i = parse_maljson (rstr, kv, kvsize);
    return kv[3].value;    
}

char * getMSISDN(json_keyval *kv, int kvsize) {
    int  i, k;
    char rstr[100];
    char jcmd[] = "{ \"action\" : \"get_system_msisdn\" }";

    send_mal_command(jcmd, rstr, sizeof(rstr), true);
    i = parse_maljson (rstr, kv, kvsize);
    return kv[3].value;    
}

char *getOperatingMode(json_keyval *kv, int kvsize) {
    int  i;
    char rstr[100];
    char jcmd[] = "{ \"action\" : \"get_operating_mode\" }";

    send_mal_command(jcmd, rstr, sizeof(rstr), true);
    i = parse_maljson (rstr, kv, kvsize);
    return kv[3].value;    
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
    char rstr[100];
    char jcmd[] = "{ \"action\" : \"get_loc_config\" }";

    memset(rstr,0x00,sizeof(rstr));
    send_mal_command(jcmd, rstr, sizeof(rstr), true);
printf("-get_loc_config returned '%s'\n",rstr);
//    i = parse_maljson (rstr, kv, kvsize);
//    return kv[3].value;    
return NULL;
}


char *getGPSlocation(json_keyval *kv, int kvsize) {
    int  i;
    char rstr[100];
    char jcmd[] = "{ \"action\" : \"get_loc_position_info\" }";

    memset(rstr,0x00,sizeof(rstr));
    send_mal_command(jcmd, rstr, sizeof(rstr), true);
printf("-get_loc_position_info returned '%s'\n",rstr);
//    i = parse_maljson (rstr, kv, kvsize);
//    return kv[3].value;    
return NULL;
}


int enableGPS(void) {
    char jcmd[] = "{ \"action\": \"set_loc_config\", \"args\": { \"loc\": \"true\" } }";
printf("-send '%s'\n",jcmd);
    send_mal_command(jcmd, NULL, 0, false);
    return 0;
}

int disableGPS(void) {
    char jcmd[] = "{ \"action\": \"set_loc_config\", \"args\": { \"loc\": \"false\" } }";
printf("-send '%s'\n",jcmd);
    send_mal_command(jcmd, NULL, 0, false);
    return 0;
}

int setGPSmode(int m) {
    char jcmd[100];
    sprintf(jcmd, "{ \"action\": \"set_loc_mode\", \"args\": { \"mode\": %d } }", m);;
printf("-send '%s'\n",jcmd);
    send_mal_command(jcmd, NULL, 0, false);
    return 0;
}

int setGPS_NMEAFilter( int f ) {
    char jcmd[100];
    sprintf(jcmd, "{ \"action\": \"set_loc_nmea_filter\", \"args\": { \"mode\": %d } }", f);;
printf("-send '%s'\n",jcmd);
    send_mal_command(jcmd, NULL, 0, false);
    return 0;
}


