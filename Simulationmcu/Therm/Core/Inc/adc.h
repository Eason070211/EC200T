/**
 * @file    adc.h
 * @brief   ADC driver for STM32F103 (using CMSIS register definitions)
 */

#ifndef __ADC_H
#define __ADC_H

#include <stdint.h>

/**
 * @brief  Initialize ADC1 for single conversion on a given channel
 * @param  channel: ADC channel (0..17). PA0=0, PA1=1, etc.
 * @param  sample_cycles: sample time (0=1.5 .. 7=239.5 cycles)
 */
void ADC_Init(uint8_t channel, uint8_t sample_cycles);

/**
 * @brief  Read ADC value (blocking, single conversion)
 * @param  channel: ADC channel
 * @return 12-bit ADC result (0-4095)
 */
uint16_t ADC_Read(uint8_t channel);

/**
 * @brief  Read ADC value in millivolts
 * @param  channel: ADC channel
 * @return Voltage in mV (0..3300)
 */
uint16_t ADC_Read_mV(uint8_t channel);

#endif /* __ADC_H */
