
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
char * getMALManVer(json_keyval *kv, int kvsize);
char * get_ipAddr(json_keyval *kv, int kvsize);
char * getModelID(json_keyval *kv, int kvsize);
char * getIMEI(json_keyval *kv, int kvsize);
char * getIMSI(json_keyval *kv, int kvsize);
char * getICCID(json_keyval *kv, int kvsize);
char * getMSISDN(json_keyval *kv, int kvsize);
char *getOperatingMode(json_keyval *kv, int kvsize);
int get_wwan_status( json_keyval *kv, int kvsize);

char *getGPSconfig(json_keyval *kv, int kvsize);
int getGPSlocation(json_keyval *kv, int kvsize);
int enableGPS(void);
int disableGPS(void);
int setGPSmode(int m);
int setGPS_NMEAFilter( int f );

int start_data_service(void);

#ifdef __cplusplus
}
#endif

#endif // __MAL_H__

