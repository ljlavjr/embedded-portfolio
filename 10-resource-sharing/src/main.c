/* ============================================
 * Project 10: Resource Sharing
 * Phase 1: Binary Semaphore (shows priority inversion)
 * ============================================ */

/* ---------------- Includes ---------------- */
#include "stm32f407.h"
#include "FreeRTOS.h"
#include "task.h"
#include "gpio.h"
#include "semphr.h"

/* ---------------- Defines ----------------- */
#define LOW_PRIO    1
#define MED_PRIO    2
#define HIG_PRIO    3
#define STACK_SIZE  256
#define BUSY_WAIT   2000000

/* ------------ Global Handles -------------- */
SemaphoreHandle_t semaphore;

/* ----------- Task Prototypes -------------- */
void vTaskLow(void *pvParameters);
void vTaskMedium(void *pvParameters);
void vTaskHigh(void *pvParameters);

/* ============================================
 * Main
 * ============================================ */
int main(void)
{
    /* ---- Hardware Init ---- */
    gpio_init(GPIOD, PD12, GPIO_MODE_OUTPUT);
    gpio_init(GPIOD, PD13, GPIO_MODE_OUTPUT);
    gpio_init(GPIOD, PD14, GPIO_MODE_OUTPUT);
    gpio_init(GPIOD, PD15, GPIO_MODE_OUTPUT);


    /* ---- Create Semaphore ---- */
    // semaphore = xSemaphoreCreateBinary();
    // xSemaphoreGive(semaphore);
    semaphore = xSemaphoreCreateMutex();
    if (semaphore == NULL) {
        // Creation failed - blink green rapidly and halt
        while (1)
        {
            toggle_pin(GPIOD, PD12);
            for (volatile uint32_t i = 0; i < 500000; i++);
        }
    }

    /* ---- Create Tasks ---- */
    xTaskCreate(
        vTaskLow,
        "Low",
        STACK_SIZE,
        NULL,
        LOW_PRIO,
        NULL
    );
     xTaskCreate(
        vTaskMedium,
        "Medium",
        STACK_SIZE,
        NULL,
        MED_PRIO,
        NULL
    );
     xTaskCreate(
        vTaskHigh,
        "High",
        STACK_SIZE,
        NULL,
        HIG_PRIO,
        NULL
    );

    /* ---- Start Scheduler ---- */
    vTaskStartScheduler();

    /* Should never reach here */
    while (1);
}

/* ============================================
 * Low Priority Task
 * - Grabs resource, holds it for a long time
 * ============================================ */
void vTaskLow(void *pvParameters)
{
    /* Optional: Initial delay to stagger startup */

    while (1)
    {
        /* Take the semaphore */
        xSemaphoreTake(semaphore, portMAX_DELAY);

        /* Show we're in critical section */
        write_pin(GPIOD, PD12, HIGH);

        /* Simulate long processing while holding resource */
        for (volatile uint32_t i = 0; i < BUSY_WAIT; i++);

        /* Done with critical section */
        write_pin(GPIOD, PD12, LOW);

        /* Release the semaphore */
        xSemaphoreGive(semaphore);

        /* Delay before next iteration */
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/* ============================================
 * Medium Priority Task  
 * - Never touches semaphore
 * - Just consumes CPU when it can run
 * ============================================ */
void vTaskMedium(void *pvParameters)
{
    /* Initial delay: let Low grab semaphore first */
    vTaskDelay(pdMS_TO_TICKS(200));

    while (1)
    {
        /* Show we're running */
        write_pin(GPIOD, PD13, HIGH);

        /* Simulate CPU-bound work (no semaphore!) */
        for (volatile uint32_t i = 0; i < BUSY_WAIT; i++);

        /* Done working */
        write_pin(GPIOD, PD13, LOW);

        /* Delay before next iteration */
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/* ============================================
 * High Priority Task
 * - Needs the resource
 * - Should run first, but will block on semaphore
 * ============================================ */
void vTaskHigh(void *pvParameters)
{
    /* Initial delay: let Low grab semaphore first */
    vTaskDelay(pdMS_TO_TICKS(100));

    while (1)
    {
        /* Show we're trying to get resource */
        write_pin(GPIOD, PD14, HIGH);

        /* Block waiting for semaphore */
        xSemaphoreTake(semaphore, portMAX_DELAY);

        /* Got it! Quick blink to show success */
        toggle_pin(GPIOD, PD14);
        for (volatile uint32_t i = 0; i < BUSY_WAIT/2; i++);
        toggle_pin(GPIOD, PD14);

        /* Release immediately - we don't hold long */
        xSemaphoreGive(semaphore);

        /* Red LED off - we're done */
        write_pin(GPIOD, PD14, LOW);

        /* Delay before next iteration */
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}