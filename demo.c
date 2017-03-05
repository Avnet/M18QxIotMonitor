#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <nettle/nettle-stdint.h>
#include <hwlib/hwlib.h>
#include <time.h>

#include "iot_monitor.h"
#include "binio.h"
#include "mytimer.h"
#include "m2x.h"
#include "http.h"

typedef struct led_val_t {
    float temp;
    float humid;
    float myAccelZ;
    float myAccelY;
    } LED_VAL;

#define RBG_LED_RED    GPIO_PIN_92
#define RBG_LED_GREEN  GPIO_PIN_101
#define RBG_LED_BLUE   GPIO_PIN_102

#define RED_LED        1  //0001 RED
#define GREEN_LED      2  //0010 GREEN
#define YELLOW_LED     3  //0011 RED + GREEN
#define BLUE_LED       4  //0100
#define MAGENTA_LED    5  //0101 BLUE + RED
#define CYAN_LED       6  //0110 GREEN+ BLUE 
#define WHITE_LED      7  //0111 GREEN+BLUE+RED

#define HGREEN_LED      {80.0, 80.0, 0.6, 0.6} 
#define HMAGENTA_LED    {80.0, 80.0, 0.7, 0.4} 
#define HBLUE_LED       {80.0, 80.0, 0.3, 0.8} 
#define HTURQUOISE_LED  {80.0, 80.0, 0.2, 0.2} 

#define LRED_LED        {1.0, 1.0, 0.6, 0.6} 
#define LWHITE_LED      {1.0, 1.0, 0.7, 0.4} 
#define LGREEN_LED      {1.0, 1.0, 0.3, 0.8} 
#define LBLUE_LED       {1.0, 1.0, 0.2, 0.2} 

#define FLOW_BASE_URL    "https://runm-east.att.io/cfb0a90848c28/eb2514c29597/c94b14f417ae42a/in/flow"
#define FLOW_INPUT_NAME  "climate"
#define FLOW_DEVICE_NAME "vstarterkit001"
#define FLOW_SERVER      "run-east.att.io"

static gpio_handle_t user_key=0, red_led=0, green_led=0, blue_led=0;
static volatile gpio_level_t button_press=0, last_val=0;
struct timespec key_press, key_release, keypress_time;


LED_VAL led_demo[] = {
    HGREEN_LED,
    HMAGENTA_LED,
    HBLUE_LED,
    HTURQUOISE_LED,
    LWHITE_LED,
    LRED_LED,
    LBLUE_LED,
    LGREEN_LED,
};

void set_color( char *color )
{
    int val=0;

    if( !strcmp(color, "BLUE") )
        val=BLUE_LED;
    else if( !strcmp(color, "GREEN") )
        val=GREEN_LED;
    else if( !strcmp(color, "BLUE") )
        val=BLUE_LED;
    else if( !strcmp(color, "MAGENTA") )
        val=MAGENTA_LED;
    else if( !strcmp(color, "TURQUOISE") )
        val=CYAN_LED;
    else if( !strcmp(color, "RED") )
        val=RED_LED;
    else if( !strcmp(color, "WHITE") )
        val=WHITE_LED;
    else
        val=0;
    printf("-DEMO: Set LED %s\n",color);
    gpio_write( red_led, (val&RED_LED)?GPIO_LEVEL_HIGH:GPIO_LEVEL_LOW );
    gpio_write( green_led, (val&GREEN_LED)?GPIO_LEVEL_HIGH:GPIO_LEVEL_LOW );
    gpio_write( blue_led, (val&BLUE_LED)?GPIO_LEVEL_HIGH:GPIO_LEVEL_LOW );

}


static int gpio_irq_callback(gpio_pin_t pin_name, gpio_irq_trig_t direction)
{
    gpio_level_t the_val=0;
    
	if (pin_name != GPIO_PIN_98)
		return 0;

        if( !button_press ) {
            button_press = 1;
            clock_gettime(CLOCK_MONOTONIC, &key_press);
            printf("-DEMO: KEY PRESS detected\n");
            }
        else {
            button_press = 0;
            clock_gettime(CLOCK_MONOTONIC, &key_release);
            if ((key_release.tv_nsec-key_press.tv_nsec)<0) {
		keypress_time.tv_sec = key_release.tv_sec-key_press.tv_sec-1;
	        } 
            else {
		keypress_time.tv_sec = key_release.tv_sec-key_press.tv_sec;
	        }
            printf("-DEMO: KEY RELEASE detected, pressed for %ld secs\n",(keypress_time.tv_sec));
            }

	return 0;
}


int command_demo_mode(int argc, const char * const * argv )
{
    int start_data_service(void);
    char cmd[256], resp[256];
    char color[10];
    int  done=0, k=0;

    printf("-Demo: Starting Demo Mode.\n");
    printf("The LED will be read while establishing a connection to FLOW\n");
    printf("It will turn GREEN once connected.  After that pressing the\n");
    printf("USER button will cause the program to send 2 messages to M2X\n");
    printf("and 1 message to FLOW.  If you hold the button down for >3 \n");
    printf("seconds, it will exit demo mode and re-enter the monitor.\n\nconnecting...\n");

    binario_io_close();

    gpio_init( GPIO_PIN_92,  &red_led );
    gpio_init( GPIO_PIN_101, &green_led );
    gpio_init( GPIO_PIN_102, &blue_led );

    gpio_dir(red_led,   GPIO_DIR_OUTPUT);
    gpio_dir(green_led, GPIO_DIR_OUTPUT);
    gpio_dir(blue_led,  GPIO_DIR_OUTPUT);

    gpio_init( GPIO_PIN_98,  &user_key );  //SW3
    gpio_dir(user_key, GPIO_DIR_INPUT);
    gpio_irq_request(user_key, GPIO_IRQ_TRIG_BOTH, gpio_irq_callback);
    button_press = 0;

    start_data_service();
    printf("-Demo: Set LED RED\n");
    // while we are waiting for a data connection, make the LED RED...
    gpio_write( red_led, GPIO_LEVEL_HIGH);

    while( headless || !done ) {
        memset(cmd, 0x00, sizeof(cmd));
        sprintf(cmd,"&temp=%4.2f&humidity=%4.2f&accelX=0.0&accelY=%3.1f&accelZ=%3.1f", 
                     led_demo[k].temp, led_demo[k].humid, led_demo[k].myAccelY, led_demo[k].myAccelZ);
        flow_get ( FLOW_BASE_URL, FLOW_INPUT_NAME, FLOW_DEVICE_NAME, FLOW_SERVER, cmd, resp, sizeof(resp));
        sscanf(resp, "{\"status\":\"accepted\",\"LED\":\"%s", color);
        printf("-Demo: flow said: %s\n",resp);
        color[strlen(color)-2] = 0x00;
        set_color("OFF");
        sleep(1);
        set_color(color);

        while( !button_press ); /* wait for a button press */
        printf("-DEMO: HTS221 data to M2X\n");
        do_hts2m2x();
        printf("\n-DEMO: A2D data to M2X\n");
        do_adc2m2x();
        k++;
        if( k > (sizeof(led_demo)/sizeof(LED_VAL)-1) ) 
            k = 0;
        while( button_press ); /* wait for the user to release the button */
        if( keypress_time.tv_sec > 3 )
            done = 1;
        }

    gpio_deinit( &red_led);
    gpio_deinit( &green_led);
    gpio_deinit( &blue_led);
    gpio_deinit( &user_key);

    printf("Restarting the Monitor...\n");
    binary_io_init();
}


