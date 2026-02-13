/* Project 09: FreeRTOS Inter-Task Communication
 * Two tasks communicate through a queue. The sender task increments a counter
 * and pushes it into the queue. The receiver task blocks until data arrives,
 * then prints it over UART.
 * Demonstrates: queues, inter-task data passing, blocking with portMAX_DELAY
 */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "uart.h"
#include "stm32f407.h"


/* Global queue handle. Both tasks need access to the same queue. */
QueueHandle_t queue; 

/* Sender task - produces data.
 * Increments a counter every 500ms and pushes it into the queue.
 * portMAX_DELAY means "block forever if the queue is full" -
 * the task sleeps until there's room. */
void vSenderTask(void *pvParameters) {
    uint32_t counter = 0;
    for (;;) {
        xQueueSend(queue, &counter, portMAX_DELAY);
        counter++;
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/* Receiver task - consumes data.
 * Blocks on xQueueReceive until the sender puts something in the queue.
 * When data arrives, this task wakes up and prints it.
 * Runs at priority 2 (higher than sender at 1) so it processes
 * data immediately when available. */
void vReceiverTask(void *pvParameters) {
    uint32_t received;
    for (;;) {
        xQueueReceive(queue, &received, portMAX_DELAY);
        uart_write_string("Count: 0x");
        uart_write_hex((uint8_t)received);
        uart_write_string("\r\n");
    }
}

/* Create queue and tasks, then hand control to the scheduler. */
int main(void) {
    uart_init(9600);

    /* Queue holds 5 items, each sizeof(uint32_t) = 4 bytes.
     * FreeRTOS copies data into the queue (by value, not by reference). */
    queue = xQueueCreate(5, sizeof(uint32_t));

    xTaskCreate(vSenderTask, "Sender", 128, NULL, 1, NULL);
    xTaskCreate(vReceiverTask, "Receiver", 128, NULL, 2, NULL);
    
    vTaskStartScheduler();
    return 0;
}