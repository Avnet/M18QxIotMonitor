
#include <stdlib.h>
#include "MAX31855.hpp"

MAX31855::MAX31855() : my_spi(0) { };

void MAX31855::init(void) {
    spi_bus_init(SPI_BUS_I, &my_spi);

// CPOL - Sets the data clock to be idle when high if set to 1, idle when low if set to 0
// CPHA - Samples data on the falling edge of the data clock when 1, rising edge when 0'
// 32 bits per word
    spi_format(my_spi, SPIMODE_CPOL_0_CPHA_0, SPI_BPW_32);
    spi_frequency(my_spi, 4000000);

    if( spi_read() ) {
        spi_bus_deinit(&my_spi);
        }
}

MAX31855::~MAX31855(void) {
    spi_bus_deinit(&my_spi);
}

//
// returns 0 if no errors, otherwise !0
//
int MAX31855::spi_read(void) {
    uint32_t txb, rxb;
    uint32_t txlen, rxlen;

    if( spi_transfer(my_spi, (uint8_t*)&txb, sizeof(uint32_t), (uint8_t*)&rxb, sizeof(uint32_t)) ) {
        errs |= 0x80;
        return -1;
        }

    thermo_temp = (rxb & 0xfffc)>>18;
    intern_temp = (rxb & 0x0000fff0)>>4;
    errs        = ((rxb>>13) & 0x18) | (rxb & 0x7);
    return 0;
}


double MAX31855::readThermo(int Celcius) {
    double v;

    if( spi_read() || (errs & 0x80))
        return 0;
    v = thermo_temp * 0.25;
    if( !Celcius ) 
        v = (v*9.0)/5.0 + 32;
    return v;
}


double MAX31855::readIntern(int Celcius) {
    double v;

    if( spi_read() || (errs & 0x80))
        return 0;
    v = thermo_temp * 0.0625;
    if( !Celcius ) 
        v = (v*9.0)/5.0 + 32;
    return v;
}
        

uint8_t MAX31855::readError() {
  return errs;
}
