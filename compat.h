#ifndef _compat_H
#define _compat_H
#include <stdint.h>
uint16_t pgm_read_word_near(uint16_t addr) __z88dk_fastcall;
uint8_t pgm_read_byte_near(uint16_t addr) __z88dk_fastcall;
#endif