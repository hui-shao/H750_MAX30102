# H750_MAX30102

使用 STM32H750XBH6 驱动 MAX30102 心率血氧模块，由于采用 HAL库，因此可以方便地移植。

### 工程说明

连线方式请参照 CubeMX 图示。其中 I2C1 用于和 MAX30102 通讯，I2C2 用于连接 128*64 的 OLED 屏幕。

如需移植：请特别留意 CubeMX 中 NVIC、GPIO，以及堆栈大小的设置。

### 相关文档

在 [Docs](./Docs) 目录内，来自官方。

分别包含了芯片使用方式，数据计算原理等。

### 开源代码引用

项目使用了如下开源代码，部分代码有些许改动。

[MAX30102_by_RF](https://github.com/aromring/MAX30102_by_RF)

[MAX30102_for_STM32_HAL](https://github.com/eepj/MAX30102_for_STM32_HAL)