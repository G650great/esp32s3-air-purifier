#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "soc/rtc_cntl_reg.h"

#include "oled.h"
#include "dht11.h"
#include "mq135.h"
#include "fan.h"
#include "key.h"
#include "wifi_app.h"
#include "mqtt_app.h"
#include "display.h"
// #include "ws2812.h"

static const char *TAG = "MAIN";

static float shared_temp = 25.0;
static float shared_humi = 60.0;
static uint8_t fan_speed = 0;
static int auto_mode = 0;
static int shared_gas_raw = -1;
static int shared_gas_mv = -1;
static float shared_gas_ppm = -1.0f;
static int wifi_connected = 0;

static void key_event_handler(uint8_t event, uint8_t key_id) {
    if (key_id == KEY_ID_10) {
        if (event == KEY_EVENT_SHORT_PRESS) {
            static const uint8_t speeds[] = {0, 25, 50, 75, 100};
            static int idx = 0;
            idx = (idx + 1) % 5;
            fan_speed = speeds[idx];
            auto_mode = 0;
            fan_set_speed(fan_speed);
            ESP_LOGI(TAG, "KEY10 Short press: fan_speed=%d", fan_speed);
        } else if (event == KEY_EVENT_LONG_PRESS) {
            auto_mode = !auto_mode;
            ESP_LOGI(TAG, "KEY10 Long press: auto_mode=%d", auto_mode);
        }
    } else if (key_id == KEY_ID_11) {
        if (event == KEY_EVENT_SHORT_PRESS) {
            uint8_t power_state = key_get_power_state();
            if (power_state) {
                ESP_LOGI(TAG, "KEY11: Power ON - resuming display and fan");
                oled_on();
                // ws2812_off();
                if (fan_speed > 0) {
                    fan_set_speed(fan_speed);
                }
            } else {
                ESP_LOGI(TAG, "KEY11: Power OFF - turning off display and fan");
                fan_set_speed(0);
                oled_off();
                // ws2812_off();
            }
        }
    }
}

static void mqtt_cmd_handler(const char *data, int len) {
    char buf[64];
    int n = len;
    if (n < 0) return;
    if (n >= (int)sizeof(buf)) n = (int)sizeof(buf) - 1;
    memcpy(buf, data, n);
    buf[n] = 0;

    if (strncmp(buf, "fan:", 4) == 0) {
        int speed = atoi(buf + 4);
        if (speed >= 0 && speed <= 100) {
            auto_mode = 0;
            fan_speed = speed;
            if (key_get_power_state()) {
                fan_set_speed(fan_speed);
            }
            ESP_LOGI(TAG, "Remote cmd: fan_speed=%d", fan_speed);
        }
    } else if (strncmp(buf, "mode:", 5) == 0) {
        int mode = atoi(buf + 5);
        if (mode == 0 || mode == 1) {
            auto_mode = mode;
            ESP_LOGI(TAG, "Remote cmd: auto_mode=%d", auto_mode);
        }
    } else if (strncmp(buf, "power:", 6) == 0) {
        int power = atoi(buf + 6);
        if (power == 0) {
            ESP_LOGI(TAG, "Remote cmd: Power OFF");
            fan_set_speed(0);
            oled_off();
            // ws2812_off();
            key_set_power_state(0);
        } else if (power == 1) {
            ESP_LOGI(TAG, "Remote cmd: Power ON");
            oled_on();
            // ws2812_off();
            if (fan_speed > 0) {
                fan_set_speed(fan_speed);
            }
            key_set_power_state(1);
        }
    }
}

static void sensor_task(void *arg) {
    float temp, humi;
    int gas_raw, gas_mv;
    float gas_ppm;
    static uint8_t last_auto_fan_speed = 0;

    while (1) {
        if (mq135_read_data(&gas_raw, &gas_mv, &gas_ppm) == 0) {
            shared_gas_raw = gas_raw;
            shared_gas_mv = gas_mv;
            shared_gas_ppm = gas_ppm;
            ESP_LOGI(TAG, "MQ135: DO=%d (%s)", gas_raw ? 1 : 0, gas_raw ? "GOOD" : "POOR");
        }
        
        for (int retry = 0; retry < 3; retry++) {
            if (dht11_read(&temp, &humi) == 0) {
                shared_temp = temp;
                shared_humi = humi;
                ESP_LOGI(TAG, "DHT11: T=%.1fC, H=%.1f%%", temp, humi);
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        
        uint8_t power_state = key_get_power_state();
        if (power_state) {
            if (auto_mode) {
                if (gas_raw == 0) {
                    fan_speed = 100;
                } else {
                    fan_speed = 25;
                }
                if (fan_speed != last_auto_fan_speed) {
                    fan_set_speed(fan_speed);
                    last_auto_fan_speed = fan_speed;
                    ESP_LOGI(TAG, "Auto mode: fan_speed=%d", fan_speed);
                }
            }
            display_main_dashboard(shared_temp, shared_humi, fan_speed, auto_mode, shared_gas_raw, wifi_connected);
        }
        
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

static void mqtt_publish_task(void *arg) {
    vTaskDelay(pdMS_TO_TICKS(5000));
    wifi_connected = 1;
    
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(5000));
        mqtt_publish_status(shared_temp, shared_humi, fan_speed, shared_gas_raw, shared_gas_mv, shared_gas_ppm, auto_mode, key_get_power_state());
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "Smart Air Purifier Starting...");

    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

    // ws2812_init();
    // ws2812_off();

    display_init();
    display_splash_screen();

    dht11_init();
    mq135_init();
    fan_init();
    key_init();
    key_set_callback(key_event_handler);

    xTaskCreate(key_scan_task, "key_scan", 2048, NULL, 10, NULL);
    xTaskCreate(sensor_task, "sensor", 4096, NULL, 5, NULL);
    xTaskCreate(mqtt_publish_task, "mqtt_pub", 4096, NULL, 5, NULL);

    wifi_init_sta();
    mqtt_app_start();
    mqtt_set_cmd_callback(mqtt_cmd_handler);

    ESP_LOGI(TAG, "System running");
}
