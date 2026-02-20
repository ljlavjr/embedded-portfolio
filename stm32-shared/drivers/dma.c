#include "stm32f407.h"
#include "dma.h"

void dma_init(uint16_t *buffer0, uint16_t *buffer1, uint16_t size) {
    // Enable DMA2 clock
    RCC->AHB1ENR |= (1 << 22);

    // Disable stream before configuring
    DMA2_Stream0->CR &= ~(1 << 0);
    while (DMA2_Stream0->CR & (1 << 0));  // Wait until disabled

    // Set number of transfers
    DMA2_Stream0->NDTR = size;

    // Set peripheral address (ADC1 data register)
    DMA2_Stream0->PAR = (uint32_t)&ADC1->DR;

    // Set memory addresses (double buffer)
    DMA2_Stream0->M0AR = (uint32_t)buffer0;
    DMA2_Stream0->M1AR = (uint32_t)buffer1;

    // Configure CR:
    // CHSEL = 0 (Channel 0 for ADC1)
    // MSIZE = 01 (16-bit memory)
    // PSIZE = 01 (16-bit peripheral)
    // MINC = 1 (memory increment)
    // CIRC = 1 (circular mode)
    // DIR = 00 (peripheral to memory)
    // DBM = 1 (double buffer mode)
    DMA2_Stream0->CR = (0b000 << 25)   // CHSEL: Channel 0
                     | (0b01 << 13)    // MSIZE: 16-bit
                     | (0b01 << 11)    // PSIZE: 16-bit
                     | (1 << 10)       // MINC: increment memory
                     | (1 << 8)        // CIRC: circular mode
                     | (0b00 << 6)     // DIR: peripheral to memory
                     | (1 << 18);      // DBM: double buffer

    // Don't enable yet - dma_start() will do that
}

void dma_start() {
    // Enable the stream (bit 0)
    DMA2_Stream0->CR |= (1 << 0);
}