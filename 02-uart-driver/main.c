#include "uart.h"

int main(void) {
    uart_init(9600);
    uart_print("Hello from ATmega2560!\r\n");

    while (1) {}
    
    return 0;
} 