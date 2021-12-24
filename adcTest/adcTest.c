
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

const float MAXWAIT = 2000;

void sleepADCTime() {
    const float conversion_factor = MAXWAIT / (1<<12);
    uint16_t result = adc_read();

    sleep_ms(result * conversion_factor);
}

int main() {
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;


    stdio_init_all();
    sleep_ms(2000);
    printf("Initialising\n");

    // initialise the LED pin
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // initialise the adc pin
    adc_init();
    adc_gpio_init(26);

    adc_select_input(0);

    while (true) {


    // test1, prints the value to stdout
        const float conversion_factor = 3.3f / (1<<12);
        uint16_t result = adc_read();

        
        printf("Raw value: 0x%03x, voltage: %f V\n", result, result * conversion_factor);
        sleep_ms(1000);

    // test2, flashes the LED for a variable length of time
        gpio_put(LED_PIN, 1);
        sleepADCTime();
        gpio_put(LED_PIN, 0);
        sleepADCTime();

    }
}