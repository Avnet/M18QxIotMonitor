// ======================================================================================
//                               COMMS & CAPTURE SDK
// ======================================================================================
// Copyright 2008-2014 STMicroelectronics. All Rights Reserved.
//
// NOTICE:  STMicroelectronics permits you to use this file in accordance with the terms
// of the STMicroelectronics license agreement accompanying it.
// ======================================================================================
#ifndef __RANGING_SENSOR_EVK_COMMS_H__
#define __RANGING_SENSOR_EVK_COMMS_H__

#ifdef _MSC_VER
#   ifdef RANGING_SENSOR_EVK_COMMS_EXPORTS
#       define COMMS_API  __declspec(dllexport)
#   else
#       define COMMS_API
#   endif
#else
#   define COMMS_API
#endif


#include "comms_platform.h"
#include "version_def.h"
#include "gpio_defs.h"

#ifdef __cplusplus
    extern "C" {
#endif

COMMS_API DWORD RANGING_SENSOR_COMMS_Get_Version(PCOMMS_VERSION_INFO pVersion);
COMMS_API version_info RANGING_SENSOR_COMMS_GetVersion(void);

//////////////////////////////////////////////////////////////////////////////
//
// Comms type-inspecific functions
//

// Error text
COMMS_API DWORD RANGING_SENSOR_COMMS_Get_Error_Text(char* strError);

// Display name
COMMS_API DWORD RANGING_SENSOR_COMMS_Get_Display_String(char* strDisplay);

// Comms types
COMMS_API DWORD RANGING_SENSOR_COMMS_Get_Num_Comms_Types(void);

COMMS_API DWORD RANGING_SENSOR_COMMS_Get_Comms_Type_String(DWORD dwIndex, char* strCommsType);

// Enum ST USB devices
COMMS_API DWORD RANGING_SENSOR_COMMS_Enum_Devices(DWORD dwDeviceIDArraySize, BYTE* pcDeviceIDArray, DWORD* pdwNumDevicesFound);



// I2C Bus speed
COMMS_API DWORD RANGING_SENSOR_COMMS_Set_I2C_Bus_Speed(DWORD I2C_bus_speed_khz);
COMMS_API DWORD RANGING_SENSOR_COMMS_Get_I2C_Bus_Speed(PUSHORT pI2C_bus_speed_khz);

// Cycle Sensor Power
COMMS_API DWORD RANGING_SENSOR_COMMS_Cycle_Sensor_Power(void);

// Read EVK Firmware Version
COMMS_API version_info RANGING_SENSOR_COMMS_GetFirmwareVersion(void);
COMMS_API DWORD RANGING_SENSOR_COMMS_Read_EVK_Firmware_Version(unsigned char* major, unsigned char* minor, unsigned char* build, unsigned int* revision);

// Current page
COMMS_API DWORD RANGING_SENSOR_COMMS_Get_Page(BYTE* pCurrent_Page);

// Regulator power control
COMMS_API DWORD RANGING_SENSOR_COMMS_Get_Power_State(DWORD regulator_bitmask, PDWORD pstate_bitmask);
COMMS_API DWORD RANGING_SENSOR_COMMS_Set_Power_State(DWORD regulator_bitmask, DWORD state_bitmask, DWORD delay_ms);

// Firmware debug output on PCB1515 comm port
COMMS_API DWORD RANGING_SENSOR_COMMS_Firmware_Trace_Config(DWORD modules, DWORD level, DWORD functions);

// Board configuration information
COMMS_API DWORD RANGING_SENSOR_COMMS_Get_Board_Configuration(PDWORD number,   
                                                   PDWORD revision,
                                                   PDWORD variant,
                                                   PDWORD serial, 
                                                   BYTE   extra[35]);
// Put the EVK into HID boot loader mode
COMMS_API DWORD RANGING_SENSOR_COMMS_Enter_Bootloader(void);


typedef enum tagGPIO_Mode
{
	GPIO_In = 0, 
	GPIO_OutPP,
	GPIO_OutOD,
    // Timer modes can be added in here once defined
} GPIO_Mode;

typedef enum tagGPIO_State
{
	GPIO_State_Low = 0,
	GPIO_State_High,
} GPIO_State;

// Send single pulse out on specified GPIO (not all GPIO support pulses - see STM32 documentation)
COMMS_API DWORD RANGING_SENSOR_COMMS_GPIO_Pulse(RS_GPIO_Pin pin, DWORD period_width_ms, DWORD pulse_width_ms);

// Get the value of a specified GPIO pin (actual pin mapping is specific to the STM32 device e.g. Ranging Sensor EVK, BabyBear EVK etc...)
COMMS_API DWORD RANGING_SENSOR_COMMS_GPIO_Get_Value(RS_GPIO_Pin pin, DWORD* pvalue);

// Set the value of a specified GPIO pin (actual pin mapping is specific to the STM32 device e.g. Ranging Sensor EVK, BabyBear EVK etc...)
COMMS_API DWORD RANGING_SENSOR_COMMS_GPIO_Set_Value(RS_GPIO_Pin pin, DWORD value);

// Set the mode of a specified GPIO pin (actual pin mapping is specific to the STM32 device e.g. Ranging Sensor EVK, BabyBear EVK etc...)
COMMS_API DWORD RANGING_SENSOR_COMMS_GPIO_Set_Mode(RS_GPIO_Pin pin, GPIO_Mode mode);

// Sets the state of individual LEDs to allow user to disable for scenarios such as dark room setups
COMMS_API DWORD RANGING_SENSOR_COMMS_Set_Led_Enable(BYTE LED, BYTE enable);

COMMS_API DWORD RANGING_SENSOR_COMMS_Write_System_I2C(
    BYTE cAddress, 
    DWORD dwNoBytes, 
    BYTE* pcWriteBuf);

COMMS_API DWORD RANGING_SENSOR_COMMS_Read_System_I2C(
    BYTE cAddress, 
    DWORD dwNoBytes, 
    BYTE* pcWriteBuf);

COMMS_API DWORD RANGING_SENSOR_COMMS_Write_Raw_I2C(
    BYTE cAddress, 
    DWORD dwNoBytes, 
    BYTE* pcWriteBuf);

COMMS_API DWORD RANGING_SENSOR_COMMS_Read_Raw_I2C(
    BYTE cAddress, 
    DWORD dwNoBytes, 
    BYTE* pcWriteBuf);


COMMS_API DWORD RANGING_SENSOR_COMMS_Init_V2W8(
    DWORD   dwCamID,
    DWORD   argc,		// Number of strings in array argv
    char*   argv[]);	// Array of command-line argument strings

COMMS_API DWORD RANGING_SENSOR_COMMS_Fini_V2W8(void);

// V2W8 read - 8 bit index, variable length data
COMMS_API DWORD RANGING_SENSOR_COMMS_Read_V2W8(
    DWORD   dwAddress,
    DWORD   dwIndexHi,
    DWORD   dwIndexLo,
    BYTE*   pcValues,
    DWORD   dwBufferSize);

// V2W8 write - 8 bit index, variable length data
COMMS_API DWORD RANGING_SENSOR_COMMS_Write_V2W8(
    DWORD   dwAddress,
    DWORD   dwIndexHi,
    DWORD   dwIndexLo,
    BYTE*   pcValues,
    DWORD   dwBufferSize);


COMMS_API DWORD RANGING_SENSOR_COMMS_Init_CCI(
    DWORD   dwCamID,
    DWORD   argc,		// Number of strings in array argv
    char*   argv[]);	// Array of command-line argument strings

COMMS_API DWORD RANGING_SENSOR_COMMS_Fini_CCI(void);

// CCI read - 16 bit index, variable length data
COMMS_API DWORD RANGING_SENSOR_COMMS_Read_CCI(
    DWORD   dwAddress,
    DWORD   dwIndexHi,
    DWORD   dwIndexLo,
    BYTE*   pcValues,
    DWORD   dwBufferSize);

// CCI write - 16 bit index, variable length data
COMMS_API DWORD RANGING_SENSOR_COMMS_Write_CCI(
    DWORD   dwAddress,
    DWORD   dwIndexHi,
    DWORD   dwIndexLo,
    BYTE*   pcValues,
    DWORD   dwBufferSize);


COMMS_API DWORD RANGING_SENSOR_COMMS_Init_SPI_V2W8(
    DWORD   dwCamID,
    DWORD   argc,		// Number of strings in array argv
    char*   argv[]);	// Array of command-line argument strings

COMMS_API DWORD RANGING_SENSOR_COMMS_Fini_SPI_V2W8(void);

// SPI read - 8 bit index, variable length data
COMMS_API DWORD RANGING_SENSOR_COMMS_Read_SPI_V2W8(
    DWORD   dwAddress,
    DWORD   dwIndexHi,
    DWORD   dwIndexLo,
    BYTE*   pcValues,
    DWORD   dwBufferSize);

// SPI write - 8 bit index, variable length data
COMMS_API DWORD RANGING_SENSOR_COMMS_Write_SPI_V2W8(
    DWORD   dwAddress,
    DWORD   dwIndexHi,
    DWORD   dwIndexLo,
    BYTE*   pcValues,
    DWORD   dwBufferSize);


COMMS_API DWORD RANGING_SENSOR_COMMS_Init_SPI_16I(
    DWORD   dwCamID,
    DWORD   argc,		// Number of strings in array argv
    char*   argv[]);	// Array of command-line argument strings

COMMS_API DWORD RANGING_SENSOR_COMMS_Fini_SPI_16I(void);

// SPI read - 16 bit index, variable length data
COMMS_API DWORD RANGING_SENSOR_COMMS_Read_SPI_16I(
    DWORD   dwAddress,
    DWORD   dwIndexHi,
    DWORD   dwIndexLo,
    BYTE*   pcValues,
    DWORD   dwBufferSize);

// SPI write - 16 bit index, variable length data
COMMS_API DWORD RANGING_SENSOR_COMMS_Write_SPI_16I(
    DWORD   dwAddress,
    DWORD   dwIndexHi,
    DWORD   dwIndexLo,
    BYTE*   pcValues,
    DWORD   dwBufferSize);

/* 
    Custom read/write/lock/unlock for the Doppler API 

    Dopper API would typically do the following for a read of register 20 in page 1...
        HANDLE lock_handle;
        RANGING_SENSOR_COMMS_Lock_V2W8(&lock_handle);
        char page_number = 1;
        RANGING_SENSOR_COMMS_Write_Nolock_V2W8(0xA6, 0x7F, &page_number, 1);
        char read_data;
        RANGING_SENSOR_COMMS_Read_Nolock_V2W8(0xA6, 20, &read_data, 1);
        RANGING_SENSOR_COMMS_Unlock_V2W8(&lock_handle);

    Dopper API would typically do the following for a write of register 10 in page 4...
        HANDLE lock_handle;
        RANGING_SENSOR_COMMS_Lock_V2W8(&lock_handle);
        char page_number = 4;
        RANGING_SENSOR_COMMS_Write_Nolock_V2W8(0xA6, 0x7F, &page_number, 1);
        char write_data = 1;
        RANGING_SENSOR_COMMS_Write_Nolock_V2W8(0xA6, 10, &write_data, 1);
        RANGING_SENSOR_COMMS_Unlock_V2W8(&lock_handle);
*/

/*
 * lock the mutex then open a handle to the Ranging Sensor EVK
 */
COMMS_API DWORD RANGING_SENSOR_COMMS_Lock_V2W8(PVOID phMutex);    // phMutex must be a pointer to a HANDLE

/*
 * Close handle to the BabyBear EVK then unlock the mutex
 */
COMMS_API DWORD RANGING_SENSOR_COMMS_Unlock_V2W8(PVOID phMutex);  // phMutex must be a pointer to a HANDLE

/*
 * read a register using V2W8 - no locking performed!
 */
COMMS_API DWORD RANGING_SENSOR_COMMS_Read_Nolock_V2W8(BYTE address, BYTE index, BYTE* dataout, BYTE buffer_size);

/*
 * write a register using V2W8 - no locking performed!
 */
COMMS_API DWORD RANGING_SENSOR_COMMS_Write_Nolock_V2W8(BYTE address, BYTE index, BYTE* datain, BYTE buffer_size);



/*
 * lock the mutex then open a handle to the Ranging Sensor EVK
 */
COMMS_API DWORD RANGING_SENSOR_COMMS_Lock_SPI_V2W8(PVOID phMutex);    // phMutex must be a pointer to a HANDLE

/*
 * Close handle to the BabyBear EVK then unlock the mutex
 */
COMMS_API DWORD RANGING_SENSOR_COMMS_Unlock_SPI_V2W8(PVOID phMutex);  // phMutex must be a pointer to a HANDLE

/*
 * read a register using SPI - no locking performed!
 */
COMMS_API DWORD RANGING_SENSOR_COMMS_Read_Nolock_SPI_V2W8(BYTE index, BYTE* dataout, BYTE buffer_size);

/*
 * write a register using SPI - no locking performed!
 */
COMMS_API DWORD RANGING_SENSOR_COMMS_Write_Nolock_SPI_V2W8(BYTE index, BYTE* datain, BYTE buffer_size);



/*
 * lock the mutex then open a handle to the Ranging Sensor EVK
 */
COMMS_API DWORD RANGING_SENSOR_COMMS_Lock_CCI(PVOID phMutex);    // phMutex must be a pointer to a HANDLE

/*
 * Close handle to the BabyBear EVK then unlock the mutex
 */
COMMS_API DWORD RANGING_SENSOR_COMMS_Unlock_CCI(PVOID phMutex);  // phMutex must be a pointer to a HANDLE

/*
 * read a register using V2W8 - no locking performed!
 */
COMMS_API DWORD RANGING_SENSOR_COMMS_Read_Nolock_CCI(BYTE address, USHORT index, BYTE* dataout, BYTE buffer_size);

/*
 * write a register using V2W8 - no locking performed!
 */
COMMS_API DWORD RANGING_SENSOR_COMMS_Write_Nolock_CCI(BYTE address, USHORT index, BYTE* datain, BYTE buffer_size);

#ifdef __cplusplus
}
#endif


#endif //__RANGING_SENSOR_EVK_COMMS_H__
