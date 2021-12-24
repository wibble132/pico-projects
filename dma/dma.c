
#include "pico/stdlib.h"
#include "hardware/dma.h"

#define LED_BASE 14
#define LED_COUNT 4
#define LED_MASK ~(~0 << LED_COUNT) << LED_BASE


int main() {

    gpio_init_mask(LED_MASK);
    gpio_set_dir_out_masked(LED_MASK);

    while (true) {
        for (int i = 0; i < LED_COUNT; ++i) {
            gpio_put(i + LED_BASE, 1);
            sleep_ms(250);
            gpio_put(i + LED_BASE, 0); 
        }
    }
}