#include <inttypes.h>


#define RED_LED_PIN GPIO_NUM_33
#define GREEN_LED_PIN GPIO_NUM_25
#define BLUE_LED_PIN GPIO_NUM_26


void rgbConfig(void);  // rgb led configuration
void setRGBLED(uint8_t , uint8_t , uint8_t);  // setting individual leds