
#include <stdio.h>
#include <curl/curl.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#include "iot_monitor.h"
#include "http.h"

typedef struct _http_info_s {
	CURL *curl;
	const char *url;
	struct curl_slist *header;
	struct curl_slist *data_field;
	struct curl_slist *last_data;
	size_t total_len;
} http_info_t;

static int rsize;

static int http_deinit(http_info_t *req)
{
    if(!req)
        return -1;
    if (req->header != NULL)  { 
        curl_slist_free_all(req->header); 
        req->header = NULL; 
        } 
    if (req->data_field != NULL) { 
        curl_slist_free_all(req->data_field); 
        req->data_field = NULL; 
        } 
    curl_easy_cleanup(req->curl); 
    return 0;
}

static int http_init(http_info_t *http_req, const int is_https)
{

    http_deinit(http_req);

    http_req->curl = curl_easy_init();

    if (is_https)
    	curl_easy_setopt(http_req->curl, CURLOPT_SSL_VERIFYPEER, 0L);

    curl_easy_setopt(http_req->curl, CURLOPT_CRLF, 1L);

    if( dbg_flag & DBG_CURL )
        curl_easy_setopt(http_req->curl, CURLOPT_VERBOSE, 1L);
    return 0;
}

static struct curl_slist *http_add_field(struct curl_slist *ptr, const char *str)
{
	ptr = curl_slist_append(ptr, str);
	return ptr;
}

static int http_field_len(struct curl_slist *ptr)
{
    struct curl_slist *sl; 
    int len=0;

    for(sl=ptr; sl!=NULL ; sl = sl->next) 
        len += strlen(sl->data); 
    return len;
}


size_t http_callback(struct curl_slist *req, size_t total_len, char *buffer, size_t size, size_t nitems)
{
    struct curl_slist *sl;
    size_t max_length = size * nitems;
    size_t to_copy = 0;

    if (total_len <= 0) {
        req = NULL;
        return 0;
        }

    for (sl=req ; sl!=NULL ; sl = sl->next) {
        size_t len = strlen(sl->data);
        if ((to_copy + len) > max_length) {
            req = sl;
            break;
            }
        memcpy(&buffer[to_copy], sl->data, len);
        to_copy += len;
        }
    total_len -= to_copy;
    return to_copy;
}


size_t http_upload_read_callback(char *buffer, size_t size, size_t nitems, void *instream)
{
    http_info_t *http_req = (http_info_t *)instream;
    return http_callback(http_req->last_data, http_req->total_len, buffer, size, nitems);
}


size_t http_post_read_callback(char *buffer, size_t size, size_t nitems, void *instream)
{
    http_info_t *http_req = (http_info_t *)instream;
    return http_callback(http_req->last_data, http_req->total_len, buffer, size, nitems);
}


static int http_post(http_info_t *http_req, const char *url)
{
    CURLcode res;

    if ((http_req == NULL) || (url == NULL)) 
    	return -1;

    curl_easy_setopt(http_req->curl, CURLOPT_POST, 1L);
    curl_easy_setopt(http_req->curl, CURLOPT_READDATA, (void *)http_req);
    curl_easy_setopt(http_req->curl, CURLOPT_TIMEOUT, 300L);
    curl_easy_setopt(http_req->curl, CURLOPT_READFUNCTION, http_post_read_callback);
    curl_easy_setopt(http_req->curl, CURLOPT_URL, url);

    http_req->total_len = http_field_len(http_req->data_field);
    if (http_req->total_len >= 0)
	curl_easy_setopt(http_req->curl, CURLOPT_POSTFIELDSIZE, http_req->total_len);
    http_req->last_data = http_req->data_field;

    if (http_req->header != NULL)
	curl_easy_setopt(http_req->curl, CURLOPT_HTTPHEADER, http_req->header);

    return ((res = curl_easy_perform(http_req->curl)) != CURLE_OK)? -1 : 0; 
}

static int http_put(http_info_t *http_req, const char *url)
{
    CURLcode res;

    if ((http_req == NULL) || (url == NULL)) {
        return -1;
        }

    if (http_req->curl == NULL) 
        return -1;

    curl_easy_setopt(http_req->curl, CURLOPT_PUT, 1L);
    curl_easy_setopt(http_req->curl, CURLOPT_READDATA, (void *)http_req);
    curl_easy_setopt(http_req->curl, CURLOPT_TIMEOUT, 300L);
    curl_easy_setopt(http_req->curl, CURLOPT_READFUNCTION, http_upload_read_callback);
    curl_easy_setopt(http_req->curl, CURLOPT_URL, url);

    http_req->total_len = http_field_len(http_req->data_field);
    if (http_req->total_len >= 0)
        curl_easy_setopt(http_req->curl, CURLOPT_INFILESIZE, http_req->total_len);
    http_req->last_data = http_req->data_field;

    if (http_req->header != NULL)
      	curl_easy_setopt(http_req->curl, CURLOPT_HTTPHEADER, http_req->header);

    return ((res = curl_easy_perform(http_req->curl)) != CURLE_OK) ? -1 : 0;
}

//
// This function creates a M2X Stream if it doesn't currently exist.  
//
int m2x_create_stream ( const char *device_id_ptr, const char *api_key_ptr, const char *stream_name_ptr )
{
	http_info_t put_req;
	char tmp_buff[64];
	char url[256];
        int  ret;

	memset(&put_req, 0, sizeof(http_info_t));
	memset(tmp_buff, 0, sizeof(tmp_buff));
	memset(url, 0, sizeof(url));

	http_init(&put_req, 0);

	sprintf(url, "http://api-m2x.att.com/v2/devices/%s/streams/%s", device_id_ptr, stream_name_ptr);
	sprintf(tmp_buff, "X-M2X-KEY: %s", api_key_ptr);

	put_req.header = http_add_field(put_req.header, "Content-Type: application/json");
	put_req.header = http_add_field(put_req.header, tmp_buff);

        ret=http_put(&put_req, url);
	http_deinit(&put_req);

	return (ret<0)? -1:0;
}

//
// This function is called to update a stream value.
//
int m2x_update_stream_value ( const char *device_id_ptr, const char *api_key_ptr, const char *stream_name_ptr, const char *stream_value_ptr)
{
    http_info_t post_req;
    char tmp_buff1[256], tmp_buff2[256], tmp_buff3[64];;
    char url[256];
    time_t t = time(NULL);;
    struct tm *tm;

    memset(&post_req, 0, sizeof(http_info_t));
    memset(tmp_buff1, 0, sizeof(tmp_buff1));
    memset(tmp_buff2, 0, sizeof(tmp_buff2));
    memset(url, 0, sizeof(url));

    tm = localtime(&t);
    http_init(&post_req, 0);

    sprintf(url, "http://api-m2x.att.com/v2/devices/%s/streams/%s/values", device_id_ptr, stream_name_ptr);
    sprintf(tmp_buff1, "X-M2X-KEY: %s", api_key_ptr);
    sprintf(tmp_buff3, "%4d-%02d-%02dT%02d:%02d:%02dZ", (tm->tm_year+1900), (tm->tm_mon+1), tm->tm_mday,
                                                            tm->tm_hour, tm->tm_min, tm->tm_sec);
    sprintf(tmp_buff2, "{ \"values\": [ { \"timestamp\": \"%s\", \"value\": %s } ] }", tmp_buff3, stream_value_ptr);

    post_req.header = http_add_field(post_req.header, "Content-Type: application/json");
    post_req.header = http_add_field(post_req.header, tmp_buff1);
    post_req.data_field = http_add_field(post_req.data_field, tmp_buff2);

    if (http_post(&post_req, url) < 0)
	return -1;

    http_deinit(&post_req);
    return 0;
}


size_t static write_callback_func(void *buffer, size_t size, size_t nmemb, void *userp)
{
    char *response_ptr =  (char*)userp;

    if( dbg_flag & DBG_FLOW )
        printf("-FLOW: IN CALLBACK: buffer=%s\n",buffer);
    if( size*nmemb > 0 )
        strncpy(&response_ptr[rsize], buffer, (size*nmemb));
    rsize += (size * nmemb);
    return(size * nmemb);
}

int http_get(http_info_t *http_req, const char *url, char *response)
{
    CURLcode res;

    if ((http_req == NULL) || (url == NULL)) {
        return -1;
        }

    if (http_req->curl == NULL) 
        return -2;

    http_req->last_data = http_req->data_field;
    http_req->total_len = http_field_len(http_req->data_field);

    curl_easy_setopt(http_req->curl, CURLOPT_URL, url);
    curl_easy_setopt(http_req->curl, CURLOPT_TIMEOUT, 300L);
    curl_easy_setopt(http_req->curl, CURLOPT_HTTPGET, 1);
    curl_easy_setopt(http_req->curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
    curl_easy_setopt(http_req->curl, CURLOPT_FOLLOWLOCATION, 1); 

    rsize = 0;
    curl_easy_setopt(http_req->curl, CURLOPT_WRITEFUNCTION, write_callback_func);
    curl_easy_setopt(http_req->curl, CURLOPT_WRITEDATA, response);

    res = curl_easy_perform(http_req->curl);
    return (res != CURLE_OK) ? -res : 0;
}



//
// This function is called to solicit an LED command from Flow based on accelerator and temp/humid data 
//

char *flow_get ( const char *flow_base_url, const char *flow_input_name, 
                 const char *flow_device_name, const char *flow_server, 
                 const char *get_cmd, char *response, int resp_size)
{
    int r;
    http_info_t get_req;
    char tmp_buff1[256];
    char url[256];

    if( dbg_flag & DBG_FLOW )
        printf("-FLOW: BASE URL: %s\n-FLOW: INPUT NAME: %s\n-FLOW: DEVICE: %s\n-FLOW: COMMAND: %s\n",
                flow_base_url, flow_input_name, flow_device_name, get_cmd);

    memset(response, 0, resp_size);
    memset(&get_req, 0, sizeof(http_info_t));
    memset(tmp_buff1, 0, sizeof(tmp_buff1));
    memset(url, 0, sizeof(url));

    http_init(&get_req, 0);

    sprintf(url, "%s/%s?serial=%s%s", flow_base_url, flow_input_name, flow_device_name, get_cmd);
    get_req.header = http_add_field(get_req.header, tmp_buff1);

    sprintf(tmp_buff1, "Host:%s", flow_server);
    get_req.header = http_add_field(get_req.header, tmp_buff1);
    get_req.header = http_add_field(get_req.header, "HTTP/1.1");
    get_req.header = http_add_field(get_req.header, "Accept: */*");

    do {
        r=http_get(&get_req, url, response);
        if (r < 0) {
            if( dbg_flag & DBG_FLOW )
                printf("-FLOW: bad response %d, wait 30- seconds\n",r);
            sleep(30);
            }
        }
    while( r );

    http_deinit(&get_req);
    return response;

}


//
// This function is called to update a LED COLOR value.
//
//curl -i -X PUT http://api-m2x.att.com/v2/devices/2ac9dc89132469eb809bea6e3a95d675/streams/rgb/value 
//     -H "X-M2X-KEY: 6cd9c60f4a4e5d91d0ec4cc79536c661" 
//     -H "Content-Type: application/json" 
//     -d "{ \"value\": \"WHITE\" }"

int m2x_update_color_value ( const char *device_id_ptr, const char *api_key_ptr, const char *stream_name_ptr, const char *stream_value_ptr)
{
    http_info_t put_req;
    char tmp_buff1[256], tmp_buff2[256], tmp_buff3[64];;
    char url[256];

    memset(&put_req, 0, sizeof(http_info_t));
    memset(tmp_buff1, 0, sizeof(tmp_buff1));
    memset(tmp_buff2, 0, sizeof(tmp_buff2));
    memset(url, 0, sizeof(url));

    http_init(&put_req, 0);

    sprintf(url, "http://api-m2x.att.com/v2/devices/%s/streams/%s/value", device_id_ptr, stream_name_ptr);
    sprintf(tmp_buff1, "X-M2X-KEY: %s", api_key_ptr);
    sprintf(tmp_buff2, "{ \"value\": \"%s\" }", stream_value_ptr);

    put_req.header = http_add_field(put_req.header, "Content-Type: application/json");
    put_req.header = http_add_field(put_req.header, tmp_buff1);
    put_req.data_field = http_add_field(put_req.data_field, tmp_buff2);

    if (http_put(&put_req, url) < 0)
	return -1;

    http_deinit(&put_req);
    return 0;
}

