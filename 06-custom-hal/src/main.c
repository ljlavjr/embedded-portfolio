#include "gpio.h"

#define PD12 12 // Green LED
#define PD13 13 // Orange LED
#define PD14 14 // Red LED
#define PD15 15 // Blue LED

void delay(volatile uint32_t count);

int main(void) {
    rcc_gpio_clock_enable(GPIOD);
    gpio_init(GPIOD, PD14, GPIO_MODE_OUTPUT);
    while (1) {
        toggle_pin(GPIOD, PD14);
        delay(500000);
        /*
        write_pin(GPIOD, PD14, HIGH);
        delay(500000);
        write_pin(GPIOD, PD14, LOW);
        delay(500000);
        */
    }
    return 0;
}

void delay(volatile uint32_t count) {
    while (count--);
}
