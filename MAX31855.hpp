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
  double readThermo(int Celcius);
  double readIntern(int Celcius);
  uint8_t readError();

 private:

  uint32_t readSPI(void);
  int16_t  thermo_temp;
  int16_t  intern_temp;
  uint8_t  errs;
  int      read31855(void);

};

#endif



