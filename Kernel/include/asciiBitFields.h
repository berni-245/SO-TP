#ifndef BITFIELDS_H
#define BITFIELDS_H

#include <stdint.h>

#define ASCII_BF_WIDTH 11
#define ASCII_BF_HEIGHT 23
#define ASCII_BF_MIN ' '
#define ASCII_BF_MAX '~'

extern const uint8_t asciiBitFields[][ASCII_BF_WIDTH * ASCII_BF_HEIGHT];
extern const uint32_t ASCII_BF_COUNT;

#endif
