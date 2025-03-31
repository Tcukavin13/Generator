/*
 * FreeRTOS V202112.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */


#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 *
 * See http://www.freertos.org/a00110.html
 *----------------------------------------------------------*/

#include <stdint.h>

#define configTrue    (1U)
#define configFalse   (0U)

/* CPU clock */
extern unsigned int system_core_clock;

#define configUSE_PREEMPTION                               configTrue
#define configUSE_IDLE_HOOK                                configFalse
#define configUSE_TICK_HOOK                                configFalse
#define configCPU_CLOCK_HZ                                (system_core_clock)
#define configTICK_RATE_HZ                                ((TickType_t)1000U)
#define configMAX_PRIORITIES                              (5U)
#define configMINIMAL_STACK_SIZE                          (128U)
#define configTOTAL_HEAP_SIZE                             ((size_t) (100U * 1024U))
#define configMAX_TASK_NAME_LEN                           (10U)
#define configUSE_TRACE_FACILITY                          configTrue
#define configUSE_16_BIT_TICKS                            configFalse
#define configIDLE_SHOULD_YIELD                           configTrue
#define configUSE_MUTEXES                                 configTrue
#define configQUEUE_REGISTRY_SIZE                         (8U)
#define configCHECK_FOR_STACK_OVERFLOW                    configFalse
#define configUSE_RECURSIVE_MUTEXES                       configTrue
#define configUSE_MALLOC_FAILED_HOOK                      configFalse
#define configUSE_APPLICATION_TASK_TAG                    configFalse
#define configUSE_COUNTING_SEMAPHORES                     configTrue
#define configGENERATE_RUN_TIME_STATS                     configFalse

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES                             configFalse
#define configMAX_CO_ROUTINE_PRIORITIES                   (2U)

/* Software timer definitions. */
#define configUSE_TIMERS                                  configTrue
#define configTIMER_TASK_PRIORITY                         (2U)
#define configTIMER_QUEUE_LENGTH                          (10U)
#define configTIMER_TASK_STACK_DEPTH                      (configMINIMAL_STACK_SIZE * 2U)

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */
#define INCLUDE_vTaskPrioritySet                          configTrue
#define INCLUDE_uxTaskPriorityGet                         configTrue
#define INCLUDE_vTaskDelete                               configTrue
#define INCLUDE_vTaskCleanUpResources                     configTrue
#define INCLUDE_vTaskSuspend                              configTrue
#define INCLUDE_vTaskDelayUntil                           configTrue
#define INCLUDE_vTaskDelay                                configTrue

/* Cortex-M specific definitions. */
#ifdef __NVIC_PRIO_BITS
    /* __BVIC_PRIO_BITS will be specified when CMSIS is being used. */
    #define configPRIO_BITS                               __NVIC_PRIO_BITS
#else
    #define configPRIO_BITS                               4        /* 15 priority levels */
#endif

/* The lowest interrupt priority that can be used in a call to a "set priority"
function. */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY           0x0F

/* The highest interrupt priority that can be used by any interrupt service
routine that makes calls to interrupt safe FreeRTOS API functions.  DO NOT CALL
INTERRUPT SAFE FREERTOS API FUNCTIONS FROM ANY INTERRUPT THAT HAS A HIGHER
PRIORITY THAN THIS! (higher priorities are lower numeric values. */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY      (5U)

/* Interrupt priorities used by the kernel port layer itself.  These are generic
to all Cortex-M ports, and do not rely on any particular library functions. */
#define configKERNEL_INTERRUPT_PRIORITY         (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8U - configPRIO_BITS))
/* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY     (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8U - configPRIO_BITS))

/* Normal assert() semantics without relying on the provision of an assert.h
header file. */
#define configASSERT(x) if((x) == 0 ) {taskDISABLE_INTERRUPTS(); for( ;; );}

/* Definitions that map the FreeRTOS port interrupt handlers to their CMSIS
standard names. */
#define vPortSVCHandler        SVC_Handler
#define xPortPendSVHandler     PendSV_Handler
#define xPortSysTickHandler    SysTick_Handler

#endif /* FREERTOS_CONFIG_H */
