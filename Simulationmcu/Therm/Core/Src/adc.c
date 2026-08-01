/**
 * @file    adc.c
 * @brief   ADC driver for STM32F103 using CMSIS (DFP) register definitions
 *
 * Note: stm32f10x.h provided by Keil.STM32F1xx_DFP pack.
 */

#include "adc.h"
#include "stm32f10x.h"

void ADC_Init(uint8_t channel, uint8_t sample_cycles)
{
    uint32_t tmp;

    /* Enable clocks for ADC1 and GPIOA */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_ADC1EN;

    /* Configure GPIO pin as analog input */
    if (channel <= 7) {
        tmp = GPIOA->CRL;
        tmp &= ~((uint32_t)0x0F << (4 * channel));
        tmp |= ((uint32_t)0x00) << (4 * channel);  /* CNF=analog, MODE=input */
        GPIOA->CRL = tmp;
    } else if (channel <= 9) {
        tmp = GPIOB->CRL;
        tmp &= ~((uint32_t)0x0F << (4 * (channel - 8)));
        tmp |= ((uint32_t)0x00) << (4 * (channel - 8));
        GPIOB->CRL = tmp;
    }

    /* Power-on ADC */
    ADC1->CR2 |= ADC_CR2_ADON;

    /* Brief delay for stabilization */
    {
        volatile uint32_t dly = 500;
        while (dly--);
    }

    /* Reset calibration */
    ADC1->CR2 |= ADC_CR2_RSTCAL;
    while (ADC1->CR2 & ADC_CR2_RSTCAL);

    /* Calibrate */
    ADC1->CR2 |= ADC_CR2_CAL;
    while (ADC1->CR2 & ADC_CR2_CAL);

    /* Set sample time for selected channel */
    if (channel <= 9) {
        tmp = ADC1->SMPR2;
        tmp &= ~((uint32_t)0x07 << (3 * channel));
        tmp |= ((uint32_t)(sample_cycles & 0x07)) << (3 * channel);
        ADC1->SMPR2 = tmp;
    } else {
        tmp = ADC1->SMPR1;
        tmp &= ~((uint32_t)0x07 << (3 * (channel - 10)));
        tmp |= ((uint32_t)(sample_cycles & 0x07)) << (3 * (channel - 10));
        ADC1->SMPR1 = tmp;
    }

    /* Disable continuous mode, right-aligned data */
    ADC1->CR2 &= ~(ADC_CR2_CONT | ADC_CR2_ALIGN);

    /* Set regular sequence length = 1 conversion */
    ADC1->SQR1 &= ~((uint32_t)0x0F << 20);  /* Clear L[3:0] */
    /* L=0 means 1 conversion, already cleared */
}

uint16_t ADC_Read(uint8_t channel)
{
    /* Set channel in SQ1 of regular sequence */
    uint32_t sqr = ADC1->SQR3;          /* SQR3 contains SQ1..SQ6 */
    sqr &= ~((uint32_t)0x1F << 0);      /* Clear SQ1 field */
    sqr |= ((uint32_t)(channel & 0x1F)) << 0;
    ADC1->SQR3 = sqr;

    /* Start conversion */
    ADC1->CR2 |= ADC_CR2_SWSTART;

    /* Wait for end of conversion flag */
    while (!(ADC1->SR & ADC_SR_EOC));

    /* Return 12-bit value (reading DR clears EOC) */
    return (uint16_t)(ADC1->DR & 0x0FFF);
}

uint16_t ADC_Read_mV(uint8_t channel)
{
    uint16_t adc_val = ADC_Read(channel);
    return (uint16_t)(((uint32_t)adc_val * 3300UL) / 4096UL);
}
