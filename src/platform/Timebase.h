#ifndef PLATFORM_TIMEBASE_H
#define PLATFORM_TIMEBASE_H

#include "Std_Types.h"
#include "../APP_CFG.h"

#if (TIMEBASE_ENABLE == CFG_ENABLE)
/* Seconds since the boot timebase started; resets on every boot. */
uint32_t Plat_Timebase_GetElapsedSec(void);

/* RTD GPT notification. Requires the existing 100 ms STM0 period. */
void Plat_Timebase_Stm0Notification(void);
#endif

#endif
