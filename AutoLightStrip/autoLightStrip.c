
// #include <stdio.h>

#include "pico/stdlib.h"
#include "pico/sem.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/adc.h"

#include "autoLightStrip.pio.h"
#include "colours.h"

// Pins used:

//    WS2812     GP0    01       40
//    PIR        GP1    02       39
//               GND    03       38
//    BUTTON     GP2    04       37
//               GP3    05       36
//               GP4    06       35
//               GP5    07       34   GP28
//               GND    08       33   GND
//               GP6    09       32   GP27
//               GP7    10       31   GP26      Potentiometer
//               GP8    11       30   RUN
//               GP9    12       29   GP22
//               GND    13       28   GND
//               GP10   14       27   GP21
//               GP11   15       26   GP20
//               GP12   16       25   GP19
//               GP13   17       24   GP18
//               GND    18       23   GND
//               GP15   19       22   GP17
//               GP16   20       21   GP16

// PIN 1  - GPIO0       - ws2812 light strip
#define WS2812_GPIO_pin 0
#define WS2812_freq 800000
#define WS2812_LED_COUNT 100
// PIN 2  - GPIO1       - pir sensor
#define PIR_GPIO_pin 1
// PIN 3 - GPIO2        - button
#define BUTTON_GPIO_pin 2

// PIN 30 - ADC0/GPIO26 - potentiometer
#define POTENTIOMETER_ACD_num 0
#define POTENTIOMETER_pin (POTENTIOMETER_ACD_num + 26)
#define POTENTIOMETER_wobble 10000

// 0 - Block Colour
// 1 - Rainbow
#define ACTIVE_PATTERN 1

#define BLOCK_COLOUR 0xA0C08000 // White
// #define BLOCK_COLOUR 0x08000000 // Green

// TODO use the second core to work out the pattern to be displayed
// -- Is this even worth it? Maybe if I have some complicated pattern that takes 2 'frames' to create the pattern
//    But unlikely to be done
//    What would the first core do in the wait time though?
//    Maybe if each core does half of the light calculations?
//    Seems like by that point you should just optimise a bit, or have a lower 'framerate'
// -- Might want something completely different running on the other core, at which point you would
//    want this to run on a single one?

// TODO Button to change the pattern/style00
// TODO Display information on the LCD screen
//     - Use second PIO to run the txuart for it
//     - See .\lcd_uart\lcd_uart.c for basic example code
//     - Display current style & brightness (2 rows :) )

// Properties of phasing
// (the animation of the lights turning on and off)
#define PHASING_DISTANCES 5 // MUST BE < 0xFF
#define PHASING_OFFSET 3
#define PHASING_PHASE_MAX 512L
// #define PHASING_PHASE_MIN 0 // fixed
#define PHASING_PHASE_STEP 1

#define MAX_BRIGHTNESS 0xFF

int dma_chan;
// Used to ensure the reset delay is completed for WS2812 LED strip,
// And slows down the patterns
static semaphore_t reset_delay_complete_sem;

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)(r) << 16) |
           ((uint32_t)(g) << 24) |
           ((uint32_t)(b) << 8);
}

// TODO add more patterns
void block_colour_pattern(uint32_t *pattern_array, uint len, uint t)
{
    for (int i = 0; i < len; ++i)
    {
        pattern_array[i] = BLOCK_COLOUR;
    }
}

static inline uint32_t colour_wheel(uint8_t pos)
{
    return pos < 85 ? urgb_u32(pos * 3, 255 - pos * 3, 0) : pos < 170 ? urgb_u32(255 - (pos - 85) * 3, 0, (pos - 85) * 3)
                                                                      : urgb_u32(0, (pos - 170) * 3, 255 - (pos - 170) * 3);
}
void rainbow_pattern(uint32_t *pattern_array, uint len, uint t)
{
    for (int i = 0; i < len; ++i)
    {
        // Don't need to worry if values>255, as colour_wheel only takes uint8, so it gets cropped
        // Mess about with multipliers here
        pattern_array[i] = colour_wheel((t + 5 * i) >> 2);
        // pattern_array[i] = colour_wheel((t + 10 * i) * 3);
    }
}
void Shartur(uint32_t *pattern_array, uint len, uint t)
{
    for (int i = 0; i < len; ++i)
    {
        pattern_array[i] = (t + i * i) & 0xFFFFFF;
    }
}

typedef void (*pattern)(uint32_t *light_array, uint len, uint t);
const struct patternType
{
    pattern pat;
    const char *name;
    uint64_t active_delay_us;
    uint64_t passive_delay_us;
} pattern_table[] = {
    {block_colour_pattern, "Solid Colour", 1000000, 1000000},
    {rainbow_pattern, "Moving Rainbow!", 1000000, 1000000},
    {Shartur, "Wtf", 1000000, 1000000}};
const int pattern_count = 3;

// Scales the see-saw wave
// x_M and t_M are the max values of x, t
// See https://www.desmos.com/calculator/an3ayahnek for graph of what happens in these two functions
static inline uint32_t phasing_f(uint32_t x, uint32_t t, uint32_t x_M, uint32_t t_M)
{
    if (t <= t_M / 2)
    {
        return ((uint32_t)255 * 2 * t * x) / (x_M * t_M);
    } // t >= t_M / 2
    return 255 - ((uint32_t)255 * 2 * (t_M - t) * (x_M - x)) / (x_M * t_M);
}

// Scales a see-saw wave such that it looks cool.
// Effect isn't that noticible and it kinda looks like it all fades in at once
void add_phasing(uint32_t *phase_wave, uint32_t *pattern_array, uint32_t length, uint32_t phasing_status, uint16_t brightness)
{
    // const float conversion_factor = MAX_BRIGHTNESS / (1 << 12);
    // ADC has some "wobble" in the reading causing the lights to flicker slightly
    // Not the best....
    // uint16_t adc_value = adc_read();//1<<9;
    for (int i = 0; i < WS2812_LED_COUNT; ++i)
    {
        if (phasing_status == 0)
        {
            pattern_array[i] = 0;
            continue;
        }
        uint32_t r = (pattern_array[i] >> 16) & 0xFF;
        uint32_t g = (pattern_array[i] >> 24) & 0xFF;
        uint32_t b = (pattern_array[i] >> 8) & 0xFF;
        // if (phasing_status < PHASING_PHASE_MAX) {
        uint32_t x = phasing_f(phase_wave[i], phasing_status, PHASING_DISTANCES, PHASING_PHASE_MAX);
        // Scale by phasing value
        r = (r * x) >> 8;
        g = (g * x) >> 8;
        b = (b * x) >> 8;
        // }

        // Scale to brightness
        // ACD is up to (1<<12) and brightness is up to (1<<8)
        r = (r * brightness * MAX_BRIGHTNESS) >> 20;
        g = (g * brightness * MAX_BRIGHTNESS) >> 20;
        b = (b * brightness * MAX_BRIGHTNESS) >> 20;

        // r = (r > MAX_BRIGHTNESS) ? MAX_BRIGHTNESS : r;
        // g = (g > MAX_BRIGHTNESS) ? MAX_BRIGHTNESS : g;
        // b = (b > MAX_BRIGHTNESS) ? MAX_BRIGHTNESS : b;

        // assert(r <= MAX_BRIGHTNESS);
        // assert(g <= MAX_BRIGHTNESS);
        // assert(b <= MAX_BRIGHTNESS);

        pattern_array[i] = urgb_u32(r, g, b);
    }
}

// Time has passed, release the sem so more data can be sent to the lights
int64_t release_reset_delay_sem(alarm_id_t id, void *user_data)
{
    sem_release(&reset_delay_complete_sem);
    return 0;
}

// Called when the set of data has finished transferring
// Need to wait for the reset time of the WS2812 LEDs before we send the next set of colours
// should be able to work with as little as 50us ? (https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf)
// Don't need to go that fast though, so use 500 for now.
void new_dma_handler()
{
    dma_hw->ints0 = 1u << dma_chan;
    add_alarm_in_us(4000, release_reset_delay_sem, NULL, true);
}

// Finds the difference between two integers
// is equivalent to abs( a-b )
uint32_t difference(int32_t a, int32_t b)
{
    return (a > b) ? a - b : b - a;
}

// Main program
int main()
{
    // stdio_init_all();

    // The onboard LED is used for testing
    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);
    gpio_put(25, 0);

// Set up the pin for the PIR sensor. No more setup is needed for the PIR sensor, it just works
#ifdef PIR_GPIO_pin
    gpio_init(PIR_GPIO_pin);
    gpio_set_dir(PIR_GPIO_pin, GPIO_IN);
    gpio_pull_down(PIR_GPIO_pin);
#endif

    adc_init();
    adc_gpio_init(POTENTIOMETER_pin);
    adc_select_input(POTENTIOMETER_ACD_num);

    gpio_init(BUTTON_GPIO_pin);
    gpio_set_dir(BUTTON_GPIO_pin, GPIO_IN);
    gpio_pull_up(BUTTON_GPIO_pin);

    bool button_pressed = false;
    int active_pattern = 0;

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
                          &pio0_hw->txf[0], // write address
                          NULL,             // Don't provide a read adress yet
                          WS2812_LED_COUNT, // Write to each LED, then stop and interrupt
                          false);           // Don't start yet

    // Tell DMA to raise irq when it finishes a block
    dma_channel_set_irq0_enabled(dma_chan, true);

    // Tell the processor to run dma_handler() when DMA_IRQ_0 is asserted
    irq_set_exclusive_handler(DMA_IRQ_0, new_dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    // Call this once to start things off and then interrupts will start it up again
    // Want to change it so things happen in the loop below
    // dma_handler();

    // static uint32_t light_array[WS2812_LED_COUNT];
    static uint32_t pattern_array[WS2812_LED_COUNT];

    static uint32_t phasing_wave[WS2812_LED_COUNT];
    for (signed int i = 0; i < WS2812_LED_COUNT; ++i)
    {
        // ABS( MOD( x-x_0, 2x_M) - x_M ) gives a see-saw wave with required properties
        // Used in the phasing functions
        phasing_wave[i] = difference((i + (PHASING_DISTANCES - PHASING_OFFSET)) % (2 * PHASING_DISTANCES), PHASING_DISTANCES);
    }

#ifdef PIR_GPIO_pin
    uint phasing_status = gpio_get(PIR_GPIO_pin) ? PHASING_PHASE_MAX : 0;
#else
    uint phasing_status = PHASING_PHASE_MAX;
#endif

    uint t = 0;
    uint16_t potentiometer_value = adc_read();

    sem_init(&reset_delay_complete_sem, 1, 1);

    while (true)
    {

        // Check for button presses
        bool button_state = !gpio_get(BUTTON_GPIO_pin);
        if (button_pressed != button_state)
        {
            button_pressed = button_state;
            if (button_pressed)
            {
                active_pattern = (active_pattern + 1) % pattern_count;
            }
        }
        gpio_put(25, active_pattern != 0);

        pattern_table[active_pattern].pat(pattern_array, WS2812_LED_COUNT, t);

#ifdef PIR_GPIO_pin
        if (gpio_get(PIR_GPIO_pin))
        {
            if (phasing_status < PHASING_PHASE_MAX)
                phasing_status += PHASING_PHASE_STEP;
        }
        else
        {
            if (phasing_status > 0)
                phasing_status -= PHASING_PHASE_STEP;
        }
#endif

        uint16_t potentiometer_new = adc_read();
        if (potentiometer_new < potentiometer_value - POTENTIOMETER_wobble ||
            potentiometer_new > potentiometer_value + POTENTIOMETER_wobble)
        {
            potentiometer_value = potentiometer_new;
        }

        // add_phasing(phasing_wave, pattern_array, WS2812_LED_COUNT, phasing_status, 1 << 9);
        add_phasing(phasing_wave, pattern_array, WS2812_LED_COUNT, phasing_status, potentiometer_value);

        // gpio_put(25, (t%2)!=0);

        // Acquire a permit to confirm that there has been enough time since the last showing of lights
        sem_acquire_blocking(&reset_delay_complete_sem);
        // Set the read address of the DMA, and trigger it to start
        dma_channel_set_read_addr(dma_chan, &pattern_array, true);

        // Uncomment to flash the pico's led when on
        // gpio_put(25, ((t / 512) % 3) <= gpio_get(PIR_GPIO_pin));

        ++t;
    }
}