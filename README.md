# ESP32-S3 智能空气净化器

基于 ESP32-S3 的智能空气净化器控制系统，支持环境监测、智能控制和本地显示。

## 功能特性

- **环境监测**：实时监测温度、湿度、空气质量
- **智能控制**：支持手动/自动两种风扇控制模式
- **本地显示**：OLED 屏幕显示实时数据和系统状态
- **按键控制**：本地按键调节风扇速度和电源开关
- **自动模式**：根据空气质量自动调节风扇速度

## 硬件要求

- **主控芯片**：ESP32-S3-WROOM-1
- **温湿度传感器**：DHT11
- **空气质量传感器**：MQ135
- **显示屏**：SSD1306 OLED (128x64)
- **风扇控制**：PWM 控制 (25kHz)

## GPIO 引脚分配

| GPIO | 功能 |
|------|------|
| GPIO10 | 按键 KEY10 (风扇速度调节/自动模式) |
| GPIO11 | 按键 KEY11 (电源开关) |
| GPIO19 | 风扇 PWM 输出 |
| GPIO48 | WS2812 RGB LED |
| I2C SDA | OLED 显示屏 |
| I2C SCL | OLED 显示屏 |
| 单线总线 | DHT11 温湿度传感器 |
| ADC | MQ135 气体传感器 |

## 软件架构

```
fan1/
├── main/
│   ├── esp32s3/           # ESP32-S3 核心代码
│   │   ├── main.c         # 主程序入口
│   │   ├── fan.c          # 风扇 PWM 控制
│   │   ├── key.c          # 按键扫描与事件处理
│   │   ├── dht11.c        # DHT11 温湿度传感器驱动
│   │   ├── mq135.c        # MQ135 气体传感器驱动
│   │   ├── oled.c         # OLED 显示驱动
│   │   ├── display.c      # 显示界面管理
│   │   ├── ws2812.c       # WS2812 RGB LED 驱动
│   │   ├── wifi_app.c     # WiFi 连接管理
│   │   └── mqtt_app.c     # MQTT 通信模块
│   └── web/               # Web 界面 (预留)
├── CMakeLists.txt
└── sdkconfig.defaults
```

## 编译与烧录

### 环境要求

- ESP-IDF v5.4.1 或更高版本
- Python 3.8+
- CMake 3.16+

### 编译命令

```bash
# 激活 ESP-IDF 环境
. ~/esp/esp-idf/export.sh          # Linux/Mac
C:\esp\v5.4.1\esp-idf\export.ps1   # Windows PowerShell

# 编译项目
idf.py build
```

### 烧录命令

```bash
idf.py -p COM17 flash
```

### 查看串口日志

```bash
idf.py -p COM17 monitor
```

## 配置说明

1. 复制 `main/esp32s3/local_config.h.example` 为 `main/local_config.h`
2. 修改 `local_config.h` 中的 WiFi 和 MQTT 配置：

```c
#define LOCAL_WIFI_SSID     "YOUR_WIFI_SSID"
#define LOCAL_WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define LOCAL_MQTT_BROKER   "mqtt://YOUR_BROKER"
```

## 使用说明

### 按键功能

- **KEY10 短按**：循环切换风扇速度 (0% → 25% → 50% → 75% → 100%)
- **KEY10 长按 (>1s)**：切换自动/手动模式
- **KEY11 短按**：切换电源开关状态

### 自动模式逻辑

| 空气质量状态 | 风扇速度 |
|-------------|----------|
| 质量差 (DO=0) | 100% |
| 质量好 (DO=1) | 25% |

## 许可证

本项目采用 MIT 许可证，详见 [LICENSE](LICENSE) 文件。

## 作者

Qing (G650great)
