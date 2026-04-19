# Project 15: Watchdog Recovery

## Overview
This project implements a hardware watchdog recovery system using the STM32F407's Independent Watchdog (IWDG) integrated with FreeRTOS. A supervisor task monitors two worker tasks using event groups. Each worker must check in within a deadline or the supervisor lets the watchdog expire, forcing a hardware reset. On reboot, the system detects the watchdog reset through RCC flags and reports it over UART. Fault injection via UART commands allows deliberate testing of the recovery path.

## Hardware
- **Board**: STM32F407G-DISC1
- **Chip**: STM32F407VGT6 (ARM Cortex-M4F)
- **ADC Channel 0**: Connected to 3.3V/GND
- **ADC Channel 16**: Internal temperature sensor
- **UART2**: Connected to Raspberry Pi 4
- **Clock**: 16 MHz HSI (no PLL)

## Concepts Learned

## System Architecture
```
                    ┌──────────────────┐
  ADC Ch0 ────────► │ ADC Task (P1)    │──────► xEventGroupSetBits(bit 0)
                    │ 500ms period     │
                    └──────────────────┘
                                                    ┌──────────────────────┐
                                                    │ Supervisor (P3)      │
                                                    │ xEventGroupWaitBits  │
                                                    │ bits 0 & 1, 3s TMO  │──── iwdg_refresh()
                                                    │                      │
                    ┌──────────────────┐            │ UART fault injection │
  ADC Ch16 ───────► │ Temp Task (P2)   │──────► xEventGroupSetBits(bit 1) │
  (temp sensor)     │ 1000ms period    │            └──────────────────────┘
                    └──────────────────┘
```
  Event Group Bits:
  Bit 0 = ADC task check-in
  Bit 1 = Temp task check-in
  Bit 2 = ADC fault inject flag
  Bit 3 = Temp fault inject flag

  ## Task Design
| Task | Period | Priority | Role |
|------|--------|----------|------|
| ADC Sensor | 500ms | 1 (lowest) | Reads ADC channel 0, checks in with supervisor |
| Temp Sensor | 1000ms | 2 | Reads internal temp sensor, checks in with supervisor |
| Supervisor | Event-driven (3s timeout) | 3 (highest) | Monitors check-ins, kicks watchdog, handles fault injection |

The supervisor has the highest priority because it owns the watchdog. If a worker task starves the supervisor and it can't run, the watchdog expires even though all tasks are healthy. The supervisor must always be able to preempt the workers.

## Watchdog Configuration
| Parameter | Value |
|-----------|-------|
| Clock Source | LSI (~32 kHz) |
| Prescaler | 4 (/64 divider) |
| Reload | 2000 |
| Timeout | (64 × 2000) / 32000 = 4 seconds |
| Supervisor Wait | 3 seconds |
| Margin | 1 second |

The supervisor waits up to 3 seconds for both tasks to check in. The IWDG timeout is 4 seconds. This leaves 1 second of margin between the supervisor's deadline and the hardware reset. The slowest worker runs every 1000ms, so it gets at least three chances to check in within the supervisor's 3 second window.

## The Supervisor Pattern

### Why Not Just Kick From Main?
In a super loop, kicking the watchdog from the main loop only proves the main loop is running. In an RTOS, individual tasks can hang, crash, or miss deadlines while other tasks continue running normally. A single kick point can't detect per-task failures.

### How Check-In Works
Each worker task calls `xEventGroupSetBits` with its assigned bit after completing its work. The supervisor calls `xEventGroupWaitBits` with `pdTRUE` for both `xClearOnExit` and `xWaitForAllBits`, meaning it blocks until every monitored task has checked in and automatically clears the bits for the next cycle. Only when all tasks have checked in does the supervisor kick the watchdog. If any task misses the deadline, the supervisor reports which task failed and intentionally does not kick the watchdog.

### Event Groups vs Other Primitives
A semaphore can signal that one event happened. A queue can pass data between tasks. Neither cleanly answers the question "have ALL of these tasks reported in?" An event group lets the supervisor wait for a specific combination of bits with a single blocking call, with automatic clearing and a timeout. It is purpose built for multi-task synchronization.

## Fault Injection
| Command | Effect |
|---------|--------|
| `a` | ADC task stops checking in (sets bit 2) |
| `t` | Temp task stops checking in (sets bit 3) |

The supervisor reads UART commands and sets fault flag bits in the event group. Worker tasks check their fault bit each loop and skip the check-in call when it is set. The task continues running and printing sensor data, but the supervisor never sees its check-in bit, times out, and lets the watchdog reset the system.

Only the supervisor reads from UART. This avoids a race condition where two tasks compete for the same received byte.

## Reset Source Detection
On boot, before any peripheral initialization, `main()` reads bit 29 (IWDGRSTF) of the RCC_CSR register to determine if the previous reset was caused by the watchdog. This must happen first because clearing the flags (by setting bit 24, RMVF) erases the information. After UART is initialized, the system reports either "WATCHDOG RESET DETECTED" or "Normal boot."

## Expected UART Output

### Normal Operation
```
Normal boot
ADC: 1234
Temp: 2048
All tasks healthy
ADC: 1230
ADC: 1235
Temp: 2050
All tasks healthy
```

### After Fault Injection ('a')
```
Injecting ADC fault
ADC: 1232
Temp: 2045
FAULT: ADC task missed deadline
```

### After Watchdog Reset
```
WATCHDOG RESET DETECTED
ADC: 1234
Temp: 2048
All tasks healthy
```

## File Structure
```
15-watchdog-recovery/
├── src/
│   ├── main.c
│   └── FreeRTOSConfig.h
├── drivers/
│   ├── gpio.c
│   ├── uart.c
│   ├── timing.c
│   ├── adc.c
│   └── iwdg.c
├── include/
│   ├── stm32f407.h
│   └── hal/
│       ├── gpio.h
│       ├── uart.h
│       ├── timing.h
│       ├── adc.h
│       └── iwdg.h
├── freertos/
│   ├── tasks.c
│   ├── event_groups.c
│   ├── list.c
│   ├── include/
│   └── portable/
│       ├── GCC/ARM_CM4F/
│       └── MemMang/
├── startup.s
├── linker.ld
└── Makefile
```