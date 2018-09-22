# Starter Kit 2 (SK2) based on the WNC M18Qx 

## M18QxIotMonitor Overview
The 'iot_monitor is a simple interactive monitor program intended to demonstrate the many features of the Start kit 2.  The **iot_monitor** is installed at the factory
and is available out of the box.  To access it you need to have ADB installed on the PC connected to the SK2 so there are a few steps necessary to get
started.  The following assumes your connected PC has Ubuntu Linux installed and running, installation of the tools on other O/Ss will be different.

## Prepare the development environment
Prepare the development environment by installing ADB and the compiler/tools needed.  The basic tools needed can be obtained from **http://github.com/Avnet/AvnetWNCSDK**

### Install ADB
1. Open up a terminal windows.
2. Install ADB by issuing the command '**sudo apt-get install adb**'
3. Clone the AvnetWNCSDK by issuing the command **'git clone http://github.com/Avnet/AvnetWNCSDK'**
4. Copy the file file **adbpub.key** from the AvnetWNCSDK directory to the ADB directory (cp ./AvnetWNCSDK/adbpub.key ~/.android/).  This installs the security key so ADB can access the SK2.
5. Connect your SK2 to the development PC
6. Verify the ADB connection by listing the attached devices, **sudo adb devices** When you do this the SK2 should be reported as:

    AvnetWNCSDK$ sudo adb devices
    List of devices attached
    WNC_ADB	device

### Install the compiler and configure
1. Goto the AvnetWNCSDK directory

2. Install the tools by executing:  **sudo ./oecorex-86_64-cortexa7-neon-vfpv4-toolchain-nodistro.0.sh**

3. Add the environment variables (you will need to do this each time you open a new terminal window for development) by executing: **" .    /usr/local/oecore-x86_64/environment-setup-cortexa7-neon-vfpv4-oe-linux-gnueabi "** *(there is a spaces after the '.')*

## Install the M18QxIotMonitor source code
1. Move up out of the AvnetWNCSDK directory.

2. Clone the M18QxIoTMonitor code by executing: **'git clone https://github.com/Avnet/M18QxIotMonitor'**

3. Change the directory to **M18QxIotMonitor**

4. Set the environment: **" .    /usr/local/oecore-x86_64/environment-setup-cortexa7-neon-vfpv4-oe-linux-gnueabi "**

5. run autogen: **./autogen.sh**

6. run configure: **"./configure ${CONFIGURE_FLAGS}"**

The tools and source code are now installed.  You can compile the code by typing: **"make"**

## Push the executable to the SK2
You use ADB to push the executable image to the SK2 and place it in the corrector location.  This location is **"/CUSTAPP/"**.  Execute the following:

    M18QxIotMonitor$ adb push iot_monitor /CUSTAPP/iot/
    [100%] /CUSTAPP/iot/iot_monitor

After the code has been pushed to the SK2, you can execute it by entering an ADB shell and running it, e.g.

    M18QxIotMonitor$ adb shell
    # cd /CUSTAPP
    /CUSTAPP # ls
    all.log           custapp.squashfs  psm
    all.log.0         fwup              upload
    all.log.1         iot               user
    /CUSTAPP # cd iot
    /CUSTAPP/iot # ./iot_monitor 
    ----------------------------------------------------------------------------
    
           ** **        **          **  ****      **  **********  ********** ®
          **   **        **        **   ** **     **  **              **
         **     **        **      **    **  **    **  **              **
        **       **        **    **     **   **   **  *********       **
       **         **        **  **      **    **  **  **              **
      **           **        ****       **     ** **  **              **
     **  .........  **        **        **      ****  **********      **
        ...........
                                        Reach Further
    
     AVNET - AT&T Global Module IoT Monitor
     Version  1.03 // Sep 21 2018 @ 16:11:08 
     Hardware Supported: WNC M18Qx Cellular Data Module
    ----------------------------------------------------------------------------
    
    MON> 


A sieries of video's are avaialble at http://cloudconnectkits.org/product/global-lte-starter-kit unter the **Training** tab.





