
#ifndef __HTS221_H__
#define __HTS221_H__
               
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/syscall.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <nettle/nettle-stdint.h>
#include <hwlib/hwlib.h>
#include <json-c/json.h>

#define HTS221_SAD			0x5F	// slave address
#define HTS221_DEVICE_ID		0xBC

//registers of HTS221
#define HTS221_REG_WHO_AM_I		0x0F
#define HTS221_REG_AV_CONF		0x10
#define HTS221_REG_CTRL_REG1		0x20
#define HTS221_REG_CTRL_REG2		0x21
#define HTS221_REG_CTRL_REG3		0x22
#define HTS221_REG_STATUS		0x27
#define HTS221_REG_HUMIDITY_OUT_L	0x28
#define HTS221_REG_HUMIDITY_OUT_H	0x29
#define HTS221_REG_TEMP_OUT_L		0x2A
#define HTS221_REG_TEMP_OUT_H		0x2B
#define HTS221_REG_T0_DEGC_X8		0x32
#define HTS221_REG_T1_DEGC_X8		0x33
#define HTS221_REG_T1_T0_MSB		0x35
#define HTS221_REG_T0_OUT_L		0x3C
#define HTS221_REG_T0_OUT_H		0x3D
#define HTS221_REG_T1_OUT_L		0x3E
#define HTS221_REG_T1_OUT_H		0x3F

#define HTS221_TEMP_AVAILABLE		0x01
#define HTS221_HUMIDITY_AVAILABLE	0x02

#define HTS221_H0_RH_X2        (uint8_t)0x30
#define HTS221_H1_RH_X2        (uint8_t)0x31
#define HTS221_T0_DEGC_X8      (uint8_t)0x32
#define HTS221_T1_DEGC_X8      (uint8_t)0x33
#define HTS221_T0_T1_DEGC_H2   (uint8_t)0x35
#define HTS221_H0_T0_OUT_L     (uint8_t)0x36
#define HTS221_H0_T0_OUT_H     (uint8_t)0x37
#define HTS221_H1_T0_OUT_L     (uint8_t)0x3A
#define HTS221_H1_T0_OUT_H     (uint8_t)0x3B
#define HTS221_T0_OUT_L        (uint8_t)0x3C
#define HTS221_T0_OUT_H        (uint8_t)0x3D
#define HTS221_T1_OUT_L        (uint8_t)0x3E
#define HTS221_T1_OUT_H        (uint8_t)0x3F

#define HTS221_HR_OUT_L_REG        (uint8_t)0x28
#define HTS221_HR_OUT_H_REG        (uint8_t)0x29

#ifdef __cplusplus
extern "C" {
#endif

int      hts221_initialize(void);
uint8_t  hts221_getDeviceID(void);
float    hts221_getHumid(void);
float    hts221_getTemp(void);

void     hts221_read(uint8_t reg_addr, uint8_t *buf, int nbr);
uint16_t hts221_read_word(uint8_t reg_addr);
uint8_t  hts221_read_byte(uint8_t reg_addr);

uint8_t  hts221_MeasurComplete(void);

#ifdef __cplusplus
}
#endif

#endif //__HTS221_H__

