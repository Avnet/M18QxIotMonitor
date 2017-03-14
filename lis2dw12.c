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
#include "iot_monitor.h"
#include "lis2dw12.h"

#define  LIS2DW12_SAD	0x19

//local caches
static i2c_handle_t lis2dw12_i2c;

void lis2dw12_read(uint8_t reg_addr, uint8_t *buf, int nbr) {
    uint16_t val;
    
    if( dbg_flag & DBG_LIS2DW12 )
        printf("-LIS2DW12: read SAD=0x%02X, REG ADDR=%d\n",LIS2DW12_SAD,reg_addr);

    i2c_write(lis2dw12_i2c, LIS2DW12_SAD, &reg_addr, 1, I2C_NO_STOP);
    i2c_read(lis2dw12_i2c, LIS2DW12_SAD, buf, nbr);
}

uint16_t lis2dw12_read_word(uint8_t reg_addr) {
    uint16_t val;
    
    if( dbg_flag & DBG_LIS2DW12 )
        printf("-LIS2DW12: read_word SAD=0x%02X, REG ADDR=%d\n",LIS2DW12_SAD,reg_addr);

    i2c_write(lis2dw12_i2c, LIS2DW12_SAD, &reg_addr, 1, I2C_NO_STOP);
    i2c_read(lis2dw12_i2c, LIS2DW12_SAD, (uint8_t*)&val, 2);
    return val;
}

uint8_t lis2dw12_read_byte(uint8_t reg_addr) {
    uint8_t value_read;
    int r;

    if( dbg_flag & DBG_LIS2DW12 )
        printf("-LIS2DW12: read_byte SAD=0x%02X, REG ADDR=%d\n",LIS2DW12_SAD, reg_addr);

    if( (r=i2c_write(lis2dw12_i2c, LIS2DW12_SAD, &reg_addr, 1, I2C_NO_STOP)) < 0 )
        fprintf(stderr,"fail to write to LIS2DW12 (0x%02X)!!\n",r);

    if( (r=i2c_read(lis2dw12_i2c, LIS2DW12_SAD, &value_read, 1)) < 0 )
        fprintf(stderr,"fail to read from LIS2DW12 (0x%02X)!!\n",r);

    return value_read;
}

void lis2dw12_write_reg(uint8_t reg_addr, uint8_t value) {
    uint8_t buffer_sent[2];
    int r;

    if( dbg_flag & DBG_LIS2DW12 )
        printf("-LIS2DW12: write_reg SAD=0x%02X, REG ADDR= %d, VALUE=0x%02X\n",LIS2DW12_SAD,reg_addr,value);

    buffer_sent[0] = reg_addr;
    buffer_sent[1] = value;
    if( (r=i2c_write(lis2dw12_i2c, LIS2DW12_SAD, buffer_sent, 2, I2C_STOP)) < 0) 
        fprintf(stderr,"fail to write to LIS2DW12 (0x%02X)!!\n",r);
}

int lis2dw12_initialize(void) {
    uint8_t value_read;
    uint8_t reg_addr = LIS2DW12_WHO_AM_I_ADDR;
    int r;

    if( !lis2dw12_i2c ) {
        if ((r=i2c_bus_init(I2C_BUS_I, &lis2dw12_i2c)) < 0) {
            fprintf(stderr,"fail to initialize I2C peripheral (0x%02X)!!\n",r);
            return (-1);
            }
        }

    if (lis2dw12_getDeviceID() != LIS2DW12_WHO_AM_I_DEF)
    	return (-2);
    else	
        return 0;
}

uint8_t lis2dw12_getDeviceID(void) {
    return lis2dw12_read_byte(LIS2DW12_WHO_AM_I_ADDR);
}

