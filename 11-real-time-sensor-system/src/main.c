/* ============================================
 * Project 11: Real Time Sensor System
 * ============================================ */

/* ---------------- Includes ---------------- */
#include "stm32f407.h"
#include "FreeRTOS.h"
#include "task.h"
#include "gpio.h"
#include "timing.h"
#include "uart.h"
#include "queue.h"

/* ---------------- Defines ----------------- */
#define STACK_SIZE  256

/* ------------ Global Handles -------------- */
static TaskTiming_t fast_sensor;
static TaskTiming_t slow_sensor;
static TaskTiming_t processing;
static TaskTiming_t reporter;

/* ----------- Task Prototypes -------------- */
void vTaskFastSensor(void *pvParameters);
void vTaskSlowSensor(void *pvParameters);
void vTaskProcessing(void *pvParameters);
void vTaskReporter(void *pvParameters);

/* ============================================
 * Main
 * ============================================ */
int main(void)
{
    /* ---- Hardware Init ---- */
    timing_init();

    /* ---- Create Tasks ---- */
    xTaskCreate(
        vTaskFastSensor,
        "Fast Sensor",
        STACK_SIZE,
        NULL,
        4,
        NULL
    );

    xTaskCreate(
        vTaskSlowSensor,
        "Slow Sensor",
        STACK_SIZE,
        NULL,
        3,
        NULL
    );

    xTaskCreate(
        vTaskProcessing,
        "Processing",
        STACK_SIZE,
        NULL,
        2,
        NULL
    );

    xTaskCreate(
        vTaskReporter,
        "Reporter",
        STACK_SIZE,
        NULL,
        1,
        NULL
    );

    /* ---- Start Scheduler ---- */
    vTaskStartScheduler();

    /* Should never reach here */
    while (1);
}

void vTaskFastSensor(void *pvParameters) {
    (void)pvParameters;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(100);

    for (;;) {
        timing_jitter(&fast_sensor, xLastWakeTime + xPeriod);
        timing_start(&fast_sensor);
        // Work
        timing_stop(&fast_sensor);
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

void vTaskSlowSensor(void *pvParameters) {
    (void)pvParameters;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(500);

    for (;;) {
        timing_jitter(&slow_sensor, xLastWakeTime + xPeriod);
        timing_start(&slow_sensor);
        // Work
        timing_stop(&slow_sensor);
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

void vTaskProcessing(void *pvParameters) {
    (void)pvParameters;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(500);

    for (;;) {
        timing_jitter(&processing, xLastWakeTime + xPeriod);
        timing_start(&processing);
        // Work
        timing_stop(&processing);
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

void vTaskReporter(void *pvParameters) {
    (void)pvParameters;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(1000);

    for (;;) {
        timing_jitter(&reporter, xLastWakeTime + xPeriod);
        timing_start(&reporter);
        // Work
        timing_stop(&reporter);
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}