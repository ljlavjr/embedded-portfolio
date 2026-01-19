#ifndef UART_H
#define UART_H

void uart_init(int baud);
void uart_transmit(char c);
char uart_receive(void);
void uart_print(const char *str);

#endif