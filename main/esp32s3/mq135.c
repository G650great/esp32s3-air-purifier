#include "mq135.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "MQ135";

#define MQ135_DO_IO   GPIO_NUM_1

void mq135_init(void) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << MQ135_DO_IO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&io_conf);
    
    ESP_LOGI(TAG, "MQ135 init done, DO=GPIO%d", MQ135_DO_IO);
}

int mq135_read(void) {
    return gpio_get_level(MQ135_DO_IO);
}

int mq135_read_raw(void) {
    return gpio_get_level(MQ135_DO_IO) ? 4095 : 0;
}

int mq135_read_mv(int *out_mv) {
    if (out_mv == NULL) return -1;
    *out_mv = gpio_get_level(MQ135_DO_IO) ? 3300 : 0;
    return 0;
}

float mq135_read_ppm(void) {
    return -1.0f;
}

int mq135_read_data(int *out_raw, int *out_mv, float *out_ppm) {
    if (out_raw == NULL || out_mv == NULL || out_ppm == NULL) return -1;
    
    int level = gpio_get_level(MQ135_DO_IO);
    *out_raw = level ? 4095 : 0;
    *out_mv = level ? 3300 : 0;
    *out_ppm = -1.0f;
    
    return 0;
}
