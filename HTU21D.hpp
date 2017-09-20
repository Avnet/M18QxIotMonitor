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

#ifndef __HTU21D_HPP__ 
#define __HTU21D_HPP__ 

#include <stdint.h>

#define TRIGGER_TEMP_MEASURE_HOLD    0xE3
#define TRIGGER_HUMD_MEASURE_HOLD    0xE5
#define TRIGGER_TEMP_MEASURE_NOHOLD  0xF3
#define TRIGGER_HUMD_MEASURE_NOHOLD  0xF5

#define RH12_T14_RESOLUTION          0b00000000
#define RH08_T12_RESOLUTION          0b00000001
#define RH10_T13_RESOLUTION          0b10000000
#define RH11_T11_RESOLUTION          0b10000001

#define EOB_LT225                    0b01000000
#define HEATER_ON                    0b00000100

#define A         8.1332
#define B         1762.39
#define C         235.66
#define PP(t)	  pow(10, (A-B/(t+C)))
#define DEW(t,h)  -(B/(log10((h*PP(t)/100))-A)+C)

class HTU21D {

public:
  HTU21D();
  ~HTU21D();

  float   readHumidity(uint8_t trig);
  float   readTemperature(uint8_t trig);

  uint8_t getOpMode(void);
  void    setOpMode(uint8_t resBits);
  
  int     getErr(void) { return _err; }
  int     reset(void);

private:

  int      _err;
  uint8_t  _OpM;
  uint16_t readMeasurment(uint8_t cmd, uint16_t dly);
  bool     writeRegister(uint8_t cmd, uint8_t data, uint8_t dsiz);
  uint8_t  check_crc(uint16_t msg, uint8_t crc);

};

#endif

