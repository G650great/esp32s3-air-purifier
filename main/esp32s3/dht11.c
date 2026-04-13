#include "dht11.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "DHT11";

#define DHT11_IO   GPIO_NUM_4

void dht11_init(void) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << DHT11_IO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&io_conf);
    ESP_LOGI(TAG, "DHT11 init done, IO%d", DHT11_IO);
}

int dht11_read(float *temp, float *humi) {
    uint8_t data[5] = {0};
    
    gpio_set_direction(DHT11_IO, GPIO_MODE_OUTPUT);
    gpio_set_level(DHT11_IO, 0);
    vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_level(DHT11_IO, 1);
    esp_rom_delay_us(30);
    
    gpio_set_direction(DHT11_IO, GPIO_MODE_INPUT);
    
    int timeout = 0;
    while (gpio_get_level(DHT11_IO) == 1) {
        if (++timeout > 1000) {
            ESP_LOGW(TAG, "timeout 1");
            return -1;
        }
        esp_rom_delay_us(1);
    }
    
    timeout = 0;
    while (gpio_get_level(DHT11_IO) == 0) {
        if (++timeout > 1000) {
            ESP_LOGW(TAG, "timeout 2");
            return -1;
        }
        esp_rom_delay_us(1);
    }
    
    timeout = 0;
    while (gpio_get_level(DHT11_IO) == 1) {
        if (++timeout > 1000) {
            ESP_LOGW(TAG, "timeout 3");
            return -1;
        }
        esp_rom_delay_us(1);
    }
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 8; j++) {
            timeout = 0;
            while (gpio_get_level(DHT11_IO) == 0) {
                if (++timeout > 2000) {
                    ESP_LOGW(TAG, "timeout 4");
                    return -1;
                }
                esp_rom_delay_us(1);
            }
            
            int high_time = 0;
            while (gpio_get_level(DHT11_IO) == 1) {
                if (++high_time > 2000) {
                    ESP_LOGW(TAG, "timeout 5");
                    return -1;
                }
                esp_rom_delay_us(1);
            }
            
            if (high_time > 50) {
                data[i] |= (1 << (7 - j));
            }
        }
    }
    
    uint8_t checksum = data[0] + data[1] + data[2] + data[3];
    if (data[4] != checksum) {
        ESP_LOGW(TAG, "checksum error");
        return -1;
    }
    
    *humi = (float)data[0];
    *temp = (float)data[2];
    return 0;
}
