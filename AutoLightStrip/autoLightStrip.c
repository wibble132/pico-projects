
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
#define WS2812_LED_COUNT 60
// PIN 2  - GPIO1       - pir sensor
#define PIR_GPIO_pin 1
// PIN 30 - ADC0/GPIO25 - potentiometer   ---- Not to be implemented at first, brightness contro (or lights style?)

// When the pio sensor detects someone, turn on the light strip

// TODO use the second core to work out the pattern to be displayed


// Properties of phasing 
// (the animation of the lights turning on and off)
#define PHASING_DISTANCES 10         // MUST BE < 0xFF
#define PHASING_OFFSET 3
#define PHASING_PHASE_MAX 32
// #define PHASING_PHASE_MIN 0 // fixed
#define PHASING_PHASE_STEP 1

int dma_chan;
static semaphore_t reset_delay_complete_sem;

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return
            ((uint32_t) (r) << 16) |
            ((uint32_t) (g) << 24) |
            ((uint32_t) (b) <<  8);
}

#define BLOCK_COLOUR 0x00002000
void block_colour_pattern(uint32_t *pattern_array, uint len, uint t) {
    for (int i = 0; i < len; ++i) {
        pattern_array[i] = BLOCK_COLOUR;
    }
}

typedef void (*pattern)(uint32_t *light_array, uint len, uint t);

static inline uint32_t phasing_f(uint32_t x, uint32_t t, uint32_t x_M, uint32_t t_M) {
    if ( t <= t_M / 2) {
        return ( (uint32_t)255 * 2 * t * x ) / ( x_M * t_M );
    } // t >= t_M / 2 
    return 255 - ( (uint32_t)255 * 2 * (t_M-t) * (x_M-x) ) / ( x_M * t_M );
}

void add_phasing(uint32_t *phase_wave, uint32_t *pattern_array, uint32_t length, uint32_t phasing_status) {
    //printf("%d: ", phasing_status);
    for (int i = 0; i < WS2812_LED_COUNT; ++i) {
        uint32_t x = phasing_f(phase_wave[i], phasing_status, PHASING_DISTANCES, PHASING_PHASE_MAX);
        uint32_t r = (pattern_array[i] >> 16) & 0xFF;
        uint32_t g = (pattern_array[i] >> 24) & 0xFF;
        uint32_t b = (pattern_array[i] >>  8) & 0xFF;
        pattern_array[i] = urgb_u32( (x*r)/255, (x*g)/255, (x*b)/255 );
        // if (phasing_status == 6) {
        //     printf("x,t: %x,%x, f(x,t): %x, rgb: %x,%x,%x\n", phase_wave[i], phasing_status, x, r, g, b);
        // }
        // printf("%2x ", (b*x)/255);
    }
    //printf("\n");
}




// int64_t startRead(alarm_id_t id, void *user_data) {
//     dma_channel_set_read_addr(dma_chan, user_data, true);
//     return 0;
// }

// void dma_handler() {
//     static uint32_t light_array[WS2812_LED_COUNT][WS2812_LED_COUNT];
//     static int lightCount = 0;
//     // Clear the interrupt request
//     dma_hw->ints0 = 1u << dma_chan;
// 
//     static bool first_run = true;
//     static bool light_on = false;
//     if (first_run) {
//         first_run = false;
//         for (int i = 0; i < WS2812_LED_COUNT; ++i) {
//         // first i lights are red, rest are off
//             for (int j = 0; j < WS2812_LED_COUNT; ++j) {
//                 light_array[i][j] = urgb_u32(0, 0x10 * (j<=i), 0);
//             }
//         }
//     }
// 
// 
//     // Set the read address of the DMA, and trigger it to start
//     add_alarm_in_ms(1000, startRead, &light_array[lightCount], true);
// 
//     lightCount = (lightCount + 1) % WS2812_LED_COUNT;
// }

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
    add_alarm_in_ms(25, release_reset_delay_sem, NULL, true); 
}


// Finds the difference between two integers
// is equivalent to abs( a-b )
uint32_t difference( int32_t a, int32_t b) {
    return (a > b) ? a-b : b-a;
}

// Main program
int main() {
    stdio_init_all();

    // The onboard LED is used for testing
    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);
    gpio_put(25, 0);

    //
    gpio_init(PIR_GPIO_pin);
    gpio_set_dir(25, PIR_GPIO_pin);
    gpio_pull_down(PIR_GPIO_pin);

    // PIO used for WS2812 LED strip
    PIO pio = pio0;
    uint sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_GPIO_pin, WS2812_freq);

    // DMA used to send the light patterns to the LED strip
    // TODO: Move this to some function to clear up the main a bit, dma_init()
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
    irq_set_exclusive_handler(DMA_IRQ_0, new_dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);


    // Call this once to start things off and then interrupts will start it up again
    // Want to change it so things happen in the loop below
    //dma_handler();



    //static uint32_t light_array[WS2812_LED_COUNT];
    static uint32_t pattern_array[WS2812_LED_COUNT];
    
    static uint32_t phasing_wave[WS2812_LED_COUNT];
    for (signed int i = 0; i < WS2812_LED_COUNT; ++i) {
        // ABS( MOD( x-x_0, 2x_M) - x_M ) gives a see-saw wave with required properties
        phasing_wave[i] = difference( ( i + (PHASING_DISTANCES - PHASING_OFFSET) ) % (2 * PHASING_DISTANCES), PHASING_DISTANCES );
    }

    uint phasing_status = 0;

    uint t = 0;

    sem_init(&reset_delay_complete_sem, 1, 1);

    // Do nothing forever here, everything else is handled with interrupts
    while (true) {

        block_colour_pattern(pattern_array, WS2812_LED_COUNT, t);

        if (gpio_get(PIR_GPIO_pin)) {
            if (phasing_status < PHASING_PHASE_MAX)
                phasing_status += PHASING_PHASE_STEP;
        } else {
            if (phasing_status > 0)
                phasing_status -= PHASING_PHASE_STEP;
        }
        add_phasing(phasing_wave, pattern_array, WS2812_LED_COUNT, phasing_status);


        gpio_put(25, (t%2)!=0);

        // Acquire a permit to confirm that there has been enough time since the last showing of lights
        sem_acquire_blocking(&reset_delay_complete_sem);
        // Set the read address of the DMA, and trigger it to start
        dma_channel_set_read_addr(dma_chan, &pattern_array, true);

        ++t;
    }

}