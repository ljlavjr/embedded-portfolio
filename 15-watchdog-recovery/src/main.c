/* ============================================
 * Project 15: Watchdog Recovery 
 * ============================================ */

/* ---------------- Includes ---------------- */
#include <stdio.h>
#include <stdbool.h>
#include "stm32f407.h"
#include "FreeRTOS.h"
#include "task.h"
#include "uart.h"
#include "gpio.h"
#include "adc.h"
#include "iwdg.h"
#include "event_groups.h"

/* ---------------- Defines ----------------- */
#define STACK_SIZE  512

/* ------------ Global Handles -------------- */
EventGroupHandle_t group;

/* ----------- Task/Function Prototypes -------------- */
void vTaskADCSensor(void *pvParameters);
void vTaskTempSensor(void *pvParameters);
void vTaskSupervisor(void *pvParameters);
void enter_safe_state(void);

void vTestTask(void *pvParameters) {
    (void)pvParameters;
    for (;;) {
        uart_write_string("task alive\r\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


/* ============================================
 * Main
 * ============================================ */
int main(void)
{
   uart_init(9600);
    uart_write_string("before scheduler\r\n");

    xTaskCreate(vTestTask, "Test", STACK_SIZE, NULL, 1, NULL);

    vTaskStartScheduler();
    uart_write_string("scheduler failed\r\n");
    while (1);
//     /* ---- Watchdog ---- */
//     // Check reset source BEFORE initializing anything
//     // Must read RCC flags before they get cleared
//     bool wdg_reset = iwdg_check();
//     // Prescaler 4 = /64 divider, reload 2000
//     // Timeout = (64 * 2000) / 32000Hz = 4 seconds
//     //iwdg_init(2000, 4);

//     /* ---- Hardware Init ---- */
//     uart_init(9600);
//     gpio_init(GPIOA, 0, GPIO_MODE_AN);
//     adc_init();

//     gpio_init(GPIOD, 12, GPIO_MODE_OUTPUT);
//     gpio_init(GPIOD, 13, GPIO_MODE_OUTPUT);
//     gpio_init(GPIOD, 14, GPIO_MODE_OUTPUT);
//     gpio_init(GPIOD, 15, GPIO_MODE_OUTPUT);

//     /* ---- Report Reset Source ---- */
//     if (wdg_reset) {
//         uart_write_string("WATCHDOG RESET DETECTED\r\n");
//     }
//     else {
//         uart_write_string("Normal boot\r\n");
//     }

//     /* ---- Create Event Group ---- */
//     // Bit 0 = ADC task check-in
//     // Bit 1 = Temp task check-in
//     // Supervisor waits for both bits before kicking watchdog
//     group = xEventGroupCreate();
//     if (group == NULL) {
//     uart_write_string("Event group FAILED\r\n");
// }
//  BaseType_t ret;
//     /* ---- Create Tasks ---- */
//     ret = xTaskCreate(
//         vTaskADCSensor,
//         "ADC",
//         STACK_SIZE,
//         NULL,
//         1,
//         NULL
//     );

//     if (ret != pdPASS) {
//     uart_write_string("ADC task FAILED\r\n");
// }

//     ret = xTaskCreate(
//         vTaskTempSensor,
//         "Temp",
//         STACK_SIZE,
//         NULL,
//         2,
//         NULL
//     );
//     if (ret != pdPASS) {
//     uart_write_string("Temp task FAILED\r\n");
// }

//     ret = xTaskCreate(
//         vTaskSupervisor,
//         "Supervisor",
//         STACK_SIZE,
//         NULL,
//         3,
//         NULL
//     );
//     if (ret != pdPASS) {
//     uart_write_string("Supervisor task FAILED\r\n");
// }

//     /* ---- Turn on LEDs ---- */
//     // Used to just stay on but then turn off when entering "safe state"
//     write_pin(GPIOD, 12, HIGH);
//     write_pin(GPIOD, 13, HIGH);
//     write_pin(GPIOD, 14, HIGH);

//     // Blue light not activated. Used to signal hard faults on the board.

//     /* ---- Start Watchdog ----*/
//     // Start IWDG right before scheduler
//     // Once started, it CANNOT be stopped
//     // 4 second countdown begins here
//     //iwdg_start();

//     /* ---- Start Scheduler ---- */
//     uart_write_string("Starting scheduler\r\n");
// vTaskStartScheduler();
// uart_write_string("Scheduler returned\r\n");

//     /* Should never reach here */
//     while (1);
}

void vTaskADCSensor(void *pvParameters) {
    (void)pvParameters;
    char buf[32];
    bool fault = false;

    for (;;) {
        EventBits_t flags = xEventGroupGetBits(group);
        if (flags & (1 << 2)) {
            fault = true;
        }
        uint16_t raw_value = adc_read(0);
        snprintf(buf, sizeof(buf), "ADC: %u\r\n", raw_value);
        uart_write_string(buf);
        // Check in with supervisor, signal that ADC task is alive
        if (!fault) {
            xEventGroupSetBits(group, (1 << 0));
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void vTaskTempSensor(void *pvParameters) {
    (void)pvParameters;
    char buf[32];
    bool fault = false;

    for (;;) {
        EventBits_t flags = xEventGroupGetBits(group);
        if (flags & (1 << 3)) {
            fault = true;
        }
        uint16_t raw_value = adc_read(16);
        snprintf(buf, sizeof(buf), "Temp: %u\r\n", raw_value);
        uart_write_string(buf);
        // Check in with supervisor, signal that Temp task is alive
        if (!fault) {
            xEventGroupSetBits(group, (1 << 1));
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void enter_safe_state(void) {
    // 1. Disable all outputs (LEDs for now)
    //  In a real system this would be motors, actuators, relays
    write_pin(GPIOD, 12, LOW);
    write_pin(GPIOD, 13, LOW);
    write_pin(GPIOD, 14, LOW);

    // 2. Report safe state entry
    uart_write_string("ENTERING SAFE STATE\r\n");

    // 3. Let the watchdog expire and reset the system
}

void vTaskSupervisor(void *pvParameters) {
    (void)pvParameters;
    const EventBits_t allBits = (1 << 0) | (1 << 1);

    for (;;) {
        int16_t ch = uart_read_char_nonblocking();
        if (ch == 'a') {
            xEventGroupSetBits(group, (1 << 2));
            uart_write_string("Injecting ADC fault\r\n");
        }
        if (ch == 't') {
            xEventGroupSetBits(group, (1 << 3));
            uart_write_string("Injecting Temp fault\r\n");
        }
        // Wait up to 3 seconds for BOTH tasks to check in
        // pdTRUE (arg 3): auto clear bits when both are set
        // pdTRUE (arg 4): wait for ALL bits, not just any
        // 3 sec wait + margin = within 4 sec IWDG timeout
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
            enter_safe_state();
            // Do NOT kick watchdog, let IWDG reset the system
        }
    }
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    (void)xTask;
    (void)pcTaskName;
    while (1);
}