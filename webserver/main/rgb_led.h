

#ifndef MAIN_RGB_LED_H_
#define MAIN_RGB_LED_H_

// RGB LED GPIOs
#define RGB_LED_RED_GPIO		5
#define RGB_LED_GREEN_GPIO		18
#define RGB_LED_BLUE_GPIO		19
#define RGB_LED_RED_GPIO_SERVER		33
#define RGB_LED_GREEN_GPIO_SERVER		25
#define RGB_LED_BLUE_GPIO_SERVER		26

// RGB LED color mix channels
#define RGB_LED_CHANNEL_NUM		3

// RGB LED configuration
typedef struct
{
	int channel;
	int gpio;
	int mode;
	int timer_index;
} ledc_info_t;

void rgb_led_wifi_app_started(void);


void rgb_led_http_server_started(void);


void rgb_led_wifi_connected(void);

void updateRGB(int R, int G, int B);

#endif /* MAIN_RGB_LED_H_ */
