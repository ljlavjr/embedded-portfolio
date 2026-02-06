#ifndef GPIO_H
#define GPIO_H

#include "stm32f407.h"

typedef enum {
    GPIO_MODE_INPUT = 0,      // 00
    GPIO_MODE_OUTPUT = 1,     // 01
    GPIO_MODE_ALT = 2,        // 10
    GPIO_MODE_AN = 3          // 11
} GPIO_MODE;

typedef enum {
    GPIO_OUTTYPE_PUSHPULL,
    GPIO_OUTTYPE_OPENDRAIN
} GPIO_OUTPUT_TYPE;

typedef enum {
    GPIO_SPEED_LOW,
    GPIO_SPEED_MEDIUM,
    GPIO_SPEED_HIGH,
    GPIO_SPEED_VHIGH
} GPIO_SPEED;

typedef enum {
    GPIO_PULL_UD_NOPUPD,
    GPIO_PULL_UD_PULLU,
    GPIO_PULL_UD_PULLD
} GPIO_PULL_UD;

#define HIGH 1
#define LOW 0

#define PA2 2   // GPIOA Pin 2
#define PA3 3   // GPIOA Pin 3
#define PA5 5   // GPIOA Pin 5
#define PA6 6   // GPIOA Pin 6
#define PA7 7   // GPIOA Pin 7


#define PD12 12 // GPIOD Pin 12 (Green LED)
#define PD13 13 // GPIOD Pin 13 (Orange LED)
#define PD14 14 // GPIOD Pin 14 (Red LED)
#define PD15 15 // GPIOD Pin 15 (Blue LED)

void rcc_gpio_clock_enable(GPIO_TypeDef *port);
void gpio_init(GPIO_TypeDef *port, uint8_t pin, GPIO_MODE mode);
void gpio_set_otype(GPIO_TypeDef *port, uint8_t pin, GPIO_OUTPUT_TYPE otype);
void gpio_set_speed(GPIO_TypeDef *port, uint8_t pin, GPIO_SPEED speed);
void gpio_set_pull(GPIO_TypeDef *port, uint8_t pin, GPIO_PULL_UD pull);
void gpio_set_alt_func(GPIO_TypeDef *port, uint8_t pin, uint8_t function);
void write_pin(GPIO_TypeDef *port, uint8_t pin, uint32_t value);
uint8_t read_pin(GPIO_TypeDef *port, uint8_t pin);
void toggle_pin(GPIO_TypeDef *port, uint8_t pin);

#endif