/**
 * @file    pwm.h
 * @brief   PWM driver for STM32F103 (using CMSIS register definitions)
 */

#ifndef __PWM_H
#define __PWM_H

#include <stdint.h>

/** Timer selection */
typedef enum {
    PWM_TIM2 = 2,   /* 32-bit timer */
    PWM_TIM3 = 3,   /* 16-bit timer */
    PWM_TIM4 = 4    /* 16-bit timer */
} PWM_Timer;

/** Channel selection */
typedef enum {
    PWM_CH1 = 1,
    PWM_CH2 = 2,
    PWM_CH3 = 3,
    PWM_CH4 = 4
} PWM_Channel;

/**
 * @brief  Initialize timer channel for PWM output
 * @param  tim:   Timer (PWM_TIM2/3/4)
 * @param  ch:    Channel (PWM_CH1..CH4)
 * @param  freq:  PWM frequency in Hz
 * @param  duty:  Initial duty cycle 0-100%
 */
void PWM_Init(PWM_Timer tim, PWM_Channel ch, uint32_t freq, uint8_t duty);

/**
 * @brief  Set PWM frequency
 * @param  tim:  Timer
 * @param  freq: Frequency in Hz
 */
void PWM_SetFreq(PWM_Timer tim, uint32_t freq);

/**
 * @brief  Set PWM duty cycle
 * @param  tim: Timer
 * @param  ch:  Channel
 * @param  duty: 0-100%
 */
void PWM_SetDuty(PWM_Timer tim, PWM_Channel ch, uint8_t duty);

#endif /* __PWM_H */
