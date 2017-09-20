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

    @file          lis2dw12.c
    @version       1.0
    @date          Sept 2017

======================================================================== */

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
#include <math.h>

#include <nettle/nettle-stdint.h>
#include <hwlib/hwlib.h>
#include "iot_monitor.h"
#include "lis2dw12.h"

typedef struct {
    char    *name;		//name of register
    uint8_t rw:1, reg_addr:7;   //address
    uint8_t value;              //current value
    uint8_t def;                //default value
    time_t  t;                  //time of last write
    } REGISTER;

float sensitivity=0;
float ACCX, ACCY, ACCZ; //linear acceleration sensors

gpio_handle_t int1_pin=0, int2_pin=0;

int lis2dw12_int1_irq(gpio_pin_t pin_name, gpio_irq_trig_t direction);
int lis2dw12_int2_irq(gpio_pin_t pin_name, gpio_irq_trig_t direction);

REGISTER lis2dw12_registers[] = {
//  Register              RW  Reg  Current Default  Time of 
//      Name                  Addr  Value   Value    access  Comments
//  --------------------- --  ----  -----   -----   -------  ----------------------
    "OUT_T_L",             0, 0x0d,   0,      0,     0,      //0- Temp sensor output
    "OUT_T_H",             0, 0x0e,   0,      0,     0,      //1- Temp sensor output
    "WHO_AM_I",            0, 0x0f,   0,     0x44,   0,      //2- Who am I ID
    "CTRL1",               1, 0x20,   0,     0x62,   0,      //3- Control Register
    "CTRL2",               1, 0x21,   0,     0x0c,   0,      //4- Control Register
    "CTRL3",               1, 0x22,   0,     0x02,   0,      //5- Control Register
    "CTRL4_INT1_PAD_CTRL", 1, 0x23,   0,     0x00,   0,      //6- Control Register
    "CTRL5_INT2_PAD_CTRL", 1, 0x24,   0,     0x00,   0,      //7- Control Register
    "CTRL6",               1, 0x25,   0,      0,     0,      //8- Control Register
    "OUT_T",               0, 0x26,   0,      0,     0,      //9- Temp sensor output
    "STATUS",              0, 0x27,   0,      0,     0,      //10- Status data register
    "OUT_X_L",             0, 0x28,   0,      0,     0,      //11- Output register
    "OUT_X_H",             0, 0x29,   0,      0,     0,      //12- Output register
    "OUT_Y_L",             0, 0x2a,   0,      0,     0,      //13- Output register
    "OUT_Y_H",             0, 0x2b,   0,      0,     0,      //14- Output register
    "OUT_Z_L",             0, 0x2c,   0,      0,     0,      //15- Output register
    "OUT_Z_H",             0, 0x2d,   0,      0,     0,      //16- Output register
    "FIFO_CTRL",           1, 0x2e,   0,     0x00,   0,      //17- FIFO controle register
    "FIFO_SAMPLES",        0, 0x2f,   0,      0,     0,      //18- Unread samples stored in FIFO
    "TAP_THS_X",           1, 0x30,   0,      0,     0,      //19- Tap thresholds
    "TAP_THS_Y",           1, 0x31,   0,      0,     0,      //20- Tap thresholds
    "TAP_THS_Z",           1, 0x32,   0,      0,     0,      //21- Tap thresholds
    "INT_DUR",             1, 0x33,   0,      0,     0,      //22- Interrupt duration
    "WAKE_UP_THS",         1, 0x34,   0,      0,     0,      //23- Tap/double-tap, activity, wakeup
    "WAKE_UP_DUR",         1, 0x35,   0,      0,     0,      //24- Wakeup duration
    "FREE_FALL",           1, 0x36,   0,      0,     0,      //25- Free-fall configuration
    "STATUS_DUP",          0, 0x37,   0,      0,     0,      //26- Status register
    "WAKE_UP_SRC",         0, 0x38,   0,      0,     0,      //27- Wakeup source
    "TAP_SRC",             0, 0x39,   0,      0,     0,      //28- Tap source
    "SIXD_SRC",            0, 0x3a,   0,      0,     0,      //29- 6D source
    "ALL_INT_SRC",         0, 0x3b,   0,      0,     0,      //30-
    "ABS_INT_X",           1, 0x3c,   0,      0,     0,      //31-
    "ABS_INT_Y",           1, 0x3d,   0,      0,     0,      //32-
    "ABS_INT_Z",           1, 0x3e,   0,      0,     0,      //33-
    "ABS_INT_CFG ",        1, 0x3f,   0,      0,     0,      //34-
}; 
#define REG_SIZE	(sizeof(lis2dw12_registers)/sizeof(REGISTER))

#define LIS2DW12_OUT_T_L             0
#define LIS2DW12_OUT_T_H             1
#define LIS2DW12_WHO_AM_I            2
#define LIS2DW12_CTRL1               3
#define LIS2DW12_CTRL2               4
#define LIS2DW12_CTRL3               5
#define LIS2DW12_CTRL4_INT1_PAD_CTRL 6
#define LIS2DW12_CTRL5_INT2_PAD_CTRL 7
#define LIS2DW12_CTRL6               8
#define LIS2DW12_OUT_T               9
#define LIS2DW12_STATUS              10
#define LIS2DW12_OUT_X_L             11
#define LIS2DW12_OUT_X_H             12
#define LIS2DW12_OUT_Y_L             13
#define LIS2DW12_OUT_Y_H             14
#define LIS2DW12_OUT_Z_L             15
#define LIS2DW12_OUT_Z_H             16
#define LIS2DW12_FIFO_CTRL           17
#define LIS2DW12_FIFO_SAMPLES        18
#define LIS2DW12_TAP_THS_X           19
#define LIS2DW12_TAP_THS_Y           20
#define LIS2DW12_TAP_THS_Z           21
#define LIS2DW12_INT_DUR             22
#define LIS2DW12_WAKE_UP_THS         23
#define LIS2DW12_WAKE_UP_DUR         24
#define LIS2DW12_FREE_FALL           25
#define LIS2DW12_STATUS_DUP          26
#define LIS2DW12_WAKE_UP_SRC         27
#define LIS2DW12_TAP_SRC             28
#define LIS2DW12_SIXD_SRC            29
#define LIS2DW12_ALL_INT_SRC         30
#define LIS2DW12_ABS_INT_X           31
#define LIS2DW12_ABS_INT_Y           32
#define LIS2DW12_ABS_INT_Z           33
#define LIS2DW12_ABS_INT_CFG         34

/*
 * Sensitivity sets in all modes but low power mode 1
 */
#define LIS2DW12_FS_2G_GAIN_LP		0.976
#define LIS2DW12_FS_4G_GAIN_LP		1.952
#define LIS2DW12_FS_8G_GAIN_LP		3.904
#define LIS2DW12_FS_16G_GAIN_LP		7.808

#define LIS2DW12_ON			0x60
#define LIS2DW12_SCONV			LIS2DW12_ON|0x0a
#define LIS2DW12_CCONV			LIS2DW12_ON|0x02


#define LIS2DW12_WHO_AM_I_ADDR		lis2dw12_registers[LIS2DW12_WHO_AM_I].reg_addr
#define LIS2DW12_WHO_AM_I_DEF		lis2dw12_registers[LIS2DW12_WHO_AM_I].def

#define READ_REGISTER(x)	        (lis2dw12_registers[x].value=lis2dw12_read_byte(lis2dw12_registers[x].reg_addr))

#define WRITE_REGISTER(x,y)      	lis2dw12_write_byte(lis2dw12_registers[x].reg_addr, y);\
                                 	lis2dw12_registers[x].t =  time(0) 

#define lis2dw12_on_or_off	 	(READ_REGISTER(LIS2DW12_CTRL1) & 0xf0)

#define HIGH_POWER_EN	                (READ_REGISTER(LIS2DW12_CTRL1) & 0x04)

#define TRIGGER_XYZ   lis2dw12_write_byte(lis2dw12_registers[LIS2DW12_CTRL3].reg_addr,0x03); \
                      while( READ_REGISTER(LIS2DW12_CTRL3) & 0x01) /*wait*/;

#define TRIGGER_TEMP  {uint8_t v=0; while( !(v & 0x40) ) v=READ_REGISTER(LIS2DW12_STATUS_DUP);}

static uint8_t lis2dw12_read_byte(uint8_t reg_addr) 
{
    i2c_handle_t my_handle=(i2c_handle_t)NULL;
    unsigned char value_read = 0;

    i2c_bus_init(I2C_BUS_I, &my_handle);
    i2c_write(my_handle, LIS2DW12_SAD, &reg_addr, 1, I2C_NO_STOP);
    i2c_read(my_handle, LIS2DW12_SAD, &value_read, 1);
    i2c_bus_deinit(&my_handle);

    return value_read;
}

static void lis2dw12_write_byte(uint8_t reg_addr, uint8_t value) {
    i2c_handle_t my_handle = 0;
    uint8_t buffer_sent[2];

    buffer_sent[0] = reg_addr;
    buffer_sent[1] = value;
    i2c_bus_init(I2C_BUS_I, &my_handle);
    i2c_write(my_handle, LIS2DW12_SAD, buffer_sent, 2, I2C_STOP);
    i2c_bus_deinit(&my_handle);

}

static inline int uint82int(uint8_t ms, uint8_t ls, unsigned hp)
{
    // incomming data is 2 bytes, in 2's complement form, need to 
    // convert it to a signed int, that is what al the shifting is about
    int v = ((ls | ms << 8)<<((sizeof(int)-2)*8)) >> ((sizeof(int)-2)*8);

    if (!hp)
        return (long)v >> 4;
    else
        return (long)v >> 2;
}

void lis2dw12_regdump(void) {
    for( int i=0; i<REG_SIZE; i++ ) 
        printf("LIS2DW12 Register %s/0x%02X = 0x%02X\n", 
                lis2dw12_registers[i].name, 
                lis2dw12_registers[i].reg_addr, 
                READ_REGISTER(i));
}

int lis2dw12_initialize(void) {
    int i, v;
    const time_t t = time(0);

//jmf    gpio_init(GPIO_PIN_6, &int1_pin);
//jmf    gpio_init(GPIO_PIN_7, &int2_pin);
//jmf    gpio_dir(int1_pin, GPIO_DIR_INPUT);
//jmf    gpio_dir(int2_pin, GPIO_DIR_INPUT);
//jmf
//jmf    if( (i=gpio_irq_request(int1_pin, GPIO_IRQ_TRIG_FALLING, lis2dw12_int1_irq)) != 0)
//jmf        printf("ERROR: can't set int1 as interrupt input. (%d)\n",i);
//jmf
//jmf    if( (i=gpio_irq_request(int2_pin, GPIO_IRQ_TRIG_FALLING, lis2dw12_int2_irq)) != 0)
//jmf        printf("ERROR: can't set int2 as interrupt input. (%d)\n",i);

    for( i=0; i<REG_SIZE; i++ ) {
        if( lis2dw12_registers[i].rw ) {
            WRITE_REGISTER(i, lis2dw12_registers[i].def);
            if( dbg_flag & DBG_LIS2DW12 )
                printf("-LIS2DW12: %s/WRITE_REGISTER(0x%02X, 0x%02X) :: 0x%02X = READ_REGISTER(0x%02X)\n",
                         lis2dw12_registers[i].name,
                         lis2dw12_registers[i].reg_addr,
                         lis2dw12_registers[i].def,
                         READ_REGISTER(i),
                         lis2dw12_registers[i].reg_addr);
            }
        }

    sensitivity = LIS2DW12_FS_2G_GAIN_LP;
    sensitivity = LIS2DW12_FS_4G_GAIN_LP;
    sensitivity = LIS2DW12_FS_8G_GAIN_LP;
    sensitivity = LIS2DW12_FS_16G_GAIN_LP;

    return (lis2dw12_getDeviceID() != LIS2DW12_WHO_AM_I_DEF)?-2:0;
}

uint8_t lis2dw12_getDeviceID(void) {
    return lis2dw12_read_byte(LIS2DW12_WHO_AM_I_ADDR);
}

float lis2dw12_readTemp12( void ) {
    int i;
    uint8_t low=0, high=0, v=0;
    float tempC, tempF;

    WRITE_REGISTER(LIS2DW12_CTRL1,LIS2DW12_CCONV);
    TRIGGER_TEMP;

    low = READ_REGISTER(LIS2DW12_OUT_T_L);
    high= READ_REGISTER(LIS2DW12_OUT_T_H);
    i = uint82int(high, low, HIGH_POWER_EN);
    tempC = (float)(i/16.0) + 25.0;
    tempF = (tempC*9.0)/5.0 + 32;
    if( dbg_flag & DBG_LIS2DW12 ){
        printf("-LIS2DW12: i+25 =%d - 0x%04X\n",i+25,i+25);
        printf("-LIS2DW12: tempC=%4.2f\n",tempC);
        printf("-LIS2DW12: tempF=%4.2f\n",tempF);
        }

    return tempF;
}

int lis2dw12_readTemp8( void ) {
    uint8_t low=0, high=0;
    int tempC=0, tempF=0, v=0;

    WRITE_REGISTER(LIS2DW12_CTRL1,LIS2DW12_CCONV);
    TRIGGER_TEMP;

    v = (int)READ_REGISTER(LIS2DW12_OUT_T);
    v = v<<((sizeof(int)-1)*8);
    v = v>>((sizeof(int)-1)*8);
    tempC = v+25;
    tempF = (tempC*9.0)/5.0 + 32;
    if( dbg_flag & DBG_LIS2DW12 ) {
        printf("-LIS2DW12: reg   =%d - 0x%02X\n",v,v);
        printf("-LIS2DW12: tempC =%d\n",tempC);
        printf("-LIS2DW12: tempF =%d\n",tempF);
        }

    return tempF;
}


static int lis2dw12_get_acc_data( void )
{
    uint8_t status=0, high, low;
    int X, Y, Z;

    WRITE_REGISTER(LIS2DW12_CTRL1,LIS2DW12_SCONV);
    TRIGGER_XYZ;

    low =READ_REGISTER(LIS2DW12_OUT_X_L);
    high=READ_REGISTER(LIS2DW12_OUT_X_H);
    X=uint82int(high,low,HIGH_POWER_EN);
    ACCX = X * sensitivity;
    if( dbg_flag & DBG_LIS2DW12 )
        printf("OUT_X_L=0x%02X, OUT_X_H=0x%02X, X=0x%04X (%d), ACCX=%5.2f\n",low,high,X,X,ACCX);

    low =READ_REGISTER(LIS2DW12_OUT_Y_L);
    high=READ_REGISTER(LIS2DW12_OUT_Y_H);
    Y=uint82int(high,low,HIGH_POWER_EN);
    ACCY = Y * sensitivity;
    if( dbg_flag & DBG_LIS2DW12 )
        printf("OUT_Y_L=0x%02X, OUT_Y_H=0x%02X, Y=0x%04X (%d), ACCY=%5.2f\n",low,high,Y,Y,ACCY);

    low =READ_REGISTER(LIS2DW12_OUT_Z_L);
    high=READ_REGISTER(LIS2DW12_OUT_Z_H);
    Z=uint82int(high,low,HIGH_POWER_EN);
    ACCZ = Z * sensitivity;
    if( dbg_flag & DBG_LIS2DW12 )
        printf("HP=%d, OUT_Z_L=0x%02X, OUT_Z_H=0x%02X, Z=0x%04X (%d), ACCZ=%5.2f\n",HIGH_POWER_EN,low,high,Z,Z,ACCZ);

    return 0;
}

int smothed_xyz( float *x, float *y, float *z )
{
    static int cnt=0;
    static float avgX=0, avgY=0, avgZ=0;
    int pos_changed;

    if (cnt<2)
        cnt++;
    
    avgX += *x;
    avgX /= cnt;
    avgY += *y;
    avgY /= cnt;
    avgZ += *z;
    avgZ /= cnt;

//    printf("avgX=%f,x=%f,xpos=%f, avgY=%f,y=%f,ypos=%f, avgZ=%f,z=%f,zpos=%f\n\n", 
//            avgX,*x,abs(avgX)-abs(*x), avgY,*y,abs(avgY)-abs(*y), avgZ,*z,abs(avgZ)-abs(*z));
    pos_changed = (((abs(avgX)-abs(*x)) > 50) || ((abs(avgY)-abs(*y)) > 50) || ((abs(avgZ)-abs(*z)) > 50)); 
    if( pos_changed ) {
        avgX = 0;
        avgY = 0;
        avgZ = 0;
        cnt = 0;
        }
    else {
        *x = avgX;
        *y = avgY;
        *z = avgZ;
        }

    return pos_changed;
}

void lis2dw12_timer_task(size_t timer_id, void * user_data) 
{
    static int c=0;
    uint8_t status = READ_REGISTER(LIS2DW12_STATUS);

//    printf("        Status Register: 0x%04X\n",status);
    if (status & 0x10)                                  //double tap occured
        printf("                   : Double Tap occured\n");

    if (status & 0x08)                                  //single tap occures
        printf("                   : Single Tap occured\n");

    if (status & 0x04)                                  //6d orientation change occured
        printf("                   : 6D orientation change occured\n");

    if (status & 0x02)                                  //free fall event occured
        printf("                   : Free Fall occured\n");

    lis2dw12_get_acc_data();

    if (smothed_xyz(&ACCX, &ACCY, &ACCZ)) {
        float rad = sqrt(ACCX*ACCX+ACCY*ACCY+ACCZ*ACCZ);
        float inc = acos(ACCZ/rad)*(180/3.1415);
        float az  = atan(ACCY/ACCX)*(180/3.1415);
        printf("\n\nChanged positon!\n");
        printf("%3d) XYZ data Avaiable: X=%+5.2f, Y=%+5.2f, Z=%+5.2f\n",
//               "    Polar Coordinates: radius=%5.2f inclination=%5.2f azimuth=%5.2f\n\n", 
           c++, ACCX, ACCY, ACCZ 
//         rad, inc, az
           );
        }
}

void lis2dw12_ot_acc_data(void)
{
    lis2dw12_get_acc_data();
    smothed_xyz(&ACCX, &ACCY, &ACCZ);
    float rad = sqrt(ACCX*ACCX+ACCY*ACCY+ACCZ*ACCZ);
    float inc = acos(ACCZ/rad)*(180/3.1415);
    float az  = atan(ACCY/ACCX)*(180/3.1415);
    printf("XYZ data Avaiable: X=%+5.2f, Y=%+5.2f, Z=%+5.2f\n"
           "Polar Coordinates: radius=%5.2f inclination=%5.2f azimuth=%5.2f\n\n", 
           ACCX, ACCY, ACCZ, rad, inc, az);
}

char **lis2dw12_m2x(void)
{
    static char xyz_data[3][16];
    static char *data[] = {
        xyz_data[0],
        xyz_data[1],
        xyz_data[2]
        };

    lis2dw12_get_acc_data();
    smothed_xyz(&ACCX, &ACCY, &ACCZ);

    memset(xyz_data, 0, sizeof(xyz_data));

    sprintf(xyz_data[0], "%f", ACCX);
    sprintf(xyz_data[1], "%f", ACCY);
    sprintf(xyz_data[2], "%f", ACCZ);

    return data;
}

//
// Use interrupts for:
//  1. Temp sensor data
//  2. XYZ data available
//
//  INT1 = FIFO READY              // xyz Data
//  INT2 = TEMP READY              // Temp Data
//

int lis2dw12_int1_irq(gpio_pin_t pin_name, gpio_irq_trig_t direction)
{
    i2c_handle_t my_handle=(i2c_handle_t)NULL;
    uint8_t addr = 26;
    unsigned char i;
    printf("INT1 occured!!  \n");

    if (pin_name != GPIO_PIN_6) {
        return 0;
        }

    return 0;
}

int lis2dw12_int2_irq(gpio_pin_t pin_name, gpio_irq_trig_t direction)
{
    printf("INT2 occured!!  \n");
    if (pin_name != GPIO_PIN_6) {
        return 0;
        }

    return 0;
}

void release_irqs(void)
{
    gpio_deinit( &int1_pin);
    gpio_deinit( &int2_pin);
}

