#ifndef __TSYS02D_HPP__
#define __TSYS02D_HPP__

#include <unistd.h>
#include <stdint.h>

#include <cstring>

extern "C" {
#include <hwlib/hwlib.h>
}

#define READT_HOLD 	0xE3
#define READT_NOHOLD	0xF3

class TSYS02D {

public:
    TSYS02D();
    ~TSYS02D();

  int     reset(void);
  int     getErr(void) { return _err; }
  uint8_t *getSN(void) { return _sn; }
  float   getTemperature(uint8_t trig);


private:
  int      _err;
  uint8_t  _sn[8];

    uint8_t  check_crc(uint16_t msg, uint8_t crc);
    uint16_t readTemp(uint8_t cmd);
    bool     writeRegister(uint8_t cmd, uint8_t data, uint8_t dsiz);

};

#endif

