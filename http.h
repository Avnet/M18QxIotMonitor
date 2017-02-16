
#ifndef __HTTP_H__
#define __HTTP_H__

#ifdef __cplusplus
extern "C" {
#endif

extern int m2x_create_stream ( const char *device_id_ptr, const char *api_key_ptr, const char *stream_name_ptr );
extern int m2x_update_stream_value ( const char *device_id_ptr, const char *api_key_ptr, 
                                     const char *stream_name_ptr, const char *stream_value_ptr);

#ifdef __cplusplus
}
#endif

#include <sys/types.h>
#include <stdint.h>
#include <nettle/nettle-stdint.h>
#include <nettle/nettle-stdint.h>
#include <hwlib/hwlib.h>

#endif // __HTTP_H__
