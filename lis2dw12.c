#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <nettle/nettle-stdint.h>
#include <hwlib/hwlib.h>

#include "lis2dw12.h"

//#define  LIS2DW12_SAD	0x18
#define  LIS2DW12_SAD	0x19

//local caches
static i2c_handle_t lis2dw12_i2c = (i2c_handle_t)NULL;

void lis2dw12_read(uint8_t reg_addr, uint8_t *buf, int nbr) {
    uint16_t val;
    
    i2c_write(lis2dw12_i2c, LIS2DW12_SAD, &reg_addr, 1, I2C_NO_STOP);
    i2c_read(lis2dw12_i2c, LIS2DW12_SAD, buf, nbr);
}

uint16_t lis2dw12_read_word(uint8_t reg_addr) {
    uint16_t val;
    
    i2c_write(lis2dw12_i2c, LIS2DW12_SAD, &reg_addr, 1, I2C_NO_STOP);
    i2c_read(lis2dw12_i2c, LIS2DW12_SAD, (uint8_t*)&val, 2);
    return val;
}

uint8_t lis2dw12_read_byte(uint8_t reg_addr) {
    uint8_t value_read;

    i2c_write(lis2dw12_i2c, LIS2DW12_SAD, &reg_addr, 1, I2C_NO_STOP);
    i2c_read(lis2dw12_i2c, LIS2DW12_SAD, &value_read, 1);
    return value_read;
}

void lis2dw12_write_reg(uint8_t reg_addr, uint8_t value) {
	uint8_t buffer_sent[2];

	buffer_sent[0] = reg_addr;
	buffer_sent[1] = value;
	i2c_write(lis2dw12_i2c, LIS2DW12_SAD, buffer_sent, 2, I2C_STOP);
}


int lis2dw12_initialize(void) {
    uint8_t value_read;
    uint8_t reg_addr = LIS2DW12_WHO_AM_I_ADDR;

    if (i2c_bus_init(I2C_BUS_I, &lis2dw12_i2c) != 0) {
        fprintf(stderr,"fail to initialize I2C peripheral !!\n");
        return (-1);
        }

//    if (lis2dw12_getDeviceID() != LIS2DW12_WHO_AM_I_DEF)
//    	return (-2);

    printf("init:i2c_write(0x%02X, 0x%02x)=%d\n", LIS2DW12_SAD, reg_addr, i2c_write(lis2dw12_i2c, LIS2DW12_SAD, &reg_addr, 1, I2C_NO_STOP));
    printf("init:i2c_read(0x%02x)=%d\n", LIS2DW12_SAD, i2c_read( lis2dw12_i2c, LIS2DW12_SAD, &value_read, 1));
    printf("init:value_read=0x%02X\n",value_read);
    if (value_read != LIS2DW12_WHO_AM_I_DEF)
    	return (-2);
    else	
        return 0;
}

uint8_t lis2dw12_getDeviceID(void) {
    return lis2dw12_read_byte(LIS2DW12_WHO_AM_I_ADDR);
}

