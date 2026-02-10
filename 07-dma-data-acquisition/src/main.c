#include "gpio.h"
#include "uart.h"
#include "adc.h"
#include "dma.h"
#include <stdint.h>

#define BUFFER_SIZE 16

uint16_t buffer0[BUFFER_SIZE];
uint16_t buffer1[BUFFER_SIZE];
volatile uint8_t buffer_ready = 0;

void delay(volatile uint32_t count);

int main(void) {
    // Initialize UART for debug
    uart_init(9600);
    uart_write_string("ADC Test");

    // Configure PA0 as analog input
    gpio_init(GPIOA, 0, GPIO_MODE_AN);

    // Initialize DMA first
    dma_init(buffer0, buffer1, BUFFER_SIZE);

    // Initialize ADC for DMA mode
    adc_init_dma(0);

    // Start DMA
    dma_start();

    // Start ADC conversions
    adc_start();

    while(1) {
        delay(1000000);

        // Print first few values from buffer0
        uart_write_string("B0: ");
        for (int i = 0; i < 4; i++) {
            uart_write_hex((buffer0[i] >> 8) & 0xFF);
            uart_write_hex(buffer0[i] & 0xFF);
            uart_write_string(" ");
        }
        uart_write_string("\r\n");
    }
    return 0;
}

void delay(volatile uint32_t count) {
    while (count--);
}