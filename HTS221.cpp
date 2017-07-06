/*
 * HTS221.cpp
 *
 */

#include <stdint.h>
#include "HTS221.hpp"

extern "C" { 
#include <hwlib/hwlib.h>
}

#define HTS221_SAD			0x5F	// slave address

static inline bool humidityReady(uint8_t data) { return (data & 0x02); }
static inline bool temperatureReady(uint8_t data) { return (data & 0x01); }

HTS221::HTS221(void) : _temperature(0.0), _humidity(0.0) 
{
    _hts221Inited = 0;
    _iAm = readRegister(WHO_AM_I);
    if (_iAm != I_AM_HTS221)
        return;
    activate();
    storeCalibration();
    _hts221Inited = 1;
}


HTS221::~HTS221(void) 
{
}


void HTS221::activate(void)
{
    uint8_t data;

    if( !_hts221Inited )
        return;
    data = readRegister(CTRL_REG1);
    data |= POWER_UP;
    data |= ODR0_SET;
    writeRegister(CTRL_REG1, data);
}


void HTS221::deactivate(void)
{
    uint8_t data;

    if( !_hts221Inited )
        return;
    data = readRegister(CTRL_REG1);
    data &= ~POWER_UP;
    writeRegister(CTRL_REG1, data);
}



bool HTS221::storeCalibration(void)
{
    uint8_t data;
    uint16_t tmp;

    if( !_hts221Inited )
        return false;
    for (int reg=CALIB_START; reg<=CALIB_END; reg++) {
        if ((reg!=CALIB_START+8) && (reg!=CALIB_START+9) && (reg!=CALIB_START+4)) {

            data = readRegister(reg);

            switch (reg) {
            case CALIB_START:
                _h0_rH = data;
                break;
            case CALIB_START+1:
            _h1_rH = data;
            break;
            case CALIB_START+2:
            _T0_degC = data;
            break;
            case CALIB_START+3:
            _T1_degC = data;
            break;

            case CALIB_START+5:
            tmp = _T0_degC;
            _T0_degC = (data&0x3)<<8;
            _T0_degC |= tmp;

            tmp = _T1_degC;
            _T1_degC = ((data&0xC)>>2)<<8;
            _T1_degC |= tmp;
            break;
            case CALIB_START+6:
            _H0_T0 = data;
            break;
            case CALIB_START+7:
            _H0_T0 |= data<<8;
            break;
            case CALIB_START+0xA:
            _H1_T0 = data;
            break;
            case CALIB_START+0xB:
            _H1_T0 |= data <<8;
            break;
            case CALIB_START+0xC:
            _T0_OUT = data;
            break;
            case CALIB_START+0xD:
            _T0_OUT |= data << 8;
            break;
            case CALIB_START+0xE:
            _T1_OUT = data;
            break;
            case CALIB_START+0xF:
            _T1_OUT |= data << 8;
            break;


            case CALIB_START+8:
            case CALIB_START+9:
            case CALIB_START+4:
            //DO NOTHING
            break;

            // to cover any possible error
            default:
                return false;
            } /* switch */
        } /* if */
    }  /* for */
    return true;
}



bool HTS221::bduActivate(void)
{
    uint8_t data;

    if( !_hts221Inited )
        return false;
    data = readRegister(CTRL_REG1);
    data |= BDU_SET;
    writeRegister(CTRL_REG1, data);

    return true;
}


bool HTS221::bduDeactivate(void)
{
    uint8_t data;

    if( !_hts221Inited )
        return false;
    data = readRegister(CTRL_REG1);
    data &= ~BDU_SET;
    writeRegister(CTRL_REG1, data);
    return true;
}



bool HTS221::getHumidity(void)
{
    bool NewData=false;
    uint8_t data   = 0;
    uint16_t h_out = 0;
    double h_temp  = 0.0;
    double hum     = 0.0;

    if( !_hts221Inited )
        return false;
    data = readRegister(STATUS_REG);

    if (data & HUMIDITY_READY) {
        data = readRegister(HUMIDITY_H_REG);
        h_out = data << 8;  // MSB
        data = readRegister(HUMIDITY_L_REG);
        h_out |= data;      // LSB

        // Decode Humidity
        hum = ((int16_t)(_h1_rH) - (int16_t)(_h0_rH))/2.0;  // remove x2 multiple

        // Calculate humidity in decimal of grade centigrades i.e. 15.0 = 150.
        h_temp = (double)(((int16_t)h_out - (int16_t)_H0_T0) * hum) / 
	         (double)((int16_t)_H1_T0 - (int16_t)_H0_T0);
        hum    = (double)((int16_t)_h0_rH) / 2.0; // remove x2 multiple
        _humidity = (hum + h_temp); // provide signed % measurement unit
        NewData=true;
    }
    return NewData;
}




bool HTS221::getTemperature(void)
{
    bool NewData=false;
    uint8_t data   = 0;
    uint16_t t_out = 0;
    double t_temp  = 0.0;
    double deg     = 0.0;

    if( !_hts221Inited )
        return false;
    data = readRegister(STATUS_REG);

    if (data & TEMPERATURE_READY) {

        data= readRegister(TEMP_H_REG);
        t_out = data  << 8; // MSB
        data = readRegister(TEMP_L_REG);
        t_out |= data;      // LSB

        // Decode Temperature
        deg    = (double)((int16_t)(_T1_degC) - (int16_t)(_T0_degC))/8.0; // remove x8 multiple

        // Calculate Temperature in decimal of grade centigrades i.e. 15.0 = 150.
        t_temp = (double)(((int16_t)t_out - (int16_t)_T0_OUT) * deg) / 
	         (double)((int16_t)_T1_OUT - (int16_t)_T0_OUT);
        deg    = (double)((int16_t)_T0_degC) / 8.0;     // remove x8 multiple
        _temperature = deg + t_temp;   // provide signed celsius measurement unit
        NewData=true;
    }

    return NewData;
}



// Read a single uint8_t from addressToRead and return it as a uint8_t
uint8_t HTS221::readRegister(uint8_t reg_addr)
{
    i2c_handle_t my_handle = 0;
    uint8_t value_read;

    i2c_bus_init(I2C_BUS_I, &my_handle);
    i2c_write(my_handle, HTS221_SAD, &reg_addr, 1, I2C_NO_STOP);
    i2c_read(my_handle, HTS221_SAD, &value_read, 1);
    i2c_bus_deinit(&my_handle);
    return value_read;
}


// Read a single uint8_t from addressToRead and return it as a uint8_t
bool HTS221::writeRegister(uint8_t reg_addr, uint8_t value)
{
    i2c_handle_t my_handle = 0;
    int i;
    uint8_t txBuff[2];

    txBuff[0] = reg_addr;
    txBuff[1] = value;
    i2c_bus_init(I2C_BUS_I, &my_handle);
    i=i2c_write(my_handle, HTS221_SAD, txBuff, 2, I2C_STOP);
    i2c_bus_deinit(&my_handle);
    return (i);
}


//
// because we are calling some of these functions from 'C', create a wrapper to access
// the C++ functionallity
//
extern "C" {

bool  hts221_getHumidity(void) { extern void *htsdev; HTS221 *p=(HTS221*)htsdev; return p->getHumidity(); }
bool  hts221_getTemperature(void) { extern void *htsdev; HTS221 *p=(HTS221*)htsdev; return p->getTemperature(); }
const double hts221_readHumidity(void) { extern void *htsdev; HTS221 *p=(HTS221*)htsdev; return p->readHumidity(); }
const double hts221_readTemperature(void) { extern void *htsdev; HTS221 *p=(HTS221*)htsdev; return p->readTemperature(); }
const uint8_t hts221_getDeviceID(void) { extern void *htsdev; HTS221 *p=(HTS221*)htsdev; return p->getDeviceID(); }

}

