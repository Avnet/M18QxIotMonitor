/* =====================================================================
   Copyright © 2016, Avnet (R)

   www.avnet.com 
 
   Licensed under the Apache License, Version 2.0 (the "License"); 
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, 
   software distributed under the License is distributed on an 
   "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
   either express or implied. See the License for the specific 
   language governing permissions and limitations under the License.

    @file          TSYS01.hpp
    @version       1.0
    @date          July 2017

======================================================================== */

#ifndef __TSYS01_HPP__
#define __TSYS01_HPP__

#include <unistd.h>
#include <stdint.h>
#include <math.h>

#include <cstring>

#define TSYS01_SAD          0x77    //I2C Salave Address

#define TSY01_RESET_CMD     0x1E
#define READ_CMD            0x00
#define ADC_CMD             0x48
#define PROM_BASE           0XA0

#define CONVRT_DLY          10000  //us = 10ms per Data Sheet
#define RESET_DLY           2800   //ms = 2.8ma per Data Sheet

extern "C" {                       //include WNC SDK for I2C functions
#include <hwlib/hwlib.h>
}

//include the C to F conversion macro if it hasn't been included
#ifndef CTOF
#define CTOF(x)  ((x)*(float)1.8+32) //celcius to Farenheight
#endif

class TSYS01 {

public:

//
//destructor does nothing
//
    ~TSYS01() {};

//
// Constructor resets device and reads the calibration constants into
//  the array of constants (k[..]);
//
    TSYS01() {
        i2c_handle_t my_handle = 0;
        uint8_t  cmd;
        uint16_t val;

        reset();
        i2c_bus_init(I2C_BUS_I, &my_handle);
        for(int i=1; i<6; i++) {
            cmd = PROM_BASE+(i*2);
            _err+=i2c_write(my_handle, TSYS01_SAD, &cmd, 1, I2C_STOP);
            _err+=i2c_read(my_handle, TSYS01_SAD, (uint8_t*)&val, 2);
            k[5-i] = ((val>>8)&0xff) | ((val<<8)&0xff00);
            }
        i2c_bus_deinit(&my_handle);
    }


//
// Reset issues a reset command to the device, clear the internal error 
// variable and waits for the RESET DELAY duration before exiting
//
    int     reset(void) {
        _err = 0;
        _err=writeRegister(TSY01_RESET_CMD, 0, 0);
        usleep(RESET_DLY);  
        return _err;
    }

//
// return the value of the internal error variable so the caller
// can determine if any errors have occured.
//
    int     getErr(void) { return _err; }

//
// read the ADC and convert the value to the temperature using
// the caculation specified in the data sheet
//
    float   getTemperature(void) {
        uint32_t adc = readADC() / 256;
    
        float temp = k[4] * -2 * 1e-21 * pow(adc,4) + 
                     k[3] *  4 * 1e-16 * pow(adc,3) +
                     k[2] * -2 * 1e-11 * pow(adc,2) +
                     k[1] *  1 * 1e-6  * adc +
                     k[0] *-1.5* 1e-2;
        return temp;
    }

private:
    int      _err;		//for tracking errors that occur
    uint16_t k[8];              //holds the TSYS01 conversion constants

//
// Routine to read the TSYS01 ADC
//
    int32_t readADC(void) {
        i2c_handle_t my_handle = 0;
        uint8_t      val[3] = {0,0,0};
        uint8_t      cmd[2] = {ADC_CMD, READ_CMD};
        int32_t      ret = 0;

        i2c_bus_init(I2C_BUS_I, &my_handle);
        _err =i2c_write(my_handle, TSYS01_SAD, cmd, 1, I2C_STOP);
        usleep(CONVRT_DLY);

        _err+=i2c_write(my_handle, TSYS01_SAD, &cmd[1], 1, I2C_STOP);
        _err+=i2c_read(my_handle, TSYS01_SAD, val, 3);
        i2c_bus_deinit(&my_handle);
        if( !_err ) 
            ret = ((int32_t)val[0]<<16) | (val[1]<<8) | val[2];
        return ret;
    }


//
// Routine to wriate data to the TSYS01
//
    int writeRegister(uint8_t cmd, uint8_t data, uint8_t dsiz) {
        int i;
        uint8_t val[2] = {cmd, data};
        i2c_handle_t my_handle = 0;

        i2c_bus_init(I2C_BUS_I, &my_handle);
        i=i2c_write(my_handle, TSYS01_SAD, val, (dsiz+1), I2C_STOP);
        i2c_bus_deinit(&my_handle);
        return (i);
    }

};

#endif

