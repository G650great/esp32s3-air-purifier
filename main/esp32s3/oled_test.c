#include "oled.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "OLED_TEST";

void oled_test_task(void *arg) {
    ESP_LOGI(TAG, "OLED test starting...");
    oled_init();
    oled_clear();
    
    oled_set_font_small();
    oled_draw_string(0, 0, "Small Font");
    
    oled_set_font_large();
    oled_draw_string(0, 20, "Large Font");
    
    oled_update();
    ESP_LOGI(TAG, "OLED test done");
    while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
}
