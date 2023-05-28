#include "sys_beeps.h"

#define BEEP_GPIO_PIN 32 // Example pin for the buzzer
void init_buzzer(){
    gpio_reset_pin(BEEP_GPIO_PIN);
    gpio_set_direction(BEEP_GPIO_PIN , GPIO_MODE_OUTPUT);
}

void shortBeep() {
    // Generate a short beep (50-100ms)
    gpio_set_level(BEEP_GPIO_PIN, 1); // Turn on the buzzer
    setRGBLED(1 , 1 , 0);
    vTaskDelay(50 / portTICK_PERIOD_MS); // Wait for 50ms
    gpio_set_level(BEEP_GPIO_PIN, 0); // Turn off the buzzer
    setRGBLED(1 , 1 , 1);
    vTaskDelay(50 / portTICK_PERIOD_MS); // Wait for 50ms
}

void longBeep() {
    // Generate a long beep (500-1000ms)
    gpio_set_level(BEEP_GPIO_PIN, 1); // Turn on the buzzer
    setRGBLED(0 , 1 , 1);
    vTaskDelay(500 / portTICK_PERIOD_MS); // Wait for 500ms
    gpio_set_level(BEEP_GPIO_PIN, 0); // Turn off the buzzer
    setRGBLED(1 , 1 , 1);
    vTaskDelay(500 / portTICK_PERIOD_MS); // Wait for 500ms
}

void continuousShortBeeps() {
    // Generate continuous short beeps (100-200ms each)
    for(int i = 0; i < 5; i++) { // Repeat 5 times
        gpio_set_level(BEEP_GPIO_PIN, 1); // Turn on the buzzer
        vTaskDelay(100 / portTICK_PERIOD_MS); // Wait for 100ms
        gpio_set_level(BEEP_GPIO_PIN, 0); // Turn off the buzzer
        vTaskDelay(100 / portTICK_PERIOD_MS); // Wait for 100ms
    }
    vTaskDelay(500 / portTICK_PERIOD_MS); // Wait for 500ms before repeating
}

void alternatingBeeps() {
    // Generate alternating short and long beeps (200-500ms each)
    for(int i = 0; i < 3; i++) { // Repeat 3 times
        gpio_set_level(BEEP_GPIO_PIN, 1); // Turn on the buzzer
        vTaskDelay(200 / portTICK_PERIOD_MS); // Wait for 200ms
        gpio_set_level(BEEP_GPIO_PIN, 0); // Turn off the buzzer
        vTaskDelay(200 / portTICK_PERIOD_MS); // Wait for 200ms
    }
    gpio_set_level(BEEP_GPIO_PIN, 1); // Turn on the buzzer
    vTaskDelay(500 / portTICK_PERIOD_MS); // Wait for 500ms
    gpio_set_level(BEEP_GPIO_PIN, 0); // Turn off the buzzer
    vTaskDelay(500 / portTICK_PERIOD_MS); // Wait for 500ms
}

void threeShortBeeps() {
    // Generate three short beeps (100-200ms each)
    for(int i = 0; i < 3; i++) { // Repeat 3 times
        gpio_set_level(BEEP_GPIO_PIN, 1); // Turn on the buzzer
        setRGBLED(0 , 1 , 1);
        vTaskDelay(100 / portTICK_PERIOD_MS); // Wait for 100ms
        gpio_set_level(BEEP_GPIO_PIN, 0); // Turn off the buzzer
        setRGBLED(1 , 1 , 1);
        vTaskDelay(100 / portTICK_PERIOD_MS); // Wait for 100ms
    }
    vTaskDelay(500 / portTICK_PERIOD_MS); // Wait for 500ms before repeating
}

void fourShortBeeps() {
    // Generate four short beeps (100-200ms each)
    for(int i = 0; i < 4; i++) {
        gpio_set_level(BEEP_GPIO_PIN, 1); // Turn on the buzzer
        vTaskDelay(100 / portTICK_PERIOD_MS); // Wait for 100ms
        gpio_set_level(BEEP_GPIO_PIN, 0); // Turn off the buzzer
        vTaskDelay(100 / portTICK_PERIOD_MS); // Wait for 100ms
    }
    vTaskDelay(500 / portTICK_PERIOD_MS); // Wait for 500ms before repeating
}

void fiveShortBeeps() {
    // Generate five short beeps (100-200ms each)
    for(int i = 0; i < 5; i++) {
        gpio_set_level(BEEP_GPIO_PIN, 1); // Turn on the buzzer
        vTaskDelay(100 / portTICK_PERIOD_MS); // Wait for 100ms
        gpio_set_level(BEEP_GPIO_PIN, 0); // Turn off the buzzer
        vTaskDelay(100 / portTICK_PERIOD_MS); // Wait for 100ms
    }
    vTaskDelay(500 / portTICK_PERIOD_MS); // Wait for 500ms before repeating
}
void twoShortBeeps() {
    // Generate five short beeps (100-200ms each)
    for(int i = 0; i < 2; i++) {
        gpio_set_level(BEEP_GPIO_PIN, 1); // Turn on the buzzer
        setRGBLED(1 , 0 , 1);
        vTaskDelay(100 / portTICK_PERIOD_MS); // Wait for 100ms
        gpio_set_level(BEEP_GPIO_PIN, 0); // Turn off the buzzer
        setRGBLED(1 , 1 , 1);
        vTaskDelay(100 / portTICK_PERIOD_MS); // Wait for 100ms
    }
    vTaskDelay(500 / portTICK_PERIOD_MS); // Wait for 500ms before repeating
}

