#ifndef SPI_H
#define SPI_H

#include <stdint.h>
#include "stm32f407.h"

#define CPHA 0  // Clock Phase
#define CPOL 1  // Clock Polarity
#define MSTR 2  // Master Selection
#define BR0 3   // Baud Rate Control 0
#define BR1 4   // Baud Rate Control 1
#define BR2 5   // Baud Rate Control 2
#define SPE 6   // SPI Enable

#define RXNE 0  // Receive Buffer Not Empty
#define TXE 1   // Transmit Buffer Empty
#define BSY 7   // Busy Flag

void spi_init(void);
uint8_t spi_transfer(uint8_t data);

#endif