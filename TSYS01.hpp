#ifndef __TSYS01_HPP__
#define __TSYS01_HPP__

#include <unistd.h>
#include <stdint.h>
#include <math.h>

#include <cstring>

extern "C" {
#include <hwlib/hwlib.h>
}


class TSYS01 {

public:
    TSYS01();
    ~TSYS01();

    int     reset(void);
    int     getErr(void) { return _err; }
    float   getTemperature(void);


private:
    int      _err;
    uint16_t k[8];

    uint16_t readTemp(uint8_t cmd);
    int32_t readADC(void);
    bool     writeRegister(uint8_t cmd, uint8_t data, uint8_t dsiz);
};

#endif

