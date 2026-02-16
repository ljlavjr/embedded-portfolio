#ifndef TIMING_H
#define TIMING_H

#include "stm32f407.h"

typedef struct {
    uint32_t start_cycles;
    uint32_t worst_case_cycles;
    uint32_t last_wake_tick;       // for jitter calculation
    uint32_t worst_jitter_ticks;
} TaskTiming_t;

void timing_init(void);
void timing_start(TaskTiming_t *t);
void timing_stop(TaskTiming_t *t);
void timing_jitter(TaskTiming_t *t, uint32_t expected_wake_tick);

#endif