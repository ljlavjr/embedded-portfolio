#include "uart.h"
#include <avr/io.h>

int main(void) {
    DDRB |= (1 << PB7);  // LED as output
    
    uart_init(9600);
    uart_print("Hello from ATmega2560!\r\n");
    uart_print("Type something:\r\n");
    
    while (1) {
        char c = uart_receive();
        uart_transmit(c);
        PORTB ^= (1 << PB7);  // toggle LED on each character
    }
    
    return 0;
}