#rem Prepare the environment.
#rem Clean objects and includes.


CC="arm-oe-linux-gnueabi-gcc  -march=armv7ve -marm -mfpu=neon-vfpv4  -mfloat-abi=softfp -mcpu=cortex-a7 --sysroot=/usr/local/oecore-x86_64/sysroots/cortexa7-neon-vfpv4-oe-linux-gnueabi"
CPP="arm-oe-linux-gnueabi-gcc -E  -march=armv7ve -marm -mfpu=neon-vfpv4  -mfloat-abi=softfp -mcpu=cortex-a7 --sysroot=/usr/local/oecore-x86_64/sysroots/cortexa7-neon-vfpv4-oe-linux-gnueabi"

CXX="arm-oe-linux-gnueabi-g++ -march=armv7ve -marm -mfpu=neon-vfpv4 -mfloat-abi=softfp -mcpu=cortex-a7 --sysroot=/usr/local/oecore-x86_64/sysroots/cortexa7-neon-vfpv4-oe-linux-gnueabi -O2 -fexpensive-optimizations -frename-registers -fomit-frame-pointer -Wl,-O1 -Wl,--hash-style=gnu -Wl,--as-needed"
CXXCPP="arm-oe-linux-gnueabi-g++  -march=armv7ve -marm -mfpu=neon-vfpv4  -mfloat-abi=softfp -mcpu=cortex-a7 --sysroot=/usr/local/oecore-x86_64/sysroots/cortexa7-neon-vfpv4-oe-linux-gnueabi -E"

CFLAGS="-static -O2 -fexpensive-optimizations -frename-registers -fomit-frame-pointer"
CXXFLAGS="-static -O2 -fexpensive-optimizations -frename-registers -fomit-frame-pointer"
CPPFLAGS= 

GCC="$CC $CFLAGS"

Debug="-O0 -g3 -Wall"
curentPath="$PWD"
ApiRoot="$curentPath"
PlatformRoot="$curentPath/platform"
ProtectedRoot="$curentPath/protected"
RangingSensorRoot="$curentPath/ranging_sensor_comms_includes"
gccInclude="-I$ApiRoot/core/inc -I$PlatformRoot/inc -I$ProtectedRoot/inc -I$RangingSensorRoot/Include -fmessage-length=0"
CoreRoot="$curentPath/core"

function cmpil {
echo ""
echo $GCC $Debug -c $gccInclude -o$curentPath/object/$2.o $1/$2.c
$GCC $Debug -c $gccInclude -o$curentPath/object/$2.o $1/$2.c
}

rm object/*
rm libVL53L1.a

# compile platform
echo compile platform
cmpil "$PlatformRoot/src" vl53l1_platform_log
cmpil "$PlatformRoot/src" vl53l1_platform_init
cmpil "$PlatformRoot/src" vl53l1_platform_ipp
cmpil "$PlatformRoot/src" vl53l1_platform

# compile api
echo compile api
cmpil "$ApiRoot/core/src" vl53l1_api
cmpil "$ApiRoot/core/src" vl53l1_api_core
cmpil "$ApiRoot/core/src" vl53l1_api_preset_modes
cmpil "$ApiRoot/core/src" vl53l1_api_strings
cmpil "$ApiRoot/core/src" vl53l1_core
cmpil "$ApiRoot/core/src" vl53l1_hist_char
cmpil $ApiRoot/core/src vl53l1_register_funcs

# compile protected
echo compile protected
cmpil "$ProtectedRoot/src" vl53l1_dmax
cmpil "$ProtectedRoot/src" vl53l1_hist_algos_gen2
cmpil "$ProtectedRoot/src" vl53l1_hist_algos_gen3
cmpil "$ProtectedRoot/src" vl53l1_hist_algos_gen4
cmpil "$ProtectedRoot/src" vl53l1_xtalk
cmpil "$ProtectedRoot/src" vl53l1_hist_funcs
cmpil "$ProtectedRoot/src" vl53l1_sigma_estimate
cmpil "$ProtectedRoot/src" vl53l1_hist_core
cmpil "$ProtectedRoot/src" vl53l1_xtalk

# compile core
echo compile core
cmpil "$CoreRoot/src" vl53l1_core_support
cmpil "$CoreRoot/src" vl53l1_nvm
cmpil "$CoreRoot/src" vl53l1_api
cmpil "$CoreRoot/src" vl53l1_api_preset_modes
cmpil "$CoreRoot/src" vl53l1_error_strings
cmpil "$CoreRoot/src" vl53l1_nvm_debug
cmpil "$CoreRoot/src" vl53l1_zone_presets
cmpil "$CoreRoot/src" vl53l1_api_calibration
cmpil "$CoreRoot/src" vl53l1_api_strings
cmpil "$CoreRoot/src" vl53l1_fpga_core
cmpil "$CoreRoot/src" vl53l1_register_funcs
cmpil "$CoreRoot/src" vl53l1_api_core
cmpil "$CoreRoot/src" vl53l1_core
cmpil "$CoreRoot/src" vl53l1_hist_char
cmpil "$CoreRoot/src" vl53l1_silicon_core
cmpil "$CoreRoot/src" vl53l1_api_debug
cmpil "$CoreRoot/src" vl53l1_wait


echo compile test app
cmpil "." vl53l1_Ranging_Example

echo ""
echo build library
ar -r libVL53L1.a  $curentPath/object/*

# echo $CXX $Debug -o$curentPath/vl53l1_Ranging_Example.exe $curentPath/object/*.o 
# $CXX -march=armv7ve -marm -mfpu=neon-vfpv4 -mfloat-abi=softfp -mcpu=cortex-a7 \
# --sysroot=/usr/local/oecore-x86_64/sysroots/cortexa7-neon-vfpv4-oe-linux-gnueabi \
# -O2 -fexpensive-optimizations -frename-registers -fomit-frame-pointer \
# -Wl,-O1 -Wl,--hash-style=gnu -Wl,--as-needed -g3 -Wall \
# -o/home/jflynn/AvNet/WNC18Qx/oldM18QxIotMonitor/vl53l0x/vl53l1_Ranging_Example.exe \
# $curentPath/object/* \
# /usr/local/oecore-x86_64/sysroots/cortexa7-neon-vfpv4-oe-linux-gnueabi/usr/lib/libjson-c.so  \
# /usr/local/oecore-x86_64/sysroots/cortexa7-neon-vfpv4-oe-linux-gnueabi/usr/lib/libcurl.so  \
# -L=/usr/lib/..//lib  \
# /usr/local/oecore-x86_64/sysroots/cortexa7-neon-vfpv4-oe-linux-gnueabi/usr/lib/libgnutls.so  \
# /usr/local/oecore-x86_64/sysroots/cortexa7-neon-vfpv4-oe-linux-gnueabi/usr/lib/libidn.so  \
# -lnettle -lhogweed -lz -lpthread -lrt \
# /usr/local/oecore-x86_64/sysroots/cortexa7-neon-vfpv4-oe-linux-gnueabi/usr/lib/libgmp.so  \
# /usr/local/oecore-x86_64/sysroots/cortexa7-neon-vfpv4-oe-linux-gnueabi/usr/lib/libhw.so 
# 

