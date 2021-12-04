#include "compat.h"

extern char *buffer;

uint32_t micros() {
    return 0;
}

uint8_t random(uint8_t x) {
    return 0;
}

uint8_t pgm_read_byte_near(uint16_t addr) {
    return buffer[addr];
}

uint16_t pgm_read_word_near(uint16_t addr) {
    return (buffer[addr + 1] << 8) | buffer[addr];
}