#include "key.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

static const char *TAG = "KEY";

#define KEY_IO_10   GPIO_NUM_10
#define KEY_IO_11   GPIO_NUM_11

static void (*key_callback)(uint8_t event, uint8_t key_id) = NULL;
static uint8_t power_on = 1;

void key_init(void) {
    gpio_config_t io_conf_10 = {
        .pin_bit_mask = (1ULL << KEY_IO_10),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf_10);

    gpio_config_t io_conf_11 = {
        .pin_bit_mask = (1ULL << KEY_IO_11),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf_11);
    
    ESP_LOGI(TAG, "KEY init done, IO10 and IO11");
}

void key_set_callback(void (*callback)(uint8_t event, uint8_t key_id)) {
    key_callback = callback;
}

uint8_t key_get_power_state(void) {
    return power_on;
}

void key_set_power_state(uint8_t state) {
    power_on = state ? 1 : 0;
    ESP_LOGI(TAG, "Power state set to: %d", power_on);
}

void key_scan_task(void *arg) {
    static uint8_t last_key_state_10 = 1;
    static uint8_t last_key_state_11 = 1;
    static uint32_t press_start_10 = 0;
    static uint32_t press_start_11 = 0;

    while (1) {
        uint8_t key_state_10 = gpio_get_level(KEY_IO_10);
        uint8_t key_state_11 = gpio_get_level(KEY_IO_11);

        if (key_state_10 == 0 && last_key_state_10 == 1) {
            press_start_10 = esp_timer_get_time() / 1000;
        }

        if (key_state_10 == 1 && last_key_state_10 == 0) {
            uint32_t press_duration = (esp_timer_get_time() / 1000) - press_start_10;
            
            if (key_callback && power_on) {
                if (press_duration > 1000) {
                    key_callback(KEY_EVENT_LONG_PRESS, KEY_ID_10);
                } else if (press_duration > 50) {
                    key_callback(KEY_EVENT_SHORT_PRESS, KEY_ID_10);
                }
            }
        }

        if (key_state_11 == 0 && last_key_state_11 == 1) {
            press_start_11 = esp_timer_get_time() / 1000;
        }

        if (key_state_11 == 1 && last_key_state_11 == 0) {
            uint32_t press_duration = (esp_timer_get_time() / 1000) - press_start_11;
            
            if (press_duration > 1000) {
                if (key_callback && power_on) {
                    key_callback(KEY_EVENT_LONG_PRESS, KEY_ID_11);
                }
            } else if (press_duration > 50) {
                power_on = !power_on;
                if (power_on) {
                    ESP_LOGI(TAG, "Power ON");
                } else {
                    ESP_LOGI(TAG, "Power OFF");
                }
                if (key_callback) {
                    key_callback(KEY_EVENT_SHORT_PRESS, KEY_ID_11);
                }
            }
        }

        last_key_state_10 = key_state_10;
        last_key_state_11 = key_state_11;
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
