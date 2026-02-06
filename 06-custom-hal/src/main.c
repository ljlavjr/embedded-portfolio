#include "gpio.h"
#include "uart.h"

void delay(volatile uint32_t count);

int main(void) {
    // Initialize LED
    rcc_gpio_clock_enable(GPIOD);
    gpio_init(GPIOD, PD14, GPIO_MODE_OUTPUT);

    // Initialize UART at 9600 baud
    uart_init(9600);

    // Send startup message
    uart_write_string("Project 06: UART HAL initialized\r\n");

    while (1) {
        toggle_pin(GPIOD, PD14);
        uart_write_string("Hello from stm32!\r\n");
        delay(500000);
    }
    return 0;
}

void delay(volatile uint32_t count) {
    while (count--);
}
