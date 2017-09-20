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

    @file          KMA36.hpp
    @version       1.0
    @date          Sept 2017

======================================================================== */

#ifndef __KMA36_HPP__
#define __KMA36_HPP__

#include <unistd.h>
#include <stdint.h>
#include <math.h>

#include <cstring>
#include <stdio.h>

#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

extern "C" {
#include <hwlib/hwlib.h>
}

#define KMA36_SAD    (0x59>>1)    // 0101 1001 
#define CONFIG_DLY   151000   // ms delay for configuration to take effect

enum config_bits {
    SLP_BIT  =       0b10000000,
    LIN_BIT  =       0b00100000,
    CNT_BIT  =       0b00010000,
    PWR_BIT  =       0b00001000,
    SPD_BIT  =       0b00000100,  //SPD_BIT=0 const=1 SPD_BIT=1 const=2
    OVSC_BIT =       0b00000011   //uses the OVSC_x defines
};

enum oversample {
    OVSC_2X  =       0b00000000,
    OVSC_4X  =       0b00000001,
    OVSC_8X  =       0b00000010,
    OVSC_32X =       0b00000011
};


// The TX Buffer index to registers per data sheet
#define KCONF        0
#define KRESH        1
#define KRESL        2

class KMA36 {

public:
    //constructor just initializes register values from KMA36, _err will be set
    KMA36(uint8_t addr=KMA36_SAD)  {
printf("constructor called. Using addr=0x%02X. now call readData()\n",addr);
             i2c_handle_t my_handle = 0;
             _kma_addr = addr;
             _err=-1;
             for( int tries=0; tries<10 && _err; tries++ ) {
                 _err =i2c_bus_init(I2C_BUS_I, &my_handle);
                 _err+=i2c_write(my_handle, _kma_addr, (uint8_t*)&_err, 1, I2C_STOP);
                 if( _err ) {
                     usleep(CONFIG_DLY);
                     i2c_bus_deinit(&my_handle);
                     }
                 }

             uint8_t *data = readData();
             if( data ) {
                 _regData[0] = _data[6];
                 _regData[KRESL] = _data[7];
                 _regData[KRESH] = _data[8];
                 }
             }

    ~KMA36(){};   //nothing to do in destructor

    int      getErr(void)         { return _err; }
    int      sleep(int setClr )   { set_setConfigReg(SLP_BIT, setClr?SLP_BIT:0); }
    int      lowPower(int setClr ){ set_setConfigReg(PWR_BIT, setClr?PWR_BIT:0); }
    int      counter(int setClr ) { set_setConfigReg(CNT_BIT, setClr?CNT_BIT:0); }
    int      fastRate(int setClr ){ set_setConfigReg(SPD_BIT, setClr?SPD_BIT:0); }
    int      setOversample(oversample rate){ set_setConfigReg(OVSC_BIT, (uint8_t)rate); }

    int      setResolution(uint16_t val)
             {
             _regData[KRESH] = (val>>8)&0xff;
             _regData[KRESL] = (val & 0xff );
             return _err=sendData();
             }

    float    readAngle(void);
    uint32_t readLinear(void);

private:
    int      _err;
    uint8_t  _data[10];
    uint8_t  _regData[4];
    uint8_t  _kma_addr;

    uint8_t  getConfigReg(void) { return _regData[0]; }
    uint16_t getResolution(void) {return ((uint16_t)_regData[KRESH]<<8) | _regData[KRESL];}
    int      set_setConfigReg(config_bits bit, uint8_t val)
             {
                 uint8_t b = _regData[0] & ~bit;
                 val = (bit==OVSC_BIT)? val : (val)?bit:0;
                 _regData[0] = b | val;
                 return _err=sendData();
             }

    uint8_t *readData(void)
             {
             i2c_handle_t my_handle = 0;
             uint8_t      tries, chksum, i;

             memset(_data,0x00,sizeof(_data));
             _err = -1;
             for( tries=0; tries<10 && _err; tries++ ) {
                 _err =i2c_bus_init(I2C_BUS_I, &my_handle);
                 _err+=i2c_read(my_handle, _kma_addr, _data, sizeof(_data));
                 if( _err ) {
                     usleep(CONFIG_DLY);
                     i2c_bus_deinit(&my_handle);
                     }
                 }
              if( !_err ){
                 i2c_bus_deinit(&my_handle);
                 for( chksum=i=0; i<sizeof(_data)-1; i++ )
                     _data[i] += chksum;
                 if( _data[i] != chksum ) {
                     _err=-4;
                     return NULL;
                     }
                 }
             return _data;
             }


    int      sendData(void)
             {
             int i;
             i2c_handle_t my_handle = 0;
printf("sendData()\n");
             _regData[3] = 0xff - (_regData[0] + _regData[1] + _regData[2]) + 1; //calculate the CHKSUM to send
i=           i2c_bus_init(I2C_BUS_I, &my_handle);
printf("init=%d\n",i);
printf("%d=i2c_writex,0x%02X,y,%d)= ",i,_kma_addr,sizeof(_data)); for(i=0;i<sizeof(_regData);i++) printf("0x%02X ",_regData[i]);
             i=i2c_write(my_handle, _kma_addr, _regData, sizeof(_regData), I2C_STOP);
i=           i2c_bus_deinit(&my_handle);
printf("\ndeinit=%d\n",i);
             usleep(CONFIG_DLY);
             return (i);
             }
};

#endif


