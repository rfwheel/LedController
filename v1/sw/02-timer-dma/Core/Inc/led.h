#ifndef LED_H
#define LED_H


#include <stdint.h>


#define NUM_LEDS 40


void led_init(void);
void led_set_color(int pos, uint8_t red, uint8_t green, uint8_t blue);
void led_update_strip(void);


#endif /* LED_H */
