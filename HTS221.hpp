/*
 * HTS221.h
 *
 * Created: 02/01/2015 20:50:30
 *  Author: smkk
 */


#ifndef HTS221_H_
#define HTS221_H_

#include "HTS221Reg.h"

#ifdef __cplusplus

class HTS221
{
public:
    HTS221(void);
    ~HTS221(void);

    void activate(void);
    void deactivate(void);

    bool bduActivate(void);
    bool bduDeactivate(void);

    bool  getHumidity(void);
    bool  getTemperature(void);
    inline const double readHumidity(void) { return _humidity; }
    inline const double readTemperature(void) { return _temperature; }

    inline const uint8_t   getDeviceID(void){ return _iAm; }

private:
    bool storeCalibration(void);
    unsigned char _h0_rH, _h1_rH;
    unsigned int  _T0_degC, _T1_degC;
    unsigned int  _H0_T0, _H1_T0;
    unsigned int  _T0_OUT, _T1_OUT;
    double _temperature;
    double _humidity;
    uint8_t _address;
    uint8_t _iAm;
    HTS221 *_hts221Ptr;

    uint8_t readRegister(uint8_t regToRead);
    bool writeRegister(uint8_t regToWrite, uint8_t dataToWrite);
};

#else

//these are the functions you can access form 'C'
int  hts221_getHumidity(void);
int  hts221_getTemperature(void);
const double hts221_readHumidity(void);
const double hts221_readTemperature(void);
const uint8_t hts221_getDeviceID(void);

#endif

#endif /* HTS221_H_ */
