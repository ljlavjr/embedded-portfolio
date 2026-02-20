# stm32-shared

Reusable drivers, FreeRTOS source, and build files for STM32F407 Discovery projects.

## Usage
Copy what you need into a new project directory:
```bash
cp stm32-shared/Makefile.template xx-new-project/Makefile
cp stm32-shared/startup.s xx-new-project/
cp stm32-shared/linker.ld xx-new-project/
cp -r stm32-shared/include xx-new-project/
cp -r stm32-shared/drivers xx-new-project/
cp -r stm32-shared/freertos xx-new-project/
```
Then edit the Makefile DRIVERS and FREERTOS_CORE lists for your project.

## Available Drivers
| Driver | Description |
|--------|-------------|
| gpio | Pin mode, output, input, alternate function |
| uart | Init, write char/string, blocking read, non-blocking read |
| adc | Single channel polling read |
| spi | Init, transmit, receive |
| dma | DMA stream configuration, double buffering |
| iwdg | Independent watchdog init, start, refresh, reset detection |
| timing | DWT cycle counter for execution time measurement |

## FreeRTOS Modules
| Module | When to include |
|--------|----------------|
| tasks.c | Always (core scheduler) |
| list.c | Always (used by tasks internally) |
| queue.c | Using queues or semaphores |
| timers.c | Using software timers |
| event_groups.c | Using event groups |