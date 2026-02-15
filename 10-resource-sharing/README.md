# Project 10: Resource Sharing with FreeRTOS

## Overview
This project demonstrates the critical problem of resource sharing in multitasking systems. When multiple tasks need access to the same resource (a peripheral, memory buffer, or data structure), you need synchronization primitives to prevent corruption. We explore binary semaphores, mutexes, priority inversion, and how priority inheritance solves it. The demo uses LEDs to make scheduler behavior visible.

## Hardware
- **Board**: STM32F407G-DISC1
- **Chip**: STM32F407VGT6 (ARM Cortex-M4F)
- **LEDs**: Onboard LEDs on PD12 (green), PD13 (orange), PD14 (red), PD15 (blue)
- **Clock**: 16 MHz HSI (no PLL)

## Concepts Learned
- Binary semaphores vs mutexes and when to use each
- Priority inversion — what it is and why it's dangerous
- Priority inheritance — how mutexes automatically fix inversion
- Critical sections and why you hold locks for minimum time
- Visualizing scheduler behavior with GPIO

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

## The Problem: Shared Resources

Imagine two tasks both trying to print debug messages over UART. Task A starts printing "Temperature: 25C", gets preempted halfway through, Task B prints "Humidity: 60%", Task A resumes. Your terminal shows garbage: `Temper Humidity: 60% ature: 25C`.

The UART is a shared resource. You need a way to say "I'm using this, everyone else wait." That's what semaphores and mutexes do.

## Binary Semaphores

A binary semaphore is a flag with two states: available (1) or taken (0).
```
┌─────────────────┐
│  Semaphore      │
│  ┌───┐          │
│  │ 1 │ Available│
│  └───┘          │
└─────────────────┘
        │
        ▼ xSemaphoreTake()
┌─────────────────┐
│  Semaphore      │
│  ┌───┐          │
│  │ 0 │ Taken    │
│  └───┘          │
└─────────────────┘
        │
        ▼ xSemaphoreGive()
┌─────────────────┐
│  Semaphore      │
│  ┌───┐          │
│  │ 1 │ Available│
│  └───┘          │
└─────────────────┘
```

**Key point:** Semaphores don't track ownership. Any task can give a semaphore, even if it didn't take it. This makes them great for signaling (ISR tells task "data ready") but problematic for resource protection.

## Mutexes

A mutex (mutual exclusion) is specifically designed for protecting shared resources. The critical difference: **ownership**. Only the task that locked the mutex can unlock it.

Why ownership matters: it enables **priority inheritance**.

## Priority Inversion: The Danger

This scenario crashed the Mars Pathfinder in 1997:
```
Time ────────────────────────────────────────────────────►

Low (Priority 1):    ▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░░░░░░░▓▓▓▓▓▓▓▓
                     ↑ takes    ↑ preempted      ↑ resumes
                     semaphore  by Medium        finally

Medium (Priority 2): ░░░░░░░░░░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░
                               ↑ runs freely, no semaphore needed

High (Priority 3):   ░░░░░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
                           ↑ blocked waiting for semaphore
                           STARVING - can't run!

▓ = running/trying   ░ = blocked/waiting
```

1. Low takes the semaphore, starts working
2. High wakes up, needs the semaphore, blocks waiting
3. Medium wakes up, doesn't need the semaphore, preempts Low
4. High is stuck — waiting for Low, which can't run because Medium keeps preempting it

The highest priority task is effectively running at the lowest priority. This is **unbounded priority inversion**.

## Priority Inheritance: The Fix

Mutexes implement priority inheritance automatically:
```
Time ────────────────────────────────────────────────────►

Low (Priority 1→3):  ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░░░░░░░
                     ↑ takes    ↑ BOOSTED to    ↑ releases
                     mutex      priority 3!     mutex, drops back

Medium (Priority 2): ░░░░░░░░░░░░░░░░░░░░▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
                                        ↑ finally runs after
                                        High is done

High (Priority 3):   ░░░░░░▓▓▓▓░░░░░░░░░▓▓▓▓▓▓▓░░░░░░░░░
                           ↑ blocked    ↑ gets mutex
                           briefly      immediately!
```

When High blocks on the mutex, FreeRTOS temporarily boosts Low to High's priority. Now Medium can't preempt Low. Low finishes quickly, releases the mutex, High runs immediately.

The inversion is bounded to just the critical section duration.

## LED Behavior

### With Binary Semaphore (Priority Inversion)
| LED | Meaning |
|-----|---------|
| Green ON | Low holds semaphore |
| Red ON | High waiting for semaphore |
| Orange CYCLING | Medium running, stealing CPU |

You'll see: Green on, Red on, Orange keeps blinking while Red is stuck. High is starving.

### With Mutex (Priority Inheritance)
| LED | Meaning |
|-----|---------|
| Green ON | Low holds mutex |
| Red ON | High waiting (briefly) |
| Orange STUCK ON | Medium frozen mid-work |

You'll see: Green on, Red on, Orange stops blinking. Low finishes fast, Red gets through, then Orange resumes.

## FreeRTOS API Used

### xSemaphoreCreateBinary()
Creates a binary semaphore.
```c
SemaphoreHandle_t sem = xSemaphoreCreateBinary();
```
- Returns `NULL` if creation failed
- **Starts empty** — must call `xSemaphoreGive()` before first use

### xSemaphoreCreateMutex()
Creates a mutex with priority inheritance.
```c
SemaphoreHandle_t mutex = xSemaphoreCreateMutex();
```
- Returns `NULL` if creation failed
- **Starts available** — no initial give needed
- Requires `#define configUSE_MUTEXES 1` in FreeRTOSConfig.h

### xSemaphoreTake()
Acquires the semaphore/mutex.
```c
xSemaphoreTake(handle, portMAX_DELAY);
```
- Blocks if not available
- `portMAX_DELAY` = wait forever

### xSemaphoreGive()
Releases the semaphore/mutex.
```c
xSemaphoreGive(handle);
```
- For mutex: only the owning task can give
- For semaphore: any task can give

## When to Use Which

| Scenario | Use |
|----------|-----|
| ISR signaling a task | Binary semaphore |
| Protecting a peripheral (UART, SPI) | Mutex |
| Counting available resources | Counting semaphore |
| Task-to-task signaling | Binary semaphore or queue |

## Build & Flash
```bash
make clean && make
st-flash write main.bin 0x08000000
```

## Switching Between Demo Modes

In `main.c`, toggle between these to see the difference:
```c
// Binary semaphore - shows priority inversion
semaphore = xSemaphoreCreateBinary();
xSemaphoreGive(semaphore);

// Mutex - shows priority inheritance fix
semaphore = xSemaphoreCreateMutex();
```

## What I Learned

**Priority inversion is real and dangerous:** It's not just a textbook problem. Mars Pathfinder had to be patched from 190 million kilometers away because of this exact bug. Anytime you have shared resources and multiple priorities, you're at risk.

**Mutexes aren't just semaphores with a different name:** The ownership tracking enables priority inheritance. This is why you use mutexes for resource protection and semaphores for signaling — they solve different problems.

**Visualizing the scheduler helps understanding:** Watching LEDs freeze and resume made priority inheritance "click" in a way that reading about it never did. When orange stopped blinking, I could see Medium getting frozen because Low was temporarily boosted.

**Hold locks for minimum time:** The longer you hold a mutex, the longer high-priority tasks might block. Get in, do the work, get out. Never call `vTaskDelay()` while holding a lock.

## Resources
- [FreeRTOS Mutexes](https://www.freertos.org/Real-time-embedded-RTOS-mutexes.html)
- [FreeRTOS Binary Semaphores](https://www.freertos.org/Embedded-RTOS-Binary-Semaphores.html)
- [Priority Inversion on Mars Pathfinder](https://www.cs.cornell.edu/courses/cs614/1999sp/papers/pathfinder.html)
- [STM32F407 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0090-stm32f405415-stm32f407417-stm32f427437-and-stm32f429439-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)