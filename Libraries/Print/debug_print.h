#ifndef DEBUG_PRINT_
#define DEBUG_PRINT_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    CHART_0,
    CHART_1,
    CHART_2,
    CHART_3,
    CHART_4,
    CHART_5
} Charts;

void print(const char *msg, ...);
void printn(int32_t num);
void plot(const int32_t point, const Charts chart);


#ifdef __cplusplus
}
#endif

#endif /* DEBUG_PRINT_ */
