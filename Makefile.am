jim_demo_CPPFLAGS = -I. -Wall \
        -g -O0 \
        -fno-inline \
        -Wl,--unresolved-symbols=ignore-in-shared-libs \
        -fno-short-enums \
        -pthread \
	-ljson-c \
        -lcurl \
        -lhw 


jim_demo_CFLAGS = -Wall \
        -g -O0 \
        -pthread \
        -fno-inline \
        -Wl,--unresolved-symbols=ignore-in-shared-libs \
        -fno-short-enums \
	-ljson-c \
        -lcurl \
        -lhw 

jim_demo_LDFLAGS = -ljson-c -lcurl -lpthread -lhw 

iot_monitor_SOURCES = main.cpp microrl.cpp MyBuffer.cpp \
                      commands.cpp mal.cpp maljson.cpp \
                      binio.c jsmn.c  hts221.c mytimer.c

bin_PROGRAMS = iot_monitor

