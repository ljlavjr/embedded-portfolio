#include "stm32f407.h"
#include "adc.h"

void adc_init(void) {
    // Enable ADC1 Clock
    RCC->APB2ENR |= (1 << 8);

    // Set Prescaler. ADC_CCR bits 17:16 = ADCPRE (prescaler)
    // 00 = /2, 01 = /4, 10 = /6, 11 = /8
    ADC_COMMON->CCR |= (0b01 << 16);

    // Configure Sample Time for channel 0 (bits 2:0 in SMPR2)
    // 0b011 = 56 cycles (reasonable default)
    ADC1->SMPR2 |= (0b011 << 0);

    // Turn on ADC
    ADC1->CR2 |= (1 << ADON);
}

void adc_init_dma(uint8_t channel) {
    // Enable ADC1 Clock
    RCC->APB2ENR |= (1 << 8);

    // Set Prescaler
    ADC_COMMON->CCR |= (0b01 << 16);

    // Configure Sample Time
    ADC1->SMPR2 |= (0b011 << 0);

    //Select Channel
    ADC1->SQR3 = channel;

    // Enable continuous mode, enable DMA mode, DMA disable selection
    ADC1->CR2 |= (1 << 1) | (1 << 8) | (1 << 9);

    // Turn on ADC
    ADC1->CR2 |= (1 << ADON);
}

void adc_start(void) {
    ADC1->CR2 |= (1 << SWSTART);
}

uint16_t adc_read(uint8_t channel) {
    // Select Channel
    ADC1->SQR3 = channel;

    // Start Conversion
    ADC1->CR2 |= (1 << SWSTART);

    // Wait for End of Conversion
    while (!(ADC1->SR & (1 << 1)));     // Bit 1: EOC

    // Return Value
    return ADC1->DR;
}