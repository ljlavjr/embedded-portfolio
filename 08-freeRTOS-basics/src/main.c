/* Project 08: FreeRTOS Basics - Blink Task
 * First FreeRTOS project. One task blinks PD12 on the Discovery board.
 * Demonstrates: task creation, scheduler startup, vTaskDelay
 */

#include "FreeRTOS.h"
#include "task.h"
#include "gpio.h"
#include "uart.h"
#include "stm32f407.h"

/* Task function. Must have this signature: void name(void *pvParameters)
 * Must contain an infinite loop - tasks should never return.
 * vTaskDelay tells the scheduler "I'm done for 500ms, run something else."
 * This is NOT a busy-wait - the scheduler actually switches to other tasks. */
void vBlinkTask(void *pvParameters) {
    for (;;) {
        toggle_pin(GPIOD, 12);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/* UART task. Runs at priority 2 (higher than blink at priority 1).
 * When both tasks are ready, this one runs first.
 * Sends a message every 1 second over UART2. */
void vUartTask(void *pvParameters) {
    for (;;) {
        uart_write_string("Hello from FreeRTOS\r\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/* main() sets up hardware and creates tasks, then hands control to the scheduler.
 * vTaskStartScheduler() never returns - the RTOS owns the CPU from that point on. */
int main(void) {
    gpio_init(GPIOD, 12, GPIO_MODE_OUTPUT);
    gpio_init(GPIOD, 14, GPIO_MODE_OUTPUT);
    write_pin(GPIOD, 14, HIGH);
    uart_init(9600);
    xTaskCreate(vBlinkTask,     // pointer to task function
                "Blink",        // name string (just for debugging)
                256,            // stack size in words
                NULL,           // parameter to pass to the task
                1,              // priority
                NULL            // task handle
    );
    xTaskCreate(vUartTask,
                "Uart",
                256,
                NULL,
                2,
                NULL
    );
    write_pin(GPIOD, 12, HIGH);
    gpio_init(GPIOD, 15, GPIO_MODE_OUTPUT);
    vTaskStartScheduler();
    return 0;
}