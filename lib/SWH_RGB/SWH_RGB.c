#include "SWH_RGB.h"
#include "driver/gpio.h"


void rgbConfig(void)
{
    // setting gpio mode of the rgb pins
    gpio_set_direction(RED_LED_PIN , GPIO_MODE_OUTPUT);
    gpio_set_direction(GREEN_LED_PIN , GPIO_MODE_OUTPUT);
    gpio_set_direction(BLUE_LED_PIN , GPIO_MODE_OUTPUT);

    gpio_set_level(RED_LED_PIN , 1);   
    gpio_set_level(GREEN_LED_PIN , 1); 
    gpio_set_level(BLUE_LED_PIN , 1); 


}
void setRGBLED(uint8_t red , uint8_t green , uint8_t blue)
{ 
    gpio_set_level(RED_LED_PIN , red);
    gpio_set_level(GREEN_LED_PIN , green);
    gpio_set_level(BLUE_LED_PIN , blue);


}
