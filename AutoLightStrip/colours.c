
#include "pico/stdlib.h"


uint32_t hslToRgb(uint32_t hsl) {
    uint8_t h = hsl >> 16;
    uint8_t s = hsl >> 8;
    uint8_t l = hsl;

    uint8_t c = (255 - abs(2 * l - 255)) * s / 255;
    uint8_t x = c * (255 - abs((h / 60) % 2 - 1)) / 255;
    uint8_t m = l - c / 2;

    uint8_t r, g, b;
    if (h < 60) {
        r = c;
        g = x;
        b = 0;
    } else if (h < 120) {
        r = x;
        g = c;
        b = 0;
    } else if (h < 180) {
        r = 0;
        g = c;
        b = x;
    } else if (h < 240) {
        r = 0;
        g = x;
        b = c;
    } else if (h < 300) {
        r = x;
        g = 0;
        b = c;
    } else {
        r = c;
        g = 0;
        b = x;
    }

    return (r + m) << 16 | (g + m) << 8 | (b + m);
}

uint8_t hue2rgb(p, q, t)
{
    if (t < 0)
        t += 1;
    if (t > 1)
        t -= 1;
    if (t < 1 / 6)
        return p + (q - p) * 6 * t;
    if (t < 1 / 2)
        return q;
    if (t < 2 / 3)
        return p + (q - p) * (2 / 3 - t) * 6;
    return p;
}

uint32_t hslToRgb(uint8_t h, uint8_t s, uint8_t l)
{
    uint8_t r, g, b;

    if (s == 0)
    {
        r = g = b = l; // achromatic
    }
    else
    {
        uint8_t q = l <= 127 ? l * (255 + s) : l + s - l * s;
        uint8_t p = 2 * l - q;
        r = hue2rgb(p, q, h + 85);
        g = hue2rgb(p, q, h);
        b = hue2rgb(p, q, h - 85);
    }

    return (r) << 16 | (g) << 8 | (b);
}