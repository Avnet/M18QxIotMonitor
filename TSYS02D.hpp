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

#ifndef __TSYS02D_HPP__
#define __TSYS02D_HPP__

#include <unistd.h>
#include <stdint.h>

#include <cstring>

#define TSYS02D_SAD	   0x40

#define TSYS02D_RESET_CMD  0xFE
#define GETSN8_CMD 	   0xFA0F
#define GETSN6_CMD 	   0xFCC9

#define TSY02_RESET_DLY    15000  //us = 15ms
#define TSY02_CONVRT_DLY   43000  //us = 43ms

extern "C" {
#include <hwlib/hwlib.h>
}

#define READT_HOLD 	   0xE3
#define READT_NOHOLD	   0xF3

//include the C to F conversion macro if it hasn't been included
#ifndef CTOF
#define CTOF(x)  ((x)*(float)1.8+32) //celcius to Farenheight
#endif

//This Polynomial is specified in the Datasheet to use in caculating the
//temperature from the raw ADC reading
#define TEMP_POLYNOMAL	175.72/(1<<16)-46.85

//This device always includes a crc byte for the 2 byte measurement from the TSYS02D. The
//POLYNOMIAL used is "x^8 + x^5 + x^4 + 1"-> (100110001 or 0x131)
//
//we want the polynomial to be a uint32_t with the msb aligned left so left shift 3 bits to align 
//the nibble, 20 bits to align in a 32-bit varable
#define SHIFTED_DIVISOR ((uint32_t)0x131<<(3+20))  

class TSYS02D {

public:
//
// Destructor does nothing.
//
    ~TSYS02D() {};  //destructor does nothing.

//
// Constructor reads the TSY02D serial number and stores it for later access. If 
// we are unable to access the TSYS02D, the constructor exits with the internal 
// error variable set as appropriate.
//
    TSYS02D() { 
        i2c_handle_t my_handle = 0;
        uint16_t val;
        uint8_t  crc, buf[14];
    
        _err = 0;
        memset(_sn,0x00, sizeof(_sn));
        reset();

        _err=writeRegister((uint8_t)(GETSN8_CMD >>8), (uint8_t)(GETSN8_CMD&0xff), 1);
        if( _err ) 
            return;
        i2c_bus_init(I2C_BUS_I, &my_handle);
        _err=i2c_read(my_handle, TSYS02D_SAD, buf, 8);
        i2c_bus_deinit(&my_handle);
        if( _err ) 
            return;
    
        _err=writeRegister((uint8_t)(GETSN6_CMD>>8), (uint8_t)(GETSN6_CMD&0xff), 1);
        i2c_bus_init(I2C_BUS_I, &my_handle);
        _err+=i2c_read(my_handle, TSYS02D_SAD, &buf[8], 6);
        i2c_bus_deinit(&my_handle);
        if( !_err ) {  //if no error then process the SN string
            for( int x=0, i=0; i<14; i+=2) {
                val= _sn[x++] = buf[i];
                if( i>7 ){
                    _sn[x++] =buf[i+1];
                    val= ((uint16_t)buf[i]<<8) | buf[++i];
                    }
                crc = buf[i+1];
                if( check_crc(val, crc) )   //if the CRC fails, signal an erro 
                    _err=-4;
                }
            }
    }
    
//
// Reset simploy issues a reset command to the TYSY02D.
//
    int     reset(void) { _err=writeRegister(TSYS02D_RESET_CMD, 0, 0); usleep(TSY02_RESET_DLY); return _err; }

//
// return the value of the error variable to the caller
//
    int     getErr(void) { return _err; }

//
// return a pointer to the TSYS02D serial number
//
    uint8_t *getSN(void) { return _sn; }

//
// return the current Temperature reading after scaling
//
    float   getTemperature(uint8_t trig) { uint16_t rawT = readTemp(trig); return ((float)rawT * TEMP_POLYNOMAL); }



private:
    int      _err;    //holds any errors that occur
    uint8_t  _sn[8];  //contains the TSYS02D serial number that was read from the device

//
// Read the raw temperature value from the TSYS02D, convert it to an unsigned 16 bit value
// and return it.  It will return 0 if an error occurs, call getErr() to ensure no errors occured.
//
    uint16_t readTemp(uint8_t cmd) {
        i2c_handle_t my_handle = 0;
        uint8_t      value_read[3] = {0,0,0};
        uint16_t     val = 0;

        _err = 0;
        i2c_bus_init(I2C_BUS_I, &my_handle);
        _err =i2c_write(my_handle, TSYS02D_SAD, &cmd, 1, I2C_STOP);
        usleep(TSY02_CONVRT_DLY);
        _err+=i2c_read(my_handle, TSYS02D_SAD, value_read, 3);
        i2c_bus_deinit(&my_handle);
        if( !_err ) {
            val = ((uint16_t)value_read[0]<<8) | value_read[1];
            uint8_t crc = value_read[2];
            if( check_crc(val, crc) ) 
                _err=-4;
            }
        return val;
    }


//
// Write the requested data to the TSYS02D
//
    bool     writeRegister(uint8_t cmd, uint8_t data, uint8_t dsiz) {
        i2c_handle_t my_handle = 0;
        uint8_t val[2] = {cmd, data};

        i2c_bus_init(I2C_BUS_I, &my_handle);
        int i=i2c_write(my_handle, TSYS02D_SAD, val, (dsiz+1), I2C_STOP);
        i2c_bus_deinit(&my_handle);
        return (i);
    }

//
// This is the routine used to caculate the CRC for received data. See above for discussion of SHIFTED_DIVISOR
//
    uint8_t  check_crc(uint16_t msg, uint8_t crc) {
        uint32_t msgPlusCRC = ((uint32_t)msg<<8)|crc;
        uint32_t divisor = SHIFTED_DIVISOR;

        for( int i=0; i<16; i++ ) {
            if( msgPlusCRC & ((uint32_t)1<<(23-i)) )
                msgPlusCRC ^= divisor;
            divisor >>= 1;
            }
        return(msgPlusCRC ^ crc);
    }

};

#endif

