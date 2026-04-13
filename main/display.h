#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <stdint.h>

void display_init(void);
void display_splash_screen(void);

void display_main_dashboard(float temp, float humi, uint8_t fan_speed, int auto_mode, int gas_raw, int wifi_connected);

void display_temp_screen(float temp);
void display_humi_screen(float humi);
void display_fan_screen(uint8_t fan_speed);
void display_air_screen(float gas_ppm, int gas_raw, int gas_mv);
void display_status_screen(float temp, float humi, uint8_t fan_speed, int gas_raw, float gas_ppm);

#endif
