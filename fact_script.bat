@echo off
::
::  This script allows the factory to prepare the SK2 for shipment by
::  installing the proper firmware, applications, and necessary script
::  files.  It then runs the factory test to verify operation and
::  if everything is correct, installs the scripts that autopower up
::  the quick-start application when powered up.
::
::  fact_script QCOM_BOOT_PORT  PATH_TO_FACT_SCRIPTS
:: 

setlocal ENABLEDELAYEDEXPANSION

set QUIET=^>NUL
set LOUD=

set VERBOSE=%QUIET%

set GSKDEV=a2e26b03f4e77aab23dbc5294b277d69
set PRODEV=4d074ddd8656173b0c4b29d2ac45ccad

set APIKEY=%GSKDEV%
set FSSRC=%1

if "%FSSRC%" EQU "" (
@echo "You must provide a path to the FACTORY_SCRIPTS"
goto err
) 

if not exist %FSSRC% (
@echo "You must provide a valid path to the FACTORY_SCRIPTS"
goto err
) 

@echo ^>^>Loading iot_monitor from %FSSRC%
@echo .
@echo .
@echo .

@echo ^>^> 1.Flash program the firmware (if needed)
call :eraseall
call :flash 

@echo Wait for Device to come on-line...
adb wait-for-device

@echo .
@echo .
@echo .

:: -------------------------------------------------
echo ^>^> 3.Put the Iot_monitor program onto the device
echo "install iot_monitor..."
adb shell "mkdir -p /CUSTAPP/iot"
adb push %FSSRC%/iot_monitor /CUSTAPP/iot/iot_monitor
adb shell "chmod +x /CUSTAPP/iot/iot_monitor"

@echo .
@echo .
@echo .

:: -------------------------------------------------
@echo ^>^> 4.Run the factory test
::  ** remove the -m flag if GPS test should be run **
adb shell "/CUSTAPP/iot/iot_monitor -r5 -m"
SET /P ans="Did it run successfully (Y/N)?"
if "%ans%" NEQ "Y" if "%ans%" NEQ "y" goto err

@echo .
@echo .
@echo .

:: -------------------------------------------------
@echo ^>^> 2.Update the malmanager.cfg file to have the correct APN set
echo "set APN correctly..."
adb shell "while [ ! -f = /CUSTAPP/user/mm_conf/malmanager.cfg ]; do sleep 1; done;"
adb shell "mv /CUSTAPP/user/mm_conf/malmanager.cfg  /CUSTAPP/user/mm_conf/malmanager.old_cfg"
adb shell "while [ ! -f = /CUSTAPP/user/mm_conf/malmanager.old_cfg ]; do sleep 1; done;"
adb shell "cat /CUSTAPP/user/mm_conf/malmanager.old_cfg | sed 's/\<name\>\ =.*;/name\ =\ \"m2m.com.attz\";/' > /CUSTAPP/user/mm_conf/malmanager.cfg"

@echo .
@echo .
@echo .

:: -------------------------------------------------
echo ^>^> 5.create run_start.sh and cust_app.sh
echo "install auto-start scripts..."
adb shell "echo 'start-stop-daemon -S -b -x /CUSTAPP/iot/run_demo.sh' > /CUSTAPP/custapp-postinit.sh"
adb shell "chmod +x /CUSTAPP/custapp-postinit.sh"
adb shell "echo '/CUSTAPP/iot/iot_monitor -q5 -a %APIKEY%' > /CUSTAPP/iot/run_demo.sh"
adb shell "chmod +x /CUSTAPP/iot/run_demo.sh"
goto okexit

:: ------------------------------------------------------------------------------------------------
:: ------------------------------------------------------------------------------------------------
:: ------------------------------------------------------------------------------------------------
:: ------------------------------------------------------------------------------------------------

:flash
echo Program new FIRMWARE
copy sbl1.mbn sbl1x.mbn /Y
copy partition.mbn.4k partition.mbn /Y

WNC_Dloader_SB3.0_Support_MDM9x07.exe -platform MDM9x07 -flashall -p %CPORT% >NUL

for /f "tokens=2 delims=: " %%a in ('fastboot.exe getvar nand-page-size 2^>^&1 ^| findstr nand-page-size') do @set nand-page-size=%%a
for /f "tokens=2 delims=: " %%a in ('fastboot.exe getvar nand-spare-size 2^>^&1 ^| findstr nand-spare-size') do @set nand-spare-size=%%a

@echo "page size is %nand-page-size%"
@echo "spare size is %nand-spare-size%"

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
::
:: Figure out where the QCOM Bootloader is (what comm port) and set the CPORT variable
::
TIMEOUT /T 2 /NOBREAK
for /f "usebackq tokens=1,2 delims=)" %%B in (`wmic path Win32_pnpentity where "caption like '%%QDLoader%%'" get caption ^| findstr "COM"` ) do set TMP=%%B
set CPORT=%TMP:~-2%
echo Using Bootloader CPORT=%CPORT%
exit /b

:: ------------------------------------------------------------------------------------------------
:err
echo "Error occured, exiting."
exit /b

:: ------------------------------------------------------------------------------------------------
:okexit
@echo .
@echo .
@echo .
echo "rebooting device..."
adb reboot
@echo wait to come on-line...
adb wait-for-device

