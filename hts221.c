
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

#include "hts221.h"

//local caches

uint8_t reg_cache_T0_degC_x8 = 0;
uint8_t reg_cache_T1_degC_x8 = 0;
uint8_t reg_cache_T1_T0_msb = 0;
uint8_t reg_cache_T0_OUT_L = 0;
uint8_t reg_cache_T0_OUT_H = 0;
uint8_t reg_cache_T1_OUT_L = 0;
uint8_t reg_cache_T1_OUT_H = 0;

void hts221_read(uint8_t reg_addr, uint8_t *buf, int nbr) {
    i2c_handle_t my_handle = 0;
    uint16_t val;
    
    i2c_bus_init(I2C_BUS_I, &my_handle);
    i2c_write(my_handle, HTS221_SAD, &reg_addr, 1, I2C_NO_STOP);
    i2c_read(my_handle, HTS221_SAD, buf, nbr);
    i2c_bus_deinit(&my_handle);
}

uint16_t hts221_read_word(uint8_t reg_addr) {
    i2c_handle_t my_handle = 0;
    uint16_t val;
    
    i2c_bus_init(I2C_BUS_I, &my_handle);
    i2c_write(my_handle, HTS221_SAD, &reg_addr, 1, I2C_NO_STOP);
    i2c_read(my_handle, HTS221_SAD, (uint8_t*)&val, 2);
    i2c_bus_deinit(&my_handle);
    return val;
}

uint8_t hts221_read_byte(uint8_t reg_addr) {
    i2c_handle_t my_handle = 0;
    uint8_t value_read;

    i2c_bus_init(I2C_BUS_I, &my_handle);
    i2c_write(my_handle, HTS221_SAD, &reg_addr, 1, I2C_NO_STOP);
    i2c_read(my_handle, HTS221_SAD, &value_read, 1);
    i2c_bus_deinit(&my_handle);
    return value_read;
}

void hts221_write_reg(uint8_t reg_addr, uint8_t value) {
    i2c_handle_t my_handle = 0;
    uint8_t buffer_sent[2];

    buffer_sent[0] = reg_addr;
    buffer_sent[1] = value;
    i2c_bus_init(I2C_BUS_I, &my_handle);
    i2c_write(my_handle, HTS221_SAD, buffer_sent, 2, I2C_STOP);
    i2c_bus_deinit(&my_handle);
}


float hts221_getHumid(void) 
{
    int16_t H0_T0_out, H1_T0_out, H_T_out;
    int16_t H0_rh, H1_rh, reg;
    float   value = -1;
  
    if (hts221_MeasurComplete() & HTS221_HUMIDITY_AVAILABLE) {
        reg = hts221_read_word(HTS221_H0_RH_X2);
        H0_rh = (reg&0x00ff)>>1;
        H1_rh = (reg&0xff00)>>9;

        H0_T0_out = hts221_read_word(HTS221_H0_T0_OUT_L);
        H1_T0_out = hts221_read_word(HTS221_H1_T0_OUT_L);
        H_T_out   = hts221_read_word(HTS221_HR_OUT_L_REG);

        value = ((uint32_t)(H_T_out - H0_T0_out)) * ((uint32_t)(H1_rh - H0_rh)*10);
        value = value/(H1_T0_out - H0_T0_out)  + H0_rh*10;
  
        if(value>1000) value = 1000;
        }
  return value;
}


float hts221_getTemp(void) {
    uint8_t tl = 0, th = 0;
    float current_measured = 0;
    float cal_t0 = 0, cal_t1 = 0, degc_t0 = 0, degc_t1 = 0;
    float ret_temperature = -101;	//temperatue is impossible -100 degree C on the earth

    if (hts221_MeasurComplete() & HTS221_TEMP_AVAILABLE) {
        tl = hts221_read_byte(HTS221_REG_TEMP_OUT_L);
        th |= hts221_read_byte(HTS221_REG_TEMP_OUT_H);
        current_measured = (float)((((uint16_t)th << 8) & 0xff00) | ((uint16_t)tl & 0x00ff));
        cal_t0 = (float)(reg_cache_T0_OUT_L | (((uint16_t)reg_cache_T0_OUT_H << 8) & 0xff00));
        cal_t1 = (float)(reg_cache_T1_OUT_L | (((uint16_t)reg_cache_T1_OUT_H << 8) & 0xff00));
        degc_t0 = (float)(reg_cache_T0_degC_x8 | (((uint16_t)reg_cache_T1_T0_msb << 8) & 0x0300)) / 8;
        degc_t1 = (float)(reg_cache_T1_degC_x8 | (((uint16_t)reg_cache_T1_T0_msb << 6) & 0x0300)) / 8;
        //Linear interpolation
        ret_temperature = degc_t0 + ( ((current_measured - cal_t0) * (degc_t1 - degc_t0)) / (cal_t1 - cal_t0) );
        }
    return ret_temperature;
}

int hts221_initialize(void) {

    if (hts221_getDeviceID() != HTS221_DEVICE_ID)
	return (-2);
	
    //initialize other registers.
    hts221_write_reg(HTS221_REG_CTRL_REG1, 0x85);	//power active, block data update until MSB LSB all ready, 1Hz report
    hts221_write_reg(HTS221_REG_CTRL_REG2, 0x00);	//normal boot, heater disabled, one shot disabled
    hts221_write_reg(HTS221_REG_CTRL_REG3, 0x00);	//active high, push-pull mode, DRDY pin disabled

    //read calibration data for later calculating.
    reg_cache_T0_degC_x8 = hts221_read_byte(HTS221_REG_T0_DEGC_X8);
    reg_cache_T1_degC_x8 = hts221_read_byte(HTS221_REG_T1_DEGC_X8);
    reg_cache_T1_T0_msb = hts221_read_byte(HTS221_REG_T1_T0_MSB);
    reg_cache_T0_OUT_L = hts221_read_byte(HTS221_REG_T0_OUT_L);
    reg_cache_T0_OUT_H = hts221_read_byte(HTS221_REG_T0_OUT_H);
    reg_cache_T1_OUT_L = hts221_read_byte(HTS221_REG_T1_OUT_L);
    reg_cache_T1_OUT_H = hts221_read_byte(HTS221_REG_T1_OUT_H);

    return 0;
}


uint8_t hts221_MeasurComplete(void) {
    return (hts221_read_byte(HTS221_REG_STATUS) & (HTS221_TEMP_AVAILABLE | HTS221_HUMIDITY_AVAILABLE));
}
 
uint8_t hts221_getDeviceID(void) {
    return (hts221_read_byte(HTS221_REG_WHO_AM_I) != HTS221_DEVICE_ID)?-1:HTS221_DEVICE_ID;
}


