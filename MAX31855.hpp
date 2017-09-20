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

    @file          MAX31855.hpp
    @version       1.0
    @date          Sept 2017

======================================================================== */
#ifndef MAX31855_H
#define MAX31855_H

extern "C" {
#include <hwlib/hwlib.h>
}

class MAX31855 {
 public:
  MAX31855();
  ~MAX31855();

  void init(void);
  int    loopbackTest();
  double readThermo(int Celcius);
  double readIntern(int Celcius);
  uint8_t readError();

 private:

  spi_handle_t myspi;
  uint32_t     readSPI(void);
  int16_t      thermo_temp;
  int16_t      intern_temp;
  uint16_t     errs;
  int          read31855(void);
};

#endif



