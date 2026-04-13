#include "oled.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "OLED_TEST";

void oled_test_task(void *arg) {
    ESP_LOGI(TAG, "OLED test starting...");
    
    oled_init();
    
    oled_clear();
    oled_draw_string(20, 20, "OLED TEST");
    oled_draw_string(10, 40, "HELLO WORLD!");
    oled_update();
    
    ESP_LOGI(TAG, "OLED test done");
    
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
