#include "ws2812.h"
#include "driver/rmt_tx.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <math.h>

static const char *TAG = "WS2812";

#define WS2812_GPIO         48
#define RMT_RESOLUTION_HZ   10000000

static rmt_channel_handle_t led_chan = NULL;
static rmt_encoder_handle_t led_encoder = NULL;
static int breathing_enabled = 0;
static uint8_t base_r = 0;
static uint8_t base_g = 255;
static uint8_t base_b = 0;
static float breathing_angle = 0.0f;

static void ws2812_send_color(uint8_t r, uint8_t g, uint8_t b);

void ws2812_init(void) {
    ESP_LOGI(TAG, "WS2812 init starting...");

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << WS2812_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    gpio_set_level(WS2812_GPIO, 0);
    vTaskDelay(pdMS_TO_TICKS(10));

    rmt_tx_channel_config_t tx_chan_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .gpio_num = WS2812_GPIO,
        .mem_block_symbols = 64,
        .resolution_hz = RMT_RESOLUTION_HZ,
        .trans_queue_depth = 4,
    };
    esp_err_t ret = rmt_new_tx_channel(&tx_chan_config, &led_chan);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create RMT channel: %s", esp_err_to_name(ret));
        return;
    }

    rmt_bytes_encoder_config_t bytes_encoder_config = {
        .bit0 = {
            .level0 = 1,
            .duration0 = 3,
            .level1 = 0,
            .duration1 = 9,
        },
        .bit1 = {
            .level0 = 1,
            .duration0 = 9,
            .level1 = 0,
            .duration1 = 3,
        },
    };
    ret = rmt_new_bytes_encoder(&bytes_encoder_config, &led_encoder);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create encoder: %s", esp_err_to_name(ret));
        return;
    }

    ret = rmt_enable(led_chan);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable RMT: %s", esp_err_to_name(ret));
        return;
    }

    ws2812_send_color(0, 0, 0);
    ESP_LOGI(TAG, "WS2812 init done, GPIO%d", WS2812_GPIO);
}

static void ws2812_send_color(uint8_t r, uint8_t g, uint8_t b) {
    if (led_chan == NULL || led_encoder == NULL) {
        return;
    }

    uint8_t grb[3] = {g, r, b};

    rmt_transmit_config_t tx_config = {
        .loop_count = 0,
    };

    esp_err_t ret = rmt_transmit(led_chan, led_encoder, grb, 3, &tx_config);
    if (ret == ESP_OK) {
        rmt_tx_wait_all_done(led_chan, pdMS_TO_TICKS(100));
    }
}

void ws2812_set_color(uint8_t r, uint8_t g, uint8_t b) {
    breathing_enabled = 0;
    base_r = r;
    base_g = g;
    base_b = b;
    ws2812_send_color(r, g, b);
}

void ws2812_off(void) {
    breathing_enabled = 0;
    ws2812_send_color(0, 0, 0);
    ESP_LOGI(TAG, "RGB OFF");
}

void ws2812_breathing_start(uint8_t r, uint8_t g, uint8_t b) {
    base_r = r;
    base_g = g;
    base_b = b;
    breathing_angle = 0.0f;
    breathing_enabled = 1;
    ESP_LOGI(TAG, "Breathing started: R=%d, G=%d, B=%d", r, g, b);
}

void ws2812_breathing_stop(void) {
    breathing_enabled = 0;
    ws2812_send_color(0, 0, 0);
    ESP_LOGI(TAG, "Breathing stopped");
}

int ws2812_is_breathing(void) {
    return breathing_enabled;
}

void ws2812_breathing_task(void) {
    if (!breathing_enabled) {
        return;
    }

    breathing_angle += 0.03f;
    if (breathing_angle >= 6.28318f) {
        breathing_angle = 0.0f;
    }

    float brightness = (sinf(breathing_angle) + 1.0f) / 2.0f;
    brightness = brightness * 0.01f + 0.005f;

    uint8_t r = (uint8_t)(base_r * brightness);
    uint8_t g = (uint8_t)(base_g * brightness);
    uint8_t b = (uint8_t)(base_b * brightness);

    ws2812_send_color(r, g, b);
}
