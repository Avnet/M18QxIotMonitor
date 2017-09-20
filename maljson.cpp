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

    @file          maljson.cpp
    @version       1.0
    @date          Sept 2017

======================================================================== */

#include <stdio.h>
#include <string.h>

#include "jsmn.h"
#include "mal.hpp"

#define VALUE   0
#define KEY     1
#define OBJECT  2
#define ARRAY   3

int parse_maljson(char *jstr, json_keyval rslts[], int s) {
    int ro=0, r, k, i, key=OBJECT;
    jsmn_parser p;
    jsmntok_t t[256]; /* We expect no more than 256 tokens */

    memset( rslts, 0x00, s);

    if (!strlen(jstr))
      return -3;

    jsmn_init(&p);
    r = jsmn_parse(&p, jstr, strlen(jstr), t, 256);
    if (r < 0) {
        return -1;
	}

    /* The top-level element must be an object */
    if (r < 1 || t[0].type != JSMN_OBJECT) {
        printf("Object expected\n");
       	return -2;
        }

    /* Loop over remaining elements of the object */
    i = 1;
    do {
    if (t[i].type == JSMN_PRIMITIVE){ //0-is a PRIMITIVE, e.g., bolean, null, symbol
        k = t[i].end-t[i].start;
        strncpy(rslts[ro++].value, (!k)?"<null>":jstr+t[i].start, (!k)?6:k);
        key = KEY;
        }
    if (t[i].type == JSMN_OBJECT){     //1-is a json object (string)
        key = KEY;
        }
    if (t[i].type == JSMN_ARRAY){      //2-is an ARRAY, need the next token to get the array name
        key = KEY;
        strcpy (rslts[ro-1].value, rslts[ro].key);
        sprintf (rslts[ro-1].key, "ARRAY(%d)",t[i].size);
        memset(rslts[ro].key,0x00,strlen(rslts[ro].key));
        }
    if (t[i].type == JSMN_STRING){     //3-is a STRING
        if (key == VALUE) {
            k = t[i].end-t[i].start;
            strncpy(rslts[ro++].value, (!k)?"<null>":jstr+t[i].start, (!k)?6:k);
            key = KEY;
            }
        else {
            key = VALUE;
            strncpy(rslts[ro].key, jstr+t[i].start,t[i].end-t[i].start);
            if (i == 1) {
                strcpy(rslts[ro].key, "OBJECT");
                strncpy(rslts[ro++].value, jstr+t[i].start, t[i].end-t[i].start);
                }
            }
        }
        i++;
    ro = (ro>s)? s-1:ro;
    }
    while( i < r );
    return ro;
}

