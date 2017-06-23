
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "iot_monitor.h"

#include "MAX31855.hpp"

MAX31855::MAX31855() { 
    spi_bus_t    bus  = SPI_BUS_II;
    spi_bpw_t    bits = SPI_BPW_32;
    spi_mode_t   mode = SPIMODE_CPOL_0_CPHA_1;
    uint32_t     freq = 960000;
    int i;

    myspi = 0;
    i=spi_bus_init(bus, &myspi);
    if( i<0 )
        printf("ERROR:spi_format()=%d\n",i);

    i=spi_format(myspi, mode, bits);
    if( i<0 )
        printf("ERROR:spi_format()=%d\n",i);

    i=spi_frequency(myspi, freq);
    if( i<0 )
        printf("ERROR:spi_frequency()=%d\n",i);
}

MAX31855::~MAX31855(void) {
    spi_bus_deinit(&myspi);
}

uint32_t MAX31855::readSPI(void) 
{
    int i;
    uint32_t     rxb, txb;

    errs = 0;
    if( spi_transfer(myspi, (uint8_t*)&txb, sizeof(uint32_t), (uint8_t*)&rxb, sizeof(uint32_t))<0)
        errs = 0x08;

    return rxb;
}

//
// returns 0 if no errors, otherwise !0
//
//     1011 0110 1111 1100 1011 0111 0001 0100
//                      || |||| |||| |||| ||||+OC Fault  (Open Circuit)
//                      || |||| |||| |||| |||+ SCG Fault (Short to GND)
//                      || |||| |||| |||| ||+  SCV Fault (Short to Vcc)
//                      || |||| |||| |||| +    RESERVED
//                      || ++++ ++++ ++++      12-bit Internal Temperature
//                      || +                   SIGN
//                      |+                     Fault when any of the SCV, SCG, or OC faults are active
//                      +                      RESERVED
//     ++++ ++++ ++++ ++                       14-bit Thermocouple Temp Data
//     +                                       SIGN
//
int MAX31855::read31855(void) {
    uint32_t  rxval = readSPI();
    
    thermo_temp = ((int32_t)rxval & 0xfffc0000)>>18;  //14-bit temp
    intern_temp = ((int32_t)rxval<<16)>>20;           //12-bit temp
    
    errs = rxval & 0x7;

    if( DBG_SPI & dbg_flag ) {
        printf("SPI: read       = 0x%08X\nSPI: thermo_temp= %d\nSPI: intern_temp= %d\nSPI: errs   =0x%02X\n", 
                rxval, thermo_temp,intern_temp,errs);
        }

    return -errs;
}


double MAX31855::readThermo(int InCelcius) {
    double v;

    if( read31855() <0 ) 
        return 0;
    v = thermo_temp * 0.25;
    if( !InCelcius ) 
        v = (v*9.0)/5.0 + 32;
    return v;
}


double MAX31855::readIntern(int InCelcius) {
    double v;

    if( read31855() <0 )
        return 0;
    v = intern_temp * 0.0625;
    if( !InCelcius ) 
        v = (v*9.0)/5.0 + 32;
    return v;
}
        

uint8_t MAX31855::readError() {
  return errs;
}


int MAX31855::loopbackTest()
{
    int i;
    uint32_t     rxb=0, txb=0x01234567;

    errs = 0;
    if( spi_transfer(myspi, (uint8_t*)&txb, sizeof(uint32_t), (uint8_t*)&rxb, sizeof(uint32_t))<0)
        errs = 0x08;

    if( DBG_SPI & dbg_flag ) 
        printf("SPI LBT: rxb = 0x%08X, txb=0x%08X, errs =0x%02X\n", rxb, txb, errs);
    return (rxb == txb);
}

