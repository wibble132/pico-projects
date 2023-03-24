
#include "pico/stdlib.h"
#include "colours.h"

inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t) (r) << 16) |
           ((uint32_t) (g) << 24) |
           ((uint32_t) (b) << 8);
}


uint32_t hue_urgb(uint32_t p, uint32_t q, uint8_t t) {
    if (t < 85) return p + (((q - p) * t) >> 8);
    if (t < 170) return q;
    return p + (((q - p) * (255 - t)) >> 8);
}

uint32_t uhsl_u32(uint8_t h, uint8_t s, uint8_t l) {

    uint8_t r, g, b;
    if (s == 0) {
        r = g = b = l;
    } else {
        uint32_t q = l < 128 ? (l * (256 + s)) >> 8 : l + s - ((l * s) >> 8);
        uint32_t p = 2 * l - q;
        r = hue_urgb(p, q, h + 85);
        g = hue_urgb(p, q, h);
        b = hue_urgb(p, q, h - 85);
    }
    return urgb_u32(r, g, b);
}

void block_colour_pattern(uint32_t *light_array, uint len, uint t) {
    for (int i = 0; i < len; i++) {
        light_array[i] = BLOCK_COLOUR;
    }
}

void rainbow_pattern(uint32_t *light_array, uint len, uint t) {
    for (int i = 0; i < len; i++) {
        light_array[i] = uhsl_u32((t + i / 8) >> 5, 255, 128);
    }
}

void Shartur(uint32_t *light_array, uint len, uint t) {
    for (int i = 0; i < len; ++i) {
        light_array[i] = (t + i * i) & 0xFFFFFF;
    }
}

// WIP
void colour_fade_pattern(uint32_t *light_array, uint len, uint t) {

    uint8_t h1 = COLOUR_FADE_HUE_1;
    uint8_t h2 = COLOUR_FADE_HUE_2;

    for (int i = 0; i < len; ++i) {

        // Interpolate between h1 and h2 using i as the index and t as time
        uint32_t x = (uint32_t) i * t;
        // 120 * sin(x) + 120
        uint32_t sx = 120 * x - (x * x * x) / 6;

        uint8_t h = h1 + ((h2 - h1) * i) / len;

        uint8_t hue = (t / 1000 + i / 250) % 255;
        light_array[i] = uhsl_u32(hue, 255, 128);
    }
}

struct colour_pattern pattern_table[] = {
        {block_colour_pattern, "Solid Colour",    1000000, 1000000},
        {rainbow_pattern,      "Moving Rainbow!", 1000000, 1000000},
        {Shartur,              "Wtf",             1000000, 1000000},
//        {colour_fade_pattern,  "Colour Fade",     1000000, 1000000}
};
int pattern_count = 3;