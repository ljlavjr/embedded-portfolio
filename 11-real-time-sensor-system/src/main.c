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
#include "adc.h"

/* ---------------- Defines ----------------- */
#define STACK_SIZE  256

typedef struct {
    uint8_t  channel;
    uint16_t raw_value;
    uint16_t timestamp;     // tick count when sampled
} SensorReading_t;

/* ------------ Global Handles -------------- */
static TaskTiming_t fast_sensor;
static TaskTiming_t slow_sensor;
static TaskTiming_t processing;
static TaskTiming_t reporter;
QueueHandle_t queue;

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

    /* ---- Create Queue ---- */
    queue = xQueueCreate(10, sizeof(SensorReading_t));

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
    SensorReading_t sensorReading;

    for (;;) {
        timing_jitter(&fast_sensor, xLastWakeTime + xPeriod);
        timing_start(&fast_sensor);
        uint16_t raw_value = adc_read(0);
        sensorReading.channel = 0;
        sensorReading.raw_value = raw_value;
        sensorReading.timestamp = xTaskGetTickCount();
        xQueueSend(queue, &sensorReading, 0);
        timing_stop(&fast_sensor);
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

void vTaskSlowSensor(void *pvParameters) {
    (void)pvParameters;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(500);
    SensorReading_t sensorReading;

    for (;;) {
        timing_jitter(&slow_sensor, xLastWakeTime + xPeriod);
        timing_start(&slow_sensor);
        uint16_t raw_value = adc_read(16);
        sensorReading.channel = 16;
        sensorReading.raw_value = raw_value;
        sensorReading.timestamp = xTaskGetTickCount();
        xQueueSend(queue, &sensorReading, 0);
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