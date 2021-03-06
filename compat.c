#include "compat.h"
#include <stdio.h>

extern char *buffer;

uint8_t pgm_read_byte_near(uint16_t addr) {
    // printf("reading address %d\n", addr);
    return *(buffer + addr);
}

uint16_t pgm_read_word_near(uint16_t addr) {
    return (pgm_read_byte_near(addr + 1) << 8) | pgm_read_byte_near(addr);
    // return (*(buffer + addr + 1) << 8) | *(buffer + addr);
}