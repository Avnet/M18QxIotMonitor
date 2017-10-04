@echo off
rem
rem  This script allows the factory to prepare the SK2 for shipment by
rem  installing the proper firmware, applications, and necessary script
rem  files.  It then runs the factory test to verify operation and
rem  if everything is correct, installs the scripts that autopower up
rem  the quick-start application when powered up.
rem
rem 1.Flash program the firmware (if needed)
rem ... whatever...

rem 2.Update the malmanager.cfg file to have the correct APN set
echo "set APN correctly..."
adb push apn_update.sh /data/
adb shell "chmod +x /data/apn_update.sh"
adb shell "/data/apn_update.sh"
echo .

rem 3.Put the Iot_monitor program onto the device
echo "install iot_monitor..."
adb shell "mkdir -p /data/iot"
adb push iot_monitor /data/iot
adb shell "chmod +x /data/iot/iot_monitor"
echo .

rem 4.Run the factory test
rem  ** remove the -m flag if GPS test should be run **
adb shell "/data/iot/iot_monitor -r5 -m"
SET /P ans="Did it run successfully (Y/N)?"
if "%ans%" NEQ "Y" AND "%ans%" NEQ "y" goto err

rem 5.push run_start.sh and cust_app.sh
echo "install auto-start scripts..."
adb push custapp-postinit.sh /data/custapp-postinit.sh
adb shell "chmod +x /data/custapp-postinit.sh"
adb push run_demo.sh /data/iot/run_demo.sh
adb shell "chmod +x /data/iot/run_demo.sh"

rem 6.reboot
adb reboot
echo "rebooting device..."
goto ok

:err
echo "Error occured, exiting."
:ok
