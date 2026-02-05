#ifndef GPIO_H
#define GPIO_H

#include "stm32f407.h"

typedef enum {
    INPUT = 0,      // 00
    OUTPUT = 1,     // 01
    ALT = 2,        // 10
    AN = 3          // 11
} GPIO_MODE;

typedef enum {
    PUSHPULL,
    OPENDRAIN
} GPIO_OUTPUT_TYPE;

typedef enum {
    LOW,
    MEDIUM,
    HIGH,
    VHIGH
} GPIO_SPEED;

typedef enum {
    NOPUPD,
    PULLU,
    PULLD
} GPIO_PULL_UD;

#endif