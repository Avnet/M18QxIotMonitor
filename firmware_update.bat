@echo off
::
::  This script will installing the firmware that is co-located in the
::  same directory as this script. 
::
::  fact_script PATH_TO_IOT_MONITOR
:: 

setlocal ENABLEDELAYEDEXPANSION

@echo ^>^> Flash program the firmware
call :eraseall
call :flash 

@echo Now, wait for the device to come back on-line...
adb wait-for-device

goto okexit

:: ----------------------------------------------------------------------------
:: ----------------------------------------------------------------------------
:: ----------------------------------------------------------------------------
:: ----------------------------------------------------------------------------

:flash
echo Program new FIRMWARE
start devmgmt.msc
echo We need to know the COM number for the Qualcomm HS-USB QDLoader--please find it
SET /P CPORT="in device manager and enter it here: "
echo Using: COM%CPORT%
copy sbl1.mbn sbl1x.mbn /Y
copy partition.mbn.4k partition.mbn /Y

WNC_Dloader_SB3.0_Support_MDM9x07.exe -platform MDM9x07 -flashall -p %CPORT% >NUL

for /f "tokens=2 delims=: " %%a in ('fastboot.exe getvar nand-page-size 2^>^&1 ^| findstr nand-page-size') do @set nand-page-size=%%a
for /f "tokens=2 delims=: " %%a in ('fastboot.exe getvar nand-spare-size 2^>^&1 ^| findstr nand-spare-size') do @set nand-spare-size=%%a

@echo page size is %nand-page-size%
@echo spare size is %nand-spare-size%

if %nand-page-size% == 2048 (
  set page-size=2k
  if %nand-spare-size% leq 64 (
    set vendor-append=.ESMT
  )else if %nand-spare-size% == 128 (
    set vendor-append=.ETRON
  )
) else if %nand-page-size% == 4096 (
  set page-size=4k
)

set "yaffs2_append=%page-size%%vendor-append%"

fastboot flash boot_b mdm9607-boot.img.%page-size% 2>NUL
fastboot flash system_b mdm9607-sysfs.ubi.%page-size% 2>NUL
fastboot flash boot_a mdm9607-boot.img.%page-size% 2>NUL
fastboot flash system_a mdm9607-sysfs.ubi.%page-size% 2>NUL
fastboot flash data mdm9607-datafs.ubi.%page-size% 2>NUL
fastboot flash firmware_b firmware.ubi.%page-size% 2>NUL
fastboot flash firmware_a firmware.ubi.%page-size% 2>NUL
fastboot reboot

exit /b


:: ------------------------------------------------------------------------------------------------
:eraseall
echo Erase FLASH
adb shell sys_reboot bootloader
IF "%ERRORLEVEL%" NEQ "-1" (
fastboot erase sbl
fastboot erase aboot
fastboot erase boot_a
fastboot erase boot_b
fastboot reboot
)
TIMEOUT /T 2 /NOBREAK
exit /b

:: ------------------------------------------------------------------------------------------------
:err
echo Error occured, exiting.
exit /b

:: ------------------------------------------------------------------------------------------------
:okexit
@echo .
@echo .
@echo .
@echo rebooting device...
adb reboot
@echo wait to come on-line...
adb wait-for-device

