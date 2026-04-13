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
#include "esp_pm.h"

#include "oled.h"

static const char *TAG = "WIFI_TEST";

static int wifi_connected = 0;
static int retry_count = 0;

#define WIFI_SSID      "YOUR_WIFI_SSID"
#define WIFI_PASS      "YOUR_WIFI_PASS"

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "WiFi started, connecting...");
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_connected = 0;
        retry_count++;
        ESP_LOGI(TAG, "WiFi disconnected, retry %d", retry_count);
        if (retry_count < 5) {
            esp_wifi_connect();
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
        wifi_connected = 1;
    }
}

static void show_status(const char *msg) {
    oled_clear();
    oled_draw_string(0, 0, "WiFi Low Power Test");
    oled_draw_string(0, 20, msg);
    oled_update();
    ESP_LOGI(TAG, "%s", msg);
}

static void test_wifi_minimal(void) {
    ESP_LOGI(TAG, "\n========================================");
    ESP_LOGI(TAG, "Minimal WiFi Test - Lowest Power Mode");
    ESP_LOGI(TAG, "========================================");
    
    show_status("Init NVS...");
    
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) {
        show_status("NVS FAIL!");
        return;
    }
    
    show_status("Init Netif...");
    ret = esp_netif_init();
    if (ret != ESP_OK) {
        show_status("Netif FAIL!");
        return;
    }
    
    esp_netif_create_default_wifi_sta();
    
    show_status("Init WiFi...");
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    cfg.static_rx_buf_num = 2;
    cfg.dynamic_rx_buf_num = 4;
    cfg.tx_buf_type = 1;
    cfg.static_tx_buf_num = 1;
    cfg.dynamic_tx_buf_num = 4;
    cfg.cache_tx_buf_num = 1;
    cfg.ampdu_rx_enable = 0;
    cfg.ampdu_tx_enable = 0;
    
    ret = esp_wifi_init(&cfg);
    if (ret != ESP_OK) {
        show_status("WiFi Init FAIL!");
        ESP_LOGE(TAG, "WiFi init failed: %s", esp_err_to_name(ret));
        return;
    }
    
    show_status("Set Low Power...");
    
    esp_wifi_set_mode(WIFI_MODE_STA);
    
    wifi_config_t wifi_config = {0};
    strcpy((char*)wifi_config.sta.ssid, WIFI_SSID);
    strcpy((char*)wifi_config.sta.password, WIFI_PASS);
    wifi_config.sta.scan_method = WIFI_FAST_SCAN;
    wifi_config.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;
    wifi_config.sta.threshold.rssi = -127;
    wifi_config.sta.threshold.authmode = WIFI_AUTH_OPEN;
    
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    
    ret = esp_wifi_set_max_tx_power(8);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "TX Power set to 8dBm (minimum)");
    } else {
        ESP_LOGE(TAG, "Failed to set TX power: %s", esp_err_to_name(ret));
    }
    
    ret = esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Power save mode: MIN_MODEM");
    }
    
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip);
    
    show_status("Starting WiFi...");
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    ret = esp_wifi_start();
    if (ret != ESP_OK) {
        show_status("WiFi Start FAIL!");
        ESP_LOGE(TAG, "WiFi start failed: %s", esp_err_to_name(ret));
        return;
    }
    
    show_status("Connecting...");
    
    int timeout = 30;
    while (!wifi_connected && timeout > 0) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        char msg[32];
        snprintf(msg, sizeof(msg), "Wait %ds...", timeout);
        show_status(msg);
        timeout--;
    }
    
    if (wifi_connected) {
        show_status("WiFi CONNECTED!");
        ESP_LOGI(TAG, "\n========================================");
        ESP_LOGI(TAG, "SUCCESS! WiFi is working!");
        ESP_LOGI(TAG, "========================================");
        
        vTaskDelay(pdMS_TO_TICKS(5000));
        
        show_status("Testing TX...");
        vTaskDelay(pdMS_TO_TICKS(2000));
        
        ret = esp_wifi_set_max_tx_power(20);
        if (ret == ESP_OK) {
            show_status("TX 20dBm OK!");
            ESP_LOGI(TAG, "TX Power 20dBm OK");
            vTaskDelay(pdMS_TO_TICKS(3000));
        }
        
        ret = esp_wifi_set_max_tx_power(44);
        if (ret == ESP_OK) {
            show_status("TX 44dBm OK!");
            ESP_LOGI(TAG, "TX Power 44dBm OK");
            vTaskDelay(pdMS_TO_TICKS(3000));
        }
        
        show_status("WiFi Test PASS!");
    } else {
        show_status("WiFi TIMEOUT!");
        ESP_LOGE(TAG, "\n========================================");
        ESP_LOGE(TAG, "FAILED! WiFi RF may be damaged");
        ESP_LOGE(TAG, "========================================");
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "WiFi Low Power Recovery Test");
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "This test tries to recover WiFi");
    ESP_LOGI(TAG, "with minimal power consumption.");
    ESP_LOGI(TAG, "========================================\n");
    
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    
    oled_init();
    oled_clear();
    oled_draw_string(0, 0, "WiFi Recovery Test");
    oled_draw_string(0, 20, "Low Power Mode");
    oled_draw_string(0, 40, "Please wait...");
    oled_update();
    
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    test_wifi_minimal();
    
    ESP_LOGI(TAG, "\nTest complete.");
    
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
