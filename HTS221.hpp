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

    @file          HTS221.hpp
    @version       1.0
    @date          Sept 2017

======================================================================== */

/*
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
    bool getCalibration(void);
    unsigned char _h0_rH, _h1_rH;
    unsigned int  _T0_degC, _T1_degC;
    unsigned int  _H0_T0, _H1_T0;
    unsigned int  _T0_OUT, _T1_OUT;
    double  _temperature;
    double  _humidity;
    uint8_t _address;
    uint8_t _iAm;
    uint8_t _hts221Inited;

    uint8_t readRegister(uint8_t regToRead);
    bool writeRegister(uint8_t regToWrite, uint8_t dataToWrite);
};

#else

//these are the functions you can access from 'C'
int  hts221_getHumidity(void);
int  hts221_getTemperature(void);
const double hts221_readHumidity(void);
const double hts221_readTemperature(void);
const uint8_t hts221_getDeviceID(void);

#endif

#endif /* HTS221_H_ */
