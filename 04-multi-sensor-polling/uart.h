#ifndef UART_H
#define UART_H

#include <stdint.h>

void uart_init(uint32_t baud);
void uart_transmit(char c);
char uart_receive(void);
void uart_print(const char *str);
void uart_readline(char *buffer, uint8_t max_len);

#endif