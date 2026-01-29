#include "uart.h"
#include <avr/io.h>

// Main function
int main(void) {
    
    uart_init(9600);
    uart_print("Hello from ATmega2560!\r\n");
    uart_print("Type something:\r\n");
    
   while (1) {
    char c = uart_receive();
    uart_print("Got: ");
    uart_transmit(c);
    uart_print("\r\n");
}
    
    return 0;
}