#include "uart.h"
#include "gpio.h"

#define RE 2    // Receiver enable
#define TE 3    // Transmitter enable
#define RXNE 5   // Receiver data register empty
#define TXE 7   // Transmit data register empty
#define UE 13   // USART enable

void uart_init(uint32_t baud_rate) {
    rcc_gpio_clock_enable(GPIOA);
    gpio_init(GPIOA, PA2, GPIO_MODE_ALT);
    gpio_init(GPIOA, PA3, GPIO_MODE_ALT);
    gpio_set_alt_func(GPIOA, PA2, 0x7);   // USART2 alt function code = 0111 (AF7)
    gpio_set_alt_func(GPIOA, PA3, 0x7);   // USART2 alt function code = 0111 (AF7)
    RCC->APB1ENR |= (1 << 17);  // Bit 17: USART2 clock enable bit
    USART2->BRR = (16000000 + (baud_rate / 2U)) / baud_rate;
    USART2->CR1 = (1 << UE) | (1 << TE) | (1 << RE);  // Bits 2, 3, 13: RE(Receiver Enable), TE(Transmitter Enable), UE(USART Enable)
}

void uart_write_char(char c) {
    while (!(USART2->SR & (1 << TXE)));   // Wait while TXE == 0
    USART2->DR = c;
}

void uart_write_string(const char *str) {
    while (*str) {
        uart_write_char(*str++);
    }
}

char uart_read_char(void) {
    while (!(USART2->SR & (1 << RXNE))); // If nothing to read wait.
    return USART2->DR;
}
