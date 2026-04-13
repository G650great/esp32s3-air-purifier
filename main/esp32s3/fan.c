#include "fan.h"
#include "driver/ledc.h"
#include "esp_log.h"

static const char *TAG = "FAN";

#define FAN_PWM_IO   GPIO_NUM_19

static uint8_t fan_speed = 0;

void fan_init(void) {
    ledc_timer_config_t timer_conf = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 25000,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&timer_conf);

    ledc_channel_config_t channel_conf = {
        .gpio_num = FAN_PWM_IO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0,
    };
    ledc_channel_config(&channel_conf);
    ESP_LOGI(TAG, "Fan PWM init done, IO%d", FAN_PWM_IO);
}

void fan_set_speed(uint8_t speed) {
    if (speed > 100) speed = 100;
    fan_speed = speed;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, speed * 255 / 100);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    ESP_LOGI(TAG, "Fan speed set to %d%%", speed);
}

uint8_t fan_get_speed(void) {
    return fan_speed;
}
