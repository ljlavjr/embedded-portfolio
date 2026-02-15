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
- Why the FPU must be enabled in startup code before any FPU instructions execute

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

## Startup Code: FPU Enable (Critical Fix)

The Cortex-M4F has a hardware FPU, but **it is disabled by default at reset**. When compiling with `-mfloat-abi=hard`, the compiler and the FreeRTOS CM4F port both emit FPU instructions. If the FPU is not enabled before these instructions execute, the CPU triggers a UsageFault, which escalates to a HardFault (since the UsageFault vector is not populated).

This must be added to `startup.s` before calling `main()`:
```asm
enable_fpu:
    ldr r0, =0xE000ED88       /* SCB->CPACR register */
    ldr r1, [r0]
    orr r1, r1, #(0xF << 20)  /* Set CP10 and CP11 to full access (bits 20-23) */
    str r1, [r0]
    dsb                        /* Data sync barrier - ensure write completes */
    isb                        /* Instruction sync barrier - flush pipeline */
```

`0xE000ED88` is the Coprocessor Access Control Register (CPACR). Bits 20-23 control access to coprocessors CP10 and CP11, which together form the FPU. Setting them to `11` (full access) enables FPU instructions in both privileged and unprivileged mode.

**Symptom when missing:** The scheduler appears to start (SVC fires successfully), but the first context switch triggers a HardFault because PendSV tries to save/restore FPU registers (S0-S15, FPSCR) that the CPU won't allow access to.

### Debugging Approach Used
1. **LED-based fault detection:** HardFault_Handler lights PD15 (blue). Seeing blue LED = confirmed HardFault, not a logic bug.
2. **Vector table verification:** Used `arm-none-eabi-nm` to confirm SVC_Handler, PendSV_Handler, and SysTick_Handler were linked at valid addresses, then `arm-none-eabi-objdump -s -j .isr_vector` to verify the vector table entries pointed to those addresses with the thumb bit set.
3. **Process of elimination:** Vector table was correct, scheduler was starting (green LED turned on before `vTaskStartScheduler()`), but HardFault fired immediately after → the crash happens during the first context switch → FPU access was the cause.

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

**The FPU must be explicitly enabled in startup code:** The Cortex-M4F FPU is off at reset. If you compile with hard float but forget to enable the FPU before the first FPU instruction runs, you get a HardFault with no obvious cause. Vendor-provided startup files (like ST's) do this automatically, but when writing your own startup from scratch, it's easy to miss. This is the kind of bug where having a HardFault handler with a visible indicator (like an LED) saves hours of confusion.

**Verify the vector table, not just the symbols:** Confirming that handler symbols exist in the ELF (`arm-none-eabi-nm`) is only half the story. You also need to verify that the vector table entries actually point to those addresses (`arm-none-eabi-objdump -s -j .isr_vector`), with the Thumb bit set (bit 0 = 1). A mismatch between symbol addresses and vector table entries means the CPU will jump to the wrong place on an interrupt.

## Useful Debug Commands
```bash
# Check that FreeRTOS handlers are linked
arm-none-eabi-nm main.elf | grep -i "svc\|pendsv\|systick"

# Dump vector table to verify entries match handler addresses (little-endian, thumb bit set)
arm-none-eabi-objdump -s -j .isr_vector main.elf

# Vector table offsets to check:
#   0x00 = Initial SP (should be top of RAM)
#   0x04 = Reset_Handler
#   0x2C = SVC_Handler
#   0x38 = PendSV_Handler
#   0x3C = SysTick_Handler
```

## Resources
- [FreeRTOS Documentation](https://www.freertos.org/Documentation/RTOS_book.html)
- [FreeRTOSConfig.h Reference](https://www.freertos.org/a00110.html)
- [FreeRTOS Kernel GitHub](https://github.com/FreeRTOS/FreeRTOS-Kernel)
- [STM32F407 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0090-stm32f405415-stm32f407417-stm32f427437-and-stm32f429439-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)