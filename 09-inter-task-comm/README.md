# Project 09: Inter-Task Communication with FreeRTOS

## Overview
This project builds on Project 08 by adding communication between tasks using FreeRTOS queues. Instead of two independent tasks, we now have a producer-consumer pattern: one task generates data and pushes it into a queue, another task blocks until data arrives and prints it over UART. This is the foundation for how real embedded systems pass sensor readings, commands, and events between different parts of the firmware.

## Hardware
- **Board**: STM32F407G-DISC1
- **Chip**: STM32F407VGT6 (ARM Cortex-M4F)
- **UART**: USART2 (PA2 TX, PA3 RX) via USB-to-TTL adapter
- **Clock**: 16 MHz HSI (no PLL)

## Concepts Learned
- FreeRTOS queues and how they work internally (copy by value)
- Producer-consumer pattern in embedded systems
- Blocking vs polling for inter-task communication
- `portMAX_DELAY` and how tasks sleep waiting for data
- Task priority and why the consumer should be higher than the producer
- How the scheduler decides which task runs when data arrives

## File Structure
```
09-inter-task-comm/
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

## How Queues Work

A queue is a FIFO (first in, first out) buffer managed by FreeRTOS. The key points:

**Copy by value:** When you call `xQueueSend(queue, &counter, ...)`, FreeRTOS copies the data into the queue's internal buffer. The sender's variable and the queue hold separate copies. This means the sender can immediately change its variable without corrupting what's in the queue.

**Blocking:** If a task tries to receive from an empty queue, it doesn't spin in a loop burning CPU. It goes to sleep. The scheduler runs other tasks until data arrives, then wakes the receiver up. This is the whole point — efficient waiting without wasting cycles.

**Thread-safe:** Multiple tasks can send to or receive from the same queue without corrupting data. FreeRTOS handles the synchronization internally by disabling interrupts briefly during queue operations.

## Producer-Consumer Pattern

```
┌──────────────┐     ┌───────────┐     ┌──────────────────┐
│  Sender Task │────>│   Queue   │────>│  Receiver Task   │
│  (Priority 1)│     │ (5 items) │     │  (Priority 2)    │
│  Count++     │     │           │     │  Print over UART  │
│  Every 500ms │     │           │     │  Blocks until     │
│              │     │           │     │  data arrives     │
└──────────────┘     └───────────┘     └──────────────────┘
```

1. Sender increments a counter and pushes it into the queue every 500ms
2. Receiver blocks on `xQueueReceive()` — sleeping, using zero CPU
3. When data arrives, the scheduler wakes the receiver immediately
4. Receiver runs first (priority 2 > sender's priority 1) and prints the value
5. Receiver blocks again, sender gets CPU back

## FreeRTOS API Used

### xQueueCreate()
Creates a queue and returns a handle.
```c
QueueHandle_t queue = xQueueCreate(5, sizeof(uint32_t));
```
- First parameter: number of items the queue can hold
- Second parameter: size of each item in bytes
- Returns `NULL` if not enough heap to allocate the queue

### xQueueSend()
Copies data into the queue.
```c
xQueueSend(queue, &counter, portMAX_DELAY);
```
- First parameter: queue handle
- Second parameter: pointer to the data to copy in
- Third parameter: how long to block if queue is full. `portMAX_DELAY` = wait forever

### xQueueReceive()
Copies data out of the queue.
```c
xQueueReceive(queue, &received, portMAX_DELAY);
```
- First parameter: queue handle
- Second parameter: pointer to buffer where data will be copied
- Third parameter: how long to block if queue is empty. `portMAX_DELAY` = wait forever

## Why the Receiver Has Higher Priority

The receiver runs at priority 2, the sender at priority 1. This matters:

When the sender pushes data into the queue, FreeRTOS checks if any task is blocked waiting on that queue. The receiver is. Since the receiver has higher priority than the sender, the scheduler immediately preempts the sender and runs the receiver. The data gets processed right away with minimal latency.

If both were the same priority, the receiver would have to wait until the sender's time slice ends or the sender calls `vTaskDelay()`. For a simple counter this doesn't matter much, but for real-time sensor data or command processing, that latency adds up.

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

Expected output (every 500ms):
```
Count: 0x00
Count: 0x01
Count: 0x02
Count: 0x03
...
Count: 0xFF
Count: 0x00  ← wraps because uart_write_hex prints one byte
```

## What I Learned

**Queues are copy-by-value:** This surprised me at first. FreeRTOS doesn't store a pointer to your variable — it memcpy's the data into the queue's internal buffer. This means you don't have to worry about the sender's variable going out of scope or being modified before the receiver reads it. For small data like integers and structs this is perfect. For large data you'd pass a pointer instead.

**Blocking is not polling:** In a bare-metal super-loop, waiting for data means spinning in a `while` loop burning CPU cycles. `xQueueReceive` with `portMAX_DELAY` puts the task to sleep — zero CPU usage. The scheduler wakes it only when data actually arrives. This is a fundamental shift in how you think about embedded code.

**Priority matters for responsiveness:** The receiver being higher priority than the sender means data gets processed immediately when it arrives. In a real system, you'd give time-critical tasks (motor control, safety checks) higher priorities and background tasks (logging, status LEDs) lower priorities.

**This is how real firmware works:** Sensor drivers push readings into queues. Processing tasks pull from queues, do math, and push results into other queues. UART/display tasks pull processed data and output it. Each piece is independent and testable. This producer-consumer pattern scales to complex systems in a way that super-loops never can.

## Resources
- [FreeRTOS Queue Documentation](https://www.freertos.org/Embedded-RTOS-Queues.html)
- [FreeRTOS Queue API Reference](https://www.freertos.org/a00018.html)
- [FreeRTOS Documentation](https://www.freertos.org/Documentation/RTOS_book.html)
- [STM32F407 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0090-stm32f405415-stm32f407417-stm32f427437-and-stm32f429439-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)