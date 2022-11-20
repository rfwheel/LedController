#ifndef WS2811_BITBANG_H
#define WS2811_BITBANG_H

#include <stdint.h>


void ws2811_reset(void);
void ws2811_write_rgb(uint8_t r, uint8_t g, uint8_t b);


#endif
