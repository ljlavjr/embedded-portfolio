#include "stm32f407.h"
#include "gpio.h"

void rcc_gpio_clock_enable(GPIO_TypeDef *port) {
    uint32_t offset = ((uint32_t)port - (uint32_t)GPIOA) / 0x400;
    RCC->AHB1ENR |= (1 << offset);
}

void gpio_init(GPIO_TypeDef *port, uint8_t pin, GPIO_MODE mode) {
    // Need to clear bits firsts then change. Standard for multi-bit fields
    port->MODER &= ~(0x3 << (pin * 2));
    port->MODER |= (mode << (pin * 2));
}

void gpio_set_otype(GPIO_TypeDef *port, uint8_t pin, GPIO_OUTPUT_TYPE otype) {
    port->OTYPER &= ~(1 << pin);
    port->OTYPER |= (otype << pin);
}

void gpio_set_speed(GPIO_TypeDef *port, uint8_t pin, GPIO_SPEED speed) {
    port->OSPEEDR &= ~(0x3 << (pin * 2));
    port->OSPEEDR |= (speed << (pin * 2));
}

void gpio_set_pull(GPIO_TypeDef *port, uint8_t pin, GPIO_PULL_UD pull) {
    port->PUPDR &= ~(0x3 << (pin * 2));
    port->PUPDR |= (pull << (pin * 2));
}

void write_pin(GPIO_TypeDef *port, uint8_t pin, uint32_t value) {
    // We use BSRR bc it does not need read-modify-write cycle.
    // Each bit is an action not state. Atmoic and preferred over ODR
    if (value == 1) {
        port->BSRR = (1 << pin);
    }
    if (value == 0) {
        port->BSRR = (1 << (pin + 16));
    }
}

uint8_t read_pin(GPIO_TypeDef *port, uint8_t pin) {
    return (port->IDR >> pin) & 0x1;
}

void toggle_pin(GPIO_TypeDef *port, uint8_t pin) {
    port->ODR ^= (1 << pin);
}