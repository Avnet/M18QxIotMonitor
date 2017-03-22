#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <nettle/nettle-stdint.h>
#include <hwlib/hwlib.h>
#include "iot_monitor.h"
#include "lis2dw12.h"

//local caches

uint8_t lis2dw12_read_byte(uint8_t reg_addr) {
    i2c_handle_t my_handle=(i2c_handle_t)NULL;
    unsigned char value_read = 0;

    i2c_bus_init(I2C_BUS_I, &my_handle);
    i2c_write(my_handle, LIS2DW12_SAD, &reg_addr, 1, I2C_NO_STOP);
    i2c_read(my_handle, LIS2DW12_SAD, &value_read, 1);
    i2c_bus_deinit(&my_handle);

    return value_read;
}

int lis2dw12_initialize(void) {
    return (lis2dw12_getDeviceID() != LIS2DW12_WHO_AM_I_DEF)?-2:0;
}

uint8_t lis2dw12_getDeviceID(void) {
    return lis2dw12_read_byte(LIS2DW12_WHO_AM_I_ADDR);
}


