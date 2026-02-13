# Project 08: FreeRTOS Basics on STM32F407

## Overview
This project introduces FreeRTOS on the STM32F407. Instead of a bare-metal super-loop where everything runs sequentially, we use an RTOS scheduler to run multiple tasks concurrently. Two tasks run independently: one blinks an LED, the other sends UART messages. This solves the timing problems from the super-loop approach in Project 04.

## Hardware
- **Board**: STM32F407G-DISC1
- **Chip**: STM32F407VGT6 (ARM Cortex-M4F)
- **LED**: PD12 (Green)
- **UART**: USART2 (PA2 TX, PA3 RX) via USB-to-TTL adapter
- **Clock**: 16 MHz HSI (no PLL)

## Concepts Learned
- What an RTOS is and how it differs from bare-metal super-loop
- Integrating FreeRTOS source into an existing bare-metal project
- FreeRTOSConfig.h and what each setting controls
- Task creation with `xTaskCreate()`
- Task priorities and preemptive scheduling
- `vTaskDelay()` vs busy-wait delays
- SysTick, PendSV, and SVC system interrupts for context switching
- Linking with nano.specs for embedded libc (memset, memcpy)
- Hardware FPU flags (`-mfloat-abi=hard -mfpu=fpv4-sp-d16`)

## File Structure
```
08-freeRTOS-basics/
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
│   ├── include/          ← FreeRTOS headers
│   └── portable/
│       ├── GCC/ARM_CM4F/ ← Cortex-M4F port (context switching)
│       └── MemMang/      ← heap_4.c (dynamic memory allocator)
├── startup.s
├── linker.ld
└── Makefile
```

## Super-Loop vs RTOS

### The Problem (Project 04)
In a super-loop, everything shares one `while(1)` loop:
```c
while (1) {
    read_adc();        // If this takes 10ms...
    send_uart();       // ...this waits 10ms
    check_button();    // ...this waits even longer
}
```
Adding features makes timing unpredictable. Nothing runs "at the same time."

### The Solution (FreeRTOS)
Each job gets its own task with its own stack and priority:
```c
void vBlinkTask(void *p)  { for(;;) { toggle_pin(...); vTaskDelay(...); } }
void vUartTask(void *p)   { for(;;) { uart_write_string(...); vTaskDelay(...); } }
```
The scheduler switches between them automatically. Each task thinks it has the CPU to itself.

## How FreeRTOS Works on Cortex-M4

FreeRTOS uses three system interrupts built into the CPU:

- **SVC (Supervisor Call)** — Fires once when `vTaskStartScheduler()` is called. Launches the very first task.
- **SysTick (System Tick Timer)** — Fires every tick (4ms at 250 Hz). Checks if any task's delay has expired or if a higher priority task is ready.
- **PendSV (Pendable Service Call)** — Does the actual context switch. Saves the current task's registers to its stack, loads the next task's registers, and execution continues in the new task.

These are mapped from FreeRTOS's internal names to our vector table names in `FreeRTOSConfig.h`:
```c
#define vPortSVCHandler     SVC_Handler
#define xPortPendSVHandler  PendSV_Handler
#define xPortSysTickHandler SysTick_Handler
```

## FreeRTOS API Used

### xTaskCreate()
Allocates a stack from the FreeRTOS heap, registers the task with the scheduler, and marks it as ready to run. The task doesn't start until `vTaskStartScheduler()` is called.
```c
xTaskCreate(
    vBlinkTask,   // Function pointer
    "Blink",      // Name (debugging only)
    128,          // Stack size in words (512 bytes)
    NULL,         // Parameter passed to task
    1,            // Priority (higher number = higher priority)
    NULL          // Task handle (for referencing later)
);
```

### vTaskStartScheduler()
Point of no return. Sets up SysTick, triggers SVC to launch the highest priority ready task, and the scheduler takes over. Code after this line never executes.

### vTaskDelay()
Tells the scheduler "I don't need the CPU for this many ticks." The scheduler switches to another task during this time. This is fundamentally different from a busy-wait delay — the CPU does useful work instead of spinning.
```c
vTaskDelay(pdMS_TO_TICKS(500));  // Sleep for 500ms, let other tasks run
```

`pdMS_TO_TICKS()` converts milliseconds to tick counts based on `configTICK_RATE_HZ`. At 250 Hz, 500ms = 125 ticks.

## FreeRTOSConfig.h Explained

| Define | Value | Purpose |
|--------|-------|---------|
| `configCPU_CLOCK_HZ` | 16000000 | System clock speed (HSI default) |
| `configTICK_RATE_HZ` | 250 | Scheduler tick frequency. 250 = 4ms resolution |
| `configMAX_PRIORITIES` | 5 | Priority levels 0 (idle) through 4 (highest) |
| `configMINIMAL_STACK_SIZE` | 128 | Minimum stack per task in words (512 bytes) |
| `configTOTAL_HEAP_SIZE` | 10240 | FreeRTOS heap for task stacks and internals |
| `configUSE_PREEMPTION` | 1 | Higher priority tasks interrupt lower ones |
| `configUSE_IDLE_HOOK` | 0 | No user callback when CPU is idle |
| `configUSE_TICK_HOOK` | 0 | No user callback on each tick |
| `configUSE_TIMERS` | 0 | Software timers disabled (not needed yet) |
| `configUSE_16_BIT_TICKS` | 0 | Use 32-bit tick counter (16-bit overflows too fast) |
| `configMAX_SYSCALL_INTERRUPT_PRIORITY` | 191 | Priority level 11/16. Interrupts above this are untouched by FreeRTOS |
| `INCLUDE_vTaskDelay` | 1 | Must explicitly enable optional API functions |

## Build & Flash
```bash
make clean && make
st-flash write main.bin 0x08000000
```

## UART Output
Connect a USB-to-TTL adapter:
- Adapter RX → PA2 (STM32 TX)
- Adapter TX → PA3 (STM32 RX)
- Adapter GND → STM32 GND

View output:
```bash
stty -F /dev/ttyUSB0 9600
cat /dev/ttyUSB0
```

Expected output (every 1 second):
```
Hello from FreeRTOS
Hello from FreeRTOS
Hello from FreeRTOS
```

## Makefile Changes from Bare-Metal

Several changes were needed to support FreeRTOS:

**FPU flags:** Changed `-mfloat-abi=soft` to `-mfloat-abi=hard -mfpu=fpv4-sp-d16`. The ARM_CM4F port requires hardware FPU support enabled.

**Linker flags:** Replaced `-nostdlib` with `-nostartfiles --specs=nano.specs`. FreeRTOS needs `memset` and `memcpy` from libc. `-nostartfiles` still skips the default startup code (we have our own), but allows linking against a minimal embedded libc. Added `-lc -lnosys -lgcc` to the link step.

**Include paths:** Added `-I freertos/include`, `-I freertos/portable/GCC/ARM_CM4F`, and `-I src` (for FreeRTOSConfig.h).

**FreeRTOS objects:** Added build rules for `tasks.o`, `queue.o`, `list.o`, `port.o`, and `heap_4.o`.

## What I Learned

**RTOS vs bare-metal is a spectrum, not a wall:** FreeRTOS isn't a full operating system. It's a ~9,000 line C library that manages task switching. My HAL drivers, register definitions, linker script, and startup code are all still mine. FreeRTOS sits on top of my bare metal.

**FreeRTOSConfig.h is the real work:** Getting the source files compiled is mechanical. Understanding what each config define does and why — that's where the learning happens. Every setting has a tradeoff between features, memory, and performance.

**The scheduler owns SysTick:** In bare-metal, I could use SysTick for my own timing. With FreeRTOS, SysTick belongs to the scheduler. If I need a custom timer, I'll use a hardware timer peripheral instead.

**vTaskDelay is not a busy wait:** This is the key insight. When a task delays, the CPU runs other tasks. In a super-loop, a delay wastes every cycle. This is why RTOS makes multi-tasking practical.

**Integration is the hard part:** Writing the two tasks took 5 minutes. Getting FreeRTOS to compile with my existing project — matching include paths, fixing linker flags, adding missing config defines, resolving the FPU requirement — that's where the real debugging happened. This is a skill that matters in industry.

## Resources
- [FreeRTOS Documentation](https://www.freertos.org/Documentation/RTOS_book.html)
- [FreeRTOSConfig.h Reference](https://www.freertos.org/a00110.html)
- [FreeRTOS Kernel GitHub](https://github.com/FreeRTOS/FreeRTOS-Kernel)
- [STM32F407 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0090-stm32f405415-stm32f407417-stm32f427437-and-stm32f429439-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)