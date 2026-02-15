# Project 10: Resource Sharing with FreeRTOS

## Overview
This project

## Hardware
- **Board**: STM32F407G-DISC1
- **Chip**: STM32F407VGT6 (ARM Cortex-M4F)
- **UART**: USART2 (PA2 TX, PA3 RX) via USB-to-TTL adapter
- **Clock**: 16 MHz HSI (no PLL)

## Concepts Learned
- FreeR

## File Structure
```
10-resource-sharing/
├── src/
│   ├── main.c
│   └── FreeRTOSConfig.h
├── drivers/
│   ├── gpio.c
│   └── uart.c
├── include/
│   ├── stm32f407.h
│   └── hal/
│       ├── gpio.h
│       └── uart.h
├── freertos/
│   ├── tasks.c
│   ├── queue.c
│   ├── list.c
│   ├── timers.c
│   ├── include/
│   └── portable/
│       ├── GCC/ARM_CM4F/
│       └── MemMang/
├── startup.s
├── linker.ld
└── Makefile
```

## How

## Produc

## FreeRTOS API Used

### xQu

## Build & Flash
```bash
make clean && make
st-flash write main.bin 0x08000000
```
## What I Learned

**Qu

## Resources
- [FreeRTOS Queue Documentation](https://www.freertos.org/Embedded-RTOS-Queues.html)
- [FreeRTOS Queue API Reference](https://www.freertos.org/a00018.html)
- [FreeRTOS Documentation](https://www.freertos.org/Documentation/RTOS_book.html)
- [STM32F407 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0090-stm32f405415-stm32f407417-stm32f427437-and-stm32f429439-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)