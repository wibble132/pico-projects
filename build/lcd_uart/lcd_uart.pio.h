// -------------------------------------------------- //
// This file is autogenerated by pioasm; do not edit! //
// -------------------------------------------------- //

#pragma once

#if !PICO_NO_HARDWARE
#include "hardware/pio.h"
#endif

// ------ //
// N_UART //
// ------ //

#define N_UART_wrap_target 0
#define N_UART_wrap 5

static const uint16_t N_UART_program_instructions[] = {
            //     .wrap_target
    0x80a0, //  0: pull   block                      
    0xfe27, //  1: set    x, 7            side 1 [6] 
    0x6601, //  2: out    pins, 1                [6] 
    0x0042, //  3: jmp    x--, 2                     
    0xb742, //  4: nop                    side 0 [7] 
    0xa642, //  5: nop                           [6] 
            //     .wrap
};

#if !PICO_NO_HARDWARE
static const struct pio_program N_UART_program = {
    .instructions = N_UART_program_instructions,
    .length = 6,
    .origin = -1,
};

static inline pio_sm_config N_UART_program_get_default_config(uint offset) {
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset + N_UART_wrap_target, offset + N_UART_wrap);
    sm_config_set_sideset(&c, 2, true, false);
    return c;
}

#include "hardware/clocks.h"
#include <stdio.h>
static inline void N_UART_program_init(PIO pio, uint sm, uint offset, uint pin, float freq) {
    pio_gpio_init(pio, pin);
    pio_sm_set_consecutive_pindirs(pio, sm, pin, 1, true);
    pio_sm_config c = N_UART_program_get_default_config(offset);
    sm_config_set_sideset_pins(&c, pin);
    sm_config_set_out_shift(&c, true, false, 8);
    sm_config_set_out_pins(&c, pin, 1);
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);
    float div = clock_get_hz(clk_sys) / (freq * 8);
    // printf("divisions: %f\n", div);
    sm_config_set_clkdiv(&c, div);
    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);
}

#endif

