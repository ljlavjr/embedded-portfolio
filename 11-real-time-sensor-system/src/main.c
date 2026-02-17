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
#include "semphr.h"
#include <stdio.h>

/* ---------------- Defines ----------------- */
#define STACK_SIZE  512
#define FILTER_SIZE 8

typedef struct {
    uint8_t  channel;
    uint16_t raw_value;
    uint32_t timestamp;     // tick count when sampled
} SensorReading_t;

typedef struct {
    uint16_t buffer[FILTER_SIZE];
    uint8_t index;
    uint8_t count;      // tracks how many samples received (up to FILTER_SIZE)
    uint32_t sum;
} MovingAverage_t;

typedef struct {
    uint16_t avg_ch0;
    uint16_t avg_ch16;
} ProcessedData_t;

/* ------------ Global Handles -------------- */
static TaskTiming_t fast_sensor;
static TaskTiming_t slow_sensor;
static TaskTiming_t processing;
static TaskTiming_t reporter;
QueueHandle_t queue;
SemaphoreHandle_t mutex;
ProcessedData_t shared_data;

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
    adc_init();
    uart_init(9600);

    /* ---- Create Queue ---- */
    queue = xQueueCreate(10, sizeof(SensorReading_t));
    mutex = xSemaphoreCreateMutex();

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
        timing_jitter(&fast_sensor, xLastWakeTime);
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
        timing_jitter(&slow_sensor, xLastWakeTime);
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
    SensorReading_t sensorReading;
    MovingAverage_t filter_ch0  = {0};
    MovingAverage_t filter_ch16 = {0};
    MovingAverage_t *f;
    uint16_t avg;

    for (;;) {
        xQueueReceive(queue, &sensorReading, portMAX_DELAY);
        timing_start(&processing);

        /* Select the correct filter based on channel */
        switch (sensorReading.channel) {
            case 0:  f = &filter_ch0;  break;
            case 16: f = &filter_ch16; break;
            default: continue;
        }

        /* Update moving average */
        f->sum -= f->buffer[f->index];
        f->buffer[f->index] = sensorReading.raw_value;
        f->sum += sensorReading.raw_value;
        f->index = (f->index + 1) % FILTER_SIZE;
        if (f->count < FILTER_SIZE) {
            f->count++;
        }
        avg = f->sum / f->count;

        /* Write result to shared data - hold mutex only for the write */
        xSemaphoreTake(mutex, portMAX_DELAY);
        if (sensorReading.channel == 0) {
            shared_data.avg_ch0 = avg;
        } else {
            shared_data.avg_ch16 = avg;
        }
        xSemaphoreGive(mutex);

        timing_stop(&processing);
    }
}

void vTaskReporter(void *pvParameters) {
    (void)pvParameters;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(1000);
    char buf[200];

    for (;;) {
        timing_jitter(&reporter, xLastWakeTime);
        timing_start(&reporter);

        xSemaphoreTake(mutex, portMAX_DELAY);
        uint16_t avg0 = shared_data.avg_ch0;
        uint16_t avg16 = shared_data.avg_ch16;
        xSemaphoreGive(mutex);

        /* Format and send sensor data */
        snprintf(buf, sizeof(buf),
                "CH0: %u    CH16: %u\r\n",
                avg0, avg16);
        uart_write_string(buf);

        /* Format and send timing stats (WCET in cycles, jitter in ticks) */
        snprintf(buf, sizeof(buf),
            "WCET  Fast:%lu Slow:%lu Proc:%lu Rep:%lu\r\n"
            "Jitter Fast:%lu Slow:%lu Rep:%lu\r\n\r\n",
            fast_sensor.worst_case_cycles,
            slow_sensor.worst_case_cycles,
            processing.worst_case_cycles,
            reporter.worst_case_cycles,
            fast_sensor.worst_jitter_ticks,
            slow_sensor.worst_jitter_ticks,
            reporter.worst_jitter_ticks);
        uart_write_string(buf);

        timing_stop(&reporter);
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}