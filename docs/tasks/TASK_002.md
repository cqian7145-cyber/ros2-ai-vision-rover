# Task 002 — VL53L0X ToF Sensor Bring-up

Status: **IN PROGRESS**

## Goal

使用 ESP32 + VL53L0X，通过 I2C 读取真实距离，并以 115200 串口输出毫米值。Windows + Arduino framework；不实现 OLED、SG90、电机、编码器、PID、ROS2 或 Wi-Fi。

前置条件：Task 001 PASS，依据用户于 2026-09-30 提供的编译、烧录、uptime 实测反馈。

## Hardware

- ESP32EA MOC / ESP32-WROOM-32E 开发板
- 支持 3.3V 供电及 ESP32 I2C 电平的 VL53L0X 模块
- USB 数据线、杜邦线；如用 ESP32 EXP1 扩展板，先核对丝印和引脚映射
- 仅 USB 供电，不连接电池、电机或其他功能模块

## Wiring

断开 USB 后接线，核对完成再上电。

| VL53L0X 模块 | ESP32 |
| --- | --- |
| VCC（部分模块标 VIN，核对模块资料） | 3.3V |
| GND | GND |
| SDA | 所选开发板默认 SDA |
| SCL | 所选开发板默认 SCL |

程序调用无参数 `Wire.begin()`。通用 ESP32 Dev Module 默认 SDA=GPIO21、SCL=GPIO22；这是开发板定义，不是对所有扩展板接口的保证。沿用 Task 001 正确板型，核对模块和扩展板丝印，不盲接未知 GPIO。

I2C 需要上拉电阻；核对模块是否已有适合 3.3V 总线的上拉，勿将总线上拉到 5V。本任务不连接或控制 XSHUT / GPIO1，模块必须能按自身设计默认启用。接线针对模块，不是直接给裸传感器芯片接 3.3V。

## Concepts

- **ToF**：根据光往返时间估计距离，类似测量“光的回声”；这里交给传感器完成。
- **I2C**：ESP32 用 SDA 传数据、SCL 提供时钟；可以类比按地址投递消息的两线总线。这里由 Wire 与 Pololu 库处理通信。
- **连续测距**：`startContinuous(200)` 让传感器按约 200 ms 周期测量；`millis()` 控制软件读取节奏。
- **单位**：`distance_mm=523` 表示返回值为 523 mm。用平整物体靠近、远离，观察变化趋势；这不是精密标定。
- **超时**：`setTimeout(500)` 限制库等待测量的时间。超时输出 `VL53L0X_TIMEOUT`，不把该次返回值作为距离。读取 API 仍会短暂阻塞，因此程序并非完全异步，故障时输出间隔可能超过 200 ms。
- **初始化失败**：输出 `I2C_INIT_FAILED` 或 `VL53L0X_INIT_FAILED`，停止测距；若初始化发生库超时，还输出 `VL53L0X_TIMEOUT`。修复接线后按 EN/RESET 重试。
- 无 timeout 仅说明没有触发库等待超时，不保证光学读数有效；过远、反光、黑色目标或强光可能导致异常读数，不能把它们一律当成 timeout。

## Build steps

1. Arduino IDE 打开 `firmware/esp32/arduino/vl53l0x_002/vl53l0x_002.ino`。
2. 沿用 Task 001 成功的开发板与 COM 口配置。
3. 在库管理器搜索 **VL53L0X**，安装作者为 **Pololu** 的库。`Wire` 随 ESP32 Arduino 开发板包提供，无需单独安装。不选 API 不同的 Adafruit 库替代。
4. 点击「验证」，成功后再「上传」。
5. 串口监视器设为 115200，按 EN/RESET 查看启动文本。

本机 2026-09-30 检测到开发板包 `espressif:esp32 3.3.10`（镜像包命名空间）。CLI 构建命令如下；不上传硬件：

```powershell
& "$env:LOCALAPPDATA\Programs\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe" compile --config-file "$env:USERPROFILE\.arduinoIDE\arduino-cli.yaml" --fqbn espressif:esp32:esp32 --build-path "$PWD\build\vl53l0x_002" "$PWD\firmware\esp32\arduino\vl53l0x_002"
```

从仓库根目录运行。其他电脑的包若为 `esp32:esp32`，应使用实际 `board listall` 返回的 FQBN，不混用包命名空间。

## Acceptance criteria

- [ ] ESP32 code compiles
- [ ] Firmware flashes successfully
- [ ] VL53L0X initializes
- [ ] Serial reports distance in millimeters
- [ ] Moving an object toward/away from sensor changes readings
- [ ] Sensor timeout is handled
- [ ] No unrelated hardware is used

## 实物验收步骤

1. 核对四线连接，仅 USB 供电，编译后上传。
2. 确认出现项目名、任务名、`VL53L0X initialized`。
3. 用平整浅色物体在传感器前约 100–500 mm 范围内往返移动，观察 `distance_mm` 总体随距离增减，保存至少 20 行真实日志。
4. 断电后移除传感器再上电，可检查初始化失败分支；这不等价于运行时 timeout 测试。重新接线前先断电。
5. 运行时 timeout 分支需单独故障测试并记录方法。可在临时测试副本中，在初始化成功后调用 `sensor.stopContinuous()` 停止测量；已有结果可能先被读出，随后检查 `VL53L0X_TIMEOUT`，且该次不输出 `distance_mm`。测试后重新上传本仓库正常版本。此故障注入只验证等待超时分支，不代表覆盖全部接线故障。
6. 出现初始化失败时检查供电、共地、SDA/SCL、模块默认启用状态，再复位。出现测距 timeout 时记录日志并检查连接；本程序不自动重新初始化传感器。

正常输出示例（非实测）：

```text
ROS2 AI Vision Rover
Task 002 VL53L0X Bring-up
VL53L0X initialized
distance_mm=523
distance_mm=519
distance_mm=517
```

超时输出：`VL53L0X_TIMEOUT`。

## Test results

日期：2026-09-30。

- Arduino CLI：已找到 IDE 自带 CLI。
- 已安装平台：`espressif:esp32 3.3.10`。
- 库检查：`No libraries installed.`，缺少 **VL53L0X by Pololu**；按用户要求不自动安装。
- 编译：已实际执行上述 CLI 命令，退出码 1；报错 `fatal error: VL53L0X.h: No such file or directory`。平台 `espressif:esp32 3.3.10` 和内置 `Wire 3.3.10` 已识别；因缺库，完整编译未通过。安装 Pololu 库后需重新验证。
- 源码静态检查通过：115200 串口、默认 Wire 初始化、200 ms 周期、500 ms timeout、连续测距和错误分支均存在；无长固定延时或无关 GPIO 控制。此检查不替代编译或硬件测试。构建输出目录已被 Git 忽略。
- 烧录、I2C 初始化、真实距离变化、故障注入和实物接线：**未测试，必须由用户完成**。

用户实测记录：

- 日期 / 板型 / COM：
- ESP32 包版本 / Pololu 库版本：
- 编译 / 上传结果：
- 正常串口日志与物体移动现象：
- 初始化失败测试结果：
- timeout 测试方法与输出：
- 是否仅使用 ESP32 + VL53L0X：

## References

- [Pololu VL53L0X library](https://github.com/pololu/vl53l0x-arduino)
- [Pololu Continuous example](https://github.com/pololu/vl53l0x-arduino/blob/master/examples/Continuous/Continuous.ino)
- [Espressif I2C API 与默认引脚](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/i2c.html)
