
#include <stdio.h>
#include <curl/curl.h>
#include <time.h>
#include <string.h>

#include "http.h"

typedef struct _http_info_s {
	CURL *curl;
	const char *url;
	struct curl_slist *header;
	struct curl_slist *data_field;
	struct curl_slist *last_data;
	size_t total_len;
} http_info_t;

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
//jmf    curl_easy_setopt(http_req->curl, CURLOPT_VERBOSE, 0L);
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

    http_req->last_data = http_req->data_field;
    http_req->total_len = http_field_len(http_req->data_field);

    curl_easy_setopt(http_req->curl, CURLOPT_URL, url);
    curl_easy_setopt(http_req->curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(http_req->curl, CURLOPT_PUT, 1L);
    curl_easy_setopt(http_req->curl, CURLOPT_READDATA, (void *)http_req);
    curl_easy_setopt(http_req->curl, CURLOPT_READFUNCTION, http_upload_read_callback);

    if (http_req->total_len >= 0)
        curl_easy_setopt(http_req->curl, CURLOPT_INFILESIZE, http_req->total_len);

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


