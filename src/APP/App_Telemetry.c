#include "App_Telemetry.h"
#include <stdio.h>

#if (TIMEBASE_ENABLE == CFG_ENABLE)
#include "../platform/Timebase.h"
#endif

#if (UART_ENABLE == CFG_ENABLE)
#include "../platform/Uart_Port.h"
#endif

#if (ADC_ENABLE == CFG_ENABLE && DMA_ENABLE == CFG_ENABLE)
#include "../platform/Adc_Port.h"
#endif

#if (UART_ENABLE == CFG_ENABLE			\
	&& UART_LOG_ENABLE == CFG_ENABLE	\
	&& TIMEBASE_ENABLE == CFG_ENABLE)
void App_Telemetry_Process(void)
{
    static uint32_t previousSec;
    uint32_t currentSec = Plat_Timebase_GetElapsedSec();

    if (previousSec != currentSec)
    {
        char text[40];
#if (ADC_ENABLE == CFG_ENABLE && DMA_ENABLE == CFG_ENABLE)
        Adc_ValueGroupType raw = Plat_Adc_ReadRaw();
#endif
        /* Preserve the existing ADC conversion: 5 V / 32768 counts. */
        float32 voltage = ((float32)raw * 5U) / 32768U;

        (void)snprintf(text, sizeof(text), "Boot Time : %lu sec", (unsigned long)currentSec);
        (void)Plat_Uart_Send(text);
        (void)snprintf(text, sizeof(text), "ADC Raw Val : %u", (unsigned int)raw);
        (void)Plat_Uart_Send(text);
        (void)snprintf(text, sizeof(text), "ADC Cal Val : %.2f [V]", (double)voltage);
        (void)Plat_Uart_Send(text);
        previousSec = currentSec;
    }
}
#endif
