#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "stm32f407.h"

/* FreeRTOS configuration for STM32F407 Discovery Board
 * This file tells FreeRTOS about our hardware and what features to enable.
 */

 /* --- System Clock --- */
/* HSI default clock, no PLL configured */
#define configCPU_CLOCK_HZ                      16000000
/* Scheduler tick rate. 250 = tick every 4ms. Higher = finer timing but more overhead */
#define configTICK_RATE_HZ                      250

/* --- Task Settings --- */
/* Number of priority levels available. 0 (lowest/idle) to 4 (highest) */
#define configMAX_PRIORITIES                    5
/* Stack size in words (not bytes). 128 words = 512 bytes per task minimum */
#define configMINIMAL_STACK_SIZE                128
/* FreeRTOS heap used by xTaskCreate to allocate task stacks and internal structures */
#define configTOTAL_HEAP_SIZE                   10240
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configCHECK_FOR_STACK_OVERFLOW          2
/* --- Scheduler Settings --- */
/* 1 = preemptive (higher priority tasks interrupt lower ones) */
#define configUSE_PREEMPTION                    1
/* Hooks are user callbacks. Idle hook runs when no tasks are ready. Tick hook runs every tick. */
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configUSE_TIMERS                        0
/* 0 = use 32-bit tick counter. 16-bit would overflow too fast */
#define configUSE_16_BIT_TICKS                  0

/* --- Interrupt Settings --- */
/* Highest interrupt priority FreeRTOS can manage (priority 11 of 16).
 * Interrupts above this (lower number) won't be delayed by the scheduler */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    191

/* --- Optional Functions --- */
/* Must explicitly enable FreeRTOS API functions we want to use */
#define configUSE_MUTEXES                       1
#define configUSE_COUNTING_SEMAPHORES           1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskDelayUntil                 1

/* --- Interrupt Handler Remapping --- */
/* FreeRTOS uses SVC (to start first task), PendSV (to context switch),
 * and SysTick (to tick the scheduler). These map FreeRTOS's internal
 * names to the names in our vector table in startup.s */
#define vPortSVCHandler             SVC_Handler
#define xPortPendSVHandler          PendSV_Handler
#define xPortSysTickHandler         SysTick_Handler

#endif