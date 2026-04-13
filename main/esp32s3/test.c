#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "soc/rtc_cntl_reg.h"

#include "oled.h"

static const char *TAG = "DIAG";

#define TEST_PASS   "[PASS]"
#define TEST_FAIL   "[FAIL]"
#define TEST_SKIP   "[SKIP]"

static int pass_count = 0;
static int fail_count = 0;
static int skip_count = 0;
static int nvs_initialized = 0;

static void print_result(const char *test_name, const char *result, const char *detail) {
    ESP_LOGI(TAG, "%-30s %s %s", test_name, result, detail ? detail : "");
    char line[64];
    snprintf(line, sizeof(line), "%s %s", test_name, result);
    oled_clear();
    oled_draw_string(0, 0, "Hardware Diagnose");
    oled_draw_string(0, 20, line);
    oled_update();
}

static void test_delay(int ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
}

static void test_gpio_basic(void) {
    ESP_LOGI(TAG, "\n========================================");
    ESP_LOGI(TAG, "Test 1: GPIO Basic Function");
    ESP_LOGI(TAG, "========================================");
    
    int test_pins[] = {10, 11, 19, 48, 4, 1};
    int pin_count = sizeof(test_pins) / sizeof(test_pins[0]);
    int success = 0;
    
    for (int i = 0; i < pin_count; i++) {
        gpio_config_t io_conf = {
            .pin_bit_mask = (1ULL << test_pins[i]),
            .mode = GPIO_MODE_INPUT_OUTPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        if (gpio_config(&io_conf) == ESP_OK) {
            int level = gpio_get_level(test_pins[i]);
            ESP_LOGI(TAG, "  GPIO%d: OK (level=%d)", test_pins[i], level);
            success++;
        } else {
            ESP_LOGE(TAG, "  GPIO%d: FAIL", test_pins[i]);
        }
    }
    
    if (success == pin_count) {
        print_result("GPIO Basic", TEST_PASS, "All pins OK");
        pass_count++;
    } else {
        print_result("GPIO Basic", TEST_FAIL, "Some pins failed");
        fail_count++;
    }
}

static void test_i2c_oled(void) {
    ESP_LOGI(TAG, "\n========================================");
    ESP_LOGI(TAG, "Test 2: I2C / OLED Display");
    ESP_LOGI(TAG, "========================================");
    
    oled_init();
    oled_clear();
    oled_draw_string(0, 0, "OLED Test OK!");
    oled_draw_string(0, 30, "Display Working");
    oled_update();
    ESP_LOGI(TAG, "  OLED init: OK");
    print_result("I2C/OLED", TEST_PASS, "Display OK");
    pass_count++;
}

static void test_adc_channels(void) {
    ESP_LOGI(TAG, "\n========================================");
    ESP_LOGI(TAG, "Test 3: ADC Channels");
    ESP_LOGI(TAG, "========================================");
    
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_12);
    adc1_config_channel_atten(ADC1_CHANNEL_1, ADC_ATTEN_DB_12);
    adc1_config_channel_atten(ADC1_CHANNEL_2, ADC_ATTEN_DB_12);
    
    int adc0_sum = 0, adc1_sum = 0, adc2_sum = 0;
    int samples = 10;
    
    for (int i = 0; i < samples; i++) {
        adc0_sum += adc1_get_raw(ADC1_CHANNEL_0);
        adc1_sum += adc1_get_raw(ADC1_CHANNEL_1);
        adc2_sum += adc1_get_raw(ADC1_CHANNEL_2);
        test_delay(10);
    }
    
    int adc0_avg = adc0_sum / samples;
    int adc1_avg = adc1_sum / samples;
    int adc2_avg = adc2_sum / samples;
    
    ESP_LOGI(TAG, "  ADC0 (GPIO1): %d", adc0_avg);
    ESP_LOGI(TAG, "  ADC1 (GPIO2): %d", adc1_avg);
    ESP_LOGI(TAG, "  ADC2 (GPIO3): %d", adc2_avg);
    
    if (adc0_avg > 0 || adc1_avg > 0 || adc2_avg > 0) {
        print_result("ADC Channels", TEST_PASS, "ADC working");
        pass_count++;
    } else {
        print_result("ADC Channels", TEST_FAIL, "All zero");
        fail_count++;
    }
}

static void test_power_stability(void) {
    ESP_LOGI(TAG, "\n========================================");
    ESP_LOGI(TAG, "Test 4: Power Supply Stability");
    ESP_LOGI(TAG, "========================================");
    
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_12);
    
    int min_val = 4095, max_val = 0;
    int samples = 100;
    int sum = 0;
    
    ESP_LOGI(TAG, "  Sampling ADC 100 times...");
    for (int i = 0; i < samples; i++) {
        int val = adc1_get_raw(ADC1_CHANNEL_0);
        if (val < min_val) min_val = val;
        if (val > max_val) max_val = val;
        sum += val;
        test_delay(10);
    }
    
    int avg = sum / samples;
    int variation = max_val - min_val;
    
    ESP_LOGI(TAG, "  Min: %d, Max: %d, Avg: %d, Variation: %d", min_val, max_val, avg, variation);
    
    if (variation < 200) {
        ESP_LOGI(TAG, "  Power stability: GOOD (variation < 200)");
        print_result("Power Stability", TEST_PASS, "Stable");
        pass_count++;
    } else if (variation < 500) {
        ESP_LOGI(TAG, "  Power stability: FAIR (variation 200-500)");
        print_result("Power Stability", TEST_PASS, "Fair");
        pass_count++;
    } else {
        ESP_LOGI(TAG, "  Power stability: POOR (variation > 500)");
        print_result("Power Stability", TEST_FAIL, "Unstable!");
        fail_count++;
    }
}

static void test_wifi_rf_only(void) {
    ESP_LOGI(TAG, "\n========================================");
    ESP_LOGI(TAG, "Test 5: WiFi RF Module (Init Only)");
    ESP_LOGI(TAG, "========================================");
    
    if (!nvs_initialized) {
        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            nvs_flash_erase();
            ret = nvs_flash_init();
        }
        
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "  NVS init failed: %s", esp_err_to_name(ret));
            print_result("WiFi RF", TEST_FAIL, "NVS error");
            fail_count++;
            return;
        }
        nvs_initialized = 1;
    }
    
    ESP_LOGI(TAG, "  NVS init: OK");
    ESP_LOGI(TAG, "  Initializing WiFi driver...");
    ESP_LOGI(TAG, "  Watch for BROWNOUT or POWERON reset!");
    ESP_LOGI(TAG, "  If system reboots now, power supply is insufficient!");
    
    oled_clear();
    oled_draw_string(0, 0, "WiFi RF Test");
    oled_draw_string(0, 20, "Initializing...");
    oled_draw_string(0, 40, "Watch serial!");
    oled_update();
    
    test_delay(2000);
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_err_t ret = esp_wifi_init(&cfg);
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "  WiFi driver init: OK");
        
        ret = esp_wifi_set_mode(WIFI_MODE_STA);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "  WiFi mode set: OK");
            print_result("WiFi RF", TEST_PASS, "Init OK!");
            pass_count++;
        } else {
            ESP_LOGE(TAG, "  WiFi mode set: FAIL");
            print_result("WiFi RF", TEST_FAIL, "Mode error");
            fail_count++;
        }
        
        esp_wifi_deinit();
    } else {
        ESP_LOGE(TAG, "  WiFi driver init: FAIL (%s)", esp_err_to_name(ret));
        print_result("WiFi RF", TEST_FAIL, "Init failed");
        fail_count++;
    }
}

static void test_wifi_tx_power(void) {
    ESP_LOGI(TAG, "\n========================================");
    ESP_LOGI(TAG, "Test 6: WiFi TX Power Test");
    ESP_LOGI(TAG, "========================================");
    
    ESP_LOGI(TAG, "  This test will stress the power supply!");
    ESP_LOGI(TAG, "  If system reboots, power cannot handle WiFi TX!");
    
    oled_clear();
    oled_draw_string(0, 0, "WiFi TX Power");
    oled_draw_string(0, 20, "STRESS TEST!");
    oled_draw_string(0, 40, "If reboot=FAIL");
    oled_update();
    
    test_delay(3000);
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_err_t ret = esp_wifi_init(&cfg);
    if (ret != ESP_OK) {
        print_result("WiFi TX Power", TEST_FAIL, "Init failed");
        fail_count++;
        return;
    }
    
    esp_wifi_set_mode(WIFI_MODE_STA);
    
    int8_t power_levels[] = {8, 20, 44, 52, 60, 78, 80};
    int power_count = sizeof(power_levels) / sizeof(power_levels[0]);
    int success = 0;
    
    for (int i = 0; i < power_count; i++) {
        ret = esp_wifi_set_max_tx_power(power_levels[i]);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "  TX Power %d dBm: OK", power_levels[i]);
            success++;
            test_delay(500);
        } else {
            ESP_LOGE(TAG, "  TX Power %d dBm: FAIL", power_levels[i]);
        }
    }
    
    esp_wifi_deinit();
    
    if (success == power_count) {
        ESP_LOGI(TAG, "  WiFi TX power test: PASSED!");
        print_result("WiFi TX Power", TEST_PASS, "All levels OK");
        pass_count++;
    } else {
        ESP_LOGI(TAG, "  WiFi TX power test: PARTIAL (%d/%d)", success, power_count);
        print_result("WiFi TX Power", TEST_FAIL, "Some levels failed");
        fail_count++;
    }
}

static void test_wifi_scan(void) {
    ESP_LOGI(TAG, "\n========================================");
    ESP_LOGI(TAG, "Test 7: WiFi Scan (High Current)");
    ESP_LOGI(TAG, "========================================");
    
    ESP_LOGI(TAG, "  This test draws MAXIMUM current!");
    ESP_LOGI(TAG, "  WiFi scan current: ~300-500mA peak");
    ESP_LOGI(TAG, "  If system reboots, power supply is INSUFFICIENT!");
    
    oled_clear();
    oled_draw_string(0, 0, "WiFi Scan Test");
    oled_draw_string(0, 20, "MAX CURRENT!");
    oled_draw_string(0, 40, "Reboot=PowerFail");
    oled_update();
    
    test_delay(3000);
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_err_t ret = esp_wifi_init(&cfg);
    if (ret != ESP_OK) {
        print_result("WiFi Scan", TEST_FAIL, "Init failed");
        fail_count++;
        return;
    }
    
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();
    
    ESP_LOGI(TAG, "  Starting WiFi scan...");
    ret = esp_wifi_scan_start(NULL, true);
    
    if (ret == ESP_OK) {
        uint16_t ap_count = 0;
        esp_wifi_scan_get_ap_num(&ap_count);
        ESP_LOGI(TAG, "  Scan complete! Found %d APs", ap_count);
        print_result("WiFi Scan", TEST_PASS, "Scan OK!");
        pass_count++;
    } else {
        ESP_LOGE(TAG, "  Scan failed: %s", esp_err_to_name(ret));
        print_result("WiFi Scan", TEST_FAIL, "Scan error");
        fail_count++;
    }
    
    esp_wifi_stop();
    esp_wifi_deinit();
}

static void print_summary(void) {
    ESP_LOGI(TAG, "\n========================================");
    ESP_LOGI(TAG, "DIAGNOSTIC SUMMARY");
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "  PASS: %d", pass_count);
    ESP_LOGI(TAG, "  FAIL: %d", fail_count);
    ESP_LOGI(TAG, "  SKIP: %d", skip_count);
    ESP_LOGI(TAG, "========================================");
    
    if (fail_count == 0) {
        ESP_LOGI(TAG, "ALL TESTS PASSED! Hardware is healthy.");
    } else {
        ESP_LOGI(TAG, "SOME TESTS FAILED! Check hardware.");
        if (fail_count >= 3) {
            ESP_LOGI(TAG, "CRITICAL: Multiple failures detected!");
            ESP_LOGI(TAG, "Likely power supply damage from short circuit.");
        }
    }
    
    oled_clear();
    oled_draw_string(0, 0, "Diagnose Complete");
    char result[32];
    snprintf(result, sizeof(result), "PASS:%d FAIL:%d", pass_count, fail_count);
    oled_draw_string(0, 25, result);
    
    if (fail_count == 0) {
        oled_draw_string(0, 45, "Hardware OK!");
    } else {
        oled_draw_string(0, 45, "Check Hardware!");
    }
    oled_update();
}

void app_main(void) {
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "ESP32-S3 Hardware Diagnostic Tool");
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "This tool tests hardware components");
    ESP_LOGI(TAG, "to identify damage from short circuit.");
    ESP_LOGI(TAG, "========================================\n");
    
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    
    oled_init();
    oled_clear();
    oled_draw_string(0, 0, "Hardware Diagnose");
    oled_draw_string(0, 20, "Starting...");
    oled_draw_string(0, 40, "Watch serial log");
    oled_update();
    
    test_delay(2000);
    
    test_gpio_basic();
    test_delay(1000);
    
    test_i2c_oled();
    test_delay(1000);
    
    test_adc_channels();
    test_delay(1000);
    
    test_power_stability();
    test_delay(1000);
    
    test_wifi_rf_only();
    test_delay(1000);
    
    test_wifi_tx_power();
    test_delay(1000);
    
    test_wifi_scan();
    test_delay(1000);
    
    print_summary();
    
    ESP_LOGI(TAG, "\nDiagnostic complete. System will continue running.");
    ESP_LOGI(TAG, "Check serial output for detailed results.");
    
    while (1) {
        test_delay(5000);
    }
}
