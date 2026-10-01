# Task 001 — Repository Bootstrap + ESP32 Bring-up

## Goal

Verify PC -> compile -> flash -> ESP32 -> serial monitor chain.

当前环境：Windows + Arduino 框架。仅使用 USB 给 ESP32 开发板供电和通信。

## Acceptance criteria

- [x] Source code exists in repository
- [ ] Code compiles
- [ ] Firmware can be flashed
- [ ] Serial monitor receives periodic output
- [ ] No motor or external power is used in this task

最后一项需由实物操作者确认。源码不访问 GPIO 或外设，不能代替现场接线检查。

## Scope

- 创建基础目录、README、`.gitignore` 与最小 Arduino sketch。
- 使用 `Serial.begin(115200)`，启动时打印项目名和任务名。
- 每次循环等待 1000 ms，再打印 `millis()` 返回的实际启动时长。
- 不使用外部库，不假设 LED 引脚，不配置未知 GPIO。
- 不使用电机、外部电源或电池；不安装 ROS 2，不实现 PID。

## Source

`firmware/esp32/arduino/bringup_001/bringup_001.ino`

## Arduino IDE 操作

1. 断开电池、外部电源、电机和其他外设，只将 ESP32 开发板通过支持数据传输的 USB 线连接 Windows PC。
2. 打开上述 `.ino` 文件，保持文件名与所在文件夹同名 `bringup_001`。
3. 在「文件 → 首选项 → 附加开发板管理器网址」保留已有条目，并按需添加：
   `https://espressif.github.io/arduino-esp32/package_esp32_index.json`。
4. 在开发板管理器安装 **esp32 by Espressif Systems**。`Arduino ESP32 Boards` 不是这里需要的通用 Espressif 开发板包。
5. 优先选择与实际开发板厂商资料一致的板型；若确认是通用 ESP32-WROOM-32E 开发板，可选择 **ESP32 Dev Module**（FQBN：`esp32:esp32:esp32`）。不要选择 ESP32-S3 或 Arduino Nano ESP32。未知 Flash 参数不要猜测，保留板型默认值并核对厂商资料。
6. 在「工具 → 端口」选择该开发板出现的 COM 口。可通过拔插 USB 比较端口列表确认，不固定假设 COM 编号。
7. 点击「验证」并记录编译结果。通过后点击「上传」。
8. 如果上传一直停在连接阶段，按开发板说明使用 BOOT / EN 进入下载模式；若没有 COM 口，先检查数据线和 Windows 设备管理器，再按实际 USB 转串口芯片选择驱动。
9. 打开串口监视器，波特率选择 **115200**。若错过启动文字，保持监视器打开并按一次 EN / RESET。
10. 连续观察至少 10 行 uptime，记录板型、开发板包版本、COM 口和实际输出，再更新验收勾选项。

预期示例（示例不是实物日志）：

```text
ROS2 AI Vision Rover
Task 001 ESP32 Bring-up
uptime_ms=1000
uptime_ms=2000
uptime_ms=3000
```

`uptime_ms` 是实际 `millis()` 值，受到初始化与串口输出耗时影响，允许小幅偏差，无需严格等于 1000 的整数倍。约 49.7 天后会自然回绕。

## 本地验证记录

检查日期：2026-09-29。

- 初始目录为空且没有 `.git`；已初始化本地 `main` 分支，没有提交、推送或配置远程。这里不是 GitHub 仓库历史的克隆。
- 目录与源码静态检查通过：要求的路径存在，波特率为 115200，两行启动文本齐全，使用 `delay(1000)` 与 `millis()` 输出 uptime，无外部库包含或 GPIO 控制。
- 忽略规则验证通过：构建输出、ROS 2 构建目录和 Python 虚拟环境均被忽略。
- 检测到 Arduino IDE 自带 CLI。已安装 `arduino:avr 1.8.7` 和 `arduino:esp32 2.0.18-arduino.5`，未安装目标平台 `esp32:esp32`。
- 实际尝试目标编译，失败原因：`Platform 'esp32:esp32' not found: platform not installed`。尚未进入源码编译阶段，不能视为编译通过或源码编译错误。
- 未执行上传，未打开硬件串口，未验证实物接线与输出。
- Git 跨账户检查出现目录所有者警告时，本次使用命令级 `-c safe.directory=D:/ros2-ai-vision-rover`，不修改全局 Git 配置。

安装 Espressif 开发板包后，可在项目根目录用 PowerShell 复现编译：

```powershell
& "$env:LOCALAPPDATA\Programs\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe" compile --config-file "$env:USERPROFILE\.arduinoIDE\arduino-cli.yaml" --fqbn esp32:esp32:esp32 --build-path "$PWD\build\bringup_001" "$PWD\firmware\esp32\arduino\bringup_001"
```

此命令匹配本机当前 Arduino IDE 安装位置；也可直接使用 IDE 的「验证」。编译、上传和串口实测分别记录，不相互替代。

安装说明来源：[Espressif Arduino-ESP32 官方文档](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html)。

## 实物验收记录（待填写）

- 日期：
- 实际开发板 / 所选板型：
- Espressif 开发板包版本：
- COM 口：
- 编译结果：
- 上传结果：
- 启动文本及连续 uptime 输出：
- 确认仅 USB 供电、未连接电机或外部电源：
