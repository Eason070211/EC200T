/**
 * @file    pwm.c
 * @brief   PWM driver for STM32F103 using CMSIS (DFP) register definitions
 */

#include "pwm.h"
#include "stm32f10x.h"

/* System clock: 72 MHz (HSE 8MHz x9 PLL) */
#define SYSCLK_HZ       72000000UL

/* Timer tick: 1 MHz (1 us resolution) */
#define TIMER_TICK_HZ   1000000UL

/* Internal per-timer state */
typedef struct {
    uint32_t psc;
    uint32_t arr;
    uint16_t ccr[4];
} timer_state_t;

static timer_state_t tim_state[4];  /* indices 2=TIM2, 3=TIM3, 4=TIM4; [0,1] unused */

static inline TIM_TypeDef *get_tim_ptr(PWM_Timer tim)
{
    switch (tim) {
        case PWM_TIM2: return TIM2;
        case PWM_TIM3: return TIM3;
        case PWM_TIM4: return TIM4;
        default:       return (TIM_TypeDef *)0;
    }
}

/* GPIO mapping for timer channel outputs:
 * TIM2: CH1=PA0, CH2=PA1, CH3=PA2, CH4=PA3
 * TIM3: CH1=PA6, CH2=PA7, CH3=PB0, CH4=PB1
 * TIM4: CH1=PB6, CH2=PB7, CH3=PB8, CH4=PB9
 */
static const struct {
    GPIO_TypeDef *port;
    uint32_t      pin;
} chan_map[] = {
    { GPIOA, 0 }, { GPIOA, 1 }, { GPIOA, 2 }, { GPIOA, 3 },  /* TIM2 CH1..CH4 */
    { GPIOA, 6 }, { GPIOA, 7 }, { GPIOB, 0 }, { GPIOB, 1 },  /* TIM3 CH1..CH4 */
    { GPIOB, 6 }, { GPIOB, 7 }, { GPIOB, 8 }, { GPIOB, 9 }   /* TIM4 CH1..CH4 */
};

static uint8_t map_idx(PWM_Timer tim, PWM_Channel ch)
{
    return (uint8_t)(((tim - PWM_TIM2) * 4) + (ch - PWM_CH1));
}

/* Helper: configure GPIO as alternate function push-pull */
static void gpio_cfg_af_pp(GPIO_TypeDef *port, uint32_t pin)
{
    volatile uint32_t *reg;
    uint32_t shift;

    if (pin < 8) {
        reg = &port->CRL;
        shift = pin * 4;
    } else {
        reg = &port->CRH;
        shift = (pin - 8) * 4;
    }

    uint32_t val = *reg;
    val &= ~((uint32_t)0x0F << shift);
    val |= ((uint32_t)0x0B) << shift;  /* CNF=10(AltFunc PP), MODE=11(50MHz) */
    *reg = val;
}

void PWM_Init(PWM_Timer tim, PWM_Channel ch, uint32_t freq, uint8_t duty)
{
    TIM_TypeDef *TIMx = get_tim_ptr(tim);
    uint8_t idx = map_idx(tim, ch);
    GPIO_TypeDef *port = chan_map[idx].port;
    uint32_t pin = chan_map[idx].pin;
    uint8_t ti = (uint8_t)tim;

    if (!TIMx) return;
    if (freq < 1) freq = 1;
    if (duty > 100) duty = 100;

    /* Enable timer clock */
    if (tim == PWM_TIM2)      RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    else if (tim == PWM_TIM3) RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    else if (tim == PWM_TIM4) RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;

    /* Enable GPIO clock */
    if (port == GPIOA)      RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    else if (port == GPIOB) RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    else if (port == GPIOC) RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;

    /* Configure GPIO as alternate function push-pull */
    gpio_cfg_af_pp(port, pin);

    /* Disable timer during configuration */
    TIMx->CR1 &= ~TIM_CR1_CEN;

    /* Calculate PSC and ARR for desired frequency */
    /* freq = SYSCLK / ((PSC+1)*(ARR+1)) */
    /* PSC fixed for 1 MHz tick */
    tim_state[ti].psc = (SYSCLK_HZ / TIMER_TICK_HZ) - 1;
    tim_state[ti].arr = (TIMER_TICK_HZ / freq) - 1;

    if (tim != PWM_TIM2) {
        if (tim_state[ti].arr > 0xFFFF) tim_state[ti].arr = 0xFFFF;
    }

    TIMx->PSC = (uint16_t)(tim_state[ti].psc & 0xFFFF);
    TIMx->ARR = (uint16_t)(tim_state[ti].arr & 0xFFFF);

    /* Enable auto-reload preload */
    TIMx->CR1 |= TIM_CR1_ARPE;

    /* Generate update event to load prescaler */
    TIMx->EGR |= TIM_EGR_UG;

    /* Configure channel for PWM mode 1 */
    uint32_t chan_ofs = (uint32_t)ch;

    if (chan_ofs <= 2) {
        /* CCMR1: CH1 (0-7) / CH2 (8-15) */
        uint32_t ccmr1 = TIMx->CCMR1;
        if (chan_ofs == 1) {
            ccmr1 &= ~0xFFUL;
            ccmr1 |= (0x06 << 4) | (0x01 << 3);  /* OC1M=PWM1, OC1PE=1 */
        } else {
            ccmr1 &= ~(0xFFUL << 8);
            ccmr1 |= ((0x06 << 4) | (0x01 << 3)) << 8;
        }
        TIMx->CCMR1 = ccmr1;
    } else {
        /* CCMR2: CH3 (0-7) / CH4 (8-15) */
        uint32_t ccmr2 = TIMx->CCMR2;
        if (chan_ofs == 3) {
            ccmr2 &= ~0xFFUL;
            ccmr2 |= (0x06 << 4) | (0x01 << 3);
        } else {
            ccmr2 &= ~(0xFFUL << 8);
            ccmr2 |= ((0x06 << 4) | (0x01 << 3)) << 8;
        }
        TIMx->CCMR2 = ccmr2;
    }

    /* Enable channel output, active high */
    TIMx->CCER &= ~(0x0FUL << (4 * (chan_ofs - 1)));
    TIMx->CCER |= (1UL << (4 * (chan_ofs - 1)));      /* CCxE = 1 */

    /* Set initial duty */
    uint32_t ccr_val = (tim_state[ti].arr * duty) / 100;
    tim_state[ti].ccr[chan_ofs - 1] = (uint16_t)ccr_val;
    *(&TIMx->CCR1 + (chan_ofs - 1)) = (uint16_t)(ccr_val & 0xFFFF);

    /* Enable counter */
    TIMx->CR1 |= TIM_CR1_CEN;
}

void PWM_SetFreq(PWM_Timer tim, uint32_t freq)
{
    TIM_TypeDef *TIMx = get_tim_ptr(tim);
    uint8_t ti = (uint8_t)tim;

    if (!TIMx || freq < 1) return;

    uint32_t new_arr = (TIMER_TICK_HZ / freq) - 1;
    if (tim != PWM_TIM2 && new_arr > 0xFFFF) new_arr = 0xFFFF;

    uint32_t old_arr = tim_state[ti].arr;
    tim_state[ti].arr = new_arr;
    TIMx->ARR = (uint16_t)(new_arr & 0xFFFF);

    /* Regenerate update */
    TIMx->EGR |= TIM_EGR_UG;

    /* Recalculate CCR values proportionally */
    for (int i = 0; i < 4; i++) {
        uint32_t new_ccr = (uint32_t)tim_state[ti].ccr[i] * (new_arr + 1) / (old_arr + 1);
        if (new_ccr > new_arr) new_ccr = new_arr;
        tim_state[ti].ccr[i] = (uint16_t)new_ccr;
        *(&TIMx->CCR1 + i) = (uint16_t)(new_ccr & 0xFFFF);
    }
}

void PWM_SetDuty(PWM_Timer tim, PWM_Channel ch, uint8_t duty)
{
    TIM_TypeDef *TIMx = get_tim_ptr(tim);
    uint8_t ti = (uint8_t)tim;

    if (!TIMx) return;
    if (duty > 100) duty = 100;

    uint32_t ccr_val = (tim_state[ti].arr * duty) / 100;
    uint8_t ci = (uint8_t)ch - 1;
    tim_state[ti].ccr[ci] = (uint16_t)ccr_val;
    *(&TIMx->CCR1 + ci) = (uint16_t)(ccr_val & 0xFFFF);
}
