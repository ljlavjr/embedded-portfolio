#ifndef ADC_H
#define ADC_H

#include "stm32f407.h"
#include <stdint.h>


#define ADON 0
#define SWSTART 30

void adc_init(void);
uint16_t adc_read(uint8_t channel);

#endif