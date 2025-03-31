#include "debug_print.h"
#include "at32f435_437.h"

#include <stdio.h>
#include <stdarg.h>

void print(const char *msg, ...)
{
    va_list va;
    char bufer[256];

    va_start(va, msg);
    vsprintf(bufer, msg, va);
    va_end(va);

    puts(bufer);
}

void printn(int32_t num)
{
    print("%d\n", num);
}

void plot(const int32_t point, const Charts chart)
{
    if(((ITM->TCR & ITM_TCR_ITMENA_Msk) != 0UL) &&  /* ITM enabled */
       ((ITM->TER & (chart + 1)) != 0UL))           /* ITM Port enabled */
    {
        while(ITM->PORT[chart].u32 == 0UL)
        {
            __NOP();
        }

        ITM->PORT[chart].u32 = (uint32_t)point;
    }
}

/* Reterget the stdout */
int puts(const char *ch)
{
    while(*ch)
    {
        ITM_SendChar((uint32_t)(*ch++));
    }

    return 1U;
}
