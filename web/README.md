# Web 控制界面

ESP32-S3 空气净化器的 Web 控制界面和 Nginx 配置。

## 文件说明

| 文件 | 功能 |
|------|------|
| `index.html` | 主控制面板（完整版） |
| `simple.html` | 简化版控制面板 |
| `debug.html` | MQTT 连接调试页面 |
| `test.html` | MQTT 功能测试页面 |
| `wifi-config.html` | ESP32 WiFi 配网页面 |
| `nginx-fan.conf` | Nginx 反向代理配置 |

## 部署说明

### 1. 网页文件部署

将 HTML 文件复制到 Nginx 的 Web 根目录：

```bash
sudo cp *.html /home/www/ai-security/fan/
```

### 2. Nginx 配置

将 `nginx-fan.conf` 中的风扇相关配置添加到 Nginx 站点配置中：

```nginx
# 风扇控制页面
location ^~ /fan/ {
    root /home/www/ai-security;
    index index.html;
    try_files $uri $uri/ =404;
}
```

### 3. 访问地址

| 页面 | 地址 |
|------|------|
| 主控制面板 | `http://服务器IP/fan/` |
| 简化版 | `http://服务器IP/fan/simple.html` |
| 调试页面 | `http://服务器IP/fan/debug.html` |
| 测试页面 | `http://服务器IP/fan/test.html` |
| WiFi 配网 | `http://服务器IP/fan/wifi-config.html` |

## MQTT 配置

- **Broker**: `ws://服务器IP:9001` (WebSocket)
- **订阅主题**: `fan/status`
- **发布主题**: `fan/cmd`

## 控制命令

| 命令 | 说明 | 示例 |
|------|------|------|
| `fan:<速度>` | 设置风扇速度 (0-100) | `fan:50` |
| `mode:<模式>` | 切换模式 (0=手动, 1=自动) | `mode:1` |
| `power:<状态>` | 电源控制 (0=关闭, 1=开启) | `power:on` |

## 状态数据格式

```json
{
  "temperature": 25.0,
  "humidity": 60.0,
  "fan_speed": 50,
  "gas_raw": 1,
  "gas_mv": 1200,
  "gas_ppm": 100.0,
  "auto_mode": 0,
  "power": 1
}
```
