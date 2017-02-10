
#if 0  //jmf
static const char json_socket[] = "/tmp/cgi-2-sys";
static const char device_id[] = "837109fb374793e2aaab3e7910a8a57b";
static const char api_key[] = "559ee802277ba7570e903db9a65c94a0";
static const char stream_name[] = "temperature_degree_C";

typedef struct _http_info_s {
	CURL *curl;
	const char *url;
	struct curl_slist *headers;
	struct curl_slist *post_fields;
	struct curl_slist *last_post;
	size_t total_len_post;
	struct curl_slist *put_fields;
	struct curl_slist *last_put;
	size_t total_len_put;
} http_info_t;


#define CHECK_IF_HTTP_REQ_EMPTY(x) \
	do { \
		if (x == NULL) \
		{ \
			LOGE("Empty http_req instance.\n"); \
			return -1; \
		} \
	} while(0)

#define CHECK_IF_HTTP_REQ_INITED(x) \
	do { \
		if (x->curl == NULL) \
		{ \
			LOGE("http_req is not initialized.\n"); \
			return -1; \
		} \
	} while(0)

#define RELEASE_HTTP_INFO(x) \
	do { \
		if (x->headers != NULL)  \
		{ \
			curl_slist_free_all(x->headers); \
			x->headers = NULL; \
		} \
		if (x->post_fields != NULL) \
		{ \
			curl_slist_free_all(x->post_fields); \
			x->post_fields = NULL; \
		} \
		if (x->put_fields != NULL) \
		{ \
			curl_slist_free_all(x->put_fields); \
			x->put_fields = NULL; \
		} \
		curl_easy_cleanup(x->curl); \
	} while(0)	

#define PRINTF_SLIST(x) \
	do { \
		struct curl_slist *sl; \
		for(sl=x ; sl!=NULL ; sl = sl->next) \
			printf("curl_slist:\n %s \n", sl->data); \
	} while(0)		

#define COUNT_SLIST_LENGTH(x, len) \
	do { \
		struct curl_slist *sl; \
		for(sl=x ; sl!=NULL ; sl = sl->next) \
			len += strlen(sl->data); \
	} while(0)		


static int http_request_init(http_info_t *http_req, const int is_https)
{
	CHECK_IF_HTTP_REQ_EMPTY(http_req);

	if (http_req->curl != NULL)
	{
		LOGI("Warning: a none released http_req(%x) is forced to re-initialized.\n", (unsigned int)http_req->curl);

		RELEASE_HTTP_INFO(http_req);
	}
	http_req->curl = curl_easy_init();

	if (is_https)
		curl_easy_setopt(http_req->curl, CURLOPT_SSL_VERIFYPEER, 0L);

	curl_easy_setopt(http_req->curl, CURLOPT_CRLF, 1L);

	curl_easy_setopt(http_req->curl, CURLOPT_VERBOSE, 1L);
	return 0;
}

static int http_request_deinit(http_info_t *http_req)
{
	CHECK_IF_HTTP_REQ_EMPTY(http_req);
	RELEASE_HTTP_INFO(http_req);
	return 0;
}

static int http_collect_header(http_info_t *http_req, const char *head_string)
{
	CHECK_IF_HTTP_REQ_EMPTY(http_req);

	http_req->headers = curl_slist_append(http_req->headers, head_string);
	LOGD("Adding a string into HTTP header:\n");
	LOGD("         %s \n", head_string);
	return 0;
}

static int http_collect_post_field(http_info_t *http_req, const char *post_content)
{
	CHECK_IF_HTTP_REQ_EMPTY(http_req);

	http_req->post_fields = curl_slist_append(http_req->post_fields, post_content);
	LOGD("Adding a string into HTTP POST field:\n");
	LOGD("         %s \n", post_content);
	return 0;
}

static int http_collect_post_field_length(http_info_t *http_req)
{
	int total_length = 0;

	CHECK_IF_HTTP_REQ_EMPTY(http_req);
	COUNT_SLIST_LENGTH(http_req->post_fields, total_length);
	return total_length;
}

static int http_collect_put_field(http_info_t *http_req, const char *put_content)
{
	CHECK_IF_HTTP_REQ_EMPTY(http_req);

	http_req->put_fields = curl_slist_append(http_req->put_fields, put_content);
	LOGD("Adding a string into HTTP PUT field:\n");
	LOGD("         %s \n", put_content);
	return 0;
}

static int http_collect_put_field_length(http_info_t *http_req)
{
	int total_length = 0;

	CHECK_IF_HTTP_REQ_EMPTY(http_req);
	COUNT_SLIST_LENGTH(http_req->put_fields, total_length);
	return total_length;
}

size_t http_post_read_callback(char *buffer, size_t size, size_t nitems, void *instream)
{
	http_info_t *http_req = (http_info_t *)instream;
	struct curl_slist *sl;
	size_t max_length = size * nitems;
	size_t to_copy = 0;

	if (http_req->total_len_post <= 0)
	{
		http_req->last_post = NULL;
		return 0;
	}

	PRINTF_SLIST(http_req->post_fields);	
	for (sl=http_req->last_post ; sl!=NULL ; sl = sl->next)
	{
		size_t len = strlen(sl->data);
		LOGI("sl->data len=%d \n", len);
		if ((to_copy + len) > max_length)
		{
			http_req->last_post = sl;
			break;
		}
		memcpy(&buffer[to_copy], sl->data, len);
		to_copy += len;
		LOGI("to_copy=%d \n", to_copy);
	}
	http_req->total_len_post -= to_copy;
	LOGI("total_len_post=%d \n", http_req->total_len_post);
	return to_copy;
}

static int http_post(http_info_t *http_req, const char *url)
{
	CURLcode res;

	if ((http_req == NULL) || (url == NULL))
	{
		LOGE("Empty http_req(%x) or/and url(%s) instance is/are input.\n", (unsigned int)http_req, url);
		return -1;
	}

	CHECK_IF_HTTP_REQ_INITED(http_req);

	LOGI("HTTP POST:\n");
	curl_easy_setopt(http_req->curl, CURLOPT_POST, 1L);

	curl_easy_setopt(http_req->curl, CURLOPT_READDATA, (void *)http_req);
	curl_easy_setopt(http_req->curl, CURLOPT_READFUNCTION, http_post_read_callback);

	http_req->total_len_post = http_collect_post_field_length(http_req);
	LOGI("total_len_post=%d \n", http_req->total_len_post);
	if (http_req->total_len_post >= 0)
		curl_easy_setopt(http_req->curl, CURLOPT_POSTFIELDSIZE, http_req->total_len_post);
	http_req->last_post = http_req->post_fields;

	curl_easy_setopt(http_req->curl, CURLOPT_URL, url);
	LOGI("      URL:%s \n", url);

	if (http_req->headers != NULL)
		curl_easy_setopt(http_req->curl, CURLOPT_HTTPHEADER, http_req->headers);

	if ((res = curl_easy_perform(http_req->curl)) != CURLE_OK)
	{
		LOGE("fail to perform http_post: url=%s, res=%s \n", url, curl_easy_strerror(res));
		return -1;
	}
	return 0;
}

size_t http_upload_read_callback(char *buffer, size_t size, size_t nitems, void *instream)
{
	http_info_t *http_req = (http_info_t *)instream;
	struct curl_slist *sl;
	size_t max_length = size * nitems;
	size_t to_copy = 0;

	if (http_req->total_len_put <= 0)
	{
		http_req->last_put = NULL;
		return 0;
	}

	PRINTF_SLIST(http_req->put_fields);	
	for (sl=http_req->last_put ; sl!=NULL ; sl = sl->next)
	{
		size_t len = strlen(sl->data);
		LOGI("sl->data len=%d \n", len);
		if ((to_copy + len) > max_length)
		{
			http_req->last_put = sl;
			break;
		}
		memcpy(&buffer[to_copy], sl->data, len);
		to_copy += len;
		LOGI("to_copy=%d \n", to_copy);
	}
	http_req->total_len_put -= to_copy;
	LOGI("total_len_put=%d \n", http_req->total_len_put);
	return to_copy;
}

static int http_put(http_info_t *http_req, const char *url)
{
	CURLcode res;

	if ((http_req == NULL) || (url == NULL))
	{
		LOGE("Empty http_req(%x) or/and url(%s) instance is/are input.\n", (unsigned int)http_req, url);
		return -1;
	}

	CHECK_IF_HTTP_REQ_INITED(http_req);

	LOGI("HTTP PUT:\n");
	curl_easy_setopt(http_req->curl, CURLOPT_UPLOAD, 1L);
	curl_easy_setopt(http_req->curl, CURLOPT_PUT, 1L);

	curl_easy_setopt(http_req->curl, CURLOPT_READDATA, (void *)http_req);
	curl_easy_setopt(http_req->curl, CURLOPT_READFUNCTION, http_upload_read_callback);

	http_req->total_len_put = http_collect_put_field_length(http_req);
	LOGI("total_len_put=%d \n", http_req->total_len_put);
	if (http_req->total_len_put >= 0)
		curl_easy_setopt(http_req->curl, CURLOPT_INFILESIZE, http_req->total_len_put);
	http_req->last_put = http_req->put_fields;

	curl_easy_setopt(http_req->curl, CURLOPT_URL, url);

	LOGI("     URL:%s \n", url);

	if (http_req->headers != NULL)
		curl_easy_setopt(http_req->curl, CURLOPT_HTTPHEADER, http_req->headers);

	if ((res = curl_easy_perform(http_req->curl)) != CURLE_OK)
	{
		LOGE("fail to perform http_put: url=%s, res=%s \n", url, curl_easy_strerror(res));
		return -1;
	}
	return 0;
}

static int m2x_create_one_stream
(
	const char *device_id_ptr,
	const char *api_key_ptr,
	const char *stream_name_ptr
)
{
	http_info_t put_req;
	char tmp_buff[64];
	char url[256];

	memset(url, 0, sizeof(url));
	sprintf(url, "http://api-m2x.att.com/v2/devices/%s/streams/%s", device_id_ptr, stream_name_ptr);

	memset(&put_req, 0, sizeof(http_info_t));
	http_request_init(&put_req, 0);

	http_collect_header(&put_req, "Content-Type: application/json");

	memset(tmp_buff, 0, sizeof(tmp_buff));
	sprintf(tmp_buff, "X-M2X-KEY: %s", api_key_ptr);
	http_collect_header(&put_req, tmp_buff);

	if (http_put(&put_req, url) < 0)
	{
		LOGE("HTTP PUT is failed.");
	}

	http_request_deinit(&put_req);
	return 0;
}

static int m2x_update_one_stream_values
(
	const char *device_id_ptr,
	const char *api_key_ptr,
	const char *stream_name_ptr,
	const char *stream_value_ptr
)
{
	http_info_t post_req;
	char tmp_buff[256];
	char tmp_timestamp[64];
	char url[256];
	time_t t;
	struct tm *tm;

	memset(url, 0, sizeof(url));
	sprintf(url, "http://api-m2x.att.com/v2/devices/%s/streams/%s/values", device_id_ptr, stream_name_ptr);

	memset(&post_req, 0, sizeof(http_info_t));
	http_request_init(&post_req, 0);

	http_collect_header(&post_req, "Content-Type: application/json");

	memset(tmp_buff, 0, sizeof(tmp_buff));
	sprintf(tmp_buff, "X-M2X-KEY: %s", api_key_ptr);
	http_collect_header(&post_req, tmp_buff);

	t = time(NULL);
	tm = localtime(&t);
	sprintf(tmp_timestamp, "%4d-%02d-%02dT%02d:%02d:%02dZ",
		(tm->tm_year+1900), (tm->tm_mon+1), tm->tm_mday,
		tm->tm_hour, tm->tm_min, tm->tm_sec);
	memset(tmp_buff, 0, sizeof(tmp_buff));
	sprintf(tmp_buff, "{ \"values\": [ { \"timestamp\": \"%s\", \"value\": %s } ] }",
		tmp_timestamp, stream_value_ptr);

	http_collect_post_field(&post_req, tmp_buff);

	if (http_post(&post_req, url) < 0)
	{
		LOGE("HTTP POST is failed.");
	}

	http_request_deinit(&post_req);
	return 0;
}

static int m2x_update_one_device_streams
(
	const char *device_id_ptr,
	const char *api_key_ptr,
	const char *stream_name_ptr,
	const char *stream_value_ptr
)
{
	http_info_t post_req;
	char tmp_buff[256];
	char tmp_timestamp[64];
	char url[256];
	time_t t;
	struct tm *tm;

	memset(url, 0, sizeof(url));
	sprintf(url, "http://api-m2x.att.com/v2/devices/%s/update", device_id_ptr);

	memset(&post_req, 0, sizeof(http_info_t));
	http_request_init(&post_req, 0);

	http_collect_header(&post_req, "Content-Type: application/json");

	memset(tmp_buff, 0, sizeof(tmp_buff));
	sprintf(tmp_buff, "X-M2X-KEY: %s", api_key_ptr);
	http_collect_header(&post_req, tmp_buff);

	t = time(NULL);
	tm = localtime(&t);
	sprintf(tmp_timestamp, "%4d-%02d-%02dT%02d:%02d:%02dZ",
		(tm->tm_year+1900), (tm->tm_mon+1), tm->tm_mday,
		tm->tm_hour, tm->tm_min, tm->tm_sec);
	memset(tmp_buff, 0, sizeof(tmp_buff));
	sprintf(tmp_buff, "{ \"timestamp\": \"%s\", \"values\": { \"%s\": %s } }",
		tmp_timestamp, stream_name_ptr, stream_value_ptr);

	http_collect_post_field(&post_req, tmp_buff);

	if (http_post(&post_req, url) < 0)
	{
		LOGE("HTTP POST is failed.");
	}

	http_request_deinit(&post_req);
	return 0;
}

int send_json_api(const char* s_addr, const char *json_cmd, char *json_resp, int len_json_resp, uint8_t wait_resp)
{
	int client_socket;
	socklen_t addr_length;
	struct sockaddr_un addr;

	strcpy(addr.sun_path, s_addr);    // max 108 bytes
	addr.sun_family = AF_UNIX;
	client_socket = socket(AF_UNIX, SOCK_STREAM, 0);
	addr_length = SUN_LEN(&addr);

	if (client_socket < 0)
	{
		LOGE("Failed to create IPC socket !\n");
		return -1;
	}

	if (connect(client_socket, (struct sockaddr*) &addr, addr_length) < 0)
	{
		close(client_socket);
		LOGE("Failed to connect with %s !\n", s_addr);
		return -1;
	}

	if (write(client_socket, json_cmd, strlen(json_cmd)) < 0)
	{
		LOGE("Failed to send a JSON API command, (%s)\n", strerror(errno));
		return -1;
	}

	if (wait_resp) {
		while (len_json_resp)
		{
			char tmp_container[1024+1]; // max length per read is 1024.
			int bytes_read = read(client_socket, tmp_container, sizeof(tmp_container)-1);
			if (bytes_read <= 0)
				break;
			if (len_json_resp >= bytes_read)
				len_json_resp -= bytes_read;
			else
			{
				LOGI("Warning, the prepared buffer is too small. Lost bytes:%d \n", (bytes_read-len_json_resp));
				len_json_resp = 0;
			}
			memcpy(json_resp, tmp_container, bytes_read);
		}
	}
	close(client_socket);
	return 0;
}


#endif //jmf

//
// start...
//

#include <stdio.h>
#include "http.h"
#include "mytimer.h"
#include <time.h>

size_t m2x_sensor_timer;
void sensor_hts221(int interval);

M2XFUNC m2xfunctions[] = {
//  name,     rate,        func(), *tmr
    "HTS221",    0, sensor_hts221, NULL,
    "ENDTAB",    0,          NULL, NULL,
};
#define _MAX_M2XFUNCTIONS	(sizeof(m2xfunctions)/sizeof(M2XFUNC))

const int _max_m2xfunctions = _MAX_M2XFUNCTIONS;

void tx2m2x_timer(size_t timer_id, void * user_data)
{
    const time_t t = time(0);
    printf("\nRead sensor and send data. %s\n",asctime(localtime(&t)));
}
 
 
void sensor_hts221(int interval)
{
    if( m2x_sensor_timer != 0 ) {  //currently have a timer running
        if( interval ) { //just want to change the rate of samples
            delete_IoTtimer(m2x_sensor_timer);
            m2x_sensor_timer = create_IoTtimer(interval, tx2m2x_timer, TIMER_PERIODIC, NULL);
printf("changed timer\n");
            }
         else {  //want to kill the timer
            delete_IoTtimer(m2x_sensor_timer);
            stop_IoTtimers();
            m2x_sensor_timer = 0;
printf("stopped timer\n");
            }
        }
    else { //don't have a timer currently running, start one up
        start_IoTtimers();
        m2x_sensor_timer = create_IoTtimer(interval, tx2m2x_timer, TIMER_PERIODIC, NULL);
printf("started timer\n");
        }
}

