
#ifdef DEBUG
#include <stdio.h>
#endif

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/adc.h"

#include "ws2812.pio.h"

#ifdef PICO_DEFAULT_WS2812_PIN
#define WS2812_PIN PICO_DEFAULT_WS2812_PIN
#else
#define WS2812_PIN 0
#endif

static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return
            ((uint32_t) (r) << 8) |
            ((uint32_t) (g) << 16) |
            (uint32_t) (b);
}

int main() {

#ifdef DEBUG
    stdio_init_all();
#endif

    // Set up the pio for the ws2812 light
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);

    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000);

    // Set up the adc for the potentiometer
    adc_init();
    adc_gpio_init(26);

    adc_select_input(0);
    
    const float conversion_factor = 100. / (1 << 12);

    while (true) {
        uint16_t result = adc_read();
        float brightness = conversion_factor * result;
#ifdef DEBUG
        printf("brightness: %f\n", brightness);
#endif
        put_pixel(urgb_u32( (uint8_t) brightness,0,0));
        sleep_ms(100);
    }

}