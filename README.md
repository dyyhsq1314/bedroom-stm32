# bedroom-stm32

基于 **STM32F103C8T6 + FreeRTOS** 的智能卧室项目：语音控制、温湿度采集、OLED 显示、蓝牙上报、红外人体检测。

![MCU](https://img.shields.io/badge/MCU-STM32F103C8T6-blue)
![RTOS](https://img.shields.io/badge/RTOS-FreeRTOS-green)
![IDE](https://img.shields.io/badge/IDE-Keil%20MDK5-orange)

---

## 功能概览

| 功能 | 说明 | 相关代码 |
|---|---|---|
| **语音控制** | 识别 `LED_ON` / `LED_OFF` / `READ_TEMP` 指令，控制蓝色 LED 或回报温湿度 | `Drivers/Bedroom/asr.c` |
| **温湿度采集** | DHT11 单总线，每 2 秒采样一次 | `Drivers/Bedroom/DTH11.c` |
| **OLED 显示** | 0.96" SSD1306 (I2C)，两行显示温度/湿度 | `Drivers/Bedroom/OLED.c` |
| **蓝牙上报** | JDY 模块串口输出 `Temp:26.5 and Humi:60.0 %` | `Drivers/Bedroom/jdy.c` |
| **红外检测** | 人体感应输入，联动 LED | `Core/Src/freertos.c` |
| **LED 控制** | 板载 LED + 蓝色 LED 独立控制 | `Drivers/Bedroom/led.c` |

> **关于 MQTT**：`Drivers/Pahomqtt/` 是 Eclipse Paho MQTT Embedded-C 客户端源码，
> **已随仓库提供但尚未接入当前工程**（未加入 Keil 编译列表、无调用点）。保留是为了后续接入时无需重新移植。

---

## 硬件

| 器件 | 型号 / 说明 |
|---|---|
| 主控 | STM32F103C8T6（64 KB Flash / 20 KB RAM，Cortex-M3） |
| 温湿度 | DHT11 |
| 显示 | 0.96 寸 SSD1306 OLED，I2C 接口 |
| 语音识别 | ASR 语音模块（串口输出识别结果字符串） |
| 蓝牙 | JDY 系列串口蓝牙模块 |
| 人体感应 | 红外热释电模块 |

## 引脚分配

| 功能 | 外设 | 引脚 | 备注 |
|---|---|---|---|
| 温湿度 | DHT11 | **PB0** | 单总线，需上拉 |
| OLED | I2C1 | **PB6 (SCL) / PB7 (SDA)** | 硬件 I2C |
| 语音模块 | USART2 | **PA2 (TX) / PA3 (RX)** | 115200 8N1，空闲中断接收 |
| 蓝牙模块 | USART1 | **PA9 (TX) / PA10 (RX)** | 9600 8N1 |
| 红外人体感应 | GPIO | **PA0** | 输入，上拉 |
| 蓝色 LED | GPIO | **PA8** | 推挽输出 |
| 板载 LED | GPIO | **PC13** | 低电平点亮 |

---

## 软件架构

基于 FreeRTOS（CMSIS-RTOS v2 封装），5 个任务：

| 任务 | 优先级 | 栈 | 周期 | 职责 |
|---|---|---|---|---|
| `Sensor` | 5 | 256 | 2 s | 采集 DHT11，写入共享 `current_data` |
| `Display` | 4 | 256 | 1 s | 刷新 OLED（持 OLED 互斥锁） |
| `Infrared` | 4 | 256 | 50 ms | 轮询红外输入，联动 LED |
| `Bluetooth` | 4 | 256 | 1.5 s | 串口上报温湿度 |
| `ASR` | 4 | 300 | 事件 | 空闲中断收到语音指令后执行动作 |

关键设计：
- **OLED 互斥锁**（`xOLEDMutex`）—— 多个任务共享 I2C 显示屏，避免总线竞争
- **UART 空闲中断**（`HAL_UARTEx_ReceiveToIdle_IT`）—— 按整帧收语音模块输出，不用逐字节拼包

## 目录结构

```
bedroom-stm32/
├── Core/                  CubeMX 生成 + 应用层（main / freertos / rtc / tim / usart …）
├── Drivers/
│   ├── Bedroom/           ★ 本项目自研驱动
│   │   ├── asr.c          语音指令解析
│   │   ├── DTH11.c        DHT11 驱动（含软件时序）
│   │   ├── OLED.c         SSD1306 驱动 + 字库
│   │   ├── jdy.c          蓝牙串口发送
│   │   └── led.c          LED 控制
│   ├── Pahomqtt/          Paho MQTT 客户端（预留，未接入）
│   ├── STM32F1xx_HAL_Driver/   ST HAL 库
│   └── CMSIS/             ARM CMSIS 头文件与启动文件
├── Middlewares/
│   └── Third_Party/FreeRTOS/   FreeRTOS 内核
├── MDK-ARM/
│   ├── 20250614.uvprojx   ★ Keil 工程文件（双击打开）
│   ├── 20250614.uvoptx    Keil 调试配置
│   ├── startup_stm32f103xb.s
│   ├── RTE/ DebugConfig/
│   └── firmware/
│       ├── 20250614.hex   已编译固件（可直接烧录）
│       └── 20250614.bin
├── docs/
│   ├── Bedroom.pptx       项目汇报
│   ├── 滤波算法.txt        十大经典滤波算法笔记
│   ├── freertos低功耗.txt  Tickless Idle 源码分析
│   └── freertos.eddx
├── 20250614.ioc           CubeMX 工程配置
└── .mxproject
```

---

## 编译与烧录

### 环境要求

- **Keil MDK-ARM 5.x**（工程基于 AC5/AC6 编译器）
- 对应器件包：`Keil.STM32F1xx_DFP`（需含 STM32F103C8 支持）
- 下载器：ST-Link / DAP-Link / J-Link 任一

### 步骤

1. 下载本仓库（`Code → Download ZIP`）并解压
2. 双击 `MDK-ARM/20250614.uvprojx` 打开工程
3. `Project → Build Target`（快捷键 **F7**）编译
4. 连接 ST-Link，`Flash → Download`（**F8**）烧录

> **不想自己编译？** 直接烧 `MDK-ARM/firmware/20250614.hex`（ST-Link Utility / STM32CubeProgrammer / FlyMcu 均可）。

### 用 CubeMX 重新生成

打开根目录 `20250614.ioc`，修改外设后 `GENERATE CODE`。注意：CubeMX 会覆盖 `Core/Src/main.c`、`usart.c` 等文件，**只保留 `USER CODE BEGIN/END` 之间的内容**；`Drivers/Bedroom/` 下的是自己写的，不受影响。

---

## 语音指令协议

ASR 模块通过 USART2 发送 ASCII 指令，固件用 `strstr` 做包含匹配：

| 指令 | 动作 | 反馈（蓝牙） |
|---|---|---|
| `LED_ON` | PA8 置高（蓝灯亮） | `LED_ON` |
| `LED_OFF` | PA8 置低（蓝灯灭） | `LED_OFF` |
| `READ_TEMP` | 回传当前温湿度 | 无（走 USART2 回传） |

新增指令：在 `Drivers/Bedroom/asr.c` 的 `asr_exec()` 里加一个 `else if` 分支即可。

---

## 已知情况

- `Drivers/Bedroom/Key.c` 是空文件（预留按键功能，未实现）
- PahoMQTT 未接入（见上文）
- 仓库已剔除 CMSIS 的 `Lib/`（ARM 数学库二进制，48 MB）与 `DSP/` 示例：Keil 工程既未链接也未引用，**不影响编译**
- 原工程的 Keil 用户界面布局文件（`.uvguix.*`）未上传，首次打开窗口布局会是默认值

## 许可

STM32 HAL / CMSIS / FreeRTOS / Paho MQTT 均遵循各自上游开源许可（见对应目录下的 LICENSE 文件），本项目自研代码（`Core/` 应用层与 `Drivers/Bedroom/`）供学习交流使用。
