#include "App_Telemetry.h"
#include "App_Log.h"
#include "../platform/Timebase.h"
#include "../platform/Adc_Port.h"

void App_Telemetry_Process(void)
{
    static uint32_t previousSec;
    uint32_t currentSec;
    if (!App_Log_IsEnabled()) { return; }
    currentSec = Plat_Timebase_GetElapsedSec();
    if (previousSec != currentSec)
    {
        Adc_ValueGroupType raw = Plat_Adc_ReadRaw();
        /* Preserve the existing ADC conversion: 5 V / 32768 counts. */
        float32 voltage = ((float32)raw * 5U) / 32768U;
        (void)App_Log_Printf("Boot Time : %lu sec", (unsigned long)currentSec);
        (void)App_Log_Printf("ADC Raw Val : %u", (unsigned int)raw);
        (void)App_Log_Printf("ADC Cal Val : %.2f [V]", (double)voltage);
        previousSec = currentSec;
    }
}
