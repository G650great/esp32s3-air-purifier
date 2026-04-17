#include "display.h"
#include "oled.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <string.h>

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
    
    // 1. 顶部状态栏 (小字体)
    oled_set_font_small();
    oled_draw_string(2, 2, wifi_connected ? "[W]" : "[ ]");
    oled_draw_string(92, 2, auto_mode ? "AUTO" : "MANUAL");
    oled_draw_line(0, 12, 127, 12);
    
    // 2. 温湿度 (大字体 8x16)
    oled_set_font_small();
    oled_draw_string(2, 18, "T:");
    oled_set_font_large();
    snprintf(buf, sizeof(buf), "%.0fC", temp);
    oled_draw_string(14, 18, buf);
    
    oled_set_font_small();
    oled_draw_string(64, 18, "H:");
    oled_set_font_large();
    snprintf(buf, sizeof(buf), "%.0f%%", humi);
    oled_draw_string(76, 18, buf);
    
    // 3. 分割线
    oled_draw_line(0, 38, 127, 38);
    
    // 4. 空气质量 (小字体)
    oled_set_font_small();
    oled_draw_string(2, 42, "AIR:");
    if (gas_raw < 0) {
        oled_draw_string(26, 42, "N/A");
    } else {
        oled_draw_string(26, 42, gas_raw == 0 ? "POOR" : "GOOD");
        oled_draw_string(90, 42, gas_raw ? "OK" : "!");
    }
    
    // 5. 风扇及进度条 (小字体)
    oled_draw_string(2, 54, "FAN:");
    snprintf(buf, sizeof(buf), "%d%%", fan_speed);
    oled_draw_string(30, 54, buf);
    oled_draw_progress_bar(55, 52, 70, 10, fan_speed);
    
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
