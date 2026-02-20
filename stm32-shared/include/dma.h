#ifndef DMA_H
#define DMA_H
#include "stm32f407.h"

void dma_init(uint16_t *buffer0, uint16_t *buffer1, uint16_t size);
void dma_start(void);

#endif