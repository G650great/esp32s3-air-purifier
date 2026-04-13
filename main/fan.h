#ifndef _FAN_H_
#define _FAN_H_

#include <stdint.h>

void fan_init(void);
void fan_set_speed(uint8_t speed);
uint8_t fan_get_speed(void);

#endif
