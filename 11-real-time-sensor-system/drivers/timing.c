#include "timing.h"
#include "stm32f407.h"
#include "task.h"

void timing_init(void) {
    /* Enable DWT access by setting TRCENA (bit 24) in CoreDebug DEMCR */
    CoreDebug->DEMCR |= (1 << 24);

    /* Reset the cycle counter */
    DWT->CYCCNT = 0;

    /* Enable the cycle counter by setting CYCCNTENA (bit 0) in DWT CTRL */
    DWT->CTRL |= (1 << 0);
}

void timing_start(TaskTiming_t *t) {
    /* Capture current cycle count as start timestamp */
    t->start_cycles = DWT->CYCCNT;
}

void timing_stop(TaskTiming_t *t) {
    /* Capture end timestamp and compute elapsed cycles */
    uint32_t end_cycles = DWT->CYCCNT;
    uint32_t delta = end_cycles - t->start_cycles;

    /* Update worst-case if this execution was the longest seen */
    if (delta > t->worst_case_cycles) {
        t->worst_case_cycles = delta;
    }
}

void timing_jitter(TaskTiming_t *t, uint32_t expected_wake_tick) {
    uint32_t actual =xTaskGetTickCount();
    uint32_t jitter = actual - expected_wake_tick;

    if (jitter > t->worst_jitter_ticks) {
        t->worst_jitter_ticks = jitter;
    }
}