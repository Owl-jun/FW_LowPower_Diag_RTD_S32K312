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
        "STANDBY BOOT standby=%u shared-source3=%u MODE=%08lx DES=%08lx FES=%08lx",
        (unsigned int)Plat_Standby_BootInfo.fromStandby,
        (unsigned int)Plat_Standby_BootInfo.fromRti,
        (unsigned long)Plat_Standby_BootInfo.modeStat,
        (unsigned long)Plat_Standby_BootInfo.resetDes,
        (unsigned long)Plat_Standby_BootInfo.resetFes);
    (void)App_Log_Printf("STANDBY BOOT WISR=%08lx WISR_64=%08lx",
        (unsigned long)Plat_Standby_BootInfo.wkpuWisr,
        (unsigned long)Plat_Standby_BootInfo.wkpuWisr64);
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
        (void)App_Log_Write("Standby blocked: check configuration");
#endif
        return;
    }
#if (UART_ENABLE == CFG_ENABLE && UART_LOG_ENABLE == CFG_ENABLE)
    App_Log_Printf("Entering Standby; wake-up is assigned to SW2");
#endif

    Plat_Standby_Enter();
}
