#ifndef _WS2812_H_
#define _WS2812_H_

#include <stdint.h>

void ws2812_init(void);
void ws2812_set_color(uint8_t r, uint8_t g, uint8_t b);
void ws2812_off(void);

void ws2812_breathing_start(uint8_t r, uint8_t g, uint8_t b);
void ws2812_breathing_stop(void);
int ws2812_is_breathing(void);
void ws2812_breathing_task(void);

#endif
