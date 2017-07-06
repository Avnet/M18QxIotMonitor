// MS5637_02BA03

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <cmath>

#include "MS5637.hpp"

extern "C" { 
#include <hwlib/hwlib.h>
}


#define MS5637_SAD 0x76       // MS5637_02BA03 I2C address

//
// The MS5637 does not have any registers to write to or read from
// so you always talk to the Slave Address.  The following are the
// command the MS5637 Accepts.  Calibration data is all off the 
// prom base address.
//
#define RESET                 0x1e
#define READ_CMD              0x00
#define PROM_READ_BASE        0xA2


// Send the 'cmd' supplied and then read 'siz' bytes. Return
// to the caller as an uint32_t.  The user specifies how many
// bytes (siz) to use when construction the uint32_t.  This 
// routines is only setup to read either 2(16bits) or three 
// bytes (24 bits).
// 
uint32_t MS5637::readRegister(uint8_t cmd, uint8_t siz)
{
    i2c_handle_t my_handle = 0;
    uint8_t      value_read[3] = {0,0,0};
    uint32_t     val = 0;

    i2c_bus_init(I2C_BUS_I, &my_handle);
    err=i2c_write(my_handle, MS5637_SAD, &cmd, 1, I2C_STOP);
    err=i2c_read(my_handle,MS5637_SAD,value_read,siz);
    i2c_bus_deinit(&my_handle);

    if( siz==3 )
        val = ((uint32_t)value_read[0]<<16) | ((uint16_t)value_read[1]<<8) | (value_read[2]);
    else
        val = ((uint32_t)value_read[0]<<8) | value_read[1];

    return val;
}


// Write a single uint8_t value 
bool MS5637::writeRegister(uint8_t cmd)
{
    i2c_handle_t my_handle = 0;
    int i;

    i2c_bus_init(I2C_BUS_I, &my_handle);
    i=i2c_write(my_handle, MS5637_SAD, (uint8_t*)&cmd, 1, I2C_STOP);
    i2c_bus_deinit(&my_handle);
    return (i);
}


//
// Constructor -- When Initialized, read the factory calibrated coefficients from the device
// and save them for later.
//
MS5637::MS5637(void): d1_mode(D1MODE_OSR256), d2_mode(D2MODE_OSR256) 
{
    err = 0;
    for(int i = 0; i < 6; i++) 
        _Coff[i] = readRegister(PROM_READ_BASE+(2*i), 2);

}

// destructor doesn't do anything
MS5637::~MS5637(void) {};


//
// setMode allows the user to set the operating mode of the MS5637 so that performance is 
// optimized for conversion speed and current consumption.  If they don't set a mode
// it will use the fast conversion/lowest accuracy setting.
//
// returns -1 if an incorrect parameter, 0 otherwise.  Parameters are defined in the 
//    associated header file.
//
int MS5637::setMode(uint8_t d1, uint8_t d2)
{
    if( (d1>D1MODE_OSR8192 || d1<D1MODE_OSR256) ||
        (d2>D2MODE_OSR8192 || d2<D2MODE_OSR256) )
        return -1;

    d1_mode = d1;
    d2_mode = d2;
    return 0;
}

//
// Performs a Pressume & Temperature Read.
//
float * MS5637::getPT(void)
{
    uint32_t dly, conv_delay[] = { 
       CONVT_OSR256, CONVT_OSR512, CONVT_OSR1024, CONVT_OSR2048, CONVT_OSR4096, CONVT_OSR8192 };
    uint32_t D1, D2, dT;
    uint32_t TEMP, Ti, OFFi, SENSi;
    uint64_t OFF;
    uint64_t SENS;

    results[0] = results[1] = 0;
//
// The only way err could be != 0 is if an error occured during initialization, so simply exit...
    if( err )
        return NULL;

    dly = conv_delay[(d1_mode&0xf)>>1]*1000;
    writeRegister(RESET);
    writeRegister(d1_mode);
    usleep(dly); //let conversion complete before reading
    D1 = readRegister(READ_CMD,3);
// If an error occurs during the device read operation, exit 
    if( err )
        return NULL;

    dly = conv_delay[(d2_mode&0xf)>>1]*1000;
    writeRegister(d2_mode);
    usleep(dly); //let conversion complete before reading
    D2 = readRegister(READ_CMD,3);
// If an error occurs during the device read operation, exit 
    if( err )
        return NULL;

    // Caculate 1st order values
    dT = D2 - (_Coff[4] * pow(2,8));
    TEMP = 2000 + (dT * (_Coff[5] / pow(2,23))); 

    // Offset and Sensitivity calculation
    OFF = _Coff[1] * 131072 + (_Coff[3] * dT) / 64;
    SENS = _Coff[0] * 65536 + (_Coff[2] * dT) / 128;

    // 2nd order temperature and pressure compensation

    if(TEMP < 2000) {
        Ti = (dT * dT) / pow(2,31);
        OFFi = 5 * (pow((TEMP-2000),2)) / 2; 
        SENSi = OFFi / 2;
        if(TEMP < -1500 ) {
            OFFi = OFFi+7 * pow((TEMP+1500),2);
            SENSi = SENSi+ 11 * pow((TEMP+1500),2);
            }
        }
    else if(TEMP >= 2000) {
        Ti = 0;
        OFFi = 0;
        SENSi = 0;
        }

    // Adjust temp, off, sens based on 2nd order compensation   
    TEMP -= Ti;
    OFF -= OFFi;
    SENS -= SENSi;

    D1 = (((D1 * SENS) / 2097152) - OFF);
    D1 /= 32768;
    results[0] = D1 / 100.0;     //in millibars
    results[1] = TEMP/100.0;     //in C

    return results;
}

