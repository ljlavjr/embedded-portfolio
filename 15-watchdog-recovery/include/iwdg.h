#ifndef IWDG_H
#define IWDG_H

#include "stm32f407.h"
#include <stdbool.h>

#define IWDG_KEY_UNLOCK     0x5555
#define IWDG_KICK           0xAAAA
#define IWDG_START          0xCCCC

void iwdg_init(uint16_t reload, uint8_t prescaler);
void iwdg_start(void);
void iwdg_refresh(void);
bool iwdg_check(void);

#endif