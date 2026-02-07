#include "spi.h"
#include "gpio.h"

void spi_init(void) {
    // Enable GPIOA clock
    rcc_gpio_clock_enable(GPIOA);

    // Configure PA5, PA6, PA7 as alternate function
    gpio_init(GPIOA, PA5, GPIO_MODE_ALT);
    gpio_init(GPIOA, PA6, GPIO_MODE_ALT);
    gpio_init(GPIOA, PA7, GPIO_MODE_ALT);

    // Set AF5 for PA5, PA6, PA7
    gpio_set_alt_func(GPIOA, PA5, 0x5);
    gpio_set_alt_func(GPIOA, PA6, 0x5);
    gpio_set_alt_func(GPIOA, PA7, 0x5);

    // Enable SPI1 clock
    RCC->APB2ENR |= (1 << 12);  // Bit 12: SPI1 clock enable bit

    // Configure CR1
    SPI1->CR1 = (1 << MSTR) | BR_DIV8 | (1 << SPE);
}

uint8_t spi_transfer(uint8_t data) {
    // Wait for TXE
    while (!(SPI1->SR & (1 << TXE)));

    // Write data to DR
    SPI1->DR = data;

    // Wait for RXNE
    while (!(SPI1->SR & (1 << RXNE)));

    // Return DR
    return SPI1->DR;
}  