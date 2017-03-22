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

  int spi_read(void);
  spi_handle_t my_spi;
  int16_t thermo_temp;
  int16_t intern_temp;
  uint8_t errs;

};

#endif



