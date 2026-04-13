#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "driver/gpio.h"
#include "esp_timer.h"

#include "esp_system.h"

#include "esp_rom_sys.h"

#if CONFIG_LOG_COLORS
#define LOG_COLOR_START  "\033["
#else
#define LOG_COLOR_START  ""
#endif

#if CONFIG_LOG_COLORS
#define LOG_COLOR_END   "\033[0m"
#else
#define LOG_COLOR_END   ""
#endif
