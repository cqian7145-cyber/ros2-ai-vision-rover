# Task 003 — SSD1306 OLED + Shared I2C Bus

Status: **IN PROGRESS**

## Goal

Windows + Arduino framework 下，先让 SSD1306 显示系统标题、Task 003 与 uptime，再让 VL53L0X 距离同时显示在 Serial 和 OLED。

Task 001、Task 002 已由用户反馈完成；本任务不实现 Motor、Encoder、Servo、Wi-Fi、ROS2 或 PID。

## Hardware

- ESP32-WROOM-32E 开发板、USB 数据线
- SSD1306 I2C OLED 模块，先按 **128x64、0x3C** 配置（尺寸尚未经实物确认）
- VL53L0X 模块，预期地址 0x29
- 杜邦线，必要时使用已核对引脚映射的扩展板

仅使用兼容 3.3V 供电与逻辑电平的模块。OLED 若为 128x32，修改两个程序的 `SCREEN_HEIGHT=32`；所有文本使用 8 像素高的小字，综合版四行占 32 像素。若实际为 SH1106 或 SPI 屏，本程序不能直接作为 SSD1306 I2C 使用，先核对型号。

## I2C architecture

```text
ESP32 Wire 主机（100 kHz）
  GPIO21 SDA ─┬─ VL53L0X SDA（0x29）
              └─ SSD1306 SDA（通常 0x3C）
  GPIO22 SCL ─┬─ VL53L0X SCL
              └─ SSD1306 SCL
  GND ───────── 两模块共地
```

两设备并联共享数据线和时钟线，使用不同地址区分。两个程序都显式调用一次 `Wire.begin(21, 22)`。SSD1306 使用同一个 `&Wire`；`display.begin(..., false, false)` 不控制 reset GPIO，也不再次初始化 Wire。OLED 构造函数指定传输及传输后时钟均为 100 kHz。

启动时扫描地址 1–126，Serial 输出 `I2C_FOUND=0x3C` 等结果。ACK 只说明地址上有设备，不证明芯片型号、显示正常或测距正确。程序会探测配置的 OLED 地址；Adafruit 的 begin 返回值本身不能作为屏幕已接入的证据。

综合版每约 200 ms 读取一次连续测距结果，同一次读取值供 Serial 与 OLED 使用。正常显示 `sensor OK`；测距超时显示 `sensor timeout` 与 `Distance: --`，Serial 输出 `VL53L0X_TIMEOUT`，不保留旧距离作为当前值。

读取是有等待的库调用，传感器 timeout 为 500 ms；因此故障时刷新可能慢于 200 ms。OLED 全帧传输也需要时间。本程序不是完全异步控制系统。`sensor OK` 表示此次未发生库超时，不是光学精度或全部 I2C 故障的验证。

## Wiring

断开 USB 后接线，确认模块丝印与供电兼容后再上电；本任务不使用电池或电机。

| ESP32 | VL53L0X | SSD1306 OLED |
| --- | --- | --- |
| 3.3V | VCC / VIN（核对模块资料） | VCC（核对模块资料） |
| GND | GND | GND |
| GPIO21 | SDA | SDA |
| GPIO22 | SCL | SCL |

I2C 上拉应指向兼容 ESP32 的 3.3V 逻辑电平，不接 5V 上拉。两模块可能都自带上拉，先核对电路，不盲目再并联电阻。OLED reset=-1，适用于无需独立复位 GPIO 的模块；VL53L0X XSHUT / GPIO1 不连接，模块须能默认启用。

如果扫描发现 OLED 为 0x3D，核对模块地址后修改两份程序的 `OLED_ADDRESS`；不要把 VL53L0X 的 0x29 当成 OLED 地址。

## Libraries

通过 Arduino IDE 库管理器安装：

- **Adafruit GFX Library**（Adafruit）
- **Adafruit SSD1306**（Adafruit）
- **Adafruit BusIO**（Adafruit，GFX 依赖；安装时接受依赖提示）
- **VL53L0X**（Pololu，仅综合版需要）
- `Wire` 随 ESP32 开发板包提供，无需单独安装。

2026-10-01 本机已安装 `VL53L0X 1.3.1` 与 `espressif:esp32 3.3.10`。查询未发现上述 Adafruit 库。本次不自动安装库。

## Build steps

1. 安装上述缺失库，沿用 Task 001 / 002 已成功的板型、端口配置。
2. 打开 `firmware/esp32/arduino/oled_003/oled_003.ino`，核对 OLED 尺寸和地址，点击验证并上传。
3. 115200 串口监视器打开后按 EN/RESET，查看扫描输出和屏幕文字。
4. 独立显示通过后，断电接入 VL53L0X；打开 `firmware/esp32/arduino/display_distance_003/display_distance_003.ino`，验证并上传。
5. 两份 sketch 分别编译和上传；综合版会替换板上的独立版，无需同时运行两个程序。

本机可从仓库根目录用 PowerShell 编译（不上传）：

```powershell
$cli = "$env:LOCALAPPDATA\Programs\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe"
& $cli compile --config-file "$env:USERPROFILE\.arduinoIDE\arduino-cli.yaml" --fqbn espressif:esp32:esp32 --build-path "$PWD\build\oled_003" "$PWD\firmware\esp32\arduino\oled_003"
& $cli compile --config-file "$env:USERPROFILE\.arduinoIDE\arduino-cli.yaml" --fqbn espressif:esp32:esp32 --build-path "$PWD\build\display_distance_003" "$PWD\firmware\esp32\arduino\display_distance_003"
```

FQBN 对应本机安装的平台命名空间，其他机器以实际开发板列表为准。

## Acceptance criteria

- [ ] I2C scanner detects OLED
- [ ] OLED initializes
- [ ] Text is visible
- [ ] uptime updates
- [ ] VL53L0X and OLED coexist on same I2C bus
- [ ] distance changes on OLED as object moves
- [ ] Serial output remains functional

## Hardware results

**未烧录、未连接串口、未观察 OLED 或距离。全部实物验收必须由用户完成。**

### 本地检查（2026-10-01）

- 两份程序均已实际调用 Arduino CLI 编译，各自退出码均为 1：`fatal error: Adafruit_GFX.h: No such file or directory`。平台与 Wire 3.3.10 已正确识别；综合版另识别 VL53L0X 1.3.1。尚未通过完整编译，安装缺失库后必须重试。
- 静态检查通过：两份程序具有显式 GPIO21/22 总线初始化、启动扫描、200 ms 调度、无二次 Wire 初始化的 OLED 配置；综合版包含连续测距与超时清除旧值分支。未使用任务外 GPIO 控制或长固定延时。静态检查不替代编译与硬件验收。
- 示例输出不属于实测记录。

### 用户实物验收步骤

1. 先接 OLED，运行独立版。串口应扫描到 OLED 地址，屏幕显示 `ROS2 AI Rover`、`Task 003`、`Uptime: XX s`，观察至少 10 秒确认 uptime 递增。
2. 断电，并联 VL53L0X 后上传综合版。重启查看扫描同时出现 0x29 和 OLED 实际地址。
3. 确认 Serial 出现初始化日志，OLED 显示 `sensor OK` 与 `Distance: ... mm`。
4. 用平整浅色物体在约 100–500 mm 范围内移动，观察 OLED 与 Serial 值同步变化，保存串口日志和屏幕照片。
5. 初始化失败时修复接线后复位。OLED 缺失时，综合版仍尝试测距并输出 Serial；传感器缺失时，OLED 显示 `sensor INIT FAILED`，距离为 `--`。
6. 若单独验收 timeout，可用临时测试副本在 `sensor.startContinuous(...)` 后停止连续测量；已有样本可能先被读取，之后应显示 timeout。此测试只验证超时分支，不等价于覆盖所有断线故障。完成后恢复正常程序。物体过远不一定触发 timeout。
7. 故障时不会自动重新初始化设备；修复后复位。不建议带电插拔模块。

实测记录（待用户填写）：

| 项目 | 结果 |
| --- | --- |
| 日期、板型、COM、库版本 | 待填写 |
| OLED 控制器、尺寸、扫描地址 | 待填写 |
| 两版本编译与上传 | 待填写 |
| 独立版文字与 uptime | 待填写 |
| 两设备同时扫描结果 | 待填写 |
| 物体移动与 OLED / Serial 日志 | 待填写 |
| timeout / 初始化失败检查（如执行） | 待填写 |

## References

- [Adafruit SSD1306 官方库与 API](https://github.com/adafruit/Adafruit_SSD1306)
- [Adafruit GFX 官方库](https://github.com/adafruit/Adafruit-GFX-Library)
- [Pololu VL53L0X 官方库](https://github.com/pololu/vl53l0x-arduino)
