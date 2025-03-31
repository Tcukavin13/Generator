/*
 * dwt.h
 *
 *  Created on: 2023 Nov 25
 *      Author: Tcukavin13
 */

#ifndef DWT_H_
#define DWT_H_

extern "C" {
extern unsigned int system_core_clock;              /* System Clock Frequency (Core Clock) */
}

class Dwt
{
    public:
        __attribute__((always_inline)) static void timerInit(void)
        {
            *((uint32_t *)dwtDemcr) |= 0x01000000U; /* Enable DWT */
            *((uint32_t *)dwtCyccnt) = 0U;          /* Clear DWT Timer */
            *((uint32_t *)dwtControl) |= 1U;        /* Start DWT Timer */
        }

        __attribute__((always_inline)) static void timerStart(void)
        {
            *((uint32_t *)dwtCyccnt) = 0U;          /* Clear DWT Timer */
        }

        __attribute__((always_inline)) static uint32_t getTicks(void)
        {
            return *((uint32_t *)dwtCyccnt);
        }

        __attribute__((always_inline)) static float getTime(void)
        {
            return (((float) (*((uint32_t *)dwtCyccnt))) / (float) system_core_clock);
        }

        __attribute__((always_inline)) static float getMaxTime(void)
        {
            static uint32_t maxCycCntValue = 0U;
            static uint32_t currentCycCntValue = 0U;

            currentCycCntValue = *((uint32_t *)dwtCyccnt);
            maxCycCntValue = (maxCycCntValue > currentCycCntValue) ? maxCycCntValue : currentCycCntValue;

            return (float)maxCycCntValue / (float)system_core_clock;
        }
        
    private:
        constexpr static uint32_t dwtControl = 0xE0001000; /* Адрес регистра управления DWT */
        constexpr static uint32_t dwtCyccnt = 0xE0001004;  /* Адрес счетного регистра таймера DWT */
        constexpr static uint32_t dwtDemcr = 0xE000EDFC;   /* Адрес регистра DEMCR */
};

#endif /* DWT_H_ */
