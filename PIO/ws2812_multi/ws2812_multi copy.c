
#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/sem.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/irq.h"

#include "ws2812_multi.pio.h"

#define NUM_PIXELS 8
#define WS2812_PIN 2

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return 
            ((uint32_t) (r) << 8) |
            ((uint32_t) (g) << 16) |
            (uint32_t) (b);
}

void pattern_snakes(uint len, uint t, uint32_t *arr) {
    for (uint i = 0; i < len; ++i) {
        uint x = (i + (t >> 1)) % 65;
        if (x < 10)
            arr[i] = urgb_u32(100, 0, 0);
        else if (x >= 15 && x < 25)
            arr[i] = urgb_u32(0, 100, 0);
        else if (x >= 30 && x < 40)
            arr[i] = urgb_u32(0, 0, 100);
        else
            arr[i] = 0;
    }
}

typedef void (*pattern)(uint len, uint t, uint32_t *arr);
const struct {
    pattern pat;
    const char *name;
} pattern_table[] = {
        {pattern_snakes, "Snakes!"}
};

uint32_t pixel_array[32] = {0};

#define DMA_CHANNEL 0
#define DMA_CHANNEL_MASK (1u << DMA_CHANNEL)

static struct semaphore reset_delay_complete_sem;
alarm_id_t reset_delay_alarm_id;

int64_t reset_delay_complete(alarm_id_t id, void *user_data) {
    reset_delay_alarm_id = 0;
    sem_release(&reset_delay_complete_sem);

    return 0;
}

void dma_irq_handler() {
    // Handle something
    // Set the timer to wait 400us before unlocking
    printf("Hello from irqs\n");

    dma_hw->ints0 = DMA_CHANNEL_MASK; // cleart IRQ
    if (reset_delay_alarm_id) cancel_alarm(reset_delay_alarm_id);
    reset_delay_alarm_id = add_alarm_in_us(400, reset_delay_complete, NULL, true);

}

void dma_init(PIO pio, uint sm) {

    printf("INITIALISING DMA\n");
    dma_claim_mask(DMA_CHANNEL_MASK);

    dma_channel_config channel_config = dma_get_channel_config(DMA_CHANNEL);
    channel_config_set_dreq(&channel_config, pio_get_dreq(pio, sm, true));
    channel_config_set_write_increment(&channel_config, false);
    channel_config_set_read_increment(&channel_config, true);
    dma_channel_configure(
        DMA_CHANNEL,
        &channel_config,
        &pio->txf[sm],
        NULL,
        1,      // CHANGE ?? how much do I want?
        false
    );

    // Call IRQ 0 when the cahnnel finishes a block
    dma_channel_set_irq0_enabled(DMA_CHANNEL, true);
    // Set the processor to run the funciton
    irq_set_exclusive_handler(DMA_IRQ_0, dma_irq_handler);
    irq_set_enabled(DMA_IRQ_0, true);
    
}

int main() {

    /// ALL TERRIBLE
    /// TRY COPYING THE dma/channel_irq example instead
    /// It uses all interrupts
    //// So maybe look at them a bit more first.... (how tf do they work? (Hint: they don't))

    stdio_init_all();

    sleep_ms(4000);
    printf("HI\n");


    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_multi_program);

    ws2821_multi_program_init(pio, sm, offset, WS2812_PIN, 800000);

    sem_init(&reset_delay_complete_sem, 1, 1);
    

    int t =0;
    while (true) {
        printf("WTF\n");
        pattern_snakes(NUM_PIXELS, t, pixel_array);
        sem_acquire_blocking(&reset_delay_complete_sem);


        dma_channel_set_read_addr(DMA_CHANNEL, &pixel_array[0], true);

        ++t;

    }
}