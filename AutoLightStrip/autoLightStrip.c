
// #include <stdio.h>

#include "pico/stdlib.h"
#include "pico/sem.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/irq.h"

#include "autoLightStrip.pio.h"


// Pins used:
// PIN 1  - GPIO0       - ws2812 light strip
#define WS2812_GPIO_pin 0
#define WS2812_freq 800000
#define WS2812_LED_COUNT 10
// PIN 2  - GPIO1       - pio sensor
#define PIO_GPIO_pin 1
// PIN 30 - ADC0/GPIO25 - potentiometer   ---- Not to be implemented at first, brightness contro (or lights style?)

// When the pio sensor detects someone, turn on the light strip

// TODO use the second core to work out the pattern to be displayed


int dma_chan;
static semaphore_t reset_delay_complete_sem;

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return
            ((uint32_t) (r) << 16) |
            ((uint32_t) (g) << 24) |
            ((uint32_t) (b) <<  8);
}


#define BLOCK_COLOUR 0x0000ff00
void block_colour_pattern(uint32_t *pattern_array, uint len, uint t) {
    for (int i = 0; i < len; ++i) {
        pattern_array[i] = BLOCK_COLOUR;
    }
}

typedef void (*pattern)(uint32_t *light_array, uint len, uint t);


int64_t startRead(alarm_id_t id, void *user_data) {
    dma_channel_set_read_addr(dma_chan, user_data, true);
    return 0;
}

void dma_handler() {
    static uint32_t light_array[WS2812_LED_COUNT][WS2812_LED_COUNT];
    static int lightCount = 0;
    // Clear the interrupt request
    dma_hw->ints0 = 1u << dma_chan;

    static bool first_run = true;
    static bool light_on = false;
    if (first_run) {
        first_run = false;
        for (int i = 0; i < WS2812_LED_COUNT; ++i) {
        // first i lights are red, rest are off
            for (int j = 0; j < WS2812_LED_COUNT; ++j) {
                light_array[i][j] = urgb_u32(0, 0x10 * (j<=i), 0);
            }
        }
    }


    // Set the read address of the DMA, and trigger it to start
    add_alarm_in_ms(1000, startRead, &light_array[lightCount], true);

    lightCount = (lightCount + 1) % WS2812_LED_COUNT;
}

int64_t release_reset_delay_sem(alarm_id_t id, void *user_data) {
    sem_release(&reset_delay_complete_sem);
    return 0;
}

// Called when the set of data has finished transferring
// Need to wait for the reset time of the WS2812 LEDs before we send the next set of colours
// should be able to work with as little as 50us ? (https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf)
// Don't need to go that fast though, so use 500 for now.
void new_dma_handler() { 
    dma_hw->ints0 = 1u << dma_chan;
    add_alarm_in_us(500, release_reset_delay_sem, NULL, true); 
}

int main() {
    // stdio_init_all();

    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);

    PIO pio = pio0;
    uint sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_GPIO_pin, WS2812_freq);

    dma_chan = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    channel_config_set_read_increment(&c, true);
    channel_config_set_dreq(&c, pio_get_dreq(pio, sm, true));

    dma_channel_configure(dma_chan, 
                        &c, 
                        &pio0_hw->txf[0],   // write address
                        NULL,               // Don't provide a read adress yet
                        WS2812_LED_COUNT,   // Write to each LED, then stop and interrupt
                        false);             // Don't start yet

    // Tell DMA to raise irq when it finishes a block
    dma_channel_set_irq0_enabled(dma_chan, true);

    // Tell the processor to run dma_handler() when DMA_IRQ_0 is asserted
    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);



    // Call this once to start things off
    dma_handler();


    // Do nothing forever here, everything else is handled with interrupts
    while (true)
        tight_loop_contents();

}