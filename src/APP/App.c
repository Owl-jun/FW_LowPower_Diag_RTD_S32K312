#include "App.h"
#include "App_Command.h"
#include "App_Standby.h"
#include "../platform/Platform_Init.h"
#include "App_Pwm.h"
#include "../platform/Uart_Port.h"
#include "../platform/Timebase.h"
#include "App_Telemetry.h"
#include "../WDG/App_Wdg.h"

void App_Init(void)
{

#if (BUILD_MODE == BM_RELEASE)
	static uint32_t u32BaseTS;
	uint32_t		u32CurTS;
#endif

    /* Preserve wakeup/reset evidence before drivers modify status flags. */
#if (STANDBY_ENABLE == CFG_ENABLE)
    App_Standby_CaptureBootInfo();
#endif
    Plat_InitHardware();
#if (STANDBY_ENABLE == CFG_ENABLE &&	\
	 UART_LOG_ENABLE == CFG_ENABLE && 	\
	 UART_ENABLE == CFG_ENABLE)
    App_Standby_ReportBootInfo();
#endif
    if (Plat_Uart_Recv_Async() != E_OK)
    {
#if (BUILD_MODE == BM_RELEASE)
        u32BaseTS = Plat_Timebase_GetElapsedSec();
#endif
        for (;;) {
#if (BUILD_MODE == BM_RELEASE)
            u32CurTS = Plat_Timebase_GetElapsedSec();
        	if (u32CurTS - u32BaseTS > T_SEC_TIMEOUT_INF_LOOP) {
        		App_Pwm_On_RedLed(CFG_ON);
        		break;
        	}
#else
        	/* RX start failed: inspect UART initialization. */
#endif
		}
    }
}

void App_Process(void)
{
    /* Preserve the existing superloop order. ISR callbacks only provide
     * inputs; command execution remains in main context. */

#if (UART_ENABLE == CFG_ENABLE)
    Plat_Uart_RxProcess();
    App_Command_Process();

#if (UART_LOG_ENABLE == CFG_ENABLE && TIMEBASE_ENABLE == CFG_ENABLE)
	App_Telemetry_Process();
#endif

#endif

#if (PWM_ENABLE == CFG_ENABLE)
    App_Pwm_Process();
#endif

#if (WATCHDOG_ENABLE == CFG_ON)
		App_Wdg_Refresh();
#endif

}
