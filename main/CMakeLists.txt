idf_component_register(
    SRCS
       
        "oled.c"
    INCLUDE_DIRS "."
    REQUIRES driver esp_timer nvs_flash bt
    PRIV_INCLUDE_DIRS 
        "${IDF_PATH}/components/bt/include/esp32/include"
        "${IDF_PATH}/components/bt/host/bluedroid/api/include/api"
)
