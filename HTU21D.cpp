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
    @date          Sept 2017

======================================================================== */

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "HTU21D.hpp"

extern "C" {
#include <hwlib/hwlib.h>
}

#define HTU21D_SAD              0x40  //Unshifted 7-bit I2C address for the sensor
#define WRITE_USER_REG          0xE6
#define READ_USER_REG           0xE7
#define SOFT_RESET              0xFE

HTU21D::HTU21D() : _err(0) 
{ 
    _OpM = getOpMode();
}

HTU21D::~HTU21D() { };

//
// user calls with trigger_mode set to one of these:
//    TRIGGER_TEMP_MEASURE_HOLD  
//    TRIGGER_HUMD_MEASURE_HOLD  
//    TRIGGER_TEMP_MEASURE_NOHOLD
//    TRIGGER_HUMD_MEASURE_NOHOLD
//
// will return the humidity reading or possibly 0
// call getErr() to check reading validity
//

uint16_t HTU21D::readMeasurment(uint8_t cmd, uint16_t dly)
{
    i2c_handle_t my_handle = 0;
    uint8_t      value_read[3] = {0,0,0};
    uint16_t     val = 0;

    _err = 0;
    i2c_bus_init(I2C_BUS_I, &my_handle);
    _err =i2c_write(my_handle, HTU21D_SAD, &cmd, 1, I2C_STOP);
    usleep(dly);
    _err+=i2c_read(my_handle, HTU21D_SAD, value_read, 3);
    i2c_bus_deinit(&my_handle);
    if( !_err ) {
        val = ((uint16_t)value_read[0]<<8) | value_read[1];
        uint8_t crc = value_read[2];
        if( check_crc(val, crc) ) 
            _err=-4;
        }
    return val&0xfffc;
}


bool HTU21D::writeRegister(uint8_t cmd, uint8_t data, uint8_t dsiz)
{
    i2c_handle_t my_handle = 0;
    uint8_t val[2] = {cmd, data};

    i2c_bus_init(I2C_BUS_I, &my_handle);
    int i=i2c_write(my_handle, HTU21D_SAD, val, (dsiz+1), I2C_STOP);
    i2c_bus_deinit(&my_handle);
    return (i);
}


int HTU21D::reset(void)
{
    _err=writeRegister(SOFT_RESET, 0, 0);
    usleep(15000);  //takes upto 15ms for reset
    return _err;
}

uint8_t HTU21D::getOpMode(void)
{
    i2c_handle_t my_handle = 0;
    uint8_t      cmd = READ_USER_REG;
    uint8_t      val = 0;
    i2c_bus_init(I2C_BUS_I, &my_handle);
    _err=i2c_write(my_handle, HTU21D_SAD, &cmd, 1, I2C_STOP);
    _err=i2c_read(my_handle,HTU21D_SAD,&val,1);
    i2c_bus_deinit(&my_handle);
    return val;
}

void HTU21D::setOpMode(uint8_t val)
{
    uint8_t CurVal = _OpM;
    uint8_t valMask = ~val;

    CurVal &= valMask;
    CurVal |= valMask;

    writeRegister(WRITE_USER_REG, CurVal, 1);
    _OpM = getOpMode();
}


float HTU21D::readHumidity(uint8_t trigger_mode)
{
    static int h_msdly [] = { 16000, 8000, 5000, 3000 };
    int i = (_OpM&0x01? 1:0) + (_OpM&0x80? 2:0);
    uint16_t dly = h_msdly[i];
    uint16_t humid = readMeasurment(trigger_mode,dly);
    if( _err )
        return 0;

    return((float)(-6 + (125*humid)/65536.0));
}


float HTU21D::readTemperature(uint8_t trigger_mode)
{
    static int t_msdly [] = { 50000, 25000, 13000, 7000 };
    int i = (_OpM&0x01? 1:0) + (_OpM&0x80? 2:0);
    uint16_t dly = t_msdly[i];
    uint16_t temp = readMeasurment(trigger_mode,dly);
    if( _err )
        return 0;
    
    return((float)(-46.85 + 175.72 * (temp/65536.0)));
}

//Give this function the 2 byte measurement and the crc byte from the HTU21D
//and check it.  Returns 0 if no errors, !0 when errors detected.
//
//POLYNOMIAL used = x^8 + x^5 + x^4 + 1 -> (100110001 or 0x131)
//
//we want the polynomial to be a uint32_t with the msb aligned left so left shift 3 bits to align 
//the nibble, 20 bits to align in a 32-bit varable

#define SHIFTED_DIVISOR ((uint32_t)0x131<<(3+20))  
uint8_t HTU21D::check_crc(uint16_t msg, uint8_t crc)
{
    uint32_t msgPlusCRC = ((uint32_t)msg<<8)|crc;
    uint32_t divisor = SHIFTED_DIVISOR;

    for( int i=0; i<16; i++ ) {
        if( msgPlusCRC & ((uint32_t)1<<(23-i)) )
            msgPlusCRC ^= divisor;
        divisor >>= 1;
        }

    return(msgPlusCRC ^ crc);
}


