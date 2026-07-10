# EdgeSight — AI Edge Sentinel

端侧 AI 全感知边缘站，基于 STM32N647 MCU 实现人形检测 + 跌倒识别 + 事件录像 + 告警推送。

## 架构

```
Camera → DCMIPP ISP → ┬─ PIPE1 → LTDC 显示
                       └─ PIPE2 → NPU 推理 → 跌倒检测 → 告警/录像
```

## 文件说明

| 文件 | 功能 |
|------|------|
| `edgesight_main.c` | 主入口，pipeline 编排 |
| `fall_detect.c/h` | 基于姿态关键点的跌倒检测算法 |
| `camera_hal.h` | 摄像头 + DCMIPP 双管线抽象 |
| `npu_hal.h` | NPU 推理引擎抽象（stedgeai） |
| `display_hal.h` | LTDC + GPU2D 显示抽象 |
| `recorder_hal.h` | H.264 编码 + SD 卡录像抽象 |
| `network_hal.h` | Ethernet + MQTT 网络告警抽象 |

## AI 模型

| 模型 | 用途 | 输入 | 格式 |
|------|------|------|------|
| st_yolo_x_nano | 人形检测 | 480×480 RGB | INT8 tflite |
| st_movenet_lightning | 姿态估计 | 192×192 RGB | INT8 tflite |

跌倒判定基于规则引擎（躯干角度 + 重心高度 + 包围框比例），无需额外模型。

## 构建

在 defconfig 中启用：
```
CONFIG_APP_EDGESIGHT=y
```

## 依赖

- NuttX RTOS
- STM32N6 HAL Driver
- stedgeai-lib (NPU runtime)
- h264encapi (VENC encoder)
- lwIP (网络协议栈)
