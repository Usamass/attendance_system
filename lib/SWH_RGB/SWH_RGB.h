#include <inttypes.h>


#define RED_LED_PIN GPIO_NUM_5
#define GREEN_LED_PIN GPIO_NUM_17
#define BLUE_LED_PIN GPIO_NUM_16


void rgbConfig(void);  // rgb led configuration
void setRGBLED(uint8_t , uint8_t , uint8_t);  // setting individual leds