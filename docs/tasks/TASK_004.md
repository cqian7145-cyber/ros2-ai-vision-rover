# Task 004 — SG90 Servo + PWM Bring-up

Status: **IN PROGRESS — hardware acceptance pending**

## Goal

在 ESP32-WROOM-32E、Arduino-ESP32 Core 3.x 上直接使用 LEDC PWM 控制 SG90 180-degree servo，理解角度命令、脉宽和 PWM 计数值的关系。

两个独立示例：

- `firmware/esp32/arduino/servo_004/servo_004.ino`：启动先发送 90 度命令，保持约 2 秒，然后每 2 秒循环 30 → 90 → 150。
- `firmware/esp32/arduino/servo_serial_004/servo_serial_004.ino`：启动默认 90 度，通过串口逐行输入角度。

仅控制 GPIO25，不实现底盘电机、编码器、ROS2、PID、Wi-Fi 或其他外设，不修改 Task 002/003 的功能。不使用 ESP32Servo 或其他外部库。

## PWM principle

PWM 是周期性高低电平信号。频率为 50 Hz，所以周期 T=1/50 s=20 ms=20000 us。可以把它理解为每隔 20 ms 发送一次长度可调的高电平“指令”。

16 bit LEDC 计数器每周期有 65536 个 tick，单个 tick 约为 0.305 us。程序使用：

```text
pulse_us = 1000 + round(angle_deg × 1000 / 180)
duty = round(pulse_us × 65536 / 20000)
```

`pulseUsToDuty()` 使用 64 位中间乘积并限制到 0–65535。`setServoAngle()` 先验证 0–180，再映射脉宽；不合法输入不会写入 PWM。

| 命令角度 | 脉宽 us | duty 计数 | 约占空比 |
| --- | --- | --- | --- |
| 0 | 1000 | 3277 | 5% |
| 30 | 1167 | 3824 | 5.835% |
| 90 | 1500 | 4915 | 7.5% |
| 150 | 1833 | 6006 | 9.165% |
| 180 | 2000 | 6554 | 10% |

Core 3.x 使用 `ledcAttach(GPIO25, 50, 16)` 分配通道，再用 `ledcWrite(GPIO25, duty)` 写入。第一个参数是 GPIO，不是旧 API 的通道号。不使用已移除的 `ledcSetup()` / `ledcAttachPin()`。两次调用均检查返回值，失败会打印错误。

## Servo pulse-width principle

舵机根据高电平持续时间选择目标位置，50 Hz 仅决定重复频率。不是用 50% 占空比表示 90 度；本例中心为 1500 us，即约 7.5%。LEDC 硬件会持续输出，无需软件每 20 ms 手动翻转 GPIO。

1000–2000 us 是本次保守起点。0–180 是软件命令坐标，不承诺实物会覆盖准确 180 度；中心、方向和可用行程因型号、舵盘安装与个体不同而变化。先空载确认 30/90/150，不直接扩展到 500–2500 us。`angle_deg` 和 `pulse_us` 是命令日志，不是位置反馈或示波器测量。

## Wiring

断开 USB 和舵机电源后接线；先确认是位置舵机，不是连续旋转版本。

| SG90 线 | 连接 |
| --- | --- |
| 信号（通常橙/黄） | ESP32 GPIO25 |
| 电源正（通常红） | 独立稳压舵机电源正极，电压按实际型号规格 |
| 地（通常棕/黑） | 舵机电源负极，并连接 ESP32 GND |

线色仅作参考，优先按实际舵机标识和资料核对。ESP32 使用 USB 供电。舵机电源正极不连接 ESP32 3.3V，也不要在 USB 供电时盲目并接开发板 5V/VIN。

## Power considerations

TowerPro 的 SG90 页面标注工作电压 4.8V。使用符合实物规格的稳压电源；若供应商明确支持 5V，可用独立稳压 5V，不能默认所有同名舵机都允许 6V。不要直接接两节 18650 电池组，也不要从 GPIO 或 ESP32 3.3V 引脚给舵机供电。

电源应能承受启动与负载电流，具体余量按实物规格与负载确定，不能只看空载电流。首次测试不装负载，留出舵盘活动空间。电源线尽量短，舵机电流回路直接回到电源负极，避免经 ESP32 细地线回流。

若出现 ESP32 brownout / 重复启动、舵机抖动、堵转嗡鸣或异常发热，先断电，检查电压跌落、共地、机械干涉和脉宽范围，不通过关闭 brownout 检测掩盖供电问题。必要时依据电源设计增加本地去耦。

GPIO25 输出为 3.3V 逻辑，须确认实际舵机接受该信号；若不接受，使用合适的单向电平转换，不能让 5V 回灌 ESP32 GPIO。

## Common ground

```text
PC USB ───────── ESP32（供电）
ESP32 GPIO25 ─── SG90 signal
ESP32 GND ──┬─── SG90 GND
            └─── 舵机电源负极
舵机电源正极 ─── SG90 VCC
```

共地提供 PWM 电平的共同参考；只有信号线而没有共地，电平可能无法可靠识别。共地不表示把两个供电源的正极并在一起。

## Build steps

1. Arduino IDE 使用已验证的 ESP32-WROOM-32E 板型与 Arduino-ESP32 Core 3.x，无需安装舵机库。
2. 打开自动版 `.ino`，点击验证。接线确认后再上传，115200 串口查看启动信息。
3. 第二个示例单独打开、验证并上传；会替换自动版。
4. 串口监视器选 **Newline** 或 **Both NL & CR**；输入 `30`、`90`、`150` 后发送。也支持 CR 单独结束。
5. 命令严格为每行 1–3 位十进制数字，范围 0–180；允许 `090`，不接受符号、空格、小数或文本。空行、`-1`、`181`、`90abc`、`90.5`、超长行都明确报错并保持上一条有效命令。CRLF 作为一个行结束符；长行丢弃至换行，不截取前缀执行。选择“无行结束符”时命令尚未提交，不会移动。

本机从仓库根目录执行：

```powershell
$cli = "$env:LOCALAPPDATA\Programs\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe"
& $cli compile --config-file "$env:USERPROFILE\.arduinoIDE\arduino-cli.yaml" --fqbn espressif:esp32:esp32 --build-path "$PWD\build\servo_004" "$PWD\firmware\esp32\arduino\servo_004"
& $cli compile --config-file "$env:USERPROFILE\.arduinoIDE\arduino-cli.yaml" --fqbn espressif:esp32:esp32 --build-path "$PWD\build\servo_serial_004" "$PWD\firmware\esp32\arduino\servo_serial_004"
```

本机平台是 `espressif:esp32 3.3.10`；其他机器按实际安装的包命名空间选择 FQBN。

## Acceptance criteria

- [x] Firmware compiles (both examples)
- [ ] Servo initializes at approximately 90 degrees
- [ ] Servo moves between 30/90/150 degrees
- [ ] ESP32 does not reset during motion
- [ ] Serial reports commanded angle and pulse width
- [ ] Serial interactive control accepts 0-180
- [ ] No unrelated hardware behavior is introduced

## Hardware test results

**未烧录、未驱动真实 SG90、未实测 PWM 波形、实际角度、电源跌落或运行中复位。实物验收由用户完成。**

### 本地验证（2026-10-01）

- 两份程序均在 `espressif:esp32 3.3.10`、FQBN `espressif:esp32:esp32` 上实际编译通过，退出码均为 0。自动版 Flash 290411 bytes，交互版 Flash 290687 bytes；两者全局 RAM 均为 22288 bytes。
- 静态 API 检查与 0/30/90/150/180 共 5 个脉宽及 duty 参考值核对通过。串口解析经代码审查覆盖空行、非数字、范围错误、超长输入丢弃、CRLF 去重；尚未通过真实串口运行验证。
- 全部 7 个既有 sketch（含 Task 002/003 与用户新增测试程序）SHA-256 与任务开始前一致。

### 用户验收步骤

1. 空载接线并确认电源电压、极性和共地；上电后观察初始约中位，不承诺舵盘机械刻度就是 90 度。
2. 自动版连续运行至少 10 个 30/90/150 循环。记录运动、抖动和串口是否再次出现启动横幅；重复启动需要排查供电。
3. 对照日志：30→1167 us、90→1500 us、150→1833 us。若有示波器，测 GPIO25 对 GND 的 20 ms 周期及对应高电平时间。
4. 上传串口版，先测试 30、90、150；确认运动无干涉后再谨慎测试 0、180 的保守脉宽端点。
5. 测试 `-1`、`181`、`abc`、`90abc`、空行、`123456789`，应报错且不执行新角度；然后发送 `90` 确认解析能恢复。分别测试 LF、CR、CRLF。
6. 确认没有使用底盘电机或无关硬件，将串口日志和实际现象填入下表。

| 记录项 | 用户实测结果 |
| --- | --- |
| 日期 / 板型 / Core / COM | 待填写 |
| 舵机型号、电源额定值及接线 | 待填写 |
| 中位、30/90/150 运动 | 待填写 |
| 连续运动期间是否复位 | 待填写 |
| 有效/无效串口命令 | 待填写 |
| 波形与物理角度（如测量） | 待填写 |

## References

- [Espressif Arduino-ESP32 LEDC API](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/ledc.html)
- [TowerPro SG90 Analog](https://towerpro.com.tw/product/sg90-analog/)
