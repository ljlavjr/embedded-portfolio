
/*
    Registers used, their address, and purpose

    RCC_AHB1ENR | 0x40023830 | Enable GPIOD clock (bit 3)
    GPIOD_MODER | 0x40020C00 | Set pin mode (2 bits per pin, 01 = output)
    GPIOD_ODR   | 0x40020C14 | Output data (toggle pins here)
*/

/*
    Defining registers:
    - 0x40023830:                           The address
    - (unsigned int *)0x40023830:           Cast address to pointer
    - (volatile unsigned int *)0x40023830:  Make it volatile
    - *(volatile unsigned int *)0x40023830: Dereference so you can read/write the value
*/

#define RCC_AHB1ENR (*(volatile unsigned int *)0x40023830)
#define GPIOD_MODER (*(volatile unsigned int *)0x40020C00)
#define GPIOD_ODR   (*(volatile unsigned int *)0x40020C14)

#define LED_PIN 12 /* PD12 = Green LED */

void delay(volatile unsigned int count) {
    while (count--);
}

int main(void) {
    // Enable the GPIO clock
    RCC_AHB1ENR |= (1 << 3);

    // Set pin mode to output (01)
    GPIOD_MODER |= (1 << 24);

    // Main loop. Toggle LED
    while (1) {
        GPIOD_ODR ^= (1 << LED_PIN);
        delay(50000);
    }
    return 0;
}