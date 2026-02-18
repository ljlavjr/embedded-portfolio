/* ============================================
 * Project : 
 * ============================================ */

/* ---------------- Includes ---------------- */
#include <stdio.h>
#include "stm32f407.h"
#include "FreeRTOS.h"
#include "task.h"
#include "uart.h"
#include "adc.h"
#include "iwdg.h"
#include "event_groups.h"

/* ---------------- Defines ----------------- */
#define STACK_SIZE  512

/* ------------ Global Handles -------------- */
EventGroupHandle_t group;

/* ----------- Task Prototypes -------------- */
void vTaskADCSensor(void *pvParameters);
void vTaskTempSensor(void *pvParameters);
void vTaskSupervisor(void *pvParameters);

/* ============================================
 * Main
 * ============================================ */
int main(void)
{
    /* ---- Watchdog ---- */
    bool wdg_reset = iwdg_check();
    iwdg_init(2000, 4);

    /* ---- Hardware Init ---- */
    uart_init(9600);
    adc_init();

    /* ---- Report Reset Source ---- */
    if (wdg_reset) {
        uart_write_string("WATCHDOG RESET DETECTED\r\n");
    }
    else {
        uart_write_string("Normal boot\r\n");
    }

    /* ---- Create Event Group ---- */
    group = xEventGroupCreate();
 
    /* ---- Create Tasks ---- */
    xTaskCreate(
        vTaskADCSensor,
        "ADC",
        STACK_SIZE,
        NULL,
        1,
        NULL
    );

    xTaskCreate(
        vTaskTempSensor,
        "Temp",
        STACK_SIZE,
        NULL,
        2,
        NULL
    );

    xTaskCreate(
        vTaskSupervisor,
        "Supervisor",
        STACK_SIZE,
        NULL,
        3,
        NULL
    );

    /* ---- Start Watchdog ----*/
    iwdg_start();

    /* ---- Start Scheduler ---- */
    vTaskStartScheduler();

    /* Should never reach here */
    while (1);
}

void vTaskADCSensor(void *pvParameters) {
    (void)pvParameters;
    char buf[32];

    for (;;) {
        uint16_t raw_value = adc_read(0);
        snprintf(buf, sizeof(buf), "ADC: %u\r\n", raw_value);
        uart_write_string(buf);
        xEventGroupSetBits(group, (1 << 0));
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void vTaskTempSensor(void *pvParameters) {
    (void)pvParameters;
    char buf[32];

    for (;;) {
        uint16_t raw_value = adc_read(16);
        snprintf(buf, sizeof(buf), "Temp: %u\r\n", raw_value);
        uart_write_string(buf);
        xEventGroupSetBits(group, (1 << 1));
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void vTaskSupervisor(void *pvParameters) {
    (void)pvParameters;
    const EventBits_t allBits = (1 << 0) | (1 << 1);

    for (;;) {
        // Wait for both tasks to check in, auto clear, 3 second timeout
        EventBits_t bits = xEventGroupWaitBits(
            group,
            allBits,
            pdTRUE,
            pdTRUE,
            pdMS_TO_TICKS(3000)
        );

        if ((bits & allBits) == allBits) {
            // Both tasks checked in, kick the watchdog
            iwdg_refresh();
            uart_write_string("All tasks healthy\r\n");
        } else {
            // At least one task missed its deadline
            if (!(bits & (1 << 0))) {
                uart_write_string("FAULT: ADC task missed deadline\r\n");
            }
            if (!(bits & (1 << 1))) {
                uart_write_string("FAULT: Temp task missed deadline\r\n");
            }
            // Do NOT kick watchdog, let IWDG reset the system
        }
    }