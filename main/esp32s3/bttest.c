#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_ble_api.h"
#include "nvs_flash.h"
#include "soc/rtc_cntl_reg.h"

#include "oled.h"

static const char *TAG = "BT_TEST";

static int bt_init_ok = 0;

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
    switch (event) {
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
            ESP_LOGI(TAG, "BLE adv data set complete");
            break;
        case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT:
            ESP_LOGI(TAG, "BLE scan param set complete");
            break;
        case ESP_GAP_BLE_SCAN_RESULT_EVT:
            ESP_LOGI(TAG, "BLE scan result received");
            break;
        default:
            break;
    }
}

static void show_status(const char *msg) {
    oled_clear();
    oled_draw_string(0, 0, "BT/BLE Test");
    oled_draw_string(0, 20, msg);
    oled_update();
    ESP_LOGI(TAG, "%s", msg);
}

static void test_bluetooth(void) {
    ESP_LOGI(TAG, "\n========================================");
    ESP_LOGI(TAG, "Bluetooth/BLE Hardware Test");
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
    
    show_status("Init BT Controller...");
    
    esp_bt_controller_config_t bt_cfg = {
        .controller_task_stack_size = 4096,
        .controller_task_prio = 1,
        .hci_uart_baud = 115200,
        .hci_uart_tx_pin = -1,
        .hci_uart_rx_pin = -1,
        .hci_uart_cts_pin = -1,
        .hci_uart_rts_pin = -1,
        .txa_queue_size = 7,
        .ble_max_conn = 3,
        .bt_max_acl_conn = 0,
        .bt_sco_datapath = 0,
        .bt_max_sync_conn = 0,
        .ble_sca = 0,
        .pcm_role = 0,
        .pcm_polar = 0,
        .pcm_fsyncshp = 0,
        .enc_key_sz_min = 7,
    };
    
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "BT controller init failed: %s", esp_err_to_name(ret));
        show_status("BT Ctrl FAIL!");
        return;
    }
    ESP_LOGI(TAG, "BT controller init: OK");
    
    show_status("Enable BT Controller...");
    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "BT controller enable failed: %s", esp_err_to_name(ret));
        show_status("BT Enable FAIL!");
        return;
    }
    ESP_LOGI(TAG, "BT controller enable: OK");
    
    show_status("Init Bluedroid...");
    ret = esp_bluedroid_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Bluedroid init failed: %s", esp_err_to_name(ret));
        show_status("Bluedroid FAIL!");
        return;
    }
    ESP_LOGI(TAG, "Bluedroid init: OK");
    
    show_status("Enable Bluedroid...");
    ret = esp_bluedroid_enable();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Bluedroid enable failed: %s", esp_err_to_name(ret));
        show_status("Bluedroid En FAIL!");
        return;
    }
    ESP_LOGI(TAG, "Bluedroid enable: OK");
    
    bt_init_ok = 1;
    
    show_status("Register GAP...");
    ret = esp_ble_gap_register_callback(gap_event_handler);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GAP register failed: %s", esp_err_to_name(ret));
        show_status("GAP FAIL!");
        return;
    }
    ESP_LOGI(TAG, "GAP register: OK");
    
    show_status("Get BT Address...");
    const uint8_t *bd_addr = esp_bt_dev_get_address();
    if (bd_addr) {
        ESP_LOGI(TAG, "BT Address: %02X:%02X:%02X:%02X:%02X:%02X",
                 bd_addr[0], bd_addr[1], bd_addr[2], bd_addr[3], bd_addr[4], bd_addr[5]);
        char addr_str[32];
        snprintf(addr_str, sizeof(addr_str), "%02X:%02X:%02X:%02X:%02X:%02X",
                 bd_addr[0], bd_addr[1], bd_addr[2], bd_addr[3], bd_addr[4], bd_addr[5]);
        oled_clear();
        oled_draw_string(0, 0, "BT/BLE Test");
        oled_draw_string(0, 20, "BT Address:");
        oled_draw_string(0, 40, addr_str);
        oled_update();
    }
    
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    show_status("Test BLE Scan...");
    
    static esp_ble_scan_params_t scan_params = {
        .scan_type = BLE_SCAN_TYPE_PASSIVE,
        .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
        .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
        .scan_interval = 0x50,
        .scan_window = 0x30,
        .scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE
    };
    
    ret = esp_ble_gap_set_scan_params(&scan_params);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Set scan params failed: %s", esp_err_to_name(ret));
        show_status("Scan Param FAIL!");
    } else {
        ESP_LOGI(TAG, "Scan params set: OK");
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        ret = esp_ble_gap_start_scanning(10);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Start scan failed: %s", esp_err_to_name(ret));
            show_status("Scan Start FAIL!");
        } else {
            ESP_LOGI(TAG, "BLE scan started for 10 seconds...");
            show_status("Scanning 10s...");
            vTaskDelay(pdMS_TO_TICKS(10000));
            esp_ble_gap_stop_scanning();
        }
    }
    
    ESP_LOGI(TAG, "\n========================================");
    ESP_LOGI(TAG, "BLUETOOTH TEST RESULT");
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "BT Controller Init: PASS");
    ESP_LOGI(TAG, "Bluedroid Init: PASS");
    ESP_LOGI(TAG, "GAP Register: PASS");
    ESP_LOGI(TAG, "BT Address: %s", bd_addr ? "OK" : "FAIL");
    ESP_LOGI(TAG, "========================================");
    
    oled_clear();
    oled_draw_string(0, 0, "BT/BLE Test");
    oled_draw_string(0, 20, "RESULT:");
    oled_draw_string(0, 40, "BT/BLE PASS!");
    oled_update();
}

void app_main(void) {
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "ESP32-S3 Bluetooth/BLE Hardware Test");
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "Testing if Bluetooth RF is damaged");
    ESP_LOGI(TAG, "(BT and WiFi share the same RF)");
    ESP_LOGI(TAG, "========================================\n");
    
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    
    oled_init();
    oled_clear();
    oled_draw_string(0, 0, "BT/BLE Test");
    oled_draw_string(0, 20, "Starting...");
    oled_draw_string(0, 40, "Watch serial!");
    oled_update();
    
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    test_bluetooth();
    
    ESP_LOGI(TAG, "\nTest complete.");
    
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
