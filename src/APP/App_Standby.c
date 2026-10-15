#include "App_Standby.h"
#include "App_Log.h"
#include "../APP_CFG.h"
#include "../platform/Standby_Port.h"

void App_Standby_CaptureBootInfo(void)
{
    Plat_Standby_CaptureBootInfo();
}

void App_Standby_ReportBootInfo(void)
{
    if (!App_Log_IsEnabled()) { return; }
    (void)App_Log_Printf(
        "Boot: standby=%u RTI-source=%u MODE=%08lx DES=%08lx FES=%08lx WISR=%08lx",
        (unsigned int)Plat_Standby_BootInfo.fromStandby,
        (unsigned int)Plat_Standby_BootInfo.fromRti,
        (unsigned long)Plat_Standby_BootInfo.modeStat,
        (unsigned long)Plat_Standby_BootInfo.resetDes,
        (unsigned long)Plat_Standby_BootInfo.resetFes,
        (unsigned long)Plat_Standby_BootInfo.wkpuWisr);
}

void App_Standby_RtiNotification(void)
{
    /* Retained for the generated GPT configuration. PIT0 NVIC IRQ is disabled
     * during the wakeup test; wakeup is handled through reset, not this hook. */
}

void App_Standby_Enter(void)
{
    if (!Plat_Standby_IsAvailable())
    {
#if (UART_ENABLE == CFG_ENABLE && UART_LOG_ENABLE == CFG_ENABLE)
        (void)App_Log_Write("Standby blocked: check configuration / watchdog.");
#endif
        return;
    }
#if (UART_ENABLE == CFG_ENABLE && UART_LOG_ENABLE == CFG_ENABLE)
    if (App_Log_Printf("Entering Standby; PIT0 RTI wake-up in about %u seconds.",
                      (unsigned int)PLAT_STANDBY_DELAY_SECONDS) != E_OK)
#endif
    {
        return;
    }
    Plat_Standby_Enter();
}
