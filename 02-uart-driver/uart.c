#include "uart.h"
#include <avr/io.h>

/*
 * ATmega2560 USART0 Registers
 *
 * UDR0 - USART Data Register
 *        Write to transmit, read to receive
 *
 * UCSR0A - Control and Status Register A
 *        Bit 7: RXC0  - Receive Complete (1 = unread data available)
 *        Bit 6: TXC0  - Transmit Complete (1 = transmission done)
 *        Bit 5: UDRE0 - Data Register Empty (1 = ready to transmit)
 *
 * UCSR0B - Control and Status Register B
 *        Bit 4: RXEN0 - Receiver Enable (1 = enable RX)
 *        Bit 3: TXEN0 - Transmitter Enable (1 = enable TX)
 *
 * UBRR0H/UBRR0L - Baud Rate Registers
 *        16-bit value that sets baud rate
 *        UBRR = (F_CPU / (16 * baud)) - 1
 */

void uart_init(uint32_t baud) {
    // Calculate the baud rate register value
    uint16_t ubrr = (F_CPU / (16UL * baud)) - 1;
    UBRR0H = (ubrr >> 8);   // high byte of UBRR0
    UBRR0L = ubrr;          // low byte of UBRR0
    // Enable TX and RX
    UCSR0B = (1 << TXEN0) | (1 << RXEN0);
} 

void uart_transmit(char c) {
    // Wait for empty transmit buffer
    while ( !(UCSR0A & (1<<UDRE0)) );

    // Put data into buffer, sends the data
    UDR0 = c;
}

char uart_receive(void) {
    // Wait for data to be received
    while( !(UCSR0A & (1<<RXC0)) );

    // Get and return received data from buffer
    return UDR0;
}

void uart_print(const char *str) {
    // Transmit string over uart
    while (*str != '\0') {
        uart_transmit(*str);
        str++;
    }
}

void uart_readline(char *buffer, uint8_t max_len) {
        uint8_t i = 0;

        while( i < max_len - 1) {
            char c = uart_receive();

            if (c == '\r' || c =='\n') break;

            uart_transmit(c);
            buffer[i] = c;
            i++;
        }
        
        buffer[i] = '\0';
        uart_print("\r\n");
}