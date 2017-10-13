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

    @file          MAL.hpp
    @version       1.0
    @date          Sept 2017

======================================================================== */

#ifndef __MAL_H__
#define __MAL_H__

typedef struct _json_keyval {
    char key[50];
    char value[50];
    } json_keyval;

#ifdef __cplusplus
extern "C" {
#endif

int parse_maljson(char *jstr, json_keyval rslts[], int s) ;
char * getFirmwareVersion(json_keyval *kv, int kvsize);
char * getAppsVersion(json_keyval *kv, int kvsize);
char * getMALManVer(json_keyval *kv, int kvsize);
char * get_ipAddr(json_keyval *kv, int kvsize);
char * getModelID(json_keyval *kv, int kvsize);
char * getIMEI(json_keyval *kv, int kvsize);
char * getIMSI(json_keyval *kv, int kvsize);
char * getICCID(json_keyval *kv, int kvsize);
char * getMSISDN(json_keyval *kv, int kvsize);
char *getOperatingMode(json_keyval *kv, int kvsize);
int get_connection_status(json_keyval *kv, int kvsize);
int get_wwan_allow_data_roaming( json_keyval *kv, int kvsize);
int get_wwan_status( json_keyval *kv, int kvsize);

char *getGPSconfig(json_keyval *kv, int kvsize);
int getGPSlocation(json_keyval *kv, int kvsize);
int enableGPS(void);
int disableGPS(void);
int resetGPS(void);
int setGPSmode(int m);
int setGPS_NMEAFilter( int f );

int start_data_service(void);

#ifdef __cplusplus
}
#endif

#endif // __MAL_H__

