#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "stm32f407.h"

#define configCPU_CLOCK_HZ                      16000000
#define configTICK_RATE_HZ                      250
#define configMAX_PRIORITIES                    5
#define configMINIMAL_STACK_SIZE                128
#define configTOTAL_HEAP_SIZE                   10240
#define configUSE_PREEMPTION                    1
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configUSE_TIMERS                        0
#define configUSE_16_BIT_TICKS                  0
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    191

#define INCLUDE_vTaskDelay                      1

#define vPortSVCHandler             SVC_Handler
#define xPortPendSVHandler          PendSV_Handler
#define xPortSysTickHandler         SysTick_Handler

#endif