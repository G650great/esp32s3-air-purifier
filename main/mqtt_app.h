#ifndef _MQTT_APP_H_
#define _MQTT_APP_H_

#include <stdint.h>

void mqtt_app_start(void);
void mqtt_publish_status(float temp, float humi, uint8_t fan_speed, int gas_raw, int gas_mv, float gas_ppm, int auto_mode, int power_state);
void mqtt_set_cmd_callback(void (*callback)(const char *data, int len));

#endif
