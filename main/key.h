#ifndef _KEY_H_
#define _KEY_H_

#include <stdint.h>

void key_init(void);
void key_set_callback(void (*callback)(uint8_t event, uint8_t key_id));
void key_scan_task(void *arg);
uint8_t key_get_power_state(void);
void key_set_power_state(uint8_t state);

#define KEY_ID_10   0
#define KEY_ID_11   1

#define KEY_EVENT_SHORT_PRESS   0
#define KEY_EVENT_LONG_PRESS    1

#endif
