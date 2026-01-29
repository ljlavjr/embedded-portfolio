#include "uart.h"
#include <avr/io.h>
#include <string.h>

// Main function
int main(void) {
    char buffer[32];

    DDRB |= (1 << PB7);
    
    uart_init(9600);
    uart_print("Hello from ATmega2560!\r\n");
    
    while (1) {
        uart_print("> ");
        uart_readline(buffer, 32);

        if (strcmp(buffer, "led on") == 0) {
            PORTB |= (1 << PB7);
            uart_print("LED on\r\n");
        }
        else if (strcmp(buffer, "led off") == 0) {
            PORTB &= ~(1 << PB7);
            uart_print("LED off\r\n");
        }
        else if (strcmp(buffer, "status") == 0) {
            if (PORTB & (1 << PB7)) {
                uart_print("LED is on\r\n");
            }
            else {
                uart_print("LED is off\r\n");
            }
        }
        else {
            uart_print("Unknown commmand\r\n");
        }
    }
    
    return 0;
}