

#include "pico/stdlib.h"

#ifndef COLOURS_H
#define COLOURS_H


#define BLOCK_COLOUR 0xA0C08000 // White
// #define BLOCK_COLOUR 0x08000000 // Green

#define COLOUR_FADE_HUE_1 0x00
#define COLOUR_FADE_HUE_2 0x80

uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b);

uint32_t uhsl_u32(uint8_t h, uint8_t s, uint8_t l);

typedef void (*pattern)(uint32_t *light_array, uint len, uint t);

void block_colour_pattern(uint32_t *light_array, uint len, uint t);
void rainbow_pattern(uint32_t *light_array, uint len, uint t);
void Shartur(uint32_t *light_array, uint len, uint t);
//void colour_fade_pattern(uint32_t *light_array, uint len, uint t);

extern struct colour_pattern {
    pattern pat;
    const char *name;
    uint64_t active_delay_us;
    uint64_t passive_delay_us;
} pattern_table[3];
extern int pattern_count;



#endif