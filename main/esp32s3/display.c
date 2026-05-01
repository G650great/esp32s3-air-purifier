#include "display.h"
#include "oled.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <string.h>

static void draw_page_indicator(uint8_t active) {
    const int y = 60;
    const int s = 3;
    const int gap = 6;
    const int x0 = 112;
    for (int i = 0; i < 2; i++) {
        int x = x0 + i * gap;
        if (i == (int)active) {
            oled_fill_rect(x, y, s, s);
        } else {
            oled_draw_rect(x, y, s, s);
        }
    }
}

void display_init(void) {
    oled_init();
}

void display_splash_screen(void) {
    oled_clear();
    
    oled_draw_rect(0, 0, 128, 64);
    oled_draw_rect(2, 2, 124, 60);
    
    oled_draw_string(28, 20, "SMART AIR");
    oled_draw_string(36, 36, "PURIFIER");
    
    oled_draw_string(44, 54, "v2.0.0");
    
    oled_update();
    vTaskDelay(pdMS_TO_TICKS(3000));
}

void display_main_dashboard(float temp, float humi, uint8_t fan_speed, int auto_mode, int gas_raw, int wifi_connected) {
    char buf[32];
    oled_clear();

    oled_draw_rect(0, 0, 128, 64);
    oled_set_font_small();
    oled_draw_string(2, 2, wifi_connected ? "[W]" : "[ ]");
    oled_draw_string(50, 2, "HOME");
    oled_draw_string(92, 2, auto_mode ? "AUTO" : "MAN");
    oled_draw_line(0, 12, 127, 12);

    oled_draw_line(63, 12, 63, 41);
    oled_draw_line(0, 42, 127, 42);

    oled_draw_string(6, 14, "TEMP");
    oled_set_font_large();
    snprintf(buf, sizeof(buf), "%.0fC", temp);
    oled_draw_string((64 - (int)strlen(buf) * 8) / 2, 22, buf);

    oled_set_font_small();
    oled_draw_string(70, 14, "HUM");
    oled_set_font_large();
    snprintf(buf, sizeof(buf), "%.0f%%", humi);
    oled_draw_string(64 + (64 - (int)strlen(buf) * 8) / 2, 22, buf);

    oled_set_font_small();
    oled_draw_string(6, 46, "AIR");
    if (gas_raw < 0) {
        oled_draw_string(30, 46, "N/A");
    } else {
        oled_draw_string(30, 46, gas_raw == 0 ? "POOR" : "GOOD");
        oled_draw_string(54, 46, gas_raw ? "OK" : "!");
    }

    oled_draw_string(70, 46, "FAN");
    snprintf(buf, sizeof(buf), "%d%%", fan_speed);
    oled_draw_string(94, 46, buf);
    oled_draw_progress_bar(70, 54, 52, 8, fan_speed);

    draw_page_indicator(0);
    oled_update();
}

void display_detail_screen(float temp, float humi, uint8_t fan_speed, int auto_mode, int gas_raw, int gas_mv, float gas_ppm, int wifi_connected) {
    char buf[32];
    oled_clear();

    oled_draw_rect(0, 0, 128, 64);
    oled_set_font_small();
    oled_draw_string(2, 2, wifi_connected ? "[W]" : "[ ]");
    oled_draw_string(44, 2, "DETAIL");
    oled_draw_string(92, 2, auto_mode ? "AUTO" : "MAN");
    oled_draw_line(0, 12, 127, 12);

    oled_draw_line(63, 12, 63, 41);
    oled_draw_line(0, 42, 127, 42);

    oled_draw_string(6, 14, "PPM");
    oled_set_font_large();
    if (gas_raw < 0 || gas_ppm < 0.0f) {
        strcpy(buf, "--");
    } else {
        snprintf(buf, sizeof(buf), "%.0f", gas_ppm);
    }
    oled_draw_string((64 - (int)strlen(buf) * 8) / 2, 22, buf);

    oled_set_font_small();
    oled_draw_string(70, 14, "FAN");
    oled_set_font_large();
    snprintf(buf, sizeof(buf), "%d%%", fan_speed);
    oled_draw_string(64 + (64 - (int)strlen(buf) * 8) / 2, 22, buf);

    oled_set_font_small();
    if (gas_raw < 0) {
        oled_draw_string(6, 46, "AIR:N/A");
    } else {
        oled_draw_string(6, 46, gas_raw == 0 ? "AIR:POOR" : "AIR:GOOD");
    }

    if (gas_raw < 0) {
        oled_draw_string(6, 54, "MV:--");
    } else {
        snprintf(buf, sizeof(buf), "MV:%d", gas_mv);
        oled_draw_string(6, 54, buf);
    }

    snprintf(buf, sizeof(buf), "T:%.1fC H:%.0f%%", temp, humi);
    oled_draw_string(60, 54, buf);

    draw_page_indicator(1);
    oled_update();
}

void display_temp_screen(float temp) {
    char buf[16];
    
    oled_clear();
    oled_draw_rect(0, 0, 128, 64);
    oled_draw_string(36, 8, "TEMPERATURE");
    oled_draw_line(4, 18, 124, 18);
    
    snprintf(buf, sizeof(buf), "%.1f C", temp);
    oled_draw_string(40, 36, buf);
    
    oled_draw_line(4, 48, 124, 48);
    
    if (temp < 20) {
        oled_draw_string(52, 58, "Cool");
    } else if (temp < 28) {
        oled_draw_string(36, 58, "Comfortable");
    } else {
        oled_draw_string(56, 58, "Hot!");
    }
    
    oled_update();
}

void display_humi_screen(float humi) {
    char buf[16];
    
    oled_clear();
    oled_draw_rect(0, 0, 128, 64);
    oled_draw_string(44, 8, "HUMIDITY");
    oled_draw_line(4, 18, 124, 18);
    
    snprintf(buf, sizeof(buf), "%.0f %%", humi);
    oled_draw_string(48, 36, buf);
    
    oled_draw_line(4, 48, 124, 48);
    
    if (humi < 40) {
        oled_draw_string(52, 58, "Dry");
    } else if (humi < 70) {
        oled_draw_string(36, 58, "Comfortable");
    } else {
        oled_draw_string(48, 58, "Humid");
    }
    
    oled_update();
}

void display_fan_screen(uint8_t fan_speed) {
    char buf[16];
    
    oled_clear();
    oled_draw_rect(0, 0, 128, 64);
    oled_draw_string(44, 8, "FAN SPEED");
    oled_draw_line(4, 18, 124, 18);
    
    snprintf(buf, sizeof(buf), "%d %%", fan_speed);
    oled_draw_string(52, 34, buf);
    
    oled_draw_progress_bar(14, 46, 100, 8, fan_speed);
    
    oled_draw_line(4, 56, 124, 56);
    
    if (fan_speed == 0) {
        oled_draw_string(56, 62, "OFF");
    } else if (fan_speed < 50) {
        oled_draw_string(56, 62, "Low");
    } else if (fan_speed < 80) {
        oled_draw_string(48, 62, "Medium");
    } else {
        oled_draw_string(56, 62, "High");
    }
    
    oled_update();
}

void display_air_screen(float gas_ppm, int gas_raw, int gas_mv) {
    char buf[24];
    
    oled_clear();
    oled_draw_rect(0, 0, 128, 64);
    oled_draw_string(34, 8, "MQ135 VALUE");
    oled_draw_line(4, 18, 124, 18);

    if (gas_raw < 0) {
        oled_draw_string(40, 36, "NO DATA");
    } else {
        snprintf(buf, sizeof(buf), "RAW:%d", gas_raw);
        oled_draw_string(28, 28, buf);
        snprintf(buf, sizeof(buf), "MV:%d", gas_mv);
        oled_draw_string(36, 40, buf);
        if (gas_ppm >= 0.0f) {
            snprintf(buf, sizeof(buf), "PPM:%.0f", gas_ppm);
            oled_draw_string(28, 52, buf);
        }
    }
    
    oled_update();
}

void display_status_screen(float temp, float humi, uint8_t fan_speed, int gas_raw, float gas_ppm) {
    char buf[16];
    
    oled_clear();
    oled_draw_rect(0, 0, 128, 64);
    oled_draw_string(36, 6, "SYSTEM INFO");
    oled_draw_line(0, 14, 127, 14);
    
    snprintf(buf, sizeof(buf), "T:%.1fC", temp);
    oled_draw_string(4, 24, buf);
    
    snprintf(buf, sizeof(buf), "H:%.0f%%", humi);
    oled_draw_string(72, 24, buf);
    
    snprintf(buf, sizeof(buf), "Fan:%d%%", fan_speed);
    oled_draw_string(4, 38, buf);
    
    if (gas_raw < 0) {
        oled_draw_string(72, 38, "G:--");
    } else {
        snprintf(buf, sizeof(buf), "G:%d", gas_raw);
        oled_draw_string(72, 38, buf);
    }

    oled_draw_line(0, 48, 127, 48);
    
    if (gas_ppm < 0.0f) {
        oled_draw_string(28, 58, "PPM:--");
    } else {
        snprintf(buf, sizeof(buf), "PPM:%.0f", gas_ppm);
        oled_draw_string(24, 58, buf);
    }
    
    oled_update();
}
