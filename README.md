# ROS2 AI Vision Rover

基于 ESP32 与编码器差速底盘的真实机器人项目。逐步构建闭环电机控制、传感器融合、ROS 2 通信、AI 视觉识别和目标跟随能力。

GitHub: https://github.com/cqian7145-cyber/ros2-ai-vision-rover

## 当前阶段

**Task 004 — SG90 Servo + PWM Bring-up（IN PROGRESS）**。
当前使用 Windows + Arduino-ESP32 Core 3.x，通过 GPIO25 原生 LEDC PWM 控制 SG90，提供自动测试与串口交互示例。
Ubuntu / ROS 2 尚未开始。当前任务仅使用 ESP32 与 SG90，不实现底盘电机、编码器、PID、ROS2 或 Wi-Fi；Task 002/003 程序保持原样。

## 当前硬件

- ESP32EA MOC / ESP32-WROOM-32E
- ESP32 EXP1 Expansion Board
- MG310 Hall Encoder Motor x2
- TB6612FNG
- MPU6050
- VL53L0X
- SG90 180-degree servo
- SSD1306 OLED
- USB UVC Camera
- 2WD differential chassis
- 2x18650 battery system
- DC-DC regulator

以上为用户提供的硬件清单；具体开发板引脚、供电和接线将在后续任务核实。Task 004 使用 USB 给 ESP32 供电，SG90 使用符合实际模块规格的独立稳压电源，信号接 GPIO25，舵机电源地与 ESP32 GND 共地；详见 Task 004 供电说明。

## 总体架构

当前实际链路：

```text
Windows / Arduino IDE -- USB 编译后上传 --> ESP32
Windows / 串口监视器 <-- USB 串口 115200 -- ESP32 uptime（Task 001 已通过）
Task 002 已通过（用户反馈）：VL53L0X -- I2C --> ESP32 -- USB 串口 --> distance_mm
Task 003 待实物验证：同一 I2C 总线上的 SSD1306 显示状态与距离
Task 004 待实物验证：角度命令 → 脉宽 → GPIO25 LEDC PWM → SG90
```

未来规划（尚未实现）：

```text
USB UVC Camera → Ubuntu 24.04 主机
                  OpenCV / YOLO → ROS 2 Jazzy → 目标跟随决策
                                      ↕ 通信协议待后续任务确定
                                    ESP32
                ┌─────────────────────┼─────────────────────┐
             TB6612FNG              传感器                 SG90 / OLED
                ↓              MPU6050 / VL53L0X
          双电机差速底盘
                ↑
             编码器反馈
```

## 目录说明

```text
firmware/esp32/
  arduino/bringup_001/  Task 001 最小串口程序
  arduino/vl53l0x_002/  Task 002 连续测距程序
  arduino/oled_003/     Task 003 OLED 文字与 uptime
  arduino/display_distance_003/ Task 003 共享总线测距显示
  arduino/servo_004/   Task 004 自动 30/90/150 度测试
  arduino/servo_serial_004/ Task 004 串口角度控制
  esp-idf/             后续 ESP-IDF 固件预留
ros2_ws/src/           后续 ROS 2 软件包预留
vision/                后续 OpenCV / YOLO 视觉模块预留
hardware/
  wiring/              接线图预留
  datasheets/          数据手册预留
docs/
  tasks/TASK_001.md    Task 001 操作步骤与历史验收记录
  tasks/TASK_002.md    Task 002 接线、概念、构建与验收记录
  tasks/TASK_003.md    Task 003 共享总线、显示与实物验收
  tasks/TASK_004.md    Task 004 PWM 原理、供电与验收
  notes/               实验笔记预留
scripts/               构建与辅助脚本预留
```

空目录用 `.gitkeep` 保留；预留目录不代表对应功能已经实现。

## 当前进度

- **Task 001 PASS**：用户于 2026-09-30 确认 ESP32-WROOM-32E 已成功编译、烧录，并通过 115200 串口持续输出 uptime。此状态来自用户实测反馈。
- **Task 002 PASS**：用户于 2026-10-01 确认 I2C detection / distance sensing 已完成；未据此补造历史日志或单独的 timeout 测试记录。
- **Task 003 IN PROGRESS**：OLED 独立版与综合版源码已创建，硬件验收待用户完成；编译结果见任务文档。

- **Task 004 IN PROGRESS**：原生 LEDC 两个示例已创建，编译结果与实物待验收项目见任务文档。

当前操作与验证状态见 [Task 004](docs/tasks/TASK_004.md)。Task 002 文档保留旧环境记录，其当前状态以上述用户反馈为准。[Task 001](docs/tasks/TASK_001.md) 保留此前环境检查记录；其旧检查状态由上述用户反馈更新。

## Roadmap

严格按任务顺序推进；以下仅为方向，后续任务范围需单独确认。

1. **Task 001 PASS**：仓库初始化、ESP32 编译 / 烧录 / 串口验证。
2. **Task 002 PASS**：VL53L0X I2C 初始化、连续测距与串口输出。
3. **Task 003 IN PROGRESS**：SSD1306 OLED 文字、uptime 与共享总线距离显示。
4. **Task 004 IN PROGRESS**：SG90 PWM 自动测试与串口角度控制。
5. 后续：核对引脚、供电与接线，分阶段验证驱动和编码器。
6. 后续：闭环电机控制与 PID。
7. 后续：IMU、测距、舵机、OLED 及传感器融合。
8. 后续：Ubuntu 24.04、ROS 2 Jazzy 与 ESP32 通信。
9. 后续：USB 摄像头、OpenCV / YOLO 识别与目标跟随。

当前任务不安装 ROS 2，不实现 PID 或电机控制。
