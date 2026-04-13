#include "mqtt_app.h"
#include "mqtt_client.h"
#include "esp_log.h"
#include <string.h>
#include <stdio.h>
#include "local_config.h"

static const char *TAG = "MQTT";

#ifndef LOCAL_MQTT_BROKER
#define MQTT_BROKER_URI   "mqtt://YOUR_BROKER_IP"
#define MQTT_BROKER_PORT 1883
#else
#define MQTT_BROKER_URI   LOCAL_MQTT_BROKER
#define MQTT_BROKER_PORT  LOCAL_MQTT_PORT
#endif

#define MQTT_TOPIC_STATUS "fan/status"
#define MQTT_TOPIC_CMD    "fan/cmd"

static esp_mqtt_client_handle_t mqtt_client = NULL;
static void (*cmd_callback)(const char *data, int len) = NULL;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;

    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT Connected to broker");
            esp_mqtt_client_subscribe(mqtt_client, MQTT_TOPIC_CMD, 0);
            ESP_LOGI(TAG, "Subscribed to topic: %s", MQTT_TOPIC_CMD);
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT Data received: %.*s", event->data_len, event->data);
            if (cmd_callback && event->topic_len == (int)strlen(MQTT_TOPIC_CMD) &&
                strncmp(event->topic, MQTT_TOPIC_CMD, event->topic_len) == 0) {
                cmd_callback(event->data, event->data_len);
            }
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGW(TAG, "MQTT Error");
            break;

        default:
            break;
    }
}

void mqtt_app_start(void) {
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = MQTT_BROKER_URI,
        .broker.address.port = MQTT_BROKER_PORT,
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);

    ESP_LOGI(TAG, "MQTT client started, connecting to %s", MQTT_BROKER_URI);
}

void mqtt_publish_status(float temp, float humi, uint8_t fan_speed, int gas_raw, int gas_mv, float gas_ppm, int auto_mode, int power_state) {
    if (mqtt_client == NULL) return;

    char payload[256];
    snprintf(payload, sizeof(payload),
             "{\"temperature\":%.1f,\"humidity\":%.1f,\"fan_speed\":%d,\"gas_raw\":%d,\"gas_mv\":%d,\"gas_ppm\":%.1f,\"auto_mode\":%d,\"power\":%d}",
             temp, humi, fan_speed, gas_raw, gas_mv, gas_ppm, auto_mode, power_state);

    int msg_id = esp_mqtt_client_publish(mqtt_client, MQTT_TOPIC_STATUS, payload, 0, 1, 0);
    ESP_LOGI(TAG, "MQTT published, msg_id=%d", msg_id);
}

void mqtt_set_cmd_callback(void (*callback)(const char *data, int len)) {
    cmd_callback = callback;
}
