
#include "TSYS02D.hpp"
#include <stdio.h>

#define TSYS02D_SAD	0x40

#define RESET_CMD 	0xFE
#define CMD_SN8 	0xFA0F
#define CMD_SN6 	0xFCC9

#define RESET_DLY 	15000  //ms
#define CONVRT_DLY	43000  //ms

uint16_t TSYS02D::readTemp(uint8_t cmd)
{
    i2c_handle_t my_handle = 0;
    uint8_t      value_read[3] = {0,0,0};
    uint16_t     val = 0;

    _err = 0;
    i2c_bus_init(I2C_BUS_I, &my_handle);
    _err =i2c_write(my_handle, TSYS02D_SAD, &cmd, 1, I2C_STOP);
    usleep(CONVRT_DLY);
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

bool TSYS02D::writeRegister(uint8_t cmd, uint8_t data, uint8_t dsiz)
{
    i2c_handle_t my_handle = 0;
    uint8_t val[2] = {cmd, data};

    i2c_bus_init(I2C_BUS_I, &my_handle);
    int i=i2c_write(my_handle, TSYS02D_SAD, val, (dsiz+1), I2C_STOP);
    i2c_bus_deinit(&my_handle);
    return (i);
}

TSYS02D::~TSYS02D() {};  //destructor does nothing.

TSYS02D::TSYS02D()
{ 
    i2c_handle_t my_handle = 0;
    uint16_t val;
    uint8_t  crc, buf[14];

    _err = 0;
    memset(_sn,0x00, sizeof(_sn));
    reset();
    _err=writeRegister((uint8_t)(CMD_SN8 >>8), (uint8_t)(CMD_SN8&0xff), 1);
    i2c_bus_init(I2C_BUS_I, &my_handle);
    _err+=i2c_read(my_handle, TSYS02D_SAD, buf, 8);
    i2c_bus_deinit(&my_handle);
    if( _err )
        return;

    _err=writeRegister((uint8_t)(CMD_SN6>>8), (uint8_t)(CMD_SN6&0xff), 1);
    i2c_bus_init(I2C_BUS_I, &my_handle);
    _err+=i2c_read(my_handle, TSYS02D_SAD, &buf[8], 6);
    i2c_bus_deinit(&my_handle);
    if( !_err ) {
        for( int x=0, i=0; i<14; i+=2) {
            val= _sn[x++] = buf[i];
            if( i>7 ){
                _sn[x++] =buf[i+1];
                val= ((uint16_t)buf[i]<<8) | buf[++i];
                }
            crc = buf[i+1];
            if( check_crc(val, crc) ) 
                _err=-4;
            }
        }
}
    
int TSYS02D::reset(void) 
{
    _err=writeRegister(RESET_CMD, 0, 0);
    usleep(RESET_DLY);  //takes upto 15ms for reset
    return _err;
}

#define TEMP_POLYNOMAL	175.72/(1<<16)-46.85
float   TSYS02D::getTemperature(uint8_t trig)
{
    uint16_t rawT = readTemp(trig);
    return ((float)rawT * TEMP_POLYNOMAL);
}

//Give this function the 2 byte measurement and the crc byte from the TSYS02D
//and check it.  Returns 0 if no errors, !0 when errors detected.
//
//POLYNOMIAL used = x^8 + x^5 + x^4 + 1 -> (100110001 or 0x131)
//
//we want the polynomial to be a uint32_t with the msb aligned left so left shift 3 bits to align 
//the nibble, 20 bits to align in a 32-bit varable

#define SHIFTED_DIVISOR ((uint32_t)0x131<<(3+20))  
uint8_t TSYS02D::check_crc(uint16_t msg, uint8_t crc)
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

