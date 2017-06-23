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



