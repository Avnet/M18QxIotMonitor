
#ifndef __LIS2DW12_H__
#define __LIS2DW12_H__
               
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/syscall.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <nettle/nettle-stdint.h>
#include <hwlib/hwlib.h>
#include <json-c/json.h>

#define LIS2DW12_DEV_NAME		"lis2dw12"

#define HZ_TO_PERIOD_NSEC(hz)	(1000 * 1000 * 1000 / ((uint32_t)(hz)))
#define MS_TO_US(x)		({ typeof(x) _x = (x); ((_x) * \
				 ((typeof(x)) 1000));})
#define US_TO_NS(x)		(MS_TO_US(x))
#define MS_TO_NS(x)		(US_TO_NS(MS_TO_US(x)))
#define US_TO_MS(x)		({ typeof(x) _x = (x); ((_x) / \
				 ((typeof(x)) 1000));})
#define NS_TO_US(x)		(US_TO_MS(x))
#define NS_TO_MS(x)		(US_TO_MS(NS_TO_US(x)))

enum {
	LIS2DW12_ACCEL = 0,
	LIS2DW12_FF,
	LIS2DW12_TAP,
	LIS2DW12_DOUBLE_TAP,
	LIS2DW12_WAKEUP,
	LIS2DW12_SENSORS_NUMB,
};

#define INPUT_EVENT_TYPE	EV_MSC
#define INPUT_EVENT_X		MSC_SERIAL
#define INPUT_EVENT_Y		MSC_PULSELED
#define INPUT_EVENT_Z		MSC_GESTURE
#define INPUT_EVENT_TIME_MSB	MSC_SCAN
#define INPUT_EVENT_TIME_LSB	MSC_MAX

#define LIS2DW12_RX_MAX_LENGTH	16
#define LIS2DW12_TX_MAX_LENGTH	16

#define to_dev(obj) container_of(obj, struct device, kobj)


#if 0  //jmf
struct reg_rw {
	uint8_t const address;
	uint8_t const init_val;
	uint8_t resume_val;
};

struct reg_r {
	const uint8_t address;
	const uint8_t init_val;
};

struct lis2dw12_transfer_buffer {
	struct mutex buf_lock;
	uint8_t rx_buf[LIS2DW12_RX_MAX_LENGTH];
	uint8_t tx_buf[LIS2DW12_TX_MAX_LENGTH] ____cacheline_aligned;
};

struct lis2dw12_data;

struct lis2dw12_transfer_function {
	int (*write)(struct lis2dw12_data *cdata, uint8_t reg_addr,
		     int len, uint8_t *data, bool b_lock);
	int (*read)(struct lis2dw12_data *cdata, uint8_t reg_addr, int len, uint8_t *data,
		    bool b_lock);
};

struct lis2dw12_sensor_data {
	struct lis2dw12_data *cdata;
	const char* name;
//	s64 timestamp;
	uint8_t enabled;
	uint32_t c_odr;
	uint32_t c_gain;
	uint8_t sindex;
	unsigned int poll_ms;
	struct input_dev *input_dev;
	struct hrtimer hr_timer;
	struct work_struct input_work;
	ktime_t oldktime;
};

struct lis2dw12_data {
	const char *name;
	uint8_t drdy_int_pin;
	uint8_t selftest_status;
	uint8_t power_mode;
	int irq;
//	s64 timestamp;
	struct device *dev;
	struct lis2dw12_sensor_data sensors[LIS2DW12_SENSORS_NUMB];
	struct mutex bank_registers_lock;
	const struct lis2dw12_transfer_function *tf;
	struct lis2dw12_transfer_buffer tb;
};

int lis2dw12_common_probe(struct lis2dw12_data *cdata, int irq, u16 bustype);
void lis2dw12_common_remove(struct lis2dw12_data *cdata, int irq);

#ifdef CONFIG_PM
int lis2dw12_common_suspend(struct lis2dw12_data *cdata);
int lis2dw12_common_resume(struct lis2dw12_data *cdata);
#endif /* CONFIG_PM */
#endif //jmf

#define LIS2DW12_SAD			0x19
#define LIS2DW12_WHO_AM_I_ADDR		0x0f
#define LIS2DW12_WHO_AM_I_DEF		0x44

#define LIS2DW12_CTRL1_ADDR		0x20
#define LIS2DW12_CTRL2_ADDR		0x21
#define LIS2DW12_CTRL3_ADDR		0x22
#define LIS2DW12_CTRL4_INT1_PAD_ADDR	0x23
#define LIS2DW12_CTRL5_INT2_PAD_ADDR	0x24
#define LIS2DW12_CTRL6_ADDR		0x25
#define LIS2DW12_OUT_T_ADDR		0x26
#define LIS2DW12_STATUS_ADDR		0x27
#define LIS2DW12_OUTX_L_ADDR		0x28

#define LIS2DW12_TAP_THS_X_ADDR		0x30
#define LIS2DW12_TAP_THS_Y_ADDR		0x31
#define LIS2DW12_TAP_THS_Z_ADDR		0x32
#define LIS2DW12_INT_DUR_ADDR		0x33
#define LIS2DW12_WAKE_UP_THS_ADDR		0x34
#define LIS2DW12_WAKE_UP_DUR_ADDR		0x35
#define LIS2DW12_FREE_FALL_ADDR		0x36
#define LIS2DW12_STATUS_DUP_ADDR		0x37
#define LIS2DW12_WAKE_UP_SRC_ADDR		0x38
#define LIS2DW12_TAP_SRC_ADDR		0x39
#define LIS2DW12_6D_SRC_ADDR		0x3a
#define LIS2DW12_ALL_INT_ADDR		0x3b

#define LIS2DW12_WAKE_UP_IA_MASK		0x40
#define LIS2DW12_DOUBLE_TAP_MASK		0x10
#define LIS2DW12_SINGLE_TAP_MASK		0x08
#define LIS2DW12_6D_IA_MASK		0x04
#define LIS2DW12_FF_IA_MASK		0x02
#define LIS2DW12_DRDY_MASK		0x01
#define LIS2DW12_EVENT_MASK		(LIS2DW12_WAKE_UP_IA_MASK | \
					 LIS2DW12_DOUBLE_TAP_MASK | \
					 LIS2DW12_SINGLE_TAP_MASK | \
					 LIS2DW12_6D_IA_MASK | \
					 LIS2DW12_FF_IA_MASK)

#define LIS2DW12_ODR_MASK			0xf0
#define LIS2DW12_ODR_POWER_OFF_VAL	0x00
#define LIS2DW12_ODR_1HZ_LP_VAL		0x01
#define LIS2DW12_ODR_12HZ_LP_VAL		0x02
#define LIS2DW12_ODR_25HZ_LP_VAL		0x03
#define LIS2DW12_ODR_50HZ_LP_VAL		0x04
#define LIS2DW12_ODR_100HZ_LP_VAL		0x05
#define LIS2DW12_ODR_200HZ_LP_VAL		0x06
#define LIS2DW12_ODR_400HZ_LP_VAL		0x06
#define LIS2DW12_ODR_800HZ_LP_VAL		0x06
#define LIS2DW12_ODR_LP_LIST_NUM		9

#define LIS2DW12_ODR_12_5HZ_HR_VAL	0x02
#define LIS2DW12_ODR_25HZ_HR_VAL		0x03
#define LIS2DW12_ODR_50HZ_HR_VAL		0x04
#define LIS2DW12_ODR_100HZ_HR_VAL		0x05
#define LIS2DW12_ODR_200HZ_HR_VAL		0x06
#define LIS2DW12_ODR_400HZ_HR_VAL		0x07
#define LIS2DW12_ODR_800HZ_HR_VAL		0x08
#define LIS2DW12_ODR_HR_LIST_NUM		8

#define LIS2DW12_LP_MODE_MASK			0x03
#define LIS2DW12_POWER_MODE_MASK		0x0c

#define LIS2DW12_FS_MASK			0x30
#define LIS2DW12_FS_2G_VAL		0x00
#define LIS2DW12_FS_4G_VAL		0x01
#define LIS2DW12_FS_8G_VAL		0x02
#define LIS2DW12_FS_16G_VAL		0x03

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

#define LIS2DW12_FS_LIST_NUM		4

#define LIS2DW12_INT1_6D_MASK		0x80
#define LIS2DW12_INT1_S_TAP_MASK		0x40
#define LIS2DW12_INT1_WAKEUP_MASK		0x20
#define LIS2DW12_INT1_FREE_FALL_MASK	0x10
#define LIS2DW12_INT1_TAP_MASK		0x08
#define LIS2DW12_INT1_DRDY_MASK		0x01
#define LIS2DW12_INT2_SLEEP_MASK		0x40
#define LIS2DW12_INT1_EVENTS_MASK		(LIS2DW12_INT1_S_TAP_MASK | \
					 LIS2DW12_INT1_WAKEUP_MASK | \
					 LIS2DW12_INT1_FREE_FALL_MASK | \
					 LIS2DW12_INT1_TAP_MASK | \
					 LIS2DW12_INT1_6D_MASK)

#define LIS2DW12_INT_DUR_SHOCK_MASK	0x03
#define LIS2DW12_INT_DUR_QUIET_MASK	0x0c
#define LIS2DW12_INT_DUR_LAT_MASK		0xf0
#define LIS2DW12_INT_DUR_MASK		(LIS2DW12_INT_DUR_SHOCK_MASK | \
					 LIS2DW12_INT_DUR_QUIET_MASK | \
					 LIS2DW12_INT_DUR_LAT_MASK)

#define LIS2DW12_INT_DUR_STAP_DEFAULT	0x06
#define LIS2DW12_INT_DUR_DTAP_DEFAULT	0x7f

#define LIS2DW12_WAKE_UP_THS_S_D_TAP_MASK	0x80
#define LIS2DW12_WAKE_UP_THS_SLEEP_MASK	0x40
#define LIS2DW12_WAKE_UP_THS_WU_MASK	0x3f
#define LIS2DW12_WAKE_UP_THS_WU_DEFAULT	0x02

#define LIS2DW12_FREE_FALL_THS_MASK	0x07
#define LIS2DW12_FREE_FALL_DUR_MASK	0xf8
#define LIS2DW12_FREE_FALL_THS_DEFAULT	0x01
#define LIS2DW12_FREE_FALL_DUR_DEFAULT	0x01

#define LIS2DW12_BDU_MASK			0x08
#define LIS2DW12_SOFT_RESET_MASK		0x40
#define LIS2DW12_LIR_MASK			0x10

#define LIS2DW12_TAP_AXIS_MASK		0xe0
#define LIS2DW12_TAP_AXIS_ANABLE_ALL	0xe0
#define LIS2DW12_TAP_THS_MASK		0x1f
#define LIS2DW12_TAP_THS_DEFAULT		0x09
#define LIS2DW12_INT2_ON_INT1_MASK	0x20

#define LIS2DW12_OUT_XYZ_SIZE		6
#define LIS2DW12_EN_BIT			0x01
#define LIS2DW12_DIS_BIT			0x00
#define LIS2DW12_EN_LP_MODE_02	0x01

#define LIS2DW12_ACCEL_FS			2
#define LIS2DW12_FF_ODR			25
#define LIS2DW12_TAP_ODR			400
#define LIS2DW12_WAKEUP_ODR		25

#define LIS2DW12_MIN_EVENT_ODR		25

#ifdef __cplusplus
extern "C" {
#endif

int      lis2dw12_initialize(void);
uint8_t  lis2dw12_getDeviceID(void);
uint8_t  lis2dw12_read_byte(uint8_t reg_addr) ;
uint16_t lis2dw12_read_word(uint8_t reg_addr) ;
void     lis2dw12_write_reg(uint8_t reg_addr, uint8_t value) ;
void     lis2dw12_read(uint8_t reg_addr, uint8_t *buf, int nbr) ;

#ifdef __cplusplus
}
#endif

#endif //__LIS2DW12_H__




