#ifndef SPI_H
#define SPI_H

#include <stdint.h>
#include "stm32f407.h"

#define CPHA 0  // Clock Phase
#define CPOL 1  // Clock Polarity
#define MSTR 2  // Master Selection
#define SPE 6   // SPI Enable
#define SSI 8   // Software Slave Management
#define SSM 9   // Internal Slave Select

#define BR_DIV8 (0b010 << 3)

#define RXNE 0  // Receive Buffer Not Empty
#define TXE 1   // Transmit Buffer Empty
#define BSY 7   // Busy Flag

void spi_init(void);
uint8_t spi_transfer(uint8_t data);

#endif