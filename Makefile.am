
ACLOCAL_AMFLAGS = -I m4

jim_demo_CPPFLAGS = -I. -Wall -static \
        -g -O0 \
        -std=c++14 \
        -std=gnu++11 \
        -fno-inline \
        -Wl,--unresolved-symbols=ignore-in-shared-libs \
        -fno-short-enums \
        -pthread \
	-ljson-c \
        -lcurl \
        -lhw 

jim_demo_CFLAGS = -I. -Wall -static \
        -g -O0 \
        -pthread \
        -fno-inline \
        -Wl,--unresolved-symbols=ignore-in-shared-libs \
        -fno-short-enums \
	-ljson-c \
        -lcurl \
        -lhw  

AM_LDFLAGS = -static -ljson-c -lcurl -lpthread -lhw 

iot_monitor_SOURCES = main.cpp microrl.cpp HTU21D.cpp  \
                      commands.cpp mal.cpp maljson.cpp qsapp.cpp \
                      MAX31855.cpp facttest.cpp MS5637.cpp \
                      binio.c jsmn.c  HTS221.cpp mytimer.c \
                      http.c lis2dw12.c m2x.c demo.c wwan.c \
                      emissions.cpp 

bin_PROGRAMS = iot_monitor 


