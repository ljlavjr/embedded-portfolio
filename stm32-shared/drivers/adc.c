#include "stm32f407.h"
#include "adc.h"

void adc_init(void) {
    // Enable ADC1 Clock
    RCC->APB2ENR |= (1 << 8);

    // Set Prescaler. ADC_CCR bits 17:16 = ADCPRE (prescaler)
    // 00 = /2, 01 = /4, 10 = /6, 11 = /8
    // Enable internal temperature sensor. ADC_CCR bit 23 = TSVREFE
    ADC_COMMON->CCR |= (0b01 << 16) | (1 << 23);

    // Configure Sample Time for channel 0 (bits 2:0 in SMPR2)
    // 0b011 = 56 cycles (reasonable default)
    // Configure Sample Time for channel 16 (bits 20:18 in SMPR1)
    // 0b111 = 480 cycles (temp sensor needs a longer sample time)
    ADC1->SMPR2 |= (0b011 << 0);
    ADC1->SMPR1 |= (0b111 << 18);

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