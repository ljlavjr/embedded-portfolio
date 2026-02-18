# Project 11: Real-Time Sensor System

## Overview
This project brings together tasks, queues, mutexes, and filtering into a complete multi-task real-time sensor system that combines the individual RTOS concepts form Projects 8-10 into a single working application. Two sensors are sampled at different rates, processed through a moving average filter, and reported over UART. What sets this project apart is the addition of DWT cycle counter instrumentation, which measures worst-case execution time and scheduling jitter to prove that every task meets its deadline.

## Hardware
- **Board**: STM32F407G-DISC1
- **Chip**: STM32F407VGT6 (ARM Cortex-M4F)
- **ADC Channel 0**: Connected to 3.3V/GND
- **ADC Channel 16**: Internal temperature sensor
- **UART2**: Connected to Raspberry Pi 4
- **Clock**: 16 MHz HSI (no PLL)

## Concepts Learned
- `vTaskDelayUntil()` for fixed-period execution vs `vTaskDelay()` drift
- DWT cycle counter for hardware-level timing instrumentation
- Moving average filter for signal processing
- Rate monotonic priority assignment
- Event-driven vs periodic task design
- Single queue with tagged data for multi-source pipelines

## File Structure
```
11-real-time-sensor-system/
├── src/
│   ├── main.c
│   └── FreeRTOSConfig.h
├── drivers/
│   ├── gpio.c
│   ├── uart.c
│   ├── adc.c
│   └── timing.c
├── include/
│   ├── stm32f407.h
│   ├── gpio.h
│   ├── uart.h
│   ├── adc.h
│   └── timing.h
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

## System Architecture
```
                    ┌──────────────────┐
  ADC Ch0 ────────► │ Fast Sensor (P4) │──┐
                    │ 100ms period     │  │
                    └──────────────────┘  │    ┌──────────────────┐     ┌─────────────┐     ┌──────────────────┐
                                          ├──► │     Queue        │────►│ Processing  │────►│ Reporter (P1)    │────► UART
                    ┌──────────────────┐  │    │ SensorReading_t  │     │ Task (P2)   │     │ 1000ms period    │
  ADC Ch16 ───────► │ Slow Sensor (P3) │──┘    └──────────────────┘     │ Moving Avg  │     └──────────────────┘
  (temp sensor)     │ 500ms period     │                                └──────┬──────┘              ▲
                    └──────────────────┘                                       │                     │
                                                                              ▼                     │
                                                                     ┌──────────────┐              │
                                                                     │ Shared Data  │──────────────┘
                                                                     │ (mutex)      │
                                                                     └──────────────┘
                                                                     
```
Both sensor tasks read their ADC channels at fixed intervals and push tagged readings into a single shared queue. The processing task blocks on the queue, waking each time a reading arrives. It runs the value through a per-channel moving average filter and writes the result into a shared struct protected by a mutex. Every 1000ms, the reporter task takes the mutex, reads the latest filtered values and timing stats, and sends a formatted report over UART.

## Task Design
| Task | Period | Priority | Role |
|------|--------|----------|------|
| Fast Sensor | 100ms | 4 (highest) | Reads ADC channel 0 |
| Slow Sensor | 500ms | 3 | Reads internal temp sensor |
| Processing | Event-driven | 2 | Filters and stores data |
| Reporter | 1000ms | 1 (lowest) | Sends stats over UART |

Priorities follow rate monotonic scheduling, which means the task that runs most frequently gets the highest priority. This makes intuitive sense: if the fast sensor only has 100ms between readings, it can't afford to wait for a 1000ms reporter task to finish printing. Giving it the highest priority guarantees it always preempts lower-frequency work and never misses a sample.
The processing task is event-driven rather than periodic because its job is to handle data as it arrives on the queue. It has no fixed rate of its own so it needs to wake up whenever either sensor pushes a reading. Using xQueueReceive with portMAX_DELAY lets it sleep until data is available, consuming zero CPU time while idle. If it were periodic, it would either poll the queue and waste cycles, or risk letting readings pile up between wake-ups.

## The Data Pipeline

### Tagged Sensor Readings
Multiple sensors feed a single queue. Each reading is tagged with its source:
```c
typedef struct {
    uint8_t  channel;
    uint16_t raw_value;
    uint32_t timestamp;
} SensorReading_t;
```

A single queue with tagged structs means the processing task only needs to block on one queue instead of polling multiple queues to check for data. If you had two separate queues, the task would need to check each one, and if it blocks on one, it might miss data arriving on the other. With a tagged struct, adding a third or fourth sensor is just a new channel value and a new filter. The queue, the processing loop, and the task structure don't change at all. This pattern scales cleanly in real systems where you might have dozens of sensors feeding one processing pipeline.

### Moving Average Filter
The processing task applies a moving average to smooth noisy sensor data:
```
Window size: 8 samples

Sample arrives → subtract oldest from sum
               → store new value in circular buffer
               → add new value to sum
               → advance index (wraps with % FILTER_SIZE)
               → average = sum / count
```
ADC readings are noisy. A single sample might spike or dip due to electrical noise, so averaging the last several readings smooths out those glitches and gives a more stable value. The circular buffer approach is efficient because each new sample takes the same amount of work regardless of window size. Instead of shifting all values down and re-adding them, you just overwrite the oldest entry, subtract it from the running sum, and add the new one.


### Shared Data with Mutex Protection
```c
typedef struct {
    uint16_t avg_ch0;
    uint16_t avg_ch16;
} ProcessedData_t;
```
The processing task writes filtered results to this struct. The reporter task reads from it. Both accesses are protected by a mutex. The critical section is kept as small as possible — only the read or write, not the formatting or filtering.

## Timing Instrumentation

### DWT Cycle Counter
The Cortex-M4 has a built-in cycle counter (`DWT->CYCCNT`) that counts CPU cycles at the core clock frequency (168 MHz). This gives cycle-accurate measurement without any external hardware.
```c
CoreDebug->DEMCR |= (1 << 24);   // Enable DWT access
DWT->CYCCNT = 0;                  // Reset counter
DWT->CTRL |= (1 << 0);           // Start counting
```

Each task captures `CYCCNT` before and after its work. The difference is the execution time in cycles. The worst case seen so far is tracked per task.

### WCET (Worst-Case Execution Time)
WCET is the longest a task has ever taken to complete its work. Embedded engineers care about it because every periodic task has a deadline, so if the work is not done before the next period starts, the system falls behind. If a task's WCET exceeds its period, it misses its deadline and the system is no longer real-time. In safety-critical systems, a missed deadline can be catastrophic. 

| Task | WCET (cycles) | WCET (µs) | Period |
|------|--------------|-----------|--------|
| Fast Sensor | 588 | 3.5 | 100 ms |
| Slow Sensor | 2187 | 13.0 | 500 ms |
| Processing | 296 | 1.8 | Event-driven |
| Reporter | 1,654,006 | 9,845 | 1000 ms |

Every task has enormous headroom. The fast sensor finishes in 3.5µs against a 100ms deadline, using less than 0.004% of its available time. Even the reporter, the heaviest task at roughly 9.8ms, uses less than 1% of its 1000ms period. The reporter's WCET is orders of magnigtude higher than the other tasks because `snprintf` is expensive becasue it performs string formatting, interger-to-ASCII conversion, and buffer management, which is far more work than a simple ADC read or a few arithmetic operations.

### Jitter
Jitter is the difference between when a task is supposed to wake up and when it actually does. In a loaded system, a higher-priority task might be running at the moment a lower-priority task is scheduled, forcing it to wait.
Consistent timing matters in real-time systems. A motor controller that updates every 1ms but occasionally slips to 5ms will cause jerky, unpredictable behavior. In control systems, inconsistent update rates can cause instability.
All measured jitter in this project was 0 ticks, meaning every task woke exactly on schedule. This is because the CPU is barely loaded. Even the heaviest task uses less than 1% of its period, so there is never contention for CPU time.
Jitter would increase with heavier task workloads, longer mutex-held critical sections, or more tasks competing at similar priorities.

### vTaskDelayUntil() vs vTaskDelay()
```
vTaskDelay(100):
  Work takes 5ms → delay 100ms → actual period = 105ms → DRIFT

  |--work--|-------delay 100ms-------|--work--|-------delay 100ms-------|
  0ms     5ms                      105ms    110ms                     215ms

vTaskDelayUntil(100):
  Work takes 5ms → delay until next 100ms mark → actual period = 100ms → FIXED

  |--work--|-------delay-------|--work--|-------delay-------|
  0ms     5ms                100ms   105ms                200ms
```
`vTaskDelayUntil()` tracks the absolute tick count of the last wake time and computes when to wake next. The period stays fixed regardless of how long the work takes.

## UART Output
```
CH0: 117    CH16: 1033
WCET  Fast:588 Slow:2187 Proc:296 Rep:1654006
Jitter Fast:0 Slow:0 Rep:0
```
- **CH0**: Averaged ADC reading from channel 0 (3.3V rail ≈ 117 raw counts)
- **CH16**: Averaged internal temperature sensor reading
- **WCET**: Worst-case execution time in CPU cycles per task
- **Jitter**: Worst-case scheduling jitter in OS ticks per periodic task

## What I Learned

**Composing RTOS pieces is easier than learning them individually**: The task setup, queues, and mutexes worked the same way as in the individual projects. The real challenge was the domain logic on top. The moving average math and circular buffer took more thought than any FreeRTOS API call.

**Computers are fast, but you should prove it**: The fast sensor finishes in 3.5 microseconds. Even the reporter, the heaviest task, completes in under 10 milliseconds against a 1 second deadline. Before this project I just assumed everything would be fast enough. Timing instrumentation forces you to verify instead of assume, and that is a habit worth building early.

**Priority assignment gets hard when you scale up**: With two sensors the answer was obvious. With a dozen, you would have to think carefully about which sensors are safety critical, which can tolerate a missed reading, and how increased CPU load affects jitter. The tagged struct and single queue pattern scales well, but timing analysis becomes essential rather than optional.

**The hardest part was holding everything in my head**: Tasks, queues, mutexes, filtering, ADC, UART, and timing instrumentation all in one project is a lot to track. I am rebuilding my ability to learn after being away from it for a while. But it compiled, it ran, and the numbers made sense. That counts for something.

## Build & Flash
```bash
make clean && make
st-flash write main.bin 0x08000000
```

## Resources
- [FreeRTOS vTaskDelayUntil](https://www.freertos.org/vtaskdelayuntil.html)
- [ARM Cortex-M4 DWT](https://developer.arm.com/documentation/ddi0439/b/Data-Watchpoint-and-Trace-Unit)
- [STM32F407 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0090-stm32f405415-stm32f407417-stm32f427437-and-stm32f429439-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)