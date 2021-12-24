
#include "pico/stdlib.h"

int main() {
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    const uint SENSOR_PIN = 1;
    gpio_init(SENSOR_PIN);
    gpio_set_dir(SENSOR_PIN, GPIO_IN);

    while (true) {
        gpio_put(LED_PIN, gpio_get(SENSOR_PIN));
        sleep_ms(10);
    }
}