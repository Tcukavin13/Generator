/*
 ============================================================================
 Name        : main.c
 Author      : Tcukavin
 Version     :
 Copyright   : Your copyright notice
 Description :
 ============================================================================
 */
extern "C" {
/* MCU */
#include "at32f435_437.h"
/* Clock */
#include "at32f435_437_clock.h"
/* FreeRTOS */
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
/* ARM Math */
#include "arm_math.h"
}

#include "dwt.h"
#include "generator.h"
#include <array>

/* Daemon Task */
static void main_task(void *pvParameters);

int main(void)
{
    /* System Init */
    SystemInit();

    /* System clock config and update system_core_clock (288 MHz) */
    system_clock_config();

    /* Set 4 bits for preemption priority, 0 bits for sub-priority */
    nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);

    /* For exec time measure */
    Dwt::timerInit();

    /* Daemon task create */
    if(pdPASS != xTaskCreate(main_task, "Daemon", configMINIMAL_STACK_SIZE * 4,
                             NULL, tskIDLE_PRIORITY + 1, NULL))
    {
        while(1){};
    }

    /* RTOS Scheduler start */
    vTaskStartScheduler();

    while(1)
    {

    }

    return 0;
}

std::array<Packet, 3> mySignal{{{2, 1000}, {3, 500}, {2, 2000}}};

//std::array<Packet, 5> mySignal{{{2, 1000}, {3, 500}, {2, 2000}, {3, 2500}, {2, 250}}};

Status status = Status::OK;

/* Main Task */
__attribute__((noreturn)) static void main_task(void *pvParameters)
{
    Generator::init();
    status = Generator::getStatus();
    vTaskDelay(250U);
    Generator::add(mySignal);
    Generator::start();
    status = Generator::getStatus();
    vTaskDelay(5U);
    Generator::stop();
    vTaskDelay(20U);
    mySignal[2U].n = 4;
    Generator::add(mySignal);
    Generator::start();

    for(;;)
    {
        vTaskDelay(1000U);
    }
}
