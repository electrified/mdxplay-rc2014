#ifndef _compat_H
#define _compat_H
#include <stdint.h>
// void delayMicroseconds(uint16_t);
uint32_t micros();
uint8_t random(uint8_t);
uint16_t pgm_read_word_near(uint16_t addr);
uint8_t pgm_read_byte_near(uint16_t addr);
#endif