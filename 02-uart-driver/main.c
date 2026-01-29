#include "uart.h"

int main(void) {
    uart_init(9600);
    uart_print("Hello from ATmega2560!\r\n");
    uart_print("Type something:\r\n");
    while (1) {
        char c = uart_receive();
        uart_transmit(c);
    }

    return 0;
} 