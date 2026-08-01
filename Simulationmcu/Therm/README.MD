Project refactored. 使用 Keil MDK + DFP 而非自定义寄存器定义。最终项目文件结构：

```
Core/
├── Inc/
│   ├── adc.h    — ADC 驱动头文件
│   └── pwm.h    — PWM 驱动头文件
└── Src/
    ├── adc.c    — ADC 驱动 (使用 ADC1->SQR3, ADC_CR2_SWSTART 等 DFP 定义)
    ├── pwm.c    — PWM 驱动 (使用 TIM_CR1_CEN, TIM_EGR_UG 等 DFP 定义)
    └── main.c   — 主程序
Therm.uvprojx    — Keil 项目 (已更新包含路径, 移除冲突文件)
```

**核心功能说明：**
- **PA0 (ADC1 CH0)** → 占空比控制 (0-100%)
- **PA1 (ADC1 CH1)** → 频率控制 (50 Hz - 20 kHz)
- **PA6 (TIM3 CH1)** → PWM 输出
- 公式：**f = 1/T**，T = (ARR+1)×(PSC+1)/72MHz

**注意：** 编译前确认：
1. RTE 管理器中已勾选 CMSIS→CORE 和 Device→GPIO (已在 uvprojx 中配置)
2. 启动文件由 DFP 自动提供 (`$$Device:STM32F103C8$Device\Source\ARM\startup_stm32f10x_md.s`)
3. 系统时钟由 SystemInit (启动文件调用) 配置为 72 MHz
4. 包含路径改为项目本地 `.\Core\Inc`，`stm32f10x.h` 由 DFP 的 $$Device 路径提供
