#include "ws2811_bitbang.h"
#include "gpio.h"


#define WS2811_GPIO (GPIOA)
#define WS2811_PIN (GPIO_PIN_10)

#define WS2811_HIGH()                         \
  do {                                        \
    WS2811_GPIO->BSRR = WS2811_PIN;           \
  } while (0)

#define WS2811_LOW()                          \
  do {                                        \
    WS2811_GPIO->BSRR = (WS2811_PIN << 16);   \
  } while (0)

#define WS2811_WAIT_62NS()                   \
  do {                                        \
    asm("nop");                               \
    asm("nop");                               \
    asm("nop");                               \
    asm("nop");                               \
  } while (0)

#define WS2811_WAIT_125NS()                   \
  do {                                        \
    WS2811_WAIT_62NS();                       \
    WS2811_WAIT_62NS();                       \
  } while (0)

#define WS2811_WAIT_500NS()                   \
  do {                                        \
    WS2811_WAIT_125NS();                      \
    WS2811_WAIT_125NS();                      \
    WS2811_WAIT_125NS();                      \
    WS2811_WAIT_125NS();                      \
  } while (0)

#define WS2811_WRITE_ZERO()                   \
  do {                                        \
    WS2811_HIGH();                            \
    WS2811_WAIT_125NS();                      \
    WS2811_LOW();                             \
    WS2811_WAIT_500NS();                      \
    WS2811_WAIT_125NS();                      \
  } while (0)

#define WS2811_WRITE_ONE()                    \
  do {                                        \
    WS2811_HIGH();                            \
    WS2811_WAIT_500NS();                      \
    WS2811_LOW();                             \
    WS2811_WAIT_125NS();                      \
    WS2811_WAIT_62NS();                       \
  } while (0)


void ws2811_reset(void) {
  int i;

  WS2811_LOW();
  for (i = 0; i < 1000; i++) {
    WS2811_WAIT_500NS();
  }
}

void ws2811_write_rgb(uint8_t r, uint8_t g, uint8_t b) {
  uint32_t color;
  uint32_t bit;

  color = (r << 16) | (b << 8) | g;
  bit = 0x800000;
  while (bit) {
    if (color & bit) {
      WS2811_WRITE_ONE();
    } else {
      WS2811_WRITE_ZERO();
    }
    bit >>= 1;
  }
}



