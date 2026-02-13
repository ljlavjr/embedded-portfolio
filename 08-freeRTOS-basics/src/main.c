#include "FreeRTOS.h"
#include "task.h"
#include "gpio.h"
#include "stm32f407.h"

void vBlinkTask(void *pvParameters) {
    for (;;) {
        toggle_pin(GPIOD, 12);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

int main(void) {
    gpio_init(GPIOD, 12, GPIO_MODE_OUTPUT);
    xTaskCreate(vBlinkTask,     // pointer to task function
                "Blink",        // name string (just for debugging)
                128,            // stack size in words
                NULL,           // parameter to pass to the task
                1,              // priority
                NULL            // task handle
    );
    vTaskStartScheduler();
    return 0;
}