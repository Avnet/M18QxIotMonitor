@echo off
::
:: This script loades the iot_monitor onto the device then runs the 
:: factory test procedure.
:: 
:: If the factory test procedure is executed successfully, the batch
:: script will set the device up to auto-start on power-up using the 
:: APIKEY specified (GSKDEV or PRODEV one of which must be set below)
::
::  fact_test PATH_TO_IOT_MONITOR
:: 

set GSKDEV=a2e26b03f4e77aab23dbc5294b277d69
set PRODEV=4d074ddd8656173b0c4b29d2ac45ccad

set APIKEY=%GSKDEV%

set FSSRC=%1

if "%FSSRC%" EQU "" (
@echo You must provide a path to the IOT_MONITOR application
goto errexit
) 

if not exist %FSSRC%\iot_monitor (
@echo You must provide a valid path to IOT_MONITOR application
goto errexit
) 

@echo ^>^>Loading iot_monitor from %FSSRC%

:: incase the autostart scripts have been installed already...

@echo checking for device attached...
adb wait-for-device
adb shell "rm /CUSTAPP/custapp-postinit.sh" >nul 2>&1
adb shell "rm /CUSTAPP/iot/run_demo.sh" >nul 2>&1
@echo Wait for Device to come on-line...
adb reboot
adb wait-for-device

:: ----------------------------------------------------------------------------
echo ^>^> Load the Iot_monitor program onto the device
adb shell "mkdir -p /CUSTAPP/iot" >nul 2>&1
adb push %FSSRC%/iot_monitor /CUSTAPP/iot/iot_monitor
adb shell "chmod +x /CUSTAPP/iot/iot_monitor"

:: ----------------------------------------------------------------------------
@echo ^>^> Run the factory test
::  ** remove the -m flag if GPS test should be run **
adb shell "/CUSTAPP/iot/iot_monitor -r5 -m"
SET /P ans="Did it run successfully (Y/N)?"
if "%ans%" EQU "y" goto ok
if "%ans%" EQU "Y" goto ok

:errexit
echo Error occured, exiting.
exit /b

:ok
:: ----------------------------------------------------------------------------
@echo ^>^> Create run_start.sh and cust_app.sh
adb shell "echo 'start-stop-daemon -S -b -x /CUSTAPP/iot/run_demo.sh' > /CUSTAPP/custapp-postinit.sh"
adb shell "chmod +x /CUSTAPP/custapp-postinit.sh"
adb shell "echo '/CUSTAPP/iot/iot_monitor -q5 -a %APIKEY%' > /CUSTAPP/iot/run_demo.sh"
adb shell "chmod +x /CUSTAPP/iot/run_demo.sh"

echo rebooting device...
adb reboot
@echo wait to come on-line...
adb wait-for-device
exit /b

