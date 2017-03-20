#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

void wwan_io(int onoff) {

    int fd;
    char buf[80]; 

    fd = open("/sys/class/leds/wwan/brightness", O_WRONLY);

    write(fd, onoff?"1":"0", 1);

    close(fd);
}
