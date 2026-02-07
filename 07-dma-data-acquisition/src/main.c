#include "gpio.h"
#include "uart.h"
#include "adc.h"
#include <stdint.h>

void delay(volatile uint32_t count);


int main(void) {
    // Initialize UART for debug
    uart_init(9600);
    uart_write_string("ADC Test");

    // Configure PA0 as analog input
    gpio_init(GPIOA, 0, GPIO_MODE_AN);

    // Initialize ADC
    adc_init();

    while(1) {
        uint16_t value = adc_read(0);

        uart_write_string("ADC: ");
        uart_write_hex((value >> 8) & 0xFF);
        uart_write_hex(value & 0xFF);
        uart_write_string("\r\n");

        delay(1000000);
    }
    return 0;
}

void delay(volatile uint32_t count) {
    while (count--);
}