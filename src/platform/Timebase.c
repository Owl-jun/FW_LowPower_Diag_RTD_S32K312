#include "Timebase.h"

#if (TIMEBASE_ENABLE == CFG_ENABLE)
static volatile uint32_t elapsedSec;

uint32_t Plat_Timebase_GetElapsedSec(void)
{
    return elapsedSec;
}

void Plat_Timebase_Stm0Notification(void)
{
    static uint8_t elapsedTicks;

    elapsedTicks++;
    if (elapsedTicks >= 10U)
    {
        elapsedSec++;
        elapsedTicks = 0U;
    }
}
#endif
