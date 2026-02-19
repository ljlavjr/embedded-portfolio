#ifndef UART_H
#define UART_H
#include "stm32f407.h"
#include <stdint.h>

void uart_init(uint32_t baud_rate);
void uart_write_char(char c);
void uart_write_string(const char *str);
char uart_read_char(void);
int16_t uart_read_char_nonblocking(void);
void uart_write_hex(uint8_t val);

#endif