#include "TSYS01.hpp"

#define TSYS01_SAD          0x77  
#define RESET_CMD           0x1E
#define READ_CMD            0x00
#define ADC_CMD             0x48
#define PROM_BASE           0XA0

#define CONVRT_DLY          10000  //ms
#define RESET_DLY           2800   //ms

TSYS01::~TSYS01() {};  //destructor does nothing

//
// Constructor resets device and reads the calibration constants in
//
TSYS01::TSYS01() 
{
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


bool TSYS01::writeRegister(uint8_t cmd, uint8_t data, uint8_t dsiz)
{
    int i;
    uint8_t val[2] = {cmd, data};
    i2c_handle_t my_handle = 0;

    i2c_bus_init(I2C_BUS_I, &my_handle);
    i=i2c_write(my_handle, TSYS01_SAD, val, (dsiz+1), I2C_STOP);
    i2c_bus_deinit(&my_handle);
    return (i);
}


int TSYS01::reset(void) 
{
    _err = 0;
    _err=writeRegister(RESET_CMD, 0, 0);
    usleep(RESET_DLY);  
    return _err;
}

int32_t TSYS01::readADC(void)
{
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


float   TSYS01::getTemperature(void)
{
    uint32_t adc = readADC() / 256;

    float temp = k[4] * -2 * 1e-21 * pow(adc,4) + 
                 k[3] *  4 * 1e-16 * pow(adc,3) +
                 k[2] * -2 * 1e-11 * pow(adc,2) +
                 k[1] *  1 * 1e-6  * adc +
                 k[0] *-1.5* 1e-2;
    return temp;
}

