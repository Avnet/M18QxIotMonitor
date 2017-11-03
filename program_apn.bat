@echo off
::
::  This script programs the APN for either 
::  GLOBAL (AES_ATT-M18QWG-M1-G) or US (AES-ATT-M18Q2FG-M1-G)
::  by specifing either GLOBAL or US as the command line argument.
::
::  program_apn (GLOBAL | US)
:: 

set APN=%1
if "%APN%" == "US" (set APN=m2m.com.attz && goto ok)
if "%APN%" == "us" (set APN=m2m.com.attz && goto ok)
if "%APN%" == "global" (set APN=gsk.com.attz && goto ok)
if "%APN%" == "GLOBAL" (set APN=gsk.com.attz && goto ok)
goto errexit

:ok
adb wait-for-device
adb shell "ls /CUSTAPP/user/mm_conf/malmanager.cfg" > nul 2>&1
IF "%ERRORLEVEL%" == "-1" goto nodevexit

set APN=%APN:~,-1%
@echo ^>^> Update malmanager.cfg to %APN% 
adb shell "while [ -f = /CUSTAPP/user/mm_conf/malmanager.cfg ]; do sleep 1; done;"
adb shell "mv /CUSTAPP/user/mm_conf/malmanager.cfg  /CUSTAPP/user/mm_conf/malmanager.old_cfg"
adb shell "while [ -f = /CUSTAPP/user/mm_conf/malmanager.old_cfg ]; do sleep 1; done;"
adb shell "cat /CUSTAPP/user/mm_conf/malmanager.old_cfg | sed 's/\<name\>\ =.*;/name\ =\ \"%APN%\";/' > /CUSTAPP/user/mm_conf/malmanager.cfg"
@echo rebooting device...
adb reboot
@echo wait to come on-line...
adb wait-for-device
exit /b

:nodevexit
@echo The module was not detected, you must make sure the device is connected
@echo and the command line is of the form:  program_apn (GLOBAL ^| US)
exit /b

:errexit
@echo An Error occured, you must make sure the device is connected and the
@echo command line is of the form:  program_apn (GLOBAL ^| US)
exit /b

