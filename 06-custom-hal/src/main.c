#include "gpio.h"
#include "uart.h"
#include "spi.h"

void delay(volatile uint32_t count);

int main(void) {
    // Initialize LED
    rcc_gpio_clock_enable(GPIOD);
    gpio_init(GPIOD, PD14, GPIO_MODE_OUTPUT);

    // Initialize UART for debug output
    uart_init(9600);
    uart_write_string("SPI Loopback Test\r\n");

    // Initialize SPI
    spi_init();

    // Test values
    uint8_t test_values[] = {0xA5, 0x5A, 0xFF, 0x00, 0x42};

    for (int i = 0; i < 5; i ++) {
        uint8_t sent = test_values[i];
        uint8_t received = spi_transfer(sent);

        uart_write_string("Sent: 0x");
        uart_write_hex(sent);
        uart_write_string(" Received: 0x");
        uart_write_hex(received);

        if (sent == received) {
            uart_write_string(" OK\r\n");
        }
        else {
            uart_write_string(" FAIL\r\n");
        }

        delay(900000);
    }

    uart_write_string("Test complete\r\n");

    while (1) {
        toggle_pin(GPIOD, PD14);
        delay(500000);
    }
    return 0;
}

void delay(volatile uint32_t count) {
    while (count--);
}
