@echo off
#
#  This script allows the factory to prepare the SK2 for shipment by
#  installing the proper firmware, applications, and necessary script
#  files.  It then runs the factory test to verify operation and
#  if everything is correct, installs the scripts that autopower up
#  the quick-start application when powered up.
#
# 1.Flash program the firmware (if needed)
# ... whatever...

# 2.Update the malmanager.cfg file to have the correct APN set
adb push apn_update.sh /data/
adb shell "chmod +x /data/apn_update.sh"
adb shell "/data/apn_update.sh"

# 3.Put the Iot_monitor program onto the device
adb shell "mkdir /data/iot"
adb push iot_monitor /data/iot

# 4.Run the factory test
#  ** remove the -m flag if GPS test should be run **
adb shell "/data/iot/iot_monitor -r5 -m"
SET /P ans="Did it run successfully (Y/N)?"
if "%ans%" != "Y" goto err
# 5.push run_start.sh and cust_app.sh
adb push custapp-postinit.sh /data
adb shell "chmod +x /data/custapp-postinit.sh"
adb push run_demo.sh /data/iot
adb shell "chmod +x /data/iot/fun_demo.sh"
# 6.reboot
adb reboot
adb wait-for-device
exit
:err
echo "Error occured, exiting."
