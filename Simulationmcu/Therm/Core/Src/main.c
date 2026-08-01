/**
 * @file    main.c
 * @brief   STM32F103 ADC-to-PWM demo using CMSIS register access
 *
 * Theory:
 *   f = 1 / T
 *   PWM period T = (ARR + 1) * (PSC + 1) / SYSCLK
 *   PWM frequency f = SYSCLK / ((PSC+1) * (ARR+1))
 *   Duty cycle = CCR / (ARR+1) * 100%
 *
 * Hardware:
 *   PA0 (ADC1 CH0) → duty control pot   (0-100%)
 *   PA1 (ADC1 CH1) → frequency control pot (50 Hz - 20 kHz)
 *   PA6 (TIM3 CH1) → PWM output
 *
 * System clock: 72 MHz (HSE 8MHz x9 PLL, configured by SystemInit in startup)
 */

#include "stm32f10x.h"
#include "adc.h"
#include "pwm.h"

/* ================ Channel assignments ================ */
#define ADC_CH_DUTY     0       /* PA0 */
#define ADC_CH_FREQ     1       /* PA1 */
#define PWM_TIM         PWM_TIM3
#define PWM_CH          PWM_CH1 /* PA6 */

/* Frequency range */
#define FREQ_MIN        50      /* Hz */
#define FREQ_MAX        20000   /* Hz */

/* Simple busy-wait delay */
static void delay_ms(volatile uint32_t ms)
{
    /* At 72 MHz, ~12000 iterations ≈ 1 ms (compiler/opt dependent) */
    while (ms--) {
        volatile uint32_t cnt = 12000;
        while (cnt--);
    }
}

int main(void)
{
    /* System clock already configured by SystemInit (called in startup) */
    /* SystemCoreClock variable from CMSIS is updated by SystemCoreClockUpdate() */
    SystemCoreClockUpdate();

    /* --- Initialize ADC1 for 2 channels --- */
    /* Sample time 239.5 cycles = stable reading */
    ADC_Init(ADC_CH_DUTY, 7);
    ADC_Init(ADC_CH_FREQ, 7);

    /* --- Initialize PWM: TIM3 CH1 on PA6, 1 kHz, 0% duty --- */
    PWM_Init(PWM_TIM, PWM_CH, 1000, 0);

    /* --- Main control loop --- */
    while (1)
    {
        /* Read duty ADC → 0-100% */
        uint16_t adc_duty = ADC_Read(ADC_CH_DUTY);
        uint8_t duty = (uint8_t)((uint32_t)adc_duty * 100UL / 4096UL);

        /* Read freq ADC → FREQ_MIN..FREQ_MAX */
        uint16_t adc_freq = ADC_Read(ADC_CH_FREQ);
        uint32_t freq = FREQ_MIN + ((uint32_t)adc_freq * (FREQ_MAX - FREQ_MIN) / 4096UL);

        /* Clamp */
        if (freq < FREQ_MIN) freq = FREQ_MIN;
        if (freq > FREQ_MAX) freq = FREQ_MAX;

        /* Update PWM */
        PWM_SetFreq(PWM_TIM, freq);
        PWM_SetDuty(PWM_TIM, PWM_CH, duty);

        /* Wait briefly */
        delay_ms(10);
    }
}
