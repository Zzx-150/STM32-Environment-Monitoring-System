# STM32-Environment-Monitoring-System
STM32F103 based environmental monitoring system

# STM32 环境监测与自动控制系统

基于 STM32F103C8T6 的环境监测与自动控制系统，包含温湿度采集、电压采集、OLED显示、按键交互、继电器报警等功能。

## 硬件平台
- MCU: STM32F103C8T6
- 温湿度: DHT11 (PA3)
- ADC: PA0 (电位器)
- OLED: PB8/PB9 (软件I2C)
- 按键: PB10 (外部中断)
- 继电器: PA1
- 串口: PA9/PA10

## 功能
- [x] DHT11 温湿度采集（单总线协议）
- [x] ADC 电压采集（DMA方式）
- [x] OLED 多界面显示（4界面切换）
- [x] 按键切换界面（外部中断）
- [x] 温度/电压双阈值报警
- [x] 继电器自动控制
- [x] 串口数据上报（printf重定向）
- [x] 滑动平均滤波
- [ ] ESP8266 WiFi 远程通信（驱动已写，硬件调试中）
- [ ] W25Q64 Flash 存储（驱动已写，未接入主程序）

## 代码结构
├── driver/          # 硬件驱动层
│   ├── dht11.c/h
│   ├── adc.c/h
│   ├── oled.c/h
│   ├── key.c/h
│   ├── usart.c/h
│   ├── relay.c/h
│   ├── esp8266.c/h
│   ├── w25q64.c/h
│   └── Delay.c/h
├── app/             # 应用层
│   ├── main.c
│   ├── data_process.c/h
│   └── display_ui.c/h
└── README.md
