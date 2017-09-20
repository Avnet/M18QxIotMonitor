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

    @file          MS5637.cpp
    @version       1.0
    @date          July 2017

======================================================================== */
// Driver for MS5637_02BA03 PMOD on the WNC M18Qx SOM 

#ifndef MS5637_HPP_
#define MS5637_HPP_

// Operating Mode Choices.  Unless over written (by calling setMode)
// OSR256 mode is used for both D1 & D2
//
//                                      Max              Typical 
//                               Conversion Time    Current    Resolution 
//Conversion Times
#define CONVT_OSR256     1     //   0.54  ms
#define CONVT_OSR512     2     //   1.06  ms
#define CONVT_OSR1024    3     //   2.08  ms
#define CONVT_OSR2048    5     //   4.13  ms
#define CONVT_OSR4096    9     //   8.22  ms
#define CONVT_OSR8192    17    //   16.44 ms

//Conversion Modes
#define D1MODE_OSR256     0x40 //                   0.63 uA     1.83 mbar
#define D1MODE_OSR512     0x42 //                   1.26 uA     1.14 mbar
#define D1MODE_OSR1024    0x44 //                   2.51 uA     0.78 mbar
#define D1MODE_OSR2048    0x46 //                   5.02 uA     0.54 mbar
#define D1MODE_OSR4096    0x48 //                  10.05 uA     0.38 mbar
#define D1MODE_OSR8192    0x4a //                  20.09 uA     0.27 mbar

#define D2MODE_OSR256     0x50 //                              0.0086 c
#define D2MODE_OSR512     0x52 //                              0.0055 c
#define D2MODE_OSR1024    0x54 //                              0.0041 c
#define D2MODE_OSR2048    0x56 //                              0.0033 c
#define D2MODE_OSR4096    0x58 //                              0.0026 c
#define D2MODE_OSR8192    0x5a //                              0.0022 c

//Macro to convert Millibars to inches of barametric pressure
#ifndef MTOI
#define MTOI(x)  ((x)*0.0295301)    //millibars to inches
#endif

class MS5637
{

public:
    MS5637(void);
    ~MS5637(void);

    float *getPT(void);
    int   setMode(uint8_t d1, uint8_t d2);
    int   getErr(void) { return err; }

private:
    uint8_t  d1_mode, d2_mode;
    uint32_t _Coff[6];
    float    results[2]; // results[0] = pressure & results[1] = temperature;
    int      err;

    uint32_t readRegister(uint8_t cmd, uint8_t siz);
    bool     writeRegister(uint8_t cmd);
};

#endif /* MS5637_H_ */
