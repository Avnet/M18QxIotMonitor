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

enum {
	LIS2DW12_LP_MODE = 0, //low-power mode
	LIS2DW12_HR_MODE,     //high-power mode
	LIS2DW12_MODE_COUNT,  //how many modes possible
};


typedef struct {
    char    *name;		//name of register
    uint8_t rw:1, reg_addr:7;   //address
    uint8_t value;              //current value
    uint8_t def;                //default value
    time_t  t;                  //time of last access
    } REGISTER;

#define SET_DEF_VAL(x,y)	(lis2dw12_registers[x].def=y)
#define CONVRT_12BIT(h,l,hp)	((((int)h<<8)|l)>>(hp?2:4))

int sensitivity=0;
int ACCX, ACCY, ACCZ; //linear acceleration sensors
int high_power_active=0;

REGISTER lis2dw12_registers[] = {
//  Register              RW  Reg  Current Default  Time of 
//      Name                  Addr  Value   Value    access  Comments
//  --------------------- --  ----  -----   -----   -------  ----------------------
    "OUT_T_L",             0, 0x0d,   0,      0,     0,      //0- Temp sensor output
    "OUT_T_H",             0, 0x0e,   0,      0,     0,      //1- Temp sensor output
    "WHO_AM_I",            0, 0x0f,   0,     0x44,   0,      //2- Who am I ID
    "CTRL1",               1, 0x20,   0,     0x63,   0,      //3- Control Registers
    "CTRL2",               1, 0x21,   0,     0x44,   0,      //4- sft rst & auto-increment address
    "CTRL3",               1, 0x22,   0,      0,     0,      //5- Control Registers
    "CTRL4_INT1_PAD_CTRL", 1, 0x23,   0,      0,     0,      //6- Control Registers
    "CTRL5_INT2_PAD_CTRL", 1, 0x24,   0,      0,     0,      //7- Control Registers
    "CTRL6",               1, 0x25,   0,      0,     0,      //8- Control Registers
    "OUT_T",               0, 0x26,   0,      0,     0,      //9- Temp sensor output
    "STATUS",              0, 0x27,   0,      0,     0,      //10- Status data register
    "OUT_X_L",             0, 0x28,   0,      0,     0,      //11- Output registers
    "OUT_X_H",             0, 0x29,   0,      0,     0,      //12- Output registers
    "OUT_Y_L",             0, 0x2a,   0,      0,     0,      //13- Output registers
    "OUT_Y_H",             0, 0x2b,   0,      0,     0,      //14- Output registers
    "OUT_Z_L",             0, 0x2c,   0,      0,     0,      //15- Output registers
    "OUT_Z_H",             0, 0x2d,   0,      0,     0,      //16- Output registers
    "FIFO_CTRL",           1, 0x2e,   0,      0,     0,      //17- FIFO controle register
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
 * Sensitivity sets in LP mode [ug]
 */
#define LIS2DW12_FS_2G_GAIN_LP		976
#define LIS2DW12_FS_4G_GAIN_LP		1952
#define LIS2DW12_FS_8G_GAIN_LP		3904
#define LIS2DW12_FS_16G_GAIN_LP		7808

/*
 * Sensitivity sets in HR mode [ug]
 */
#define LIS2DW12_FS_2G_GAIN_HR		244
#define LIS2DW12_FS_4G_GAIN_HR		488
#define LIS2DW12_FS_8G_GAIN_HR		976
#define LIS2DW12_FS_16G_GAIN_HR		1952

#define LIS2DW12_WHO_AM_I_ADDR		lis2dw12_registers[LIS2DW12_WHO_AM_I].reg_addr
#define LIS2DW12_WHO_AM_I_DEF		lis2dw12_registers[LIS2DW12_WHO_AM_I].def

#define READ_REGISTER(x)	lis2dw12_read_byte(lis2dw12_registers[x].reg_addr); \
                                lis2dw12_registers[x].t =  time(0) 

uint8_t lis2dw12_read_byte(uint8_t reg_addr) {
    i2c_handle_t my_handle=(i2c_handle_t)NULL;
    unsigned char value_read = 0;

    i2c_bus_init(I2C_BUS_I, &my_handle);
    i2c_write(my_handle, LIS2DW12_SAD, &reg_addr, 1, I2C_NO_STOP);
    i2c_read(my_handle, LIS2DW12_SAD, &value_read, 1);
    i2c_bus_deinit(&my_handle);

    return value_read;
}

#define WRITE_REGISTER(x,y)     lis2dw12_write_byte(lis2dw12_registers[x].reg_addr, y);\
                                lis2dw12_registers[x].t =  time(0) 

void lis2dw12_write_byte(uint8_t reg_addr, uint8_t value) {
    i2c_handle_t my_handle = 0;
    uint8_t buffer_sent[2];

    buffer_sent[0] = reg_addr;
    buffer_sent[1] = value;
    i2c_bus_init(I2C_BUS_I, &my_handle);
    i2c_write(my_handle, LIS2DW12_SAD, buffer_sent, 2, I2C_STOP);
    i2c_bus_deinit(&my_handle);

}

int lis2dw12_initialize(void) {
    int i, v;
    const time_t t = time(0);

    for( i=REG_SIZE-1; i; i-- ) {
        if( lis2dw12_registers[i].rw ) {
            if( dbg_flag & DBG_LIS2DW12 )
                printf("-LIS2DW12: initialize %s (0x%02X) with 0x%02X\n",
                         lis2dw12_registers[i].name,
                         lis2dw12_registers[i].reg_addr,
                         lis2dw12_registers[i].def);
            WRITE_REGISTER(i, lis2dw12_registers[i].def);
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

void lis2dw12_regdump(void) {
    int i,v;

    for( i=0; i<REG_SIZE; i++ ) {
        v=READ_REGISTER(i);
        printf("LIS2DW12 Register %s = 0x%02X\n", lis2dw12_registers[i].name, v);
        }
}

int lis2dw12_readTemp( uint8_t flag12bit ) {
    uint8_t low=0, high=0;
    int tempC=0, tempF=0, v=0;

    while( !(v & 0x40) ) 
        v=READ_REGISTER(LIS2DW12_STATUS_DUP);
      /* wait for temp data to be obtained */;
    if( flag12bit ) {
        low = READ_REGISTER(LIS2DW12_OUT_T_L);
        high= READ_REGISTER(LIS2DW12_OUT_T_H);
        tempC = CONVRT_12BIT(high, low, high_power_active) / 16;
        }
    else {
        tempC = READ_REGISTER(LIS2DW12_OUT_T);
        }

    tempC += 25;
    tempF = (tempC*9.0)/5.0 + 32;

    return tempF;
}

void lis2dw12_onoff( uint8_t on ) {
    uint8_t reg    = READ_REGISTER(LIS2DW12_CTRL1);
printf("in lis2dw12_onoff\n");
    if( on ){
        high_power_active=(on & 0x40);
        WRITE_REGISTER(LIS2DW12_CTRL1, (reg & 0x0f)|on);
        }
    else {
        high_power_active=0;
        WRITE_REGISTER(LIS2DW12_CTRL1, (reg & 0x0f));
        }
}


static int lis2dw12_get_acc_data( void )
{
    uint8_t high, low;
    int X, Y, Z;

    //
    //read the X, Y, and Z registers and scale results
    //

    low =READ_REGISTER(LIS2DW12_OUT_X_L);
    high=READ_REGISTER(LIS2DW12_OUT_X_H);
    X=CONVRT_12BIT(high,low,high_power_active);
    ACCX = X * sensitivity;
printf("X:%d(0x%04X), high=0x%02x, low=0x%02x, %d\n",ACCX, X, high, low, sensitivity);
    low =READ_REGISTER(LIS2DW12_OUT_Y_L);
    high=READ_REGISTER(LIS2DW12_OUT_Y_H);
    Y=CONVRT_12BIT(high,low,high_power_active);
    ACCY = Y * sensitivity;
printf("Y:%d(0x%04x), high=0x%02x, low=0x%02x, %d\n",ACCY, Y, high, low, sensitivity);

    low =READ_REGISTER(LIS2DW12_OUT_Z_L);
    high=READ_REGISTER(LIS2DW12_OUT_Z_H);
    Z=CONVRT_12BIT(high,low,high_power_active);
    ACCZ = Z * sensitivity;
printf("Z:%d(0x%04X), high=0x%02x, low=0x%02x, %d\n",ACCZ, Z, high, low, sensitivity);

    return 0;
}

int lis2dw12_set_mode(uint8_t ctrl1, uint8_t ctrl6 )
{
    uint8_t reg, odr, mode, lp_mode, fs_mode;

printf("in lis2dw12_set_mode\n");
    reg    = READ_REGISTER(LIS2DW12_CTRL1);
    odr    = ctrl1 & 0xf0;
    mode   = ctrl1 & 0xc0;
    lp_mode= ctrl1 & 3;
    fs_mode= ctrl6 & 0x30;
    high_power_active= ctrl1 & 0x40;

    if( !(ctrl1&0xf0) ) {                           //first see if we want to power-down the device
        lis2dw12_onoff( 0 );
        WRITE_REGISTER(LIS2DW12_CTRL1, reg&0x0f);
        return 0;
        }

// ok we want to turn it on, what power-mode do we want and
// what sensitivity factor do we need...

    WRITE_REGISTER(LIS2DW12_CTRL1, ctrl1);
    WRITE_REGISTER(LIS2DW12_CTRL6, ctrl6);

    switch( fs_mode ) {
        case 0x00: //2g
            sensitivity = high_power_active? LIS2DW12_FS_2G_GAIN_HR : LIS2DW12_FS_2G_GAIN_LP;
            break;
        case 0x10: //4g
            sensitivity = high_power_active? LIS2DW12_FS_4G_GAIN_HR : LIS2DW12_FS_4G_GAIN_LP;
            break;
        case 0x20: //8g
            sensitivity = high_power_active? LIS2DW12_FS_8G_GAIN_HR : LIS2DW12_FS_8G_GAIN_LP;
            break;
        case 0x30: //16g
            sensitivity = high_power_active? LIS2DW12_FS_16G_GAIN_HR : LIS2DW12_FS_16G_GAIN_LP;
            break;
        }

    return 0;
}

#define LIS2DW12_INT_DUR_STAP_DEFAULT	0x00
#define LIS2DW12_INT_DUR_DTAP_DEFAULT	0x00
#define LIS2DW12_WAKE_UP_THS_WU_DEFAULT	0x02

int lis2dw12_configure_tap_event(int single_tap) {
    if (single_tap) {
        WRITE_REGISTER(LIS2DW12_INT_DUR, LIS2DW12_INT_DUR_STAP_DEFAULT);
        WRITE_REGISTER(LIS2DW12_WAKE_UP_THS, LIS2DW12_WAKE_UP_THS_WU_DEFAULT);
        }
    else {
        WRITE_REGISTER(LIS2DW12_INT_DUR, LIS2DW12_INT_DUR_DTAP_DEFAULT);
        WRITE_REGISTER(LIS2DW12_WAKE_UP_THS, (0x80|LIS2DW12_WAKE_UP_THS_WU_DEFAULT));
        }

    return 0;
}


void lis2dw12_timer_task(size_t timer_id, void * user_data) 
{
    uint8_t status = READ_REGISTER(LIS2DW12_STATUS);

    printf("        Status Register: 0x%04X\n",status);
    if (status & 0x10)                                  //double tap occured
        printf("                   : Double Tap occured\n");

    if (status & 0x08)                                  //single tap occures
        printf("                   : Single Tap occured\n");

    if (status & 0x04)                                  //6d orientation change occured
        printf("                   : 6D orientation change occured\n");

    if (status & 0x02)                                  //free fall event occured
        printf("                   : Free Fall occured\n");

    if (status & 0x01) {                                //data read event occured
        lis2dw12_get_acc_data();
        printf("  XYZ data Avaiable: X=%d, Y=%d, Z=%d\n", ACCX, ACCY, ACCZ);
        }

}

