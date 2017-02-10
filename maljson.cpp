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
    memset( rslts, 0x00, sizeof(json_keyval)*s);

    if (!strlen(jstr))
      return -3;

    jsmn_init(&p);
    r = jsmn_parse(&p, jstr, strlen(jstr), t, sizeof(t)/sizeof(t[0]));
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
